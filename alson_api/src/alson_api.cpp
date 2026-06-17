#include "alson_api.h"
#include "canworker.h"  // 内部包含，不对外暴露

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
    , m_worker(new CanWorker(this))
{
    // 转发信号
    connect(m_worker, &CanWorker::frameReceived,
            this, &Alson_api::frameReceived);
    connect(m_worker, &CanWorker::errorOccurred,
            this, &Alson_api::errorOccurred);
    connect(m_worker, &CanWorker::connected,
            this, &Alson_api::connected);
    connect(m_worker, &CanWorker::disconnected,
            this, &Alson_api::disconnected);
    connect(m_worker, &CanWorker::frameSent,
            this, &Alson_api::frameSent);
}

Alson_api::~Alson_api()
{
    // m_worker 会随父对象自动销毁
}

bool Alson_api::startCan(const QString &interface, quint32 bitrate)
{
    return m_worker->startCan(interface, bitrate);
}

void Alson_api::stopCan(const QString &interface)
{
    m_worker->stopCan(interface);
}

void Alson_api::stopAllCan()
{
    m_worker->stopAllCan();
}

void Alson_api::sendFrame(const QString &interface, uint id,
                          const QByteArray &data, bool isExtended)
{
    m_worker->sendFrame(interface, id, data, isExtended);
}

void Alson_api::addPeriodicFrame(const QString &interface, uint id,
                                 const QByteArray &data, bool isExtended,
                                 int periodMs, int offsetMs)
{
    m_worker->addPeriodicFrame(interface, id, data, isExtended, periodMs, offsetMs);
}

void Alson_api::updatePeriodicFrameData(const QString &interface, uint id,
                                        const QByteArray &data, bool isExtended)
{
    m_worker->updatePeriodicFrameData(interface, id, data, isExtended);
}

void Alson_api::removePeriodicFrame(const QString &interface, uint id, bool isExtended)
{
    m_worker->removePeriodicFrame(interface, id, isExtended);
}

void Alson_api::clearPeriodicFrames(const QString &interface)
{
    m_worker->clearPeriodicFrames(interface);
}
