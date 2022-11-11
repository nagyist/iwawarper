//---------------------------------------------------
// IwRenderInstance
// 各フレームごとに作られる、レンダリング計算を行う実体
// IwRenderCommand::onPreviewから作られる
//---------------------------------------------------
#ifndef IWRENDERINSTANCE_H
#define IWRENDERINSTANCE_H

#include <QList>
#include <QMap>
#include <QString>
#include <QPointF>
#include <QVector3D>
#include <array>
#include <QRunnable>
#include <QImage>
#include "trasterimage.h"
#include "halfedge.h"
#include "rendersettings.h"
class IwProject;
class IwLayer;
class ShapePair;
class RenderProgressPopup;
class IwRenderInstance;

enum CorrVecId { CORRVEC_SRC = 0, CORRVEC_DST, CORRVEC_MIDDLE };
enum WarpStyle { WARP_FIXED = 0, WARP_SLIDING };

// 対応点ベクトル
struct CorrVector {
  QPointF from_p[2];
  QPointF to_p[2];
  int stackOrder;  // レイヤインデックス ＝ 重ね順
  bool isEdge;     // 輪郭ならtrue, シェイプ由来ならtrue
  int indices[2];  // samplePointsのインデックス
};

class MapTrianglesToRaster_Worker : public QRunnable {
  int m_from, m_to;  // faceのインデックス
  HEModel* m_model;
  IwRenderInstance* m_parentInstance;
  QPointF m_sampleOffset, m_outputOffset;
  TRaster64P m_outRas;
  TRaster64P m_srcRas;
  bool* m_subPointOccupation;
  int m_subAmount;
  AlphaMode m_alphaMode;
  const QImage m_shapeAlphaImg;

  void run() override;

public:
  MapTrianglesToRaster_Worker(int from, int to, HEModel* model,
                              IwRenderInstance* parentInstance,
                              QPointF sampleOffset, QPointF outputOffset,
                              TRaster64P outRas, TRaster64P srcRas,
                              bool* subPointOccupation, int subAmount,
                              AlphaMode alphaMode, QImage shapeAlphaImg)
      : m_from(from)
      , m_to(to)
      , m_model(model)
      , m_parentInstance(parentInstance)
      , m_sampleOffset(sampleOffset)
      , m_outputOffset(outputOffset)
      , m_outRas(outRas)
      , m_srcRas(srcRas)
      , m_subPointOccupation(subPointOccupation)
      , m_subAmount(subAmount)
      , m_alphaMode(alphaMode)
      , m_shapeAlphaImg(shapeAlphaImg) {}
};

class CombineResults_Worker : public QRunnable {
  int m_from, m_to;
  TRaster64P m_outRas;
  QList<bool*>& m_subPointOccupationList;
  QList<TRaster64P>& m_outRasList;
  void run() override;

public:
  CombineResults_Worker(int from, int to, TRaster64P outRas,
                        QList<bool*>& subPointOccupationList,
                        QList<TRaster64P>& outRasList)
      : m_from(from)
      , m_to(to)
      , m_outRas(outRas)
      , m_subPointOccupationList(subPointOccupationList)
      , m_outRasList(outRasList) {}
};

class ResampleResults_Worker : public QRunnable {
  int m_from, m_to;
  TRaster64P m_outRas;
  TRaster64P m_retRas;
  int m_subAmount;
  bool m_antialias;
  void run() override;

public:
  ResampleResults_Worker(int from, int to, TRaster64P outRas, TRaster64P retRas,
                         int subAmount, bool antialias)
      : m_from(from)
      , m_to(to)
      , m_outRas(outRas)
      , m_retRas(retRas)
      , m_subAmount(subAmount)
      , m_antialias(antialias) {}
};

class IwRenderInstance : public QObject, public QRunnable {
  Q_OBJECT

  int m_frame;
  IwProject* m_project;
  bool m_isPreview;
  RenderProgressPopup* m_popup;
  int m_precision;
  double m_faceSizeThreshold;
  double m_smoothingThreshold;
  WarpStyle m_warpStyle;

  unsigned int m_taskId;

  bool m_antialias;

  void run() override;

  // 結果をキャッシュに格納
  void storeImageToCache(TRaster64P morphedRaster);

  // シェイプの親子グループごとに計算する
  TRaster64P warpLayer(IwLayer* layer, QList<ShapePair*>& shapes,
                       bool isPreview, QPoint& origin = QPoint());

  //---------------

  // 素材ラスタを得る
  TRasterP getLayerRaster(IwLayer* layer);

  // ワープ先のCorrVecの長さに合わせ、
  // あまり短いベクトルはまとめながら格納していく
  void getCorrVectors(IwLayer* layer, QList<ShapePair*>& shapes,
                      QList<CorrVector>& corrVectors,
                      QVector<QPointF>& parentShapeVerticesFrom,
                      QVector<QPointF>& parentShapeVerticesTo);

  // fromShapeを元に、メッシュを切る Halfedge使う版
  void HEcreateTriangleMesh(QList<CorrVector>& corrVectors, HEModel& model,
                            const QPolygonF& parentShapePolygon,
                            const TDimension& srcDim);

  // ゆがんだ形状を描画する_マルチスレッド版
  TRaster64P HEmapTrianglesToRaster_Multi(HEModel& model, TRaster64P srcRas,
                                          ShapePair* shape, QPoint& origin,
                                          const QPolygonF& parentShapePolygon);

  // 三角形の位置をキャッシュする
  void HEcacheTriangles(HEModel& model, ShapePair* shape,
                        const TDimension& srcDim,
                        const QPolygonF& parentShapePolygon);

  //--------------------------------
  // ファイルに書き出す
  void saveImage(TRaster64P ras);
  //--------------------------------

public:
  IwRenderInstance(int frame, IwProject* project, bool isPreview,
                   RenderProgressPopup* popup = nullptr);
  void doPreview();
  void doRender();

  bool isCanceled();

  bool antialiasEnabled() const { return m_antialias; }

signals:
  void renderStarted(int frame, unsigned int taskId);
  void renderFinished(int frame, unsigned int taskId);
};

#endif