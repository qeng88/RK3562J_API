#include "canthread.h"
#include <QCanBus>
#include <QDebug>
#include <QThread>

CanThread::CanThread(QObject *parent)
    : QObject(parent)
    , m_scheduleTimer(new QTimer(this))
{
    m_scheduleTimer->setInterval(10);
    m_scheduleTimer->setTimerType(Qt::PreciseTimer);
    connect(m_scheduleTimer, &QTimer::timeout,
            this, &CanThread::processTxSchedule);
}

CanThread::~CanThread()
{
    doStopAll();
}

void CanThread::doStart(const QString &interface, quint32 bitrate)
{
    if (interface.trimmed().isEmpty()) {
        emit errorOccurred(interface, "CAN接口名为空");
        return;
    }

    if (m_devices.contains(interface)) {
        emit errorOccurred(interface, "接口已在运行");
        return;
    }

    if (!setupDevice(interface, bitrate)) {
        emit errorOccurred(interface, "设备初始化失败");
        return;
    }

    m_isRunning = true;
    if (!m_scheduleTimer->isActive())
        m_scheduleTimer->start();

    //qDebug() << "CAN接口启动成功:" << interface
    //         << "线程ID:" << QThread::currentThreadId();
}

void CanThread::doStop(const QString &interface)
{
    if (!m_devices.contains(interface))
        return;

    cleanupDevice(interface);

    if (m_devices.isEmpty()) {
        m_isRunning = false;
        if (m_scheduleTimer)
            m_scheduleTimer->stop();
    }
}

void CanThread::doStopAll()
{
    const QStringList interfaces = m_devices.keys();
    for (const QString &interface : interfaces)
        cleanupDevice(interface);

    m_isRunning = false;
    if (m_scheduleTimer)
        m_scheduleTimer->stop();
}

bool CanThread::setupDevice(const QString &interface, quint32 bitrate)
{
    QString errorString;
    QCanBusDevice *device = QCanBus::instance()->createDevice(
        QStringLiteral("socketcan"), interface, &errorString);

    if (!device) {
        emit errorOccurred(interface, QString("创建设备失败: %1").arg(errorString));
        return false;
    }

    device->setConfigurationParameter(QCanBusDevice::BitRateKey, QVariant());

    CanDeviceInfo *info = new CanDeviceInfo;
    info->interface = interface;
    info->device = device;
    info->bitrate = bitrate;
    info->isConnected = false;
    info->elapsedTimer.start();

    connect(device, &QCanBusDevice::framesReceived,
            this, &CanThread::onFramesReceived);
    connect(device, &QCanBusDevice::errorOccurred,
            this, &CanThread::onErrorOccurred);

    if (!device->connectDevice()) {
        emit errorOccurred(interface, QString("连接设备失败: %1").arg(device->errorString()));
        delete device;
        delete info;
        return false;
    }

    info->isConnected = true;
    m_devices.insert(interface, info);

    emit connected(interface);
    qDebug() << "CAN设备已连接:" << interface << "状态:" << device->state();
    return true;
}

void CanThread::cleanupDevice(const QString &interface)
{
    CanDeviceInfo *info = m_devices.take(interface);
    if (!info)
        return;

    if (info->device) {
        if (info->device->state() == QCanBusDevice::ConnectedState) {
            info->device->disconnectDevice();
            qDebug() << "CAN设备已断开:" << interface;
        }
        delete info->device;
        info->device = nullptr;
    }

    info->isConnected = false;
    delete info;
    emit disconnected(interface);
}

void CanThread::sendFrameNow(const QString &interface, uint id, const QByteArray &data, bool isExtended)
{
    CanDeviceInfo *info = m_devices.value(interface, nullptr);
    if (!info || !info->isConnected) {
        emit errorOccurred(interface, "设备未连接，无法发送");
        emit frameSent(interface, false, id);
        return;
    }

    QCanBusFrame frame(id, data);
    frame.setExtendedFrameFormat(isExtended);
    writeCanFrame(info, frame);
}

void CanThread::addPeriodicFrame(const QString &interface, uint id, const QByteArray &data,
                                 bool isExtended, int periodMs, int offsetMs)
{
    if (periodMs <= 0) {
        emit errorOccurred(interface, "添加周期帧失败: periodMs 必须大于 0");
        return;
    }

    CanDeviceInfo *info = m_devices.value(interface, nullptr);
    if (!info || !info->isConnected) {
        emit errorOccurred(interface, "添加周期帧失败: 设备未连接");
        return;
    }

    if (!info->elapsedTimer.isValid())
        info->elapsedTimer.start();

    const qint64 now = info->elapsedTimer.elapsed();
    const qint64 nextDue = now + qMax(0, offsetMs);

    const int index = findPeriodicFrame(info, id, isExtended);
    if (index >= 0) {
        info->periodicFrames[index].data = data;
        info->periodicFrames[index].periodMs = periodMs;
        info->periodicFrames[index].nextDueMs = nextDue;
        return;
    }

    PeriodicCanFrame item;
    item.id = id;
    item.data = data;
    item.isExtended = isExtended;
    item.periodMs = periodMs;
    item.nextDueMs = nextDue;
    info->periodicFrames.append(item);
}

void CanThread::updatePeriodicFrameData(const QString &interface, uint id,
                                        const QByteArray &data, bool isExtended)
{
    CanDeviceInfo *info = m_devices.value(interface, nullptr);
    if (!info) {
        emit errorOccurred(interface, "更新周期帧失败: 设备未连接");
        return;
    }

    const int index = findPeriodicFrame(info, id, isExtended);
    if (index < 0) {
        emit errorOccurred(interface,
                           QString("更新周期帧失败: 未找到 ID 0x%1").arg(id, 0, 16));
        return;
    }

    info->periodicFrames[index].data = data;
}

void CanThread::removePeriodicFrame(const QString &interface, uint id, bool isExtended)
{
    CanDeviceInfo *info = m_devices.value(interface, nullptr);
    if (!info) {
        emit errorOccurred(interface, "删除周期帧失败: 设备未连接");
        return;
    }

    const int index = findPeriodicFrame(info, id, isExtended);
    if (index >= 0)
        info->periodicFrames.removeAt(index);
}

void CanThread::clearPeriodicFrames(const QString &interface)
{
    CanDeviceInfo *info = m_devices.value(interface, nullptr);
    if (!info) {
        emit errorOccurred(interface, "清空周期帧失败: 设备未连接");
        return;
    }

    info->periodicFrames.clear();
}

void CanThread::processTxSchedule()
{
    if (!m_isRunning)
        return;

    for (CanDeviceInfo *info : m_devices.values()) {
        if (!info || !info->isConnected || !info->device ||
            info->device->state() != QCanBusDevice::ConnectedState) {
            continue;
        }

        if (!info->elapsedTimer.isValid())
            info->elapsedTimer.start();

        const qint64 now = info->elapsedTimer.elapsed();
        for (PeriodicCanFrame &item : info->periodicFrames) {
            if (now < item.nextDueMs)
                continue;

            QCanBusFrame frame(item.id, item.data);
            frame.setExtendedFrameFormat(item.isExtended);
            writeCanFrame(info, frame);

            do {
                item.nextDueMs += item.periodMs;
            } while (item.nextDueMs <= now);
        }
    }
}

bool CanThread::writeCanFrame(CanDeviceInfo *info, const QCanBusFrame &frame)
{
    const QString interface = info ? info->interface : QString();

    if (!frame.isValid()) {
        emit errorOccurred(interface, "尝试发送无效帧");
        emit frameSent(interface, false, frame.frameId());
        return false;
    }

    if (!info || !info->device || info->device->state() != QCanBusDevice::ConnectedState) {
        emit errorOccurred(interface, "设备未连接，无法发送");
        emit frameSent(interface, false, frame.frameId());
        return false;
    }

    const bool success = info->device->writeFrame(frame);

    emit frameSent(interface, success, frame.frameId());

    if (!success){
        emit errorOccurred(interface, QString("发送失败: %1").arg(info->device->errorString()));
    }

    return success;
}

int CanThread::findPeriodicFrame(const CanDeviceInfo *info, uint id, bool isExtended) const
{
    if (!info)
        return -1;

    for (int i = 0; i < info->periodicFrames.size(); ++i) {
        const PeriodicCanFrame &item = info->periodicFrames.at(i);
        if (item.id == id && item.isExtended == isExtended)
            return i;
    }
    return -1;
}

CanDeviceInfo *CanThread::findDeviceInfoByDevice(QObject *device) const
{
    for (CanDeviceInfo *info : m_devices.values()) {
        if (info && info->device == device)
            return info;
    }
    return nullptr;
}

void CanThread::onFramesReceived()
{
    CanDeviceInfo *info = findDeviceInfoByDevice(sender());
    if (!info || !info->device || info->device->state() != QCanBusDevice::ConnectedState)
        return;

    while (info->device->framesAvailable()) {
        const QCanBusFrame frame = info->device->readFrame();

        if (!frame.isValid()) {
            qWarning() << "收到无效帧:" << info->interface;
            continue;
        }

        if (frame.frameType() == QCanBusFrame::ErrorFrame) {
            emit errorOccurred(info->interface,
                               QString("收到错误帧: %1")
                                   .arg(info->device->interpretErrorFrame(frame)));
            continue;
        }

        emit frameReceived(info->interface,
                           frame.frameId(),
                           frame.payload(),
                           frame.hasExtendedFrameFormat());
    }
}

void CanThread::onErrorOccurred(QCanBusDevice::CanBusError error)
{
    CanDeviceInfo *info = findDeviceInfoByDevice(sender());
    const QString interface = info ? info->interface : QString();
    QCanBusDevice *device = info ? info->device : nullptr;

    QString errorMsg;
    switch (error) {
    case QCanBusDevice::ReadError:
        errorMsg = "读取错误";
        break;
    case QCanBusDevice::WriteError:
        errorMsg = "写入错误";
        break;
    case QCanBusDevice::ConnectionError:
        errorMsg = "连接错误";
        break;
    case QCanBusDevice::ConfigurationError:
        errorMsg = "配置错误";
        break;
    case QCanBusDevice::NoError:
        return;
    case QCanBusDevice::UnknownError:
    default:
        errorMsg = "未知错误";
        break;
    }

    const QString fullError = QString("[%1] %2")
                                  .arg(errorMsg)
                                  .arg(device ? device->errorString() : QString());
    emit errorOccurred(interface, fullError);
    //qCritical() << "CAN错误:" << interface << fullError;
}
