#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowTitle("RK3568J - 全功能 API 封装 - demo");

    alson = Alson_api::instance();
    if (!alson) qCritical() << "无法获取 AlsonAPI 实例";

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Widget::update);
    timer->start(1000);

    Functions();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::update()
{
    ui->dateTime->setText(alson->getRTC());
    ui->cpuused->setText(alson->getDeviceInfo("CPU负载率"));
    ui->canstatus->setText(alson->getDeviceInfo("CAN状态"));
    ui->cpuTemp->setText(alson->getDeviceInfo("CPU温度"));
}

void Widget::Functions()
{
    QTimer::singleShot(2000,this,[this](){
        Can();  // 硬件没有准备好，需要延时启动

        Light();
        RTC();
        RetainData();
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
        Q_UNUSED(interface);Q_UNUSED(err);
        //qWarning() << "CAN错误:" << interface << err;
    });

    // CAN是否已经连接，连接成功后才能进行数据发送
    QObject::connect(alson, &Alson_api::connected,
                     [this](const QString &interface) {
        //qDebug() << "CAN 已连接:" << interface;

        if(interface == "can0"){
            QByteArray data = QByteArray::fromHex("01020304"); // 发送测试帧
            alson->sendFrame("can0", 0x123, data);

            QByteArray periodicData = QByteArray::fromHex("AABBCCDDEEFF");
            //alson->addPeriodicFrame("can0", 0x456, periodicData, false, 100); // 添加周期性帧
            for (int i = 0; i < 0x20; ++i) {
                quint32 id = 0x100 + i;
                int offsetMs = i;   // 100个帧分散到100ms内，每1ms一个
                alson->addPeriodicFrame("can0", id, periodicData, false, 100, offsetMs);
            }
        }

        if(interface == "can1"){
            QByteArray rawData(8, 0);   // 固定为8个字节，且初始化为0
            rawData[0] = 0x11; rawData[1] = 0x22; rawData[2] = 0x33; rawData[3] = 0x44;
            rawData[4] = 0x55; rawData[5] = 0x66; rawData[6] = 0x77; rawData[7] = 0x88;
            alson->sendFrame("can1", 0x182, rawData);

            QByteArray datas(8, 0xff);
            alson->sendFrame("can1", 0x282, datas);

            QByteArray periodicData;
            periodicData.resize(8);     // 固定为8个字节
            periodicData[0] = 0x10; periodicData[1] = 0x20; periodicData[2] = 0x30; periodicData[3] = 0x40;
            periodicData[4] = 0x50; periodicData[5] = 0x60; periodicData[6] = 0x70; periodicData[7] = 0x80;
            alson->addPeriodicFrame("can1", 0x18FF0102, periodicData, true, 100);
        }
    });

    // 启动 CAN 接口
    alson->startCan("can0", 250000);
    alson->startCan("can1", 500000);

    qDebug() << "";
}

void Widget::DevicesInfo()
{
    qDebug() << "错误示例:" << alson->getDeviceInfo("31313");
    qDebug() << "芯片型号:" << alson->getDeviceInfo("芯片型号");
    qDebug() << "CPU核心数:" << alson->getDeviceInfo("CPU核心数");
    qDebug() << "CPU架构:" << alson->getDeviceInfo("CPU架构");
    qDebug() << "CPU频率:" << alson->getDeviceInfo("CPU频率");
    qDebug() << "内存容量:" << alson->getDeviceInfo("内存容量");
    qDebug() << "闪存容量:" << alson->getDeviceInfo("闪存容量");
    qDebug() << "系统版本:" << alson->getDeviceInfo("系统版本");
    qDebug() << "设备序列号:" << alson->getDeviceInfo("设备序列号");
    qDebug() << "MAC地址:" << alson->getDeviceInfo("MAC地址");
    qDebug() << "网络节点状态:" << alson->getDeviceInfo("网络节点状态");
    qDebug() << "CPU负载率:" << alson->getDeviceInfo("CPU负载率");
    qDebug() << "";

    qDebug() << "料号:" << alson->getDeviceInfo("料号");
    ui->pn->setText(alson->getDeviceInfo("料号"));
    qDebug() << "驱动版本:" << alson->getDeviceInfo("驱动版本");
    ui->driverversion->setText(alson->getDeviceInfo("驱动版本"));
    qDebug() << "构建时间:" << alson->getDeviceInfo("构建时间");
    qDebug() << "屏幕分辨率:" << alson->getDeviceInfo("屏幕分辨率");
    ui->resolution->setText(alson->getDeviceInfo("屏幕分辨率"));
    qDebug() << "可用空间:" << alson->getDeviceInfo("可用空间");
    qDebug() << "BSP版本:" << alson->getDeviceInfo("BSP版本");
    qDebug() << "CAN数量:" << alson->getDeviceInfo("CAN数量");
    qDebug() << "CAN状态:" << alson->getDeviceInfo("CAN状态");
    qDebug() << "CPU温度:" << alson->getDeviceInfo("CPU温度");

    qDebug() << "库版本:" << alson->getApiVersion();
    ui->version->setText(alson->getApiVersion());
    qDebug() << "";
}

void Widget::on_horizontalSlider_valueChanged(int value)
{
    bool succes = alson->setBrightness(value);
    qDebug() << "亮度是否设置成功:" << (succes ? "成功" : "失败");
    qDebug() << "当前亮度值:" << alson->getBrightness();

    alson->setSaveRetain("light",value);
}

void Widget::Light()
{
    qDebug() << "当前亮度值:" << alson->getBrightness();
    qDebug() << "最大亮度值:" << alson->getMaxBrightness();
}

void Widget::RTC()
{
    //bool succes = alson->setRTC("2026-06-29 11:58:00");
    //qDebug() << "时间是否设置成功:" << (succes ? "成功" : "失败");
    qDebug() << "当前系统时间:" << alson->getRTC();
    qDebug() << "";
}

void Widget::RetainData()
{
    QVariant cout = alson->getReadRetain("cout", 0);
    qDebug() << "Reatin文件读取1:" << cout;

    auto v1 = alson->getReadRetain("testInt", 0);
    auto v2 = alson->getReadRetain("testFloat", 0.00);
    auto v3 = alson->getReadRetain("testQString", QString("hello"));

    auto c1 = alson->getReadRetain("color", QColor(255, 0, 0));
    auto c2 = alson->getReadRetain("point", QPoint(100, 0));
    auto c3 = alson->getReadRetain("time", QDateTime::currentDateTime());

    qDebug() << "Reatin文件读取2:" << v1 << v2 << v3 << c1 << c2 << c3;

    if(cout != 100){
        alson->setSaveRetain("cout", 100);

        alson->setSaveRetain("testInt", 200);
        alson->setSaveRetain("testFloat", 88.88);
        alson->setSaveRetain("testQString", QString("word"));

        alson->setSaveRetain("color", QColor(255, 255, 0));
        alson->setSaveRetain("point", QPoint(100, 200));
        alson->setSaveRetain("time", QDateTime::currentDateTime());

        qDebug() << "Reatin文件保存...";
    }

    QVariant v4 = alson->getReadRetain("testInt").toInt();
    QVariant v5 = alson->getReadRetain("testFloat").toFloat();
    QVariant v6 = alson->getReadRetain("testQString");

    QVariant c4 = alson->getReadRetain("color").value<QColor>();
    QVariant c5 = alson->getReadRetain("point").toPoint();
    QVariant c6 = alson->getReadRetain("time").toString();

    qDebug() << "Reatin文件读取3:" << alson->getReadRetain("cout") << v4 << v5 << v6 << c4 << c5 << c6;
    qDebug() << "";
}



