#include "colorconverter.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ColorConverter w;
    w.show();
    return QApplication::exec();
}
