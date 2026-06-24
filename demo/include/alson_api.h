#ifndef ALSON_API_H
#define ALSON_API_H

#include <QObject>

#if defined(ALSON_API_LIBRARY)
#  define ALSON_API_EXPORT Q_DECL_EXPORT
#else
#  define ALSON_API_EXPORT Q_DECL_IMPORT
#endif

class Alson_apiPrivate;

class ALSON_API_EXPORT Alson_api : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     */
    static Alson_api* instance();

    /**
     * @brief 释放单例实例
     */
    static void destroy();

    // CAN
    /**
     * @brief 启动CAN接口
     * @param interface CAN接口名称（如 "can0"）
     * @param bitrate 波特率（默认500000）
     * @return 成功返回true
     */
    bool startCan(const QString &interface, quint32 bitrate = 500000);

    /**
     * @brief 停止CAN接口
     * @param interface CAN接口名称
     */
    void stopCan(const QString &interface);

    /**
     * @brief 停止所有CAN接口
     */
    void stopAllCan();

    /**
     * @brief 发送CAN帧
     * @param interface CAN接口名称
     * @param id 帧ID
     * @param data 数据（最多8字节）
     * @param isExtended 是否为扩展帧
     */
    void sendFrame(const QString &interface, uint id, const QByteArray &data, bool isExtended = false);

    /**
     * @brief 添加周期性发送帧
     * @param interface CAN接口名称
     * @param id 帧ID
     * @param data 数据
     * @param isExtended 是否为扩展帧
     * @param periodMs 发送周期（毫秒）
     * @param offsetMs 初始偏移（毫秒）
     */
    void addPeriodicFrame(const QString &interface, uint id, const QByteArray &data, bool isExtended, int periodMs, int offsetMs = 0);

    /**
     * @brief 更新周期性帧数据
     * @param interface CAN接口名称
     * @param id 帧ID
     * @param data 新数据
     * @param isExtended 是否为扩展帧
     */
    void updatePeriodicFrameData(const QString &interface, uint id, const QByteArray &data, bool isExtended = false);

    /**
     * @brief 移除周期性帧
     * @param interface CAN接口名称
     * @param id 帧ID
     * @param isExtended 是否为扩展帧
     */
    void removePeriodicFrame(const QString &interface, uint id, bool isExtended = false);

    /**
     * @brief 清除所有周期性帧
     * @param interface CAN接口名称
     */
    void clearPeriodicFrames(const QString &interface);

    //系统信息
    /**
     * @type 板卡型号 CPU核心数 CPU架构 CPU频率 DDR容量 eMMC容量 系统版本 设备序列号 MAC地址 网络节点状态 CPU负载率
     */
    QString readDeviceInfo(const QString &type);

    /**
     * @return 库版本
     */
    QString readApiVersion();

    //亮度功能
    /**
     * @brief 亮度设置
     * @return 成功返回true
     */
    bool setBrightness(int value);
    /**
     * @return 返回当前亮度值
     */
    int getBrightness() const;

    /**
     * @return 返回最大可设亮度值
     */
    int getMaxBrightness() const;

signals:
    // CAN
    void frameReceived(const QString &interface, uint id,const QByteArray &data, bool isExtended); // 接收到CAN帧信号
    void errorOccurred(const QString &interface, const QString &error); // 错误发生信号
    void connected(const QString &interface); // 设备连接信号
    void disconnected(const QString &interface); // 设备断开信号
    void frameSent(const QString &interface, bool success, uint id); // 帧发送完成信号

private:
    explicit Alson_api(QObject *parent = nullptr);
    ~Alson_api() override;

    static Alson_api *m_instance;
    Alson_apiPrivate *d = nullptr;
};

#endif // ALSON_API_H
