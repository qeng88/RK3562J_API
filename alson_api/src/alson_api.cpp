#include "alson_api.h"
#include "canworker.h"
#include "devicesinfo.h"
#include "brightness.h"
#include "rtc.h"
#include "retain.h"

class Alson_apiPrivate
{
public:
    CanWorker *m_worker = nullptr;
    DevicesInfo *m_info = nullptr;
    Brightness *m_bright = nullptr;
    RTC *m_rtc = nullptr;
    Retain *m_retain = nullptr;
};

// 静态成员初始化
Alson_api* Alson_api::instance()
{
    static Alson_api instance;
    return &instance;
}

Alson_api::Alson_api(QObject *parent)
    : QObject(parent)
    , d(new Alson_apiPrivate)
{
    d->m_worker = new CanWorker(this);
    d->m_info = new DevicesInfo(this);
    d->m_bright = new Brightness(this);
    d->m_rtc = new RTC(this);
    d->m_retain = new Retain(this);

    // 转发信号
    connect(d->m_worker, &CanWorker::frameReceived, this, &Alson_api::frameReceived);
    connect(d->m_worker, &CanWorker::errorOccurred, this, &Alson_api::errorOccurred);
    connect(d->m_worker, &CanWorker::connected, this, &Alson_api::connected);
    connect(d->m_worker, &CanWorker::disconnected, this, &Alson_api::disconnected);
    connect(d->m_worker, &CanWorker::frameSent, this, &Alson_api::frameSent);
}

Alson_api::~Alson_api()
{
    delete d;
}

/**
 *@brief CAN
**/
bool Alson_api::startCan(const QString &interface, quint32 bitrate)
{
    return d->m_worker->startCan(interface, bitrate);
}

void Alson_api::stopCan(const QString &interface)
{
    d->m_worker->stopCan(interface);
}

void Alson_api::stopAllCan()
{
    d->m_worker->stopAllCan();
}

void Alson_api::sendFrame(const QString &interface, uint id,
                          const QByteArray &data, bool isExtended)
{
    d->m_worker->sendFrame(interface, id, data, isExtended);
}

void Alson_api::addPeriodicFrame(const QString &interface, uint id,
                                 const QByteArray &data, bool isExtended,
                                 int periodMs, int offsetMs)
{
    d->m_worker->addPeriodicFrame(interface, id, data, isExtended, periodMs, offsetMs);
}

void Alson_api::updatePeriodicFrameData(const QString &interface, uint id,
                                        const QByteArray &data, bool isExtended)
{
    d->m_worker->updatePeriodicFrameData(interface, id, data, isExtended);
}

void Alson_api::removePeriodicFrame(const QString &interface, uint id, bool isExtended)
{
    d->m_worker->removePeriodicFrame(interface, id, isExtended);
}

void Alson_api::clearPeriodicFrames(const QString &interface)
{
    d->m_worker->clearPeriodicFrames(interface);
}


/**
 *@brief 系统信息
**/
QString Alson_api::getDeviceInfo(const QString &type)
{
    return d->m_info->readDeviceInfo(type);
}

QString Alson_api::getApiVersion()
{
    return "Alson_api 1.0.0";
}


/**
 *@brief 亮度功能
**/
bool Alson_api::setBrightness(int value)
{
    return  d->m_bright->writeBrightness(value);
}

int Alson_api::getBrightness() const
{
    return d->m_bright->readBrightness();
}

int Alson_api::getMaxBrightness() const
{
    return d->m_bright->readMaxBrightness();
}

/**
 *@brief RTC
**/
bool Alson_api::setRTC(const QString &datetime)
{
    return d->m_rtc->writeDateTime(datetime);
}

QString Alson_api::getRTC() const
{
    return d->m_rtc->readDateTime();
}

/**
 *@brief Retain
**/
void Alson_api::setSaveRetain(const QString &key, const QVariant &value)
{
    d->m_retain->setSaveRetain(key,value);
}

QVariant Alson_api::getReadRetain(const QString &key) const
{
    return  d->m_retain->getReadRetain(key);
}

QVariant Alson_api::getReadRetain(const QString &key, const QVariant &defaultValue) const
{
    return  d->m_retain->getReadRetain(key,defaultValue);
}

void Alson_api::clearRetain()
{
    d->m_retain->clear();
}
