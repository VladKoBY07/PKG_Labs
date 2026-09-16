/********************************************************************************
** Form generated from reading UI file 'colorconverter.ui'
**
** Created by: Qt User Interface Compiler version 6.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COLORCONVERTER_H
#define UI_COLORCONVERTER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ColorConverter
{
public:
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *topLayout;
    QGroupBox *previewGroup;
    QVBoxLayout *previewLayout;
    QLabel *colorPreview;
    QLabel *gamutWarningLabel;
    QGroupBox *mapGroup;
    QVBoxLayout *mapLayout;
    QWidget *colorMap;
    QHBoxLayout *modelsLayout;
    QGroupBox *hlsGroup;
    QVBoxLayout *hlsLayout;
    QHBoxLayout *hLayout;
    QLabel *hLabel;
    QSlider *hSlider;
    QDoubleSpinBox *hSpinBox;
    QHBoxLayout *lLayout;
    QLabel *lLabel;
    QSlider *lSlider;
    QDoubleSpinBox *lSpinBox;
    QHBoxLayout *sLayout;
    QLabel *sLabel;
    QSlider *sSlider;
    QDoubleSpinBox *sSpinBox;
    QSpacerItem *hlsSpacer;
    QGroupBox *xyzGroup;
    QVBoxLayout *xyzLayout;
    QHBoxLayout *xLayout;
    QLabel *xLabel;
    QSlider *xSlider;
    QDoubleSpinBox *xSpinBox;
    QHBoxLayout *yLayout;
    QLabel *yLabel;
    QSlider *ySlider;
    QDoubleSpinBox *ySpinBox;
    QHBoxLayout *zLayout;
    QLabel *zLabel;
    QSlider *zSlider;
    QDoubleSpinBox *zSpinBox;
    QSpacerItem *xyzSpacer;
    QGroupBox *labGroup;
    QVBoxLayout *labLayout;
    QHBoxLayout *labLLayout;
    QLabel *labLLabel;
    QSlider *labLSlider;
    QDoubleSpinBox *labLSpinBox;
    QHBoxLayout *aLayout;
    QLabel *aLabel;
    QSlider *aSlider;
    QDoubleSpinBox *aSpinBox;
    QHBoxLayout *labBLayout;
    QLabel *labBLabel;
    QSlider *labBSlider;
    QDoubleSpinBox *labBSpinBox;
    QSpacerItem *labSpacer;
    QMenuBar *menuBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ColorConverter)
    {
        if (ColorConverter->objectName().isEmpty())
            ColorConverter->setObjectName("ColorConverter");
        ColorConverter->resize(1200, 760);
        ColorConverter->setMinimumSize(QSize(1000, 650));
        centralWidget = new QWidget(ColorConverter);
        centralWidget->setObjectName("centralWidget");
        mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setSpacing(12);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(12, 12, 12, 12);
        topLayout = new QHBoxLayout();
        topLayout->setSpacing(12);
        topLayout->setObjectName("topLayout");
        previewGroup = new QGroupBox(centralWidget);
        previewGroup->setObjectName("previewGroup");
        previewGroup->setMinimumSize(QSize(260, 270));
        previewLayout = new QVBoxLayout(previewGroup);
        previewLayout->setSpacing(8);
        previewLayout->setObjectName("previewLayout");
        colorPreview = new QLabel(previewGroup);
        colorPreview->setObjectName("colorPreview");
        colorPreview->setMinimumSize(QSize(220, 180));
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(colorPreview->sizePolicy().hasHeightForWidth());
        colorPreview->setSizePolicy(sizePolicy);
        colorPreview->setStyleSheet(QString::fromUtf8("background-color: #ff0000;\n"
"border: 2px solid #555555;\n"
"border-radius: 4px;"));
        colorPreview->setAlignment(Qt::AlignmentFlag::AlignCenter);

        previewLayout->addWidget(colorPreview);

        gamutWarningLabel = new QLabel(previewGroup);
        gamutWarningLabel->setObjectName("gamutWarningLabel");
        gamutWarningLabel->setMinimumSize(QSize(220, 35));
        gamutWarningLabel->setStyleSheet(QString::fromUtf8("color: #c62828;\n"
"font-weight: bold;"));
        gamutWarningLabel->setWordWrap(true);
        gamutWarningLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        previewLayout->addWidget(gamutWarningLabel);


        topLayout->addWidget(previewGroup);

        mapGroup = new QGroupBox(centralWidget);
        mapGroup->setObjectName("mapGroup");
        mapLayout = new QVBoxLayout(mapGroup);
        mapLayout->setObjectName("mapLayout");
        colorMap = new QWidget(mapGroup);
        colorMap->setObjectName("colorMap");
        colorMap->setMinimumSize(QSize(380, 240));
        sizePolicy.setHeightForWidth(colorMap->sizePolicy().hasHeightForWidth());
        colorMap->setSizePolicy(sizePolicy);

        mapLayout->addWidget(colorMap);


        topLayout->addWidget(mapGroup);


        mainLayout->addLayout(topLayout);

        modelsLayout = new QHBoxLayout();
        modelsLayout->setSpacing(12);
        modelsLayout->setObjectName("modelsLayout");
        hlsGroup = new QGroupBox(centralWidget);
        hlsGroup->setObjectName("hlsGroup");
        hlsLayout = new QVBoxLayout(hlsGroup);
        hlsLayout->setSpacing(10);
        hlsLayout->setObjectName("hlsLayout");
        hLayout = new QHBoxLayout();
        hLayout->setObjectName("hLayout");
        hLabel = new QLabel(hlsGroup);
        hLabel->setObjectName("hLabel");
        hLabel->setMinimumSize(QSize(25, 0));

        hLayout->addWidget(hLabel);

        hSlider = new QSlider(hlsGroup);
        hSlider->setObjectName("hSlider");
        hSlider->setOrientation(Qt::Orientation::Horizontal);

        hLayout->addWidget(hSlider);

        hSpinBox = new QDoubleSpinBox(hlsGroup);
        hSpinBox->setObjectName("hSpinBox");
        hSpinBox->setMinimumSize(QSize(90, 0));
        hSpinBox->setDecimals(2);

        hLayout->addWidget(hSpinBox);


        hlsLayout->addLayout(hLayout);

        lLayout = new QHBoxLayout();
        lLayout->setObjectName("lLayout");
        lLabel = new QLabel(hlsGroup);
        lLabel->setObjectName("lLabel");
        lLabel->setMinimumSize(QSize(25, 0));

        lLayout->addWidget(lLabel);

        lSlider = new QSlider(hlsGroup);
        lSlider->setObjectName("lSlider");
        lSlider->setOrientation(Qt::Orientation::Horizontal);

        lLayout->addWidget(lSlider);

        lSpinBox = new QDoubleSpinBox(hlsGroup);
        lSpinBox->setObjectName("lSpinBox");
        lSpinBox->setMinimumSize(QSize(90, 0));
        lSpinBox->setDecimals(2);

        lLayout->addWidget(lSpinBox);


        hlsLayout->addLayout(lLayout);

        sLayout = new QHBoxLayout();
        sLayout->setObjectName("sLayout");
        sLabel = new QLabel(hlsGroup);
        sLabel->setObjectName("sLabel");
        sLabel->setMinimumSize(QSize(25, 0));

        sLayout->addWidget(sLabel);

        sSlider = new QSlider(hlsGroup);
        sSlider->setObjectName("sSlider");
        sSlider->setOrientation(Qt::Orientation::Horizontal);

        sLayout->addWidget(sSlider);

        sSpinBox = new QDoubleSpinBox(hlsGroup);
        sSpinBox->setObjectName("sSpinBox");
        sSpinBox->setMinimumSize(QSize(90, 0));
        sSpinBox->setDecimals(2);

        sLayout->addWidget(sSpinBox);


        hlsLayout->addLayout(sLayout);

        hlsSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        hlsLayout->addItem(hlsSpacer);


        modelsLayout->addWidget(hlsGroup);

        xyzGroup = new QGroupBox(centralWidget);
        xyzGroup->setObjectName("xyzGroup");
        xyzLayout = new QVBoxLayout(xyzGroup);
        xyzLayout->setSpacing(10);
        xyzLayout->setObjectName("xyzLayout");
        xLayout = new QHBoxLayout();
        xLayout->setObjectName("xLayout");
        xLabel = new QLabel(xyzGroup);
        xLabel->setObjectName("xLabel");
        xLabel->setMinimumSize(QSize(25, 0));

        xLayout->addWidget(xLabel);

        xSlider = new QSlider(xyzGroup);
        xSlider->setObjectName("xSlider");
        xSlider->setOrientation(Qt::Orientation::Horizontal);

        xLayout->addWidget(xSlider);

        xSpinBox = new QDoubleSpinBox(xyzGroup);
        xSpinBox->setObjectName("xSpinBox");
        xSpinBox->setMinimumSize(QSize(90, 0));
        xSpinBox->setDecimals(4);

        xLayout->addWidget(xSpinBox);


        xyzLayout->addLayout(xLayout);

        yLayout = new QHBoxLayout();
        yLayout->setObjectName("yLayout");
        yLabel = new QLabel(xyzGroup);
        yLabel->setObjectName("yLabel");
        yLabel->setMinimumSize(QSize(25, 0));

        yLayout->addWidget(yLabel);

        ySlider = new QSlider(xyzGroup);
        ySlider->setObjectName("ySlider");
        ySlider->setOrientation(Qt::Orientation::Horizontal);

        yLayout->addWidget(ySlider);

        ySpinBox = new QDoubleSpinBox(xyzGroup);
        ySpinBox->setObjectName("ySpinBox");
        ySpinBox->setMinimumSize(QSize(90, 0));
        ySpinBox->setDecimals(4);

        yLayout->addWidget(ySpinBox);


        xyzLayout->addLayout(yLayout);

        zLayout = new QHBoxLayout();
        zLayout->setObjectName("zLayout");
        zLabel = new QLabel(xyzGroup);
        zLabel->setObjectName("zLabel");
        zLabel->setMinimumSize(QSize(25, 0));

        zLayout->addWidget(zLabel);

        zSlider = new QSlider(xyzGroup);
        zSlider->setObjectName("zSlider");
        zSlider->setOrientation(Qt::Orientation::Horizontal);

        zLayout->addWidget(zSlider);

        zSpinBox = new QDoubleSpinBox(xyzGroup);
        zSpinBox->setObjectName("zSpinBox");
        zSpinBox->setMinimumSize(QSize(90, 0));
        zSpinBox->setDecimals(4);

        zLayout->addWidget(zSpinBox);


        xyzLayout->addLayout(zLayout);

        xyzSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        xyzLayout->addItem(xyzSpacer);


        modelsLayout->addWidget(xyzGroup);

        labGroup = new QGroupBox(centralWidget);
        labGroup->setObjectName("labGroup");
        labLayout = new QVBoxLayout(labGroup);
        labLayout->setSpacing(10);
        labLayout->setObjectName("labLayout");
        labLLayout = new QHBoxLayout();
        labLLayout->setObjectName("labLLayout");
        labLLabel = new QLabel(labGroup);
        labLLabel->setObjectName("labLLabel");
        labLLabel->setMinimumSize(QSize(25, 0));

        labLLayout->addWidget(labLLabel);

        labLSlider = new QSlider(labGroup);
        labLSlider->setObjectName("labLSlider");
        labLSlider->setOrientation(Qt::Orientation::Horizontal);

        labLLayout->addWidget(labLSlider);

        labLSpinBox = new QDoubleSpinBox(labGroup);
        labLSpinBox->setObjectName("labLSpinBox");
        labLSpinBox->setMinimumSize(QSize(90, 0));
        labLSpinBox->setDecimals(2);

        labLLayout->addWidget(labLSpinBox);


        labLayout->addLayout(labLLayout);

        aLayout = new QHBoxLayout();
        aLayout->setObjectName("aLayout");
        aLabel = new QLabel(labGroup);
        aLabel->setObjectName("aLabel");
        aLabel->setMinimumSize(QSize(25, 0));

        aLayout->addWidget(aLabel);

        aSlider = new QSlider(labGroup);
        aSlider->setObjectName("aSlider");
        aSlider->setOrientation(Qt::Orientation::Horizontal);

        aLayout->addWidget(aSlider);

        aSpinBox = new QDoubleSpinBox(labGroup);
        aSpinBox->setObjectName("aSpinBox");
        aSpinBox->setMinimumSize(QSize(90, 0));
        aSpinBox->setDecimals(2);

        aLayout->addWidget(aSpinBox);


        labLayout->addLayout(aLayout);

        labBLayout = new QHBoxLayout();
        labBLayout->setObjectName("labBLayout");
        labBLabel = new QLabel(labGroup);
        labBLabel->setObjectName("labBLabel");
        labBLabel->setMinimumSize(QSize(25, 0));

        labBLayout->addWidget(labBLabel);

        labBSlider = new QSlider(labGroup);
        labBSlider->setObjectName("labBSlider");
        labBSlider->setOrientation(Qt::Orientation::Horizontal);

        labBLayout->addWidget(labBSlider);

        labBSpinBox = new QDoubleSpinBox(labGroup);
        labBSpinBox->setObjectName("labBSpinBox");
        labBSpinBox->setMinimumSize(QSize(90, 0));
        labBSpinBox->setDecimals(2);

        labBLayout->addWidget(labBSpinBox);


        labLayout->addLayout(labBLayout);

        labSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        labLayout->addItem(labSpacer);


        modelsLayout->addWidget(labGroup);


        mainLayout->addLayout(modelsLayout);

        ColorConverter->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(ColorConverter);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 1200, 25));
        ColorConverter->setMenuBar(menuBar);
        statusBar = new QStatusBar(ColorConverter);
        statusBar->setObjectName("statusBar");
        ColorConverter->setStatusBar(statusBar);

        retranslateUi(ColorConverter);

        QMetaObject::connectSlotsByName(ColorConverter);
    } // setupUi

    void retranslateUi(QMainWindow *ColorConverter)
    {
        ColorConverter->setWindowTitle(QCoreApplication::translate("ColorConverter", "\320\232\320\276\320\275\320\262\320\265\321\200\321\202\320\265\321\200 \321\206\320\262\320\265\321\202\320\276\320\262: HLS, XYZ, LAB", nullptr));
        previewGroup->setTitle(QCoreApplication::translate("ColorConverter", "\320\242\320\265\320\272\321\203\321\211\320\270\320\271 \321\206\320\262\320\265\321\202", nullptr));
        colorPreview->setText(QString());
        gamutWarningLabel->setText(QString());
        mapGroup->setTitle(QCoreApplication::translate("ColorConverter", "\320\232\320\260\321\200\321\202\320\260 \321\206\320\262\320\265\321\202\320\260", nullptr));
        hlsGroup->setTitle(QCoreApplication::translate("ColorConverter", "HLS", nullptr));
        hLabel->setText(QCoreApplication::translate("ColorConverter", "H", nullptr));
        lLabel->setText(QCoreApplication::translate("ColorConverter", "L", nullptr));
        sLabel->setText(QCoreApplication::translate("ColorConverter", "S", nullptr));
        xyzGroup->setTitle(QCoreApplication::translate("ColorConverter", "XYZ", nullptr));
        xLabel->setText(QCoreApplication::translate("ColorConverter", "X", nullptr));
        yLabel->setText(QCoreApplication::translate("ColorConverter", "Y", nullptr));
        zLabel->setText(QCoreApplication::translate("ColorConverter", "Z", nullptr));
        labGroup->setTitle(QCoreApplication::translate("ColorConverter", "LAB", nullptr));
        labLLabel->setText(QCoreApplication::translate("ColorConverter", "L*", nullptr));
        aLabel->setText(QCoreApplication::translate("ColorConverter", "a*", nullptr));
        labBLabel->setText(QCoreApplication::translate("ColorConverter", "b*", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ColorConverter: public Ui_ColorConverter {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COLORCONVERTER_H
