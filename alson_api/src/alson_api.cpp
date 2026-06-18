#include "alson_api.h"
#include "canworker.h"
#include "devicesinfo.h"

class Alson_apiPrivate
{
public:
    CanWorker *m_worker = nullptr;
    DevicesInfo *m_info = nullptr;
};

// 静态成员初始化
Alson_api* Alson_api::m_instance = nullptr;

Alson_api* Alson_api::instance()
{
    if (!m_instance) {
        m_instance = new Alson_api();
    }
    return m_instance;
}

void Alson_api::destroy()
{
    delete m_instance;
    m_instance = nullptr;
}

Alson_api::Alson_api(QObject *parent)
    : QObject(parent)
    , d(new Alson_apiPrivate)
{
    d->m_worker = new CanWorker(this);
    d->m_info = new DevicesInfo(this);

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


QString Alson_api::readDeviceInfo(const QString &type)
{
    return d->m_info->readDeviceInfo(type);
}

QString Alson_api::readApiVersion()
{
    return "Alson_api 1.0.0";
}
