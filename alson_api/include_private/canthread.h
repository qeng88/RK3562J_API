#ifndef CANTHREAD_H
#define CANTHREAD_H

#include <QObject>
#include <QCanBusDevice>
#include <QCanBusFrame>
#include <QTimer>
#include <QList>
#include <QByteArray>
#include <QElapsedTimer>
#include <QMap>
#include <atomic>
#include <QProcess>

#include "alson_api_global.h"

/**
 * @brief 周期性CAN帧结构体
 * 
 * 用于存储需要周期性发送的CAN帧配置信息
 */
struct PeriodicCanFrame
{
    uint id = 0;                // CAN帧ID（标准帧或扩展帧）
    QByteArray data;            // 帧数据（最大8字节）
    bool isExtended = false;    // 是否为扩展帧（true:29位扩展帧, false:11位标准帧）
    int periodMs = 100;         // 发送周期（毫秒）
    qint64 nextDueMs = 0;       // 下一次发送的到期时间（毫秒时间戳）
};

/**
 * @brief CAN设备信息结构体
 * 
 * 存储每个CAN接口的完整状态和配置信息
 */
struct CanDeviceInfo
{
    QString interface;              // CAN接口名称（如 "can0"）
    quint32 bitrate = 500000;       // 波特率（默认500kbps）
    QCanBusDevice *device = nullptr; // Qt CAN总线设备对象指针
    bool isConnected = false;        // 连接状态标志
    QList<PeriodicCanFrame> periodicFrames; // 周期性发送帧列表
    QElapsedTimer elapsedTimer;      // 高精度计时器（用于周期性发送调度）
};

/**
 * @brief CAN总线工作线程类
 * 
 * 继承自QObject，通常移动到独立的QThread中执行
 * 负责实际CAN设备的打开/关闭、帧收发、周期性帧调度等底层操作
 */
class ALSON_API_EXPORT CanThread : public QObject
{
    Q_OBJECT

public:
    explicit CanThread(QObject *parent = nullptr);
    ~CanThread();

signals:
    // 接收到CAN帧时发出的信号
    void frameReceived(const QString &interface, uint id, const QByteArray &data, bool isExtended);
    // 发生错误时发出的信号
    void errorOccurred(const QString &interface, const QString &error);
    // 成功连接设备时发出的信号
    void connected(const QString &interface);
    // 设备断开连接时发出的信号
    void disconnected(const QString &interface);
    // 帧发送完成时发出的信号（成功或失败）
    void frameSent(const QString &interface, bool success, uint id);

public slots:
    // 启动指定CAN接口
    void doStart(const QString &interface, quint32 bitrate);
    // 停止指定CAN接口
    void doStop(const QString &interface);
    // 停止所有CAN接口
    void doStopAll();

    // 立即发送一帧CAN数据
    void sendFrameNow(const QString &interface, uint id, const QByteArray &data, bool isExtended = false);
    // 添加周期性发送的CAN帧
    void addPeriodicFrame(const QString &interface, uint id, const QByteArray &data,
                          bool isExtended, int periodMs, int offsetMs = 0);
    // 更新已存在的周期性帧的数据内容（ID保持不变）
    void updatePeriodicFrameData(const QString &interface, uint id, const QByteArray &data, bool isExtended = false);
    // 移除指定的周期性帧
    void removePeriodicFrame(const QString &interface, uint id, bool isExtended = false);
    // 清除指定接口的所有周期性帧
    void clearPeriodicFrames(const QString &interface);

private slots:
    // 处理接收到的CAN帧（由QCanBusDevice的framesReceived信号触发）
    void onFramesReceived();
    // 处理CAN总线错误
    void onErrorOccurred(QCanBusDevice::CanBusError error);
    // 周期性发送调度器（由定时器触发，检查并发送到期的周期性帧）
    void processTxSchedule();

private:
    // 设置并打开CAN设备
    bool setupDevice(const QString &interface, quint32 bitrate);
    // 清理并关闭CAN设备
    void cleanupDevice(const QString &interface);
    // 实际写入CAN帧到总线
    bool writeCanFrame(CanDeviceInfo *deviceInfo, const QCanBusFrame &frame);
    // 在设备信息中查找指定ID的周期性帧索引
    int findPeriodicFrame(const CanDeviceInfo *deviceInfo, uint id, bool isExtended) const;
    // 通过设备对象指针查找对应的设备信息
    CanDeviceInfo *findDeviceInfoByDevice(QObject *device) const;

    QTimer *m_scheduleTimer = nullptr;                       // 周期性发送调度定时器
    QMap<QString, CanDeviceInfo*> m_devices;                 // 接口名 -> 设备信息映射表
    std::atomic_bool m_isRunning { false };                  // 线程运行标志（原子操作，线程安全）
};

#endif // CANTHREAD_H
