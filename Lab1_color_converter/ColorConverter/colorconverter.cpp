#include "colorconverter.h"
#include "ui_colorconverter.h"
#include "colormath.h"

#include <QColor>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QRect>
#include <QSignalBlocker>
#include <QSlider>

#include <algorithm>
#include <cmath>

ColorConverter::ColorConverter(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ColorConverter)
    , m_updating(false)
    , m_currentHls{180.0, 0.5, 1.0}
    , m_currentXyz{0.0, 0.0, 0.0}
    , m_currentLab{0.0, 0.0, 0.0}
{
    ui->setupUi(this);
    setupControls();
    setupConnections();

    const Hls hls{180.0, 0.5, 1.0};
    double r = 0.0, g = 0.0, b = 0.0;
    hlsToRgb(hls, r, g, b);

    const Xyz xyz = rgbToXyz(r, g, b);
    const Lab lab = xyzToLab(xyz);
    updateAllColors(hls, xyz, lab, !isRgbInGamut(r, g, b));
}

ColorConverter::~ColorConverter()
{
    delete ui;
}

void ColorConverter::setupControls()
{
    ui->colorMap->installEventFilter(this);
    ui->colorMap->setMouseTracking(true);

    for (QSlider *slider : {
             ui->hSlider, ui->lSlider, ui->sSlider,
             ui->xSlider, ui->ySlider, ui->zSlider,
             ui->labLSlider, ui->aSlider, ui->labBSlider
         }) {
        slider->setRange(0, 1000);
    }

    auto configureSpinBox = [](QDoubleSpinBox *spinBox,
                               double minimum,
                               double maximum) {
        spinBox->setRange(minimum, maximum);
        spinBox->setDecimals(0);
        spinBox->setSingleStep(1.0);
        spinBox->setKeyboardTracking(false);
    };

    configureSpinBox(ui->hSpinBox, 0.0, 360.0);
    configureSpinBox(ui->lSpinBox, 0.0, 100.0);
    configureSpinBox(ui->sSpinBox, 0.0, 100.0);
    configureSpinBox(ui->xSpinBox, 0.0, 100.0);
    configureSpinBox(ui->ySpinBox, 0.0, 100.0);
    configureSpinBox(ui->zSpinBox, 0.0, 100.0);
    configureSpinBox(ui->labLSpinBox, 0.0, 100.0);
    configureSpinBox(ui->aSpinBox, -200.0, 200.0);
    configureSpinBox(ui->labBSpinBox, -200.0, 200.0);
    configureSpinBox(ui->saturationSpinBox, 0.0, 100.0);

    ui->saturationSlider->setRange(0, 100);
    ui->saturationSlider->setValue(100);
    ui->saturationSpinBox->setValue(100.0);

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
    connect(ui->hSlider, &QSlider::valueChanged, this, &ColorConverter::onHlsSliderChanged);
    connect(ui->lSlider, &QSlider::valueChanged, this, &ColorConverter::onHlsSliderChanged);
    connect(ui->sSlider, &QSlider::valueChanged, this, &ColorConverter::onHlsSliderChanged);

    connect(ui->hSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorConverter::onHlsSpinBoxChanged);
    connect(ui->lSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorConverter::onHlsSpinBoxChanged);
    connect(ui->sSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorConverter::onHlsSpinBoxChanged);

    connect(ui->xSlider, &QSlider::valueChanged, this, &ColorConverter::onXyzSliderChanged);
    connect(ui->ySlider, &QSlider::valueChanged, this, &ColorConverter::onXyzSliderChanged);
    connect(ui->zSlider, &QSlider::valueChanged, this, &ColorConverter::onXyzSliderChanged);

    connect(ui->xSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorConverter::onXyzSpinBoxChanged);
    connect(ui->ySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorConverter::onXyzSpinBoxChanged);
    connect(ui->zSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorConverter::onXyzSpinBoxChanged);

    connect(ui->labLSlider, &QSlider::valueChanged, this, &ColorConverter::onLabSliderChanged);
    connect(ui->aSlider, &QSlider::valueChanged, this, &ColorConverter::onLabSliderChanged);
    connect(ui->labBSlider, &QSlider::valueChanged, this, &ColorConverter::onLabSliderChanged);

    connect(ui->labLSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorConverter::onLabSpinBoxChanged);
    connect(ui->aSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorConverter::onLabSpinBoxChanged);
    connect(ui->labBSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorConverter::onLabSpinBoxChanged);

    connect(ui->saturationSlider, &QSlider::valueChanged, this, &ColorConverter::onSaturationSliderChanged);
    connect(ui->saturationSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorConverter::onSaturationSpinBoxChanged);
}

bool ColorConverter::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != ui->colorMap) {
        return QMainWindow::eventFilter(watched, event);
    }

    if (event->type() == QEvent::Paint) {
        QPainter painter(ui->colorMap);
        painter.setRenderHint(QPainter::Antialiasing);
        const QRect rect = ui->colorMap->rect();
        if (rect.width() <= 1 || rect.height() <= 1) return true;

        QImage image(rect.size(), QImage::Format_RGB32);
        const double maxX = rect.width() - 1.0;
        const double maxY = rect.height() - 1.0;
        const double saturation = saturationFromControls();

        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                const Hls hls{
                    x / maxX * 360.0,
                    1.0 - y / maxY,
                    saturation
                };
                image.setPixelColor(x, y, hlsToQColor(hls));
            }
        }

        painter.drawImage(rect, image);
        painter.setPen(QPen(Qt::black, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(rect.adjusted(1, 1, -1, -1));

        int markerX = static_cast<int>(std::round(
            m_currentHls.h / 360.0 * (rect.width() - 1)));
        int markerY = static_cast<int>(std::round(
            (1.0 - m_currentHls.l) * (rect.height() - 1)));

        markerX = std::clamp(markerX, 0, rect.width() - 1);
        markerY = std::clamp(markerY, 0, rect.height() - 1);

        painter.setPen(QPen(Qt::white, 3));
        painter.drawEllipse(QPoint(markerX, markerY), 8, 8);
        painter.setPen(QPen(Qt::black, 1));
        painter.drawEllipse(QPoint(markerX, markerY), 8, 8);
        return true;
    }

    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseMove) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() != Qt::LeftButton &&
            !mouseEvent->buttons().testFlag(Qt::LeftButton)) return true;

        const QRect rect = ui->colorMap->rect();
        if (rect.width() <= 1 || rect.height() <= 1) return true;

        const double maxX = rect.width() - 1.0;
        const double maxY = rect.height() - 1.0;
        const double x = std::clamp(mouseEvent->position().x(), 0.0, maxX);
        const double y = std::clamp(mouseEvent->position().y(), 0.0, maxY);

        const Hls hls{
            x / maxX * 360.0,
            1.0 - y / maxY,
            saturationFromControls()
        };

        double r = 0.0, g = 0.0, b = 0.0;
        hlsToRgb(hls, r, g, b);
        const Xyz xyz = rgbToXyz(r, g, b);
        const Lab lab = xyzToLab(xyz);
        updateAllColors(hls, xyz, lab, !isRgbInGamut(r, g, b));
        return true;
    }

    return QMainWindow::eventFilter(watched, event);
}

void ColorConverter::onHlsSliderChanged()
{
    if (m_updating) return;
    const Hls hls{
        coordinateFromSlider(ui->hSlider, 0.0, 360.0),
        coordinateFromSlider(ui->lSlider, 0.0, 100.0) / 100.0,
        coordinateFromSlider(ui->sSlider, 0.0, 100.0) / 100.0
    };

    double r = 0.0, g = 0.0, b = 0.0;
    hlsToRgb(hls, r, g, b);
    const Xyz xyz = rgbToXyz(r, g, b);
    const Lab lab = xyzToLab(xyz);
    updateAllColors(hls, xyz, lab, !isRgbInGamut(r, g, b));
}

void ColorConverter::onHlsSpinBoxChanged()
{
    if (m_updating) return;
    const Hls hls = hlsFromControls();
    double r = 0.0, g = 0.0, b = 0.0;
    hlsToRgb(hls, r, g, b);
    const Xyz xyz = rgbToXyz(r, g, b);
    const Lab lab = xyzToLab(xyz);
    updateAllColors(hls, xyz, lab, !isRgbInGamut(r, g, b));
}

void ColorConverter::onXyzSliderChanged()
{
    if (m_updating) return;
    const Xyz xyz{
        coordinateFromSlider(ui->xSlider, 0.0, 100.0),
        coordinateFromSlider(ui->ySlider, 0.0, 100.0),
        coordinateFromSlider(ui->zSlider, 0.0, 100.0)
    };

    double r = 0.0, g = 0.0, b = 0.0;
    xyzToRgb(xyz, r, g, b);
    bool outOfGamut = false;
    const Hls hls = rgbToDisplayHls(r, g, b, outOfGamut, m_currentHls);
    updateAllColors(hls, xyz, xyzToLab(xyz), outOfGamut);
}

void ColorConverter::onXyzSpinBoxChanged()
{
    if (m_updating) return;
    const Xyz xyz = xyzFromControls();
    double r = 0.0, g = 0.0, b = 0.0;
    xyzToRgb(xyz, r, g, b);
    bool outOfGamut = false;
    const Hls hls = rgbToDisplayHls(r, g, b, outOfGamut, m_currentHls);
    updateAllColors(hls, xyz, xyzToLab(xyz), outOfGamut);
}

void ColorConverter::onLabSliderChanged()
{
    if (m_updating) return;
    const Lab lab{
        coordinateFromSlider(ui->labLSlider, 0.0, 100.0),
        coordinateFromSlider(ui->aSlider, -200.0, 200.0),
        coordinateFromSlider(ui->labBSlider, -200.0, 200.0)
    };

    const Xyz xyz = labToXyz(lab);
    double r = 0.0, g = 0.0, b = 0.0;
    xyzToRgb(xyz, r, g, b);
    bool outOfGamut = false;
    const Hls hls = rgbToDisplayHls(r, g, b, outOfGamut, m_currentHls);
    updateAllColors(hls, xyz, lab, outOfGamut);
}

void ColorConverter::onLabSpinBoxChanged()
{
    if (m_updating) return;
    const Lab lab = labFromControls();
    const Xyz xyz = labToXyz(lab);
    double r = 0.0, g = 0.0, b = 0.0;
    xyzToRgb(xyz, r, g, b);
    bool outOfGamut = false;
    const Hls hls = rgbToDisplayHls(r, g, b, outOfGamut, m_currentHls);
    updateAllColors(hls, xyz, lab, outOfGamut);
}

void ColorConverter::onSaturationSliderChanged(int value)
{
    if (m_updating) return;
    const Hls hls{m_currentHls.h, m_currentHls.l, value / 100.0};
    double r = 0.0, g = 0.0, b = 0.0;
    hlsToRgb(hls, r, g, b);
    const Xyz xyz = rgbToXyz(r, g, b);
    updateAllColors(hls, xyz, xyzToLab(xyz), !isRgbInGamut(r, g, b));
}

void ColorConverter::onSaturationSpinBoxChanged(double value)
{
    if (m_updating) return;
    const Hls hls{m_currentHls.h, m_currentHls.l, value / 100.0};
    double r = 0.0, g = 0.0, b = 0.0;
    hlsToRgb(hls, r, g, b);
    const Xyz xyz = rgbToXyz(r, g, b);
    updateAllColors(hls, xyz, xyzToLab(xyz), !isRgbInGamut(r, g, b));
}

void ColorConverter::setSaturationControl(double saturation)
{
    if (!std::isfinite(saturation) || saturation < 0.0 || saturation > 1.0) return;
    const int value = static_cast<int>(std::round(saturation * 100.0));
    const QSignalBlocker sliderBlocker(ui->saturationSlider);
    const QSignalBlocker spinBoxBlocker(ui->saturationSpinBox);
    ui->saturationSlider->setValue(value);
    ui->saturationSpinBox->setValue(value);
}

double ColorConverter::saturationFromControls() const
{
    return ui->saturationSpinBox->value() / 100.0;
}

void ColorConverter::updateAllColors(const Hls &hls, const Xyz &xyz, const Lab &lab, bool outOfGamut)
{
    m_updating = true;
    m_currentHls = hls;
    m_currentXyz = xyz;
    m_currentLab = lab;

    updatePreview(hlsToQColor(hls), outOfGamut);
    setHlsControls(hls);
    setXyzControls(xyz);
    setLabControls(lab);
    setSaturationControl(hls.s);
    updateColorMap();
    m_updating = false;
}

void ColorConverter::updatePreview(const QColor &color, bool outOfGamut)
{
    ui->colorPreview->setStyleSheet(
        QString("background-color: %1; border: 2px solid #555555; border-radius: 4px;").arg(color.name()));

    if (outOfGamut) {
        ui->gamutWarningLabel->setText("Цвет находится вне диапазона RGB. Отображён приближенный цвет");
        ui->gamutWarningLabel->setStyleSheet("color: #c62828; font-weight: bold;");
    } else {
        ui->gamutWarningLabel->clear();
        ui->gamutWarningLabel->setStyleSheet(QString());
    }
}

void ColorConverter::updateColorMap()
{
    ui->colorMap->update();
}

void ColorConverter::setCoordinate(QSlider *slider, QDoubleSpinBox *spinBox, double value, double minimum, double maximum)
{
    if (!std::isfinite(value) || value < minimum || value > maximum) return;
    const QSignalBlocker sliderBlocker(slider);
    const QSignalBlocker spinBoxBlocker(spinBox);
    const double ratio = (value - minimum) / (maximum - minimum);
    slider->setValue(static_cast<int>(std::round(ratio * 1000.0)));
    spinBox->setValue(std::round(value));
}

double ColorConverter::coordinateFromSlider(QSlider *slider, double minimum, double maximum) const
{
    return minimum + slider->value() / 1000.0 * (maximum - minimum);
}

void ColorConverter::setHlsControls(const Hls &hls)
{
    setCoordinate(ui->hSlider, ui->hSpinBox, hls.h, 0.0, 360.0);
    setCoordinate(ui->lSlider, ui->lSpinBox, hls.l * 100.0, 0.0, 100.0);
    setCoordinate(ui->sSlider, ui->sSpinBox, hls.s * 100.0, 0.0, 100.0);
}

void ColorConverter::setXyzControls(const Xyz &xyz)
{
    setCoordinate(ui->xSlider, ui->xSpinBox, xyz.x, 0.0, 100.0);
    setCoordinate(ui->ySlider, ui->ySpinBox, xyz.y, 0.0, 100.0);
    setCoordinate(ui->zSlider, ui->zSpinBox, xyz.z, 0.0, 100.0);
}

void ColorConverter::setLabControls(const Lab &lab)
{
    setCoordinate(ui->labLSlider, ui->labLSpinBox, lab.l, 0.0, 100.0);
    setCoordinate(ui->aSlider, ui->aSpinBox, lab.a, -200.0, 200.0);
    setCoordinate(ui->labBSlider, ui->labBSpinBox, lab.b, -200.0, 200.0);
}

Hls ColorConverter::hlsFromControls() const
{
    return Hls{ui->hSpinBox->value(), ui->lSpinBox->value() / 100.0, ui->sSpinBox->value() / 100.0};
}

Xyz ColorConverter::xyzFromControls() const
{
    return Xyz{ui->xSpinBox->value(), ui->ySpinBox->value(), ui->zSpinBox->value()};
}

Lab ColorConverter::labFromControls() const
{
    return Lab{ui->labLSpinBox->value(), ui->aSpinBox->value(), ui->labBSpinBox->value()};
}

QColor ColorConverter::hlsToQColor(const Hls &hls) const
{
    double r = 0.0, g = 0.0, b = 0.0;
    hlsToRgb(hls, r, g, b);
    r = std::clamp(r, 0.0, 255.0);
    g = std::clamp(g, 0.0, 255.0);
    b = std::clamp(b, 0.0, 255.0);
    return QColor::fromRgb(static_cast<int>(std::round(r)), static_cast<int>(std::round(g)), static_cast<int>(std::round(b)));
}
