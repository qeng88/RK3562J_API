#ifndef CANWORKER_H
#define CANWORKER_H

#include <QThread>
#include <QMap>
#include <atomic>
#include <QMutex>
#include <QSet>
#include <libsocketcan.h>
#include "canthread.h"

class CanWorker : public QThread
{
    Q_OBJECT

public:
    explicit CanWorker(QObject *parent = nullptr);
    ~CanWorker();

    // 以下为公共API接口（在主线程调用，会通过信号槽转发到工作线程）
    
    // 启动CAN接口（参数：接口名，波特率）
    bool startCan(const QString &interface, quint32 bitrate = 500000);
    // 停止指定CAN接口
    void stopCan(const QString &interface);
    // 停止所有CAN接口
    void stopAllCan();

    // 立即发送一帧CAN数据
    void sendFrame(const QString &interface, uint id, const QByteArray &data, bool isExtended = false);
    // 添加周期性发送帧
    void addPeriodicFrame(const QString &interface, uint id, const QByteArray &data,
                          bool isExtended, int periodMs, int offsetMs = 0);
    // 更新周期性帧数据
    void updatePeriodicFrameData(const QString &interface, uint id, const QByteArray &data, bool isExtended = false);
    // 移除周期性帧
    void removePeriodicFrame(const QString &interface, uint id, bool isExtended = false);
    // 清除所有周期性帧
    void clearPeriodicFrames(const QString &interface);

signals:
    // 以下为向上层（主线程/UI）发出的信号
    
    // 接收到CAN帧
    void frameReceived(const QString &interface, uint id, const QByteArray &data, bool isExtended);
    // 发生错误
    void errorOccurred(const QString &interface, const QString &error);
    // 设备已连接
    void connected(const QString &interface);
    // 设备已断开
    void disconnected(const QString &interface);
    // 帧已发送
    void frameSent(const QString &interface, bool success, uint id);

    // 以下为内部请求信号（主线程 -> 工作线程）
    
    // 请求启动CAN接口
    void requestStart(const QString &interface, quint32 bitrate);
    // 请求停止CAN接口
    void requestStop(const QString &interface);
    // 请求停止所有CAN接口
    void requestStopAll();
    // 请求发送帧
    void requestSendFrame(const QString &interface, uint id, const QByteArray &data, bool isExtended);
    // 请求添加周期性帧
    void requestAddPeriodicFrame(const QString &interface, uint id, const QByteArray &data,
                                 bool isExtended, int periodMs, int offsetMs);
    // 请求更新周期性帧数据
    void requestUpdatePeriodicFrameData(const QString &interface, uint id,
                                        const QByteArray &data, bool isExtended);
    // 请求移除周期性帧
    void requestRemovePeriodicFrame(const QString &interface, uint id, bool isExtended);
    // 请求清除所有周期性帧
    void requestClearPeriodicFrames(const QString &interface);

protected:
    // 线程执行入口函数
    void run() override;

private:
    // 配置SocketCAN（Linux原生CAN接口）
    bool configureSocketCan(const QString &interface, quint32 bitrate);

    QMutex m_mutex;                                   // 互斥锁（保护共享数据）
    QMap<QString, quint32> m_pendingInterfaces;      // 待启动的接口列表（接口名 -> 波特率）
    QSet<QString> m_startedInterfaces;               // 已启动的接口集合
};

#endif // CANWORKER_H
