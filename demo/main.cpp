#include "widget.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    const QString path = "/sys/devices/platform/ff740000.usb2-phy/otg_mode";
    QFile file(path);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "打开失败：" << file.errorString();
    }else{
        QString name("otg");
        file.write(name.toUtf8());
        file.close();
        qDebug() << "当前模式: " << name;
    }

    Widget w;
    w.show();
    return a.exec();
}
