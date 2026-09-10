#include "colorconverter.h"
#include "./ui_colorconverter.h"

ColorConverter::ColorConverter(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ColorConverter)
{
    ui->setupUi(this);
}

ColorConverter::~ColorConverter()
{
    delete ui;
}
