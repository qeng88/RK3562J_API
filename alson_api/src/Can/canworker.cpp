#include "canworker.h"
#include <QMetaObject>
#include <QDebug>
#include <errno.h>
#include <string.h>
#include <QLibrary>
#include <linux/types.h>


CanWorker::CanWorker(QObject *parent)
    : QThread(parent)
{
}

CanWorker::~CanWorker()
{
    stopAllCan();
}

bool CanWorker::configureSocketCan(const QString &interface, quint32 bitrate)
{
    const QByteArray ifname = interface.toLocal8Bit();
    const char *dev = ifname.constData();

    if (can_do_stop(dev) < 0) {
        qWarning() << "CAN down 失败，可能接口本来就是 down:"
                   << interface << strerror(errno);
    }

    if (can_set_bitrate(dev, bitrate) < 0) {
        const QString err = QString::fromLocal8Bit(strerror(errno));
        qCritical() << "设置 CAN 波特率失败:" << interface << bitrate << err;
        emit errorOccurred(interface, QString("设置 CAN 波特率失败: %1").arg(err));
        return false;
    }

    if (can_do_start(dev) < 0) {
        const QString err = QString::fromLocal8Bit(strerror(errno));
        qCritical() << "CAN up 失败:" << interface << err;
        emit errorOccurred(interface, QString("CAN up 失败: %1").arg(err));
        return false;
    }

    qInfo() << "libsocketcan 配置" << interface << "波特率" << bitrate << "成功";
    return true;
}


bool CanWorker::startCan(const QString &interface, quint32 bitrate)
{
    if (interface.trimmed().isEmpty()) {
        emit errorOccurred(interface, "CAN接口名为空");
        return false;
    }

    {
        QMutexLocker locker(&m_mutex);
        if (m_startedInterfaces.contains(interface)) {
            emit errorOccurred(interface, "接口已在运行");
            return false;
        }
    }

    if (!configureSocketCan(interface, bitrate))
        return false;

    {
        QMutexLocker locker(&m_mutex);
        m_startedInterfaces.insert(interface);
    }

    if (!QThread::isRunning()) {
        {
            QMutexLocker locker(&m_mutex);
            m_pendingInterfaces.insert(interface, bitrate);
        }
        start();
        return true;
    }

    emit requestStart(interface, bitrate);
    return true;
}

void CanWorker::stopCan(const QString &interface)
{
    {
        QMutexLocker locker(&m_mutex);
        m_pendingInterfaces.remove(interface);
        m_startedInterfaces.remove(interface);
    }

    if (!QThread::isRunning())
        return;

    emit requestStop(interface);
}

void CanWorker::stopAllCan()
{
    {
        QMutexLocker locker(&m_mutex);
        m_pendingInterfaces.clear();
        m_startedInterfaces.clear();
    }

    if (!QThread::isRunning())
        return;

    emit requestStopAll();

    quit();

    if (!wait(3000)) {
        qWarning() << "CAN线程退出超时";
    }
}

void CanWorker::sendFrame(const QString &interface, uint id, const QByteArray &data, bool isExtended)
{
    if (!QThread::isRunning()) {
        emit errorOccurred(interface, "CAN线程未运行，无法发送");
        return;
    }

    emit requestSendFrame(interface, id, data, isExtended);
}

void CanWorker::addPeriodicFrame(const QString &interface, uint id, const QByteArray &data,
                                 bool isExtended, int periodMs, int offsetMs)
{
    if (!QThread::isRunning()) {
        emit errorOccurred(interface, "CAN线程未运行，无法添加周期帧");
        return;
    }

    emit requestAddPeriodicFrame(interface, id, data, isExtended, periodMs, offsetMs);
}

void CanWorker::updatePeriodicFrameData(const QString &interface, uint id,
                                        const QByteArray &data, bool isExtended)
{
    if (!QThread::isRunning()) {
        emit errorOccurred(interface, "CAN线程未运行，无法更新周期帧");
        return;
    }

    emit requestUpdatePeriodicFrameData(interface, id, data, isExtended);
}

void CanWorker::removePeriodicFrame(const QString &interface, uint id, bool isExtended)
{
    if (!QThread::isRunning()) {
        emit errorOccurred(interface, "CAN线程未运行，无法删除周期帧");
        return;
    }

    emit requestRemovePeriodicFrame(interface, id, isExtended);
}

void CanWorker::clearPeriodicFrames(const QString &interface)
{
    if (!QThread::isRunning()) {
        emit errorOccurred(interface, "CAN线程未运行，无法清空周期帧");
        return;
    }

    emit requestClearPeriodicFrames(interface);
}

void CanWorker::run()
{
    //qDebug() << "CAN线程启动，线程ID:" << QThread::currentThreadId();

    CanThread canThread;

    connect(&canThread, &CanThread::frameReceived,
            this, &CanWorker::frameReceived, Qt::QueuedConnection);
    connect(&canThread, &CanThread::errorOccurred,
            this, &CanWorker::errorOccurred, Qt::QueuedConnection);
    connect(&canThread, &CanThread::connected,
            this, &CanWorker::connected, Qt::QueuedConnection);
    connect(&canThread, &CanThread::disconnected,
            this, &CanWorker::disconnected, Qt::QueuedConnection);
    connect(&canThread, &CanThread::frameSent,
            this, &CanWorker::frameSent, Qt::QueuedConnection);

    connect(this, &CanWorker::requestStart,
            &canThread, &CanThread::doStart, Qt::QueuedConnection);
    connect(this, &CanWorker::requestStop,
            &canThread, &CanThread::doStop, Qt::QueuedConnection);
    connect(this, &CanWorker::requestStopAll,
            &canThread, &CanThread::doStopAll, Qt::QueuedConnection);
    connect(this, &CanWorker::requestSendFrame,
            &canThread, &CanThread::sendFrameNow, Qt::QueuedConnection);
    connect(this, &CanWorker::requestAddPeriodicFrame,
            &canThread, &CanThread::addPeriodicFrame, Qt::QueuedConnection);
    connect(this, &CanWorker::requestUpdatePeriodicFrameData,
            &canThread, &CanThread::updatePeriodicFrameData, Qt::QueuedConnection);
    connect(this, &CanWorker::requestRemovePeriodicFrame,
            &canThread, &CanThread::removePeriodicFrame, Qt::QueuedConnection);
    connect(this, &CanWorker::requestClearPeriodicFrames,
            &canThread, &CanThread::clearPeriodicFrames, Qt::QueuedConnection);

    QMap<QString, quint32> pending;
    {
        QMutexLocker locker(&m_mutex);
        pending = m_pendingInterfaces;
    }

    for (auto it = pending.constBegin(); it != pending.constEnd(); ++it) {
        emit requestStart(it.key(), it.value());
    }

    exec();

    canThread.doStopAll();

    qDebug() << "CAN线程退出";
}
