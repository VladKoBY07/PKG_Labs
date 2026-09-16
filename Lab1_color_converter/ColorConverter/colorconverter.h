#ifndef COLORCONVERTER_H
#define COLORCONVERTER_H

#include <QMainWindow>
#include "colormath.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ColorConverter;
}
QT_END_NAMESPACE

class QEvent;
class QObject;
class QColor;
class QDoubleSpinBox;
class QSlider;

class ColorConverter : public QMainWindow
{
    Q_OBJECT

public:
    explicit ColorConverter(QWidget *parent = nullptr);
    ~ColorConverter();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onHlsSliderChanged();
    void onHlsSpinBoxChanged();
    void onXyzSliderChanged();
    void onXyzSpinBoxChanged();
    void onLabSliderChanged();
    void onLabSpinBoxChanged();

private:
    void setupControls();
    void setupConnections();
    void updateAllColors(const Hls &hls, const Xyz &xyz, const Lab &lab, bool outOfGamut);
    void updatePreview(const QColor &color, bool outOfGamut);
    void updateColorMap();
    void setCoordinate(QSlider *slider, QDoubleSpinBox *spinBox,
                       double value, double minimum, double maximum);
    double coordinateFromSlider(QSlider *slider, double minimum, double maximum) const;
    void setHlsControls(const Hls &hls);
    void setXyzControls(const Xyz &xyz);
    void setLabControls(const Lab &lab);
    Hls hlsFromControls() const;
    Xyz xyzFromControls() const;
    Lab labFromControls() const;
    QColor hlsToQColor(const Hls &hls) const;

private:
    Ui::ColorConverter *ui;
    bool m_updating;
    Hls m_currentHls;
};

#endif // COLORCONVERTER_H