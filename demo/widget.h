#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>
#include <QTimer>

#include "alson_api.h"


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

public slots:
    void update();

private slots:
    void on_horizontalSlider_valueChanged(int value);

private:
    Ui::Widget *ui;
    Alson_api *alson = nullptr;

    void Functions();
    void Can();
    void DevicesInfo();
    void Light();
    void RTC();

};
#endif // WIDGET_H
