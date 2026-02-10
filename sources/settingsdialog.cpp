//------------------------------------
// SettingsDialog
//------------------------------------
#include "settingsdialog.h"

#include "iwapp.h"
#include "iwprojecthandle.h"
#include "iwproject.h"
#include "mainwindow.h"
#include "menubarcommandids.h"

#include "preferences.h"
#include "rendersettings.h"

#include "myslider.h"
#include "iwtrianglecache.h"

#include <QComboBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QIntValidator>

//------------------------------------
SettingsDialog::SettingsDialog()
    : IwDialog(IwApp::instance()->getMainWindow(), "SettingsDialog", false) {
  setWindowTitle(tr("Settings"));
  //-- オブジェクトの宣言
  // RenderSettingsより
  m_warpPrecisionSlider = new MyIntSlider(0, 5, this);
  m_faceSizeThresSlider = new MyIntSlider(10, 50, this);
  m_alphaModeCombo      = new QComboBox(this);
  m_resampleModeCombo   = new QComboBox(this);
  m_imageShrinkSlider   = new MyIntSlider(1, 4, this);
  m_antialiasCheckBox   = new QCheckBox(tr("Shape Antialias"), this);
  m_matteDilateSlider   = new MyIntSlider(0, 3, this);

  // Preferenceより
  m_bezierPrecisionCombo = new QComboBox(this);
  m_vertexBufferSizeEdit = new QLineEdit(this);

  //-- プロパティの設定
  m_bezierPrecisionCombo->addItem(tr("Low"), Preferences::LOW);
  m_bezierPrecisionCombo->addItem(tr("Medium"), Preferences::MEDIUM);
  m_bezierPrecisionCombo->addItem(tr("High"), Preferences::HIGH);
  m_bezierPrecisionCombo->addItem(tr("Super High"), Preferences::SUPERHIGH);

  QStringList aModeList;
  aModeList << "Source"
            << "Shape";
  m_alphaModeCombo->addItems(aModeList);

  m_resampleModeCombo->addItem(tr("Area Average"), AreaAverage);
  m_resampleModeCombo->addItem(tr("Nearest Neighbor"), NearestNeighbor);
  m_resampleModeCombo->addItem(tr("Morphological Supersampling"),
                               MorphologicalSupersampling);
  m_resampleModeCombo->setToolTip(tr(
      "PLEASE NOTE: The \"Morphological Supersampling\" option is intended \n"
      "to be used when deforming so-called \"color binary images\", \n"
      "such as those commonly used in the Japanese animation industry \n"
      "for character Levels."));

  m_vertexBufferSizeEdit->setValidator(new QIntValidator(10000, INT_MAX));
  m_vertexBufferSizeEdit->setFixedWidth(100);

  QLabel* note = new QLabel(
      tr("* Changes will take effect the next time you run IwaWarper"));
  note->setStyleSheet("font-size: 10px; font: italic;");

  //-- レイアウト
  QVBoxLayout* mainLay = new QVBoxLayout();
  mainLay->setSpacing(10);
  mainLay->setMargin(10);
  {
    QGroupBox* projectSettingsBox = new QGroupBox(tr("Project Settings"), this);
    QGridLayout* projectSettingsLay = new QGridLayout();
    projectSettingsLay->setHorizontalSpacing(5);
    projectSettingsLay->setVerticalSpacing(8);
    projectSettingsLay->setMargin(10);
    {
      projectSettingsLay->addWidget(new QLabel(tr("Number of subdivision:")), 0,
                                    0, Qt::AlignRight | Qt::AlignVCenter);
      projectSettingsLay->addWidget(m_warpPrecisionSlider, 0, 1);

      projectSettingsLay->addWidget(new QLabel(tr("Maximum face size:")), 1, 0,
                                    Qt::AlignRight | Qt::AlignVCenter);
      projectSettingsLay->addWidget(m_faceSizeThresSlider, 1, 1);

      projectSettingsLay->addWidget(new QLabel(tr("Alpha Mode:")), 2, 0,
                                    Qt::AlignRight | Qt::AlignVCenter);
      projectSettingsLay->addWidget(m_alphaModeCombo, 2, 1,
                                    Qt::AlignLeft | Qt::AlignVCenter);

      projectSettingsLay->addWidget(new QLabel(tr("Resample Mode:")), 3, 0,
                                    Qt::AlignRight | Qt::AlignVCenter);
      projectSettingsLay->addWidget(m_resampleModeCombo, 3, 1,
                                    Qt::AlignLeft | Qt::AlignVCenter);

      projectSettingsLay->addWidget(m_antialiasCheckBox, 4, 0, 1, 2);

      projectSettingsLay->addWidget(new QLabel(tr("Image Shrink:")), 5, 0,
                                    Qt::AlignRight | Qt::AlignVCenter);
      projectSettingsLay->addWidget(m_imageShrinkSlider, 5, 1);

      projectSettingsLay->addWidget(new QLabel(tr("Matte Dilate:")), 6, 0,
                                    Qt::AlignRight | Qt::AlignVCenter);
      projectSettingsLay->addWidget(m_matteDilateSlider, 6, 1);
    }
    projectSettingsLay->setColumnStretch(0, 0);
    projectSettingsLay->setColumnStretch(1, 1);
    projectSettingsBox->setLayout(projectSettingsLay);
    mainLay->addWidget(projectSettingsBox, 0);

    QGroupBox* userPreferencesBox = new QGroupBox(tr("User Preferences"), this);
    QGridLayout* userPreferencesLay = new QGridLayout();
    userPreferencesLay->setHorizontalSpacing(5);
    userPreferencesLay->setVerticalSpacing(8);
    userPreferencesLay->setMargin(10);
    {
      userPreferencesLay->addWidget(new QLabel(tr("Bezier Precision:")), 0, 0,
                                    Qt::AlignRight | Qt::AlignVCenter);
      userPreferencesLay->addWidget(m_bezierPrecisionCombo, 0, 1,
                                    Qt::AlignLeft | Qt::AlignVCenter);

      userPreferencesLay->addWidget(new QLabel(tr("Vertex Buffer Size*:")), 1,
                                    0, Qt::AlignRight | Qt::AlignVCenter);
      userPreferencesLay->addWidget(m_vertexBufferSizeEdit, 1, 1,
                                    Qt::AlignLeft | Qt::AlignVCenter);
    }
    userPreferencesLay->setColumnStretch(0, 0);
    userPreferencesLay->setColumnStretch(1, 1);
    userPreferencesBox->setLayout(userPreferencesLay);
    mainLay->addWidget(userPreferencesBox, 0);

    mainLay->addWidget(note, 0);

    mainLay->addStretch(1);
  }
  setLayout(mainLay);

  //-- シグナル／スロット接続
  connect(m_warpPrecisionSlider, SIGNAL(valueChanged(bool)), this,
          SLOT(onPrecisionValueChanged(bool)));
  connect(m_faceSizeThresSlider, SIGNAL(valueChanged(bool)), this,
          SLOT(onFaceSizeValueChanged(bool)));
  connect(m_alphaModeCombo, SIGNAL(activated(int)), this,
          SLOT(onAlphaModeComboActivated(int)));
  connect(m_resampleModeCombo, SIGNAL(activated(int)), this,
          SLOT(onResampleModeComboActivated()));
  connect(m_imageShrinkSlider, SIGNAL(valueChanged(bool)), this,
          SLOT(onImageShrinkChanged(bool)));
  connect(m_antialiasCheckBox, SIGNAL(clicked(bool)), this,
          SLOT(onAntialiasClicked(bool)));
  connect(m_matteDilateSlider, SIGNAL(valueChanged(bool)), this,
          SLOT(onMatteDilateValueChanged(bool)));

  connect(m_bezierPrecisionCombo, SIGNAL(activated(int)), this,
          SLOT(onBezierPrecisionComboChanged(int)));
  connect(m_vertexBufferSizeEdit, SIGNAL(editingFinished()), this,
          SLOT(onVertexBufferSizeEdited()));
}

//------------------------------------
// projectが切り替わったら、内容を更新する
//------------------------------------
void SettingsDialog::showEvent(QShowEvent* event) {
  IwDialog::showEvent(event);

  IwProjectHandle* ph = IwApp::instance()->getCurrentProject();
  if (ph)
    connect(ph, SIGNAL(projectSwitched()), this, SLOT(onProjectSwitched()));

  onProjectSwitched();
}

//------------------------------------
void SettingsDialog::hideEvent(QHideEvent* event) {
  IwDialog::hideEvent(event);

  IwProjectHandle* ph = IwApp::instance()->getCurrentProject();
  if (ph)
    disconnect(ph, SIGNAL(projectSwitched()), this, SLOT(onProjectSwitched()));
}

//------------------------------------
// 表示更新
//------------------------------------

void SettingsDialog::onProjectSwitched() {
  IwProject* project = IwApp::instance()->getCurrentProject()->getProject();
  if (!project) return;

  int prec = (int)Preferences::instance()->getBezierActivePrecision();
  m_bezierPrecisionCombo->setCurrentIndex(
      m_bezierPrecisionCombo->findData(prec));
  m_vertexBufferSizeEdit->setText(
      QString::number(Preferences::instance()->vertexBufferSize()));

  RenderSettings* settings = project->getRenderSettings();
  if (settings) {
    m_warpPrecisionSlider->setValue(settings->getWarpPrecision());
    m_faceSizeThresSlider->setValue(settings->getFaceSizeThreshold());
    m_alphaModeCombo->setCurrentIndex((int)settings->getAlphaMode());
    m_resampleModeCombo->setCurrentIndex(
        m_resampleModeCombo->findData((int)settings->getResampleMode()));
    m_imageShrinkSlider->setValue(settings->getImageShrink());
    m_antialiasCheckBox->setChecked(settings->getAntialias());
    m_matteDilateSlider->setValue(settings->getMatteDilate());
  }
  update();
}

//------------------------------------
// Preference
//------------------------------------
void SettingsDialog::onBezierPrecisionComboChanged(int /*index*/) {
  IwProject* project = IwApp::instance()->getCurrentProject()->getProject();
  if (!project) return;
  Preferences::BezierActivePrecision prec =
      (Preferences::BezierActivePrecision)(m_bezierPrecisionCombo->currentData()
                                               .toInt());
  Preferences::instance()->setBezierActivePrecision(prec);

  IwApp::instance()->getCurrentProject()->notifyViewSettingsChanged();
}

void SettingsDialog::onVertexBufferSizeEdited() {
  bool ok;
  int count = m_vertexBufferSizeEdit->text().toInt(&ok);
  if (ok)
    Preferences::instance()->setVertexBufferSize(count);
  else
    m_vertexBufferSizeEdit->setText(
        QString::number(Preferences::instance()->vertexBufferSize()));
}

//------------------------------------
// RenderSettings
//------------------------------------
void SettingsDialog::onPrecisionValueChanged(bool isDragging) {
  if (isDragging) return;
  // 現在のプロジェクトのOutputSettingsを取得
  IwProject* project = IwApp::instance()->getCurrentProject()->getProject();
  if (!project) return;
  RenderSettings* settings = project->getRenderSettings();
  if (!settings) return;

  int val = m_warpPrecisionSlider->value();
  if (val == settings->getWarpPrecision()) return;

  settings->setWarpPrecision(val);
  IwTriangleCache::instance()->invalidateAllCache();
}

void SettingsDialog::onFaceSizeValueChanged(bool isDragging) {
  if (isDragging) return;
  // 現在のプロジェクトのOutputSettingsを取得
  IwProject* project = IwApp::instance()->getCurrentProject()->getProject();
  if (!project) return;
  RenderSettings* settings = project->getRenderSettings();
  if (!settings) return;

  double val = (double)m_faceSizeThresSlider->value();
  if (val == settings->getFaceSizeThreshold()) return;

  settings->setFaceSizeThreshold(val);
  IwTriangleCache::instance()->invalidateAllCache();
}

void SettingsDialog::onAlphaModeComboActivated(int index) {
  // 現在のプロジェクトのOutputSettingsを取得
  IwProject* project = IwApp::instance()->getCurrentProject()->getProject();
  if (!project) return;
  RenderSettings* settings = project->getRenderSettings();
  if (!settings) return;

  if (index == (int)settings->getAlphaMode()) return;

  settings->setAlphaMode((AlphaMode)index);
}

void SettingsDialog::onResampleModeComboActivated() {
  // 現在のプロジェクトのOutputSettingsを取得
  IwProject* project = IwApp::instance()->getCurrentProject()->getProject();
  if (!project) return;
  RenderSettings* settings = project->getRenderSettings();
  if (!settings) return;

  ResampleMode rMode =
      (ResampleMode)(m_resampleModeCombo->currentData().toInt());
  if (rMode == settings->getResampleMode()) return;

  settings->setResampleMode(rMode);
}

void SettingsDialog::onImageShrinkChanged(bool isDragging) {
  if (isDragging) return;
  IwProject* project = IwApp::instance()->getCurrentProject()->getProject();
  if (!project) return;
  RenderSettings* settings = project->getRenderSettings();
  if (!settings) return;

  int val = m_imageShrinkSlider->value();
  if (val == settings->getImageShrink()) return;

  settings->setImageShrink(val);
  IwApp::instance()->getCurrentProject()->notifyProjectChanged();
}

void SettingsDialog::onAntialiasClicked(bool on) {
  IwProject* project = IwApp::instance()->getCurrentProject()->getProject();
  if (!project) return;
  RenderSettings* settings = project->getRenderSettings();
  if (!settings) return;

  if (on == settings->getAntialias()) return;

  settings->setAntialias(on);
  IwApp::instance()->getCurrentProject()->notifyProjectChanged();
}

void SettingsDialog::onMatteDilateValueChanged(bool isDragging) {
  if (isDragging) return;
  // 現在のプロジェクトのOutputSettingsを取得
  IwProject* project = IwApp::instance()->getCurrentProject()->getProject();
  if (!project) return;
  RenderSettings* settings = project->getRenderSettings();
  if (!settings) return;

  int val = m_matteDilateSlider->value();
  if (val == settings->getMatteDilate()) return;

  settings->setMatteDilate(val);
  IwApp::instance()->getCurrentProject()->notifyProjectChanged();
}

//---------------------------------------------------
OpenPopupCommandHandler<SettingsDialog> openSettingsDialog(MI_Preferences);