#ifndef COLORCONVERTER_H
#define COLORCONVERTER_H

#include <QColor>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class ColorConverter;
}
QT_END_NAMESPACE

class ColorConverter : public QMainWindow
{
    Q_OBJECT

public:
    explicit ColorConverter(QWidget *parent = nullptr);
    ~ColorConverter() override;

private:
    Ui::ColorConverter *ui;
};
#endif // COLORCONVERTER_H
