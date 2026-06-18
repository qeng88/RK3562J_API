#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    alson = Alson_api::instance();
    if (!alson) qCritical() << "无法获取 DeviceSDK 实例";

    Functions();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::Functions()
{
    QTimer::singleShot(2000,this,[this](){
        Can();  // 硬件没有准备好，需要延时启动

    });

    DevicesInfo();
}

void Widget::Can()
{
    // can数据接收
    connect(alson, &Alson_api::frameReceived,
                     [](const QString &interface, uint id, const QByteArray &data, bool isExtended) {
        Q_UNUSED(interface);Q_UNUSED(id);Q_UNUSED(data);Q_UNUSED(isExtended);
        qDebug() << "收到帧 - 接口:" << interface
                 << "ID:" << QString("0x%1").arg(id, 0, 16)
                 << "数据:" << data.toHex(' ')
                 << "扩展帧:" << (isExtended ? "是" : "否");
    });

    // can数据是否发送成功
    connect(alson, &Alson_api::frameSent,
            [](const QString &interface, bool ok, uint id) {
        Q_UNUSED(interface);Q_UNUSED(ok);Q_UNUSED(id);
        //qDebug() << "发送结果:" << interface << ok << QString("0x%1").arg(id, 0, 16);
    });

    // can数据错误解析
    connect(alson, &Alson_api::errorOccurred,
            [](const QString &interface, const QString &err) {
        qWarning() << "CAN错误:" << interface << err;
    });

    // CAN是否已经连接，连接成功后才能进行数据发送
    QObject::connect(alson, &Alson_api::connected,
                     [this](const QString &interface) {
        //qDebug() << "CAN 已连接:" << interface;

        if(interface == "can0"){
            QByteArray data = QByteArray::fromHex("01020304"); // 发送测试帧
            alson->sendFrame("can0", 0x123, data);

            QByteArray periodicData = QByteArray::fromHex("AABBCCDD");
            alson->addPeriodicFrame("can0", 0x456, periodicData, false, 100); // 添加周期性帧
        }

        if(interface == "can1"){
            QByteArray data = QByteArray::fromHex("11223344");
            alson->sendFrame("can1", 0x182, data);

            QByteArray periodicData = QByteArray::fromHex("1020304050");
            alson->addPeriodicFrame("can1", 0x18FF0102, periodicData, true, 100);
        }
    });

    // 启动 CAN 接口
    alson->startCan("can0", 250000);
    alson->startCan("can1", 500000);
}

void Widget::DevicesInfo()
{
    qDebug() << "错误输入:" << alson->readDeviceInfo("31313");
    qDebug() << "板卡型号:" << alson->readDeviceInfo("板卡型号");
    qDebug() << "CPU核心数:" << alson->readDeviceInfo("CPU核心数");
    qDebug() << "CPU架构:" << alson->readDeviceInfo("CPU架构");
    qDebug() << "CPU频率:" << alson->readDeviceInfo("CPU频率");
    qDebug() << "DDR容量:" << alson->readDeviceInfo("DDR容量");
    qDebug() << "eMMC容量:" << alson->readDeviceInfo("eMMC容量");
    qDebug() << "系统版本:" << alson->readDeviceInfo("系统版本");
    qDebug() << "设备序列号:" << alson->readDeviceInfo("设备序列号");
    qDebug() << "MAC地址:" << alson->readDeviceInfo("MAC地址");
    qDebug() << "网络节点状态:" << alson->readDeviceInfo("网络节点状态");
    qDebug() << "CPU负载率:" << alson->readDeviceInfo("CPU负载率");
    qDebug() << "库版本:" << alson->readApiVersion();
    qDebug() << Qt::endl;
}

