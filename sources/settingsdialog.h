//------------------------------------
// SettingsDialog
//------------------------------------
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "iwdialog.h"

class QComboBox;
class QLineEdit;
class MyIntSlider;
class QCheckBox;

class SettingsDialog : public IwDialog {
  Q_OBJECT

  // RenderSettingsより
  MyIntSlider* m_warpPrecisionSlider;
  MyIntSlider* m_faceSizeThresSlider;
  QComboBox* m_alphaModeCombo;
  QComboBox* m_resampleModeCombo;
  MyIntSlider* m_imageShrinkSlider;
  QCheckBox* m_antialiasCheckBox;
  MyIntSlider* m_matteDilateSlider;

  // Preferenceより
  QComboBox* m_bezierPrecisionCombo;
  QLineEdit* m_vertexBufferSizeEdit;

public:
  SettingsDialog();

protected:
  // projectが切り替わったら、内容を更新する
  void showEvent(QShowEvent*);
  void hideEvent(QHideEvent*);

protected slots:
  void onProjectSwitched();

  // RenderSettingsより
  void onPrecisionValueChanged(bool isDragging);
  void onFaceSizeValueChanged(bool isDragging);
  void onAlphaModeComboActivated(int index);
  void onResampleModeComboActivated();
  void onImageShrinkChanged(bool isDragging);
  void onAntialiasClicked(bool on);
  void onMatteDilateValueChanged(bool isDragging);
  // Preferenceより
  void onBezierPrecisionComboChanged(int index);
  void onVertexBufferSizeEdited();
};

#endif