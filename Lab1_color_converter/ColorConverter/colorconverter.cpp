#include "colorconverter.h"
#include "ui_colorconverter.h"

#include <QColor>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QSignalBlocker>
#include <QSlider>

#include <algorithm>
#include <cmath>

ColorConverter::ColorConverter(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ColorConverter)
    , m_updating(false)
    , m_currentHls{0.0, 0.5, 1.0}
{
    ui->setupUi(this);

    setupControls();
    setupConnections();

    Hls hls;
    hls.h = 0.0;
    hls.l = 0.5;
    hls.s = 1.0;

    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    hlsToRgb(hls, r, g, b);

    Xyz xyz = rgbToXyz(r, g, b);
    Lab lab = xyzToLab(xyz);

    const bool outOfGamut =
        !isRgbInGamut(r, g, b);

    updateAllColors(
        hls,
        xyz,
        lab,
        outOfGamut
        );
}

ColorConverter::~ColorConverter()
{
    delete ui;
}

void ColorConverter::setupControls()
{
    ui->colorMap->installEventFilter(this);
    ui->colorMap->setMouseTracking(true);

    ui->hSlider->setRange(0, 1000);
    ui->lSlider->setRange(0, 1000);
    ui->sSlider->setRange(0, 1000);

    ui->xSlider->setRange(0, 1000);
    ui->ySlider->setRange(0, 1000);
    ui->zSlider->setRange(0, 1000);

    ui->labLSlider->setRange(0, 1000);
    ui->aSlider->setRange(0, 1000);
    ui->labBSlider->setRange(0, 1000);

    // HLS
    ui->hSpinBox->setRange(0.0, 360.0);
    ui->hSpinBox->setDecimals(2);
    ui->hSpinBox->setSingleStep(1.0);
    ui->hSpinBox->setKeyboardTracking(false);

    ui->lSpinBox->setRange(0.0, 100.0);
    ui->lSpinBox->setDecimals(2);
    ui->lSpinBox->setSingleStep(1.0);
    ui->lSpinBox->setKeyboardTracking(false);

    ui->sSpinBox->setRange(0.0, 100.0);
    ui->sSpinBox->setDecimals(2);
    ui->sSpinBox->setSingleStep(1.0);
    ui->sSpinBox->setKeyboardTracking(false);

    // XYZ
    ui->xSpinBox->setRange(0.0, 100.0);
    ui->xSpinBox->setDecimals(4);
    ui->xSpinBox->setSingleStep(0.1);
    ui->xSpinBox->setKeyboardTracking(false);

    ui->ySpinBox->setRange(0.0, 100.0);
    ui->ySpinBox->setDecimals(4);
    ui->ySpinBox->setSingleStep(0.1);
    ui->ySpinBox->setKeyboardTracking(false);

    ui->zSpinBox->setRange(0.0, 100.0);
    ui->zSpinBox->setDecimals(4);
    ui->zSpinBox->setSingleStep(0.1);
    ui->zSpinBox->setKeyboardTracking(false);

    // LAB
    ui->labLSpinBox->setRange(0.0, 100.0);
    ui->labLSpinBox->setDecimals(2);
    ui->labLSpinBox->setSingleStep(1.0);
    ui->labLSpinBox->setKeyboardTracking(false);

    ui->aSpinBox->setRange(-200.0, 200.0);
    ui->aSpinBox->setDecimals(2);
    ui->aSpinBox->setSingleStep(1.0);
    ui->aSpinBox->setKeyboardTracking(false);

    ui->labBSpinBox->setRange(-200.0, 200.0);
    ui->labBSpinBox->setDecimals(2);
    ui->labBSpinBox->setSingleStep(1.0);
    ui->labBSpinBox->setKeyboardTracking(false);

    ui->colorPreview->setMinimumSize(220, 180);

    ui->colorPreview->setStyleSheet(
        "background-color: #ff0000;"
        "border: 2px solid #555555;"
        "border-radius: 4px;"
        );

    ui->gamutWarningLabel->clear();
    ui->gamutWarningLabel->setWordWrap(true);
    ui->gamutWarningLabel->setAlignment(Qt::AlignCenter);
}

void ColorConverter::setupConnections()
{
    // HLS
    connect(
        ui->hSlider,
        &QSlider::valueChanged,
        this,
        &ColorConverter::onHlsSliderChanged
        );

    connect(
        ui->lSlider,
        &QSlider::valueChanged,
        this,
        &ColorConverter::onHlsSliderChanged
        );

    connect(
        ui->sSlider,
        &QSlider::valueChanged,
        this,
        &ColorConverter::onHlsSliderChanged
        );

    connect(
        ui->hSpinBox,
        QOverload<double>::of(
            &QDoubleSpinBox::valueChanged
            ),
        this,
        &ColorConverter::onHlsSpinBoxChanged
        );

    connect(
        ui->lSpinBox,
        QOverload<double>::of(
            &QDoubleSpinBox::valueChanged
            ),
        this,
        &ColorConverter::onHlsSpinBoxChanged
        );

    connect(
        ui->sSpinBox,
        QOverload<double>::of(
            &QDoubleSpinBox::valueChanged
            ),
        this,
        &ColorConverter::onHlsSpinBoxChanged
        );

    // XYZ
    connect(
        ui->xSlider,
        &QSlider::valueChanged,
        this,
        &ColorConverter::onXyzSliderChanged
        );

    connect(
        ui->ySlider,
        &QSlider::valueChanged,
        this,
        &ColorConverter::onXyzSliderChanged
        );

    connect(
        ui->zSlider,
        &QSlider::valueChanged,
        this,
        &ColorConverter::onXyzSliderChanged
        );

    connect(
        ui->xSpinBox,
        QOverload<double>::of(
            &QDoubleSpinBox::valueChanged
            ),
        this,
        &ColorConverter::onXyzSpinBoxChanged
        );

    connect(
        ui->ySpinBox,
        QOverload<double>::of(
            &QDoubleSpinBox::valueChanged
            ),
        this,
        &ColorConverter::onXyzSpinBoxChanged
        );

    connect(
        ui->zSpinBox,
        QOverload<double>::of(
            &QDoubleSpinBox::valueChanged
            ),
        this,
        &ColorConverter::onXyzSpinBoxChanged
        );

    // LAB
    connect(
        ui->labLSlider,
        &QSlider::valueChanged,
        this,
        &ColorConverter::onLabSliderChanged
        );

    connect(
        ui->aSlider,
        &QSlider::valueChanged,
        this,
        &ColorConverter::onLabSliderChanged
        );

    connect(
        ui->labBSlider,
        &QSlider::valueChanged,
        this,
        &ColorConverter::onLabSliderChanged
        );

    connect(
        ui->labLSpinBox,
        QOverload<double>::of(
            &QDoubleSpinBox::valueChanged
            ),
        this,
        &ColorConverter::onLabSpinBoxChanged
        );

    connect(
        ui->aSpinBox,
        QOverload<double>::of(
            &QDoubleSpinBox::valueChanged
            ),
        this,
        &ColorConverter::onLabSpinBoxChanged
        );

    connect(
        ui->labBSpinBox,
        QOverload<double>::of(
            &QDoubleSpinBox::valueChanged
            ),
        this,
        &ColorConverter::onLabSpinBoxChanged
        );
}

bool ColorConverter::eventFilter(
    QObject *watched,
    QEvent *event
    )
{
    if (watched != ui->colorMap) {
        return QMainWindow::eventFilter(
            watched,
            event
            );
    }

    if (event->type() == QEvent::Paint) {
        QPainter painter(ui->colorMap);
        painter.setRenderHint(QPainter::Antialiasing);

        const QRect rect =
            ui->colorMap->rect();

        if (rect.width() <= 1 ||
            rect.height() <= 1) {
            return true;
        }

        QImage image(
            rect.size(),
            QImage::Format_RGB32
            );

        for (int y = 0;
             y < image.height();
             ++y) {
            const double lightness =
                1.0 -
                y /
                    static_cast<double>(
                        image.height() - 1
                        );

            for (int x = 0;
                 x < image.width();
                 ++x) {
                const double hue =
                    x /
                    static_cast<double>(
                        image.width() - 1
                        ) *
                    360.0;

                Hls hls;

                hls.h = hue;
                hls.l = lightness;
                hls.s = 1.0;

                double r = 0.0;
                double g = 0.0;
                double b = 0.0;

                hlsToRgb(
                    hls,
                    r,
                    g,
                    b
                    );

                QColor color;

                if (isRgbInGamut(r, g, b)) {
                    color = QColor::fromRgb(
                        static_cast<int>(std::round(r)),
                        static_cast<int>(std::round(g)),
                        static_cast<int>(std::round(b))
                        );
                } else {
                    color = QColor(150, 150, 150);
                }

                image.setPixelColor(
                    x,
                    y,
                    color
                    );
            }
        }

        painter.drawImage(
            rect,
            image
            );

        painter.setPen(
            QPen(Qt::black, 2)
            );

        painter.setBrush(
            Qt::NoBrush
            );

        painter.drawRect(
            rect.adjusted(
                1,
                1,
                -1,
                -1
                )
            );

        int markerX =
            static_cast<int>(
                m_currentHls.h /
                360.0 *
                rect.width()
                );

        int markerY =
            static_cast<int>(
                (1.0 - m_currentHls.l) *
                rect.height()
                );

        if (markerX < 0) {
            markerX = 0;
        }

        if (markerX >= rect.width()) {
            markerX = rect.width() - 1;
        }

        if (markerY < 0) {
            markerY = 0;
        }

        if (markerY >= rect.height()) {
            markerY = rect.height() - 1;
        }

        painter.setPen(
            QPen(Qt::white, 3)
            );

        painter.drawEllipse(
            QPoint(markerX, markerY),
            8,
            8
            );

        painter.setPen(
            QPen(Qt::black, 1)
            );

        painter.drawEllipse(
            QPoint(markerX, markerY),
            8,
            8
            );

        return true;
    }

    if (event->type() ==
            QEvent::MouseButtonPress ||
        event->type() ==
            QEvent::MouseMove) {
        auto *mouseEvent =
            static_cast<QMouseEvent *>(
                event
                );

        const bool leftButtonPressed =
            mouseEvent->button() ==
                Qt::LeftButton ||
            mouseEvent->buttons().testFlag(
                Qt::LeftButton
                );

        if (!leftButtonPressed) {
            return true;
        }

        const QRect rect =
            ui->colorMap->rect();

        if (rect.width() <= 0 ||
            rect.height() <= 0) {
            return true;
        }

        double x =
            mouseEvent->position().x();

        double y =
            mouseEvent->position().y();

        if (x < 0.0) {
            x = 0.0;
        }

        if (x > rect.width()) {
            x = rect.width();
        }

        if (y < 0.0) {
            y = 0.0;
        }

        if (y > rect.height()) {
            y = rect.height();
        }

        const double hue =
            x /
            static_cast<double>(rect.width()) *
            360.0;

        const double lightness =
            1.0 -
            y /
                static_cast<double>(rect.height());

        Hls hls;

        hls.h = hue;
        hls.l = lightness;
        hls.s = 1.0;

        double r = 0.0;
        double g = 0.0;
        double b = 0.0;

        hlsToRgb(
            hls,
            r,
            g,
            b
            );

        Xyz xyz =
            rgbToXyz(
                r,
                g,
                b
                );

        Lab lab =
            xyzToLab(
                xyz
                );

        const bool outOfGamut =
            !isRgbInGamut(
                r,
                g,
                b
                );

        updateAllColors(
            hls,
            xyz,
            lab,
            outOfGamut
            );

        return true;
    }

    return QMainWindow::eventFilter(
        watched,
        event
        );
}

void ColorConverter::onHlsSliderChanged()
{
    if (m_updating) {
        return;
    }

    Hls hls;

    hls.h =
        coordinateFromSlider(
            ui->hSlider,
            0.0,
            360.0
            );

    hls.l =
        coordinateFromSlider(
            ui->lSlider,
            0.0,
            100.0
            ) /
        100.0;

    hls.s =
        coordinateFromSlider(
            ui->sSlider,
            0.0,
            100.0
            ) /
        100.0;

    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    hlsToRgb(
        hls,
        r,
        g,
        b
        );

    Xyz xyz =
        rgbToXyz(
            r,
            g,
            b
            );

    Lab lab =
        xyzToLab(
            xyz
            );

    const bool outOfGamut =
        !isRgbInGamut(
            r,
            g,
            b
            );

    updateAllColors(
        hls,
        xyz,
        lab,
        outOfGamut
        );
}

void ColorConverter::onHlsSpinBoxChanged()
{
    if (m_updating) {
        return;
    }

    const Hls hls =
        hlsFromControls();

    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    hlsToRgb(
        hls,
        r,
        g,
        b
        );

    Xyz xyz =
        rgbToXyz(
            r,
            g,
            b
            );

    Lab lab =
        xyzToLab(
            xyz
            );

    const bool outOfGamut =
        !isRgbInGamut(
            r,
            g,
            b
            );

    updateAllColors(
        hls,
        xyz,
        lab,
        outOfGamut
        );
}

void ColorConverter::onXyzSliderChanged()
{
    if (m_updating) {
        return;
    }

    Xyz xyz;

    xyz.x =
        coordinateFromSlider(
            ui->xSlider,
            0.0,
            100.0
            );

    xyz.y =
        coordinateFromSlider(
            ui->ySlider,
            0.0,
            100.0
            );

    xyz.z =
        coordinateFromSlider(
            ui->zSlider,
            0.0,
            100.0
            );

    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    xyzToRgb(
        xyz,
        r,
        g,
        b
        );

    bool outOfGamut = false;

    Hls hls =
        rgbToDisplayHls(
            r,
            g,
            b,
            outOfGamut
            );

    Lab lab =
        xyzToLab(
            xyz
            );

    updateAllColors(
        hls,
        xyz,
        lab,
        outOfGamut
        );
}

void ColorConverter::onXyzSpinBoxChanged()
{
    if (m_updating) {
        return;
    }

    const Xyz xyz =
        xyzFromControls();

    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    xyzToRgb(
        xyz,
        r,
        g,
        b
        );

    bool outOfGamut = false;

    Hls hls =
        rgbToDisplayHls(
            r,
            g,
            b,
            outOfGamut
            );

    Lab lab =
        xyzToLab(
            xyz
            );

    updateAllColors(
        hls,
        xyz,
        lab,
        outOfGamut
        );
}

void ColorConverter::onLabSliderChanged()
{
    if (m_updating) {
        return;
    }

    Lab lab;

    lab.l =
        coordinateFromSlider(
            ui->labLSlider,
            0.0,
            100.0
            );

    lab.a =
        coordinateFromSlider(
            ui->aSlider,
            -200.0,
            200.0
            );

    lab.b =
        coordinateFromSlider(
            ui->labBSlider,
            -200.0,
            200.0
            );

    Xyz xyz =
        labToXyz(
            lab
            );

    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    xyzToRgb(
        xyz,
        r,
        g,
        b
        );

    bool outOfGamut = false;

    Hls hls =
        rgbToDisplayHls(
            r,
            g,
            b,
            outOfGamut
            );

    updateAllColors(
        hls,
        xyz,
        lab,
        outOfGamut
        );
}

void ColorConverter::onLabSpinBoxChanged()
{
    if (m_updating) {
        return;
    }

    const Lab lab =
        labFromControls();

    Xyz xyz =
        labToXyz(
            lab
            );

    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    xyzToRgb(
        xyz,
        r,
        g,
        b
        );

    bool outOfGamut = false;

    Hls hls =
        rgbToDisplayHls(
            r,
            g,
            b,
            outOfGamut
            );

    updateAllColors(
        hls,
        xyz,
        lab,
        outOfGamut
        );
}

void ColorConverter::updateAllColors(
    const Hls &hls,
    const Xyz &xyz,
    const Lab &lab,
    bool outOfGamut
    )
{
    m_updating = true;

    m_currentHls = hls;

    QColor color;

    if (outOfGamut) {
        color =
            QColor(
                150,
                150,
                150
                );
    } else {
        color =
            hlsToQColor(
                hls
                );
    }

    updatePreview(
        color,
        outOfGamut
        );

    setHlsControls(
        hls
        );

    setXyzControls(
        xyz
        );

    setLabControls(
        lab
        );

    updateColorMap();

    m_updating = false;
}

void ColorConverter::updatePreview(
    const QColor &color,
    bool outOfGamut
    )
{
    ui->colorPreview->setStyleSheet(
        QString(
            "background-color: %1;"
            "border: 2px solid #555555;"
            "border-radius: 4px;"
            ).arg(
                color.name()
                )
        );

    if (outOfGamut) {
        ui->gamutWarningLabel->setText(
            "Цвет находится вне диапазона RGB "
            "и не может быть отображён точно"
            );

        ui->gamutWarningLabel->setStyleSheet(
            "color: #c62828;"
            "font-weight: bold;"
            );
    } else {
        ui->gamutWarningLabel->clear();

        ui->gamutWarningLabel->setStyleSheet(
            QString()
            );
    }
}

void ColorConverter::updateColorMap()
{
    ui->colorMap->update();
}

void ColorConverter::setCoordinate(
    QSlider *slider,
    QDoubleSpinBox *spinBox,
    double value,
    double minimum,
    double maximum
    )
{
    if (!std::isfinite(value) ||
        value < minimum ||
        value > maximum) {
        return;
    }

    const QSignalBlocker sliderBlocker(
        slider
        );

    const QSignalBlocker spinBoxBlocker(
        spinBox
        );

    const double ratio =
        (value - minimum) /
        (maximum - minimum);

    slider->setValue(
        static_cast<int>(
            std::round(
                ratio * 1000.0
                )
            )
        );

    spinBox->setValue(
        value
        );
}

double ColorConverter::coordinateFromSlider(
    QSlider *slider,
    double minimum,
    double maximum
    ) const
{
    const double ratio =
        slider->value() /
        1000.0;

    return minimum +
           ratio *
               (maximum - minimum);
}

void ColorConverter::setHlsControls(
    const Hls &hls
    )
{
    setCoordinate(
        ui->hSlider,
        ui->hSpinBox,
        hls.h,
        0.0,
        360.0
        );

    setCoordinate(
        ui->lSlider,
        ui->lSpinBox,
        hls.l * 100.0,
        0.0,
        100.0
        );

    setCoordinate(
        ui->sSlider,
        ui->sSpinBox,
        hls.s * 100.0,
        0.0,
        100.0
        );
}

void ColorConverter::setXyzControls(
    const Xyz &xyz
    )
{
    setCoordinate(
        ui->xSlider,
        ui->xSpinBox,
        xyz.x,
        0.0,
        100.0
        );

    setCoordinate(
        ui->ySlider,
        ui->ySpinBox,
        xyz.y,
        0.0,
        100.0
        );

    setCoordinate(
        ui->zSlider,
        ui->zSpinBox,
        xyz.z,
        0.0,
        100.0
        );
}

void ColorConverter::setLabControls(
    const Lab &lab
    )
{
    setCoordinate(
        ui->labLSlider,
        ui->labLSpinBox,
        lab.l,
        0.0,
        100.0
        );

    setCoordinate(
        ui->aSlider,
        ui->aSpinBox,
        lab.a,
        -200.0,
        200.0
        );

    setCoordinate(
        ui->labBSlider,
        ui->labBSpinBox,
        lab.b,
        -200.0,
        200.0
        );
}

ColorConverter::Hls
ColorConverter::hlsFromControls() const
{
    Hls hls;

    hls.h =
        ui->hSpinBox->value();

    hls.l =
        ui->lSpinBox->value() /
        100.0;

    hls.s =
        ui->sSpinBox->value() /
        100.0;

    return hls;
}

ColorConverter::Xyz
ColorConverter::xyzFromControls() const
{
    Xyz xyz;

    xyz.x =
        ui->xSpinBox->value();

    xyz.y =
        ui->ySpinBox->value();

    xyz.z =
        ui->zSpinBox->value();

    return xyz;
}

ColorConverter::Lab
ColorConverter::labFromControls() const
{
    Lab lab;

    lab.l =
        ui->labLSpinBox->value();

    lab.a =
        ui->aSpinBox->value();

    lab.b =
        ui->labBSpinBox->value();

    return lab;
}

QColor ColorConverter::hlsToQColor(
    const Hls &hls
    ) const
{
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    hlsToRgb(
        hls,
        r,
        g,
        b
        );

    if (!isRgbInGamut(r, g, b)) {
        return QColor(
            150,
            150,
            150
            );
    }

    return QColor::fromRgb(
        static_cast<int>(
            std::round(r)
            ),
        static_cast<int>(
            std::round(g)
            ),
        static_cast<int>(
            std::round(b)
            )
        );
}

bool ColorConverter::isRgbInGamut(
    double r,
    double g,
    double b
    ) const
{
    constexpr double epsilon = 1e-9;

    return std::isfinite(r) &&
           std::isfinite(g) &&
           std::isfinite(b) &&
           r >= -epsilon &&
           r <= 255.0 + epsilon &&
           g >= -epsilon &&
           g <= 255.0 + epsilon &&
           b >= -epsilon &&
           b <= 255.0 + epsilon;
}

ColorConverter::Hls
ColorConverter::rgbToDisplayHls(
    double r,
    double g,
    double b,
    bool &outOfGamut
    ) const
{
    outOfGamut =
        !isRgbInGamut(
            r,
            g,
            b
            );

    if (outOfGamut) {
        return m_currentHls;
    }

    return rgbToHls(
        r,
        g,
        b
        );
}

ColorConverter::Hls
ColorConverter::rgbToHls(
    double r,
    double g,
    double b
    ) const
{
    r /= 255.0;
    g /= 255.0;
    b /= 255.0;

    const double maximum =
        std::max({r, g, b});

    const double minimum =
        std::min({r, g, b});

    const double difference =
        maximum - minimum;

    Hls hls;

    hls.l =
        (maximum + minimum) / 2.0;

    if (difference == 0.0) {
        hls.h = 0.0;
        hls.s = 0.0;
        return hls;
    }

    if (hls.l <= 0.5) {
        hls.s =
            difference /
            (maximum + minimum);
    } else {
        hls.s =
            difference /
            (2.0 - maximum - minimum);
    }

    if (maximum == r) {
        hls.h =
            60.0 *
            std::fmod(
                (g - b) / difference,
                6.0
                );
    } else if (maximum == g) {
        hls.h =
            60.0 *
            ((b - r) / difference + 2.0);
    } else {
        hls.h =
            60.0 *
            ((r - g) / difference + 4.0);
    }

    if (hls.h < 0.0) {
        hls.h += 360.0;
    }

    return hls;
}

// HLS -> RGB

double Value(double hue, double m1, double m2){
    while (hue < 0.0)
        hue += 360.0;
    while (hue >= 360.0)
        hue -= 360.0;

    if(hue < 60.0){
        return m1 + (m2 - m1) * hue / 60.0;
    }
    if(hue < 180.0){
        return m2;
    }
    if(hue < 240.0){
        return m1 + (m2 - m1)*(240.0 - hue) / 60;
    }
    return m1;
}

void ColorConverter::hlsToRgb(
    const Hls &hls,
    double &r,
    double &g,
    double &b
    ) const
{
    double h = hls.h;
    double l = hls.l;
    double s = hls.s;

    double m2;
    if(l < 0.5){
        m2 = l * (1.0 + s);
    } else {
        m2 = l + s - l * s;
    }

    if(s < 1e-9){
        if(h == -1.0 || std::isnan(h)){
            throw std::runtime_error("Ошибка: H = ndf!");
        } else {
            r = l;
            g = l;
            b = l;
        }
    } else {
        double m1 = 2.0 * l - m2;
        r = Value(h + 120.0, m1, m2);
        g = Value(h, m1, m2);
        b = Value(h - 120.0, m1, m2);
    }
}

// RGB -> XYZ

double rgbxyzF(double x){
    if(x >= 0.04045){
        return std::pow( ((x + 0.055)/1.055), 2.4 );
    }
    return x/12.92;
}

ColorConverter::Xyz
ColorConverter::rgbToXyz(
    double r,
    double g,
    double b
    ) const
{
    double Rn = rgbxyzF(r/255) * 100;
    double Gn = rgbxyzF(g/255) * 100;
    double Bn = rgbxyzF(b/255) * 100;

    Xyz xyz;

    xyz.x = 0.412453 * Rn + 0.357580 * Gn + 0.180423 * Bn;
    xyz.y = 0.212671 * Rn + 0.715160 * Gn + 0.072169 * Bn;
    xyz.z = 0.019334 * Rn + 0.119193 * Gn + 0.950227 * Bn;

    return xyz;
}

// XYZ -> RGB
double xyzrgbF(double x){
    if(x >= 0.0031308){
        return 1.055 * std::pow(x, 1/2.4) - 0.055;
    }
    return 12.92 * x;
}

void ColorConverter::xyzToRgb(
    const Xyz &xyz,
    double &r,
    double &g,
    double &b
    ) const
{
    double x = xyz.x;
    double y = xyz.y;
    double z = xyz.z;

    double Rn =  3.2406 * (x/100) - 1.5372 * (y/100) - 0.4986 * (z/100);
    double Gn = -0.9689 * (x/100) + 1.8758 * (y/100) + 0.0415 * (z/100);
    double Bn =  0.0557 * (x/100) - 0.2040 * (y/100) + 1.0570 * (z/100);

    r = xyzrgbF(Rn) * 255;
    g = xyzrgbF(Gn) * 255;
    b = xyzrgbF(Bn) * 255;
}

// XYZ -> LAB
double xyzlabF(double x){
    if(x >= 0.008856){
        return std::pow(x, 1.0/3.0);
    }
    return 7.787 * x + 16.0/116.0;
}

ColorConverter::Lab
ColorConverter::xyzToLab(
    const Xyz &xyz
    ) const
{
    Lab lab;

    double x = xyz.x;
    double y = xyz.y;
    double z = xyz.z;

    const double Xw = 95.047;
    const double Yw = 100.0;
    const double Zw = 108.883;

    lab.l = 116.0 * xyzlabF(y/Yw) - 16;
    lab.a = 500.0 * (xyzlabF(x/Xw) - xyzlabF(y/Yw));
    lab.b = 200.0 * (xyzlabF(y/Yw) - xyzlabF(z/Zw));

    return lab;
}

// LAB -> XYZ

ColorConverter::Xyz
ColorConverter::labToXyz(
    const Lab &lab
    ) const
{
    Xyz xyz;

    // TODO

    return xyz;
}