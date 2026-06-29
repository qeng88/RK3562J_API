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
     * @type 板卡型号 CPU核心数 CPU架构 CPU频率 内存容量 闪存容量 系统版本 MAC地址 网络节点状态 CPU负载率
     * @type 料号 驱动版本 构建时间 屏幕分辨率 可用空间 BSP版本 CAN数量 CAN状态 CPU温度
     */
    QString getDeviceInfo(const QString &type);

    /**
     * @return 库版本
     */
    QString getApiVersion();

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

    //时间设置
    /**
     * @brief 时间设置
     * @param 固定格式要求 "yyyy-MM-dd HH:mm:ss"  例: "2020-06-01 08:30:00"
     * @return 成功返回true
     */
    bool setRTC(const QString &datetime);
    /**
     * @return 返回系统时间
     */
    QString getRTC() const;

    //Retain 存储功能
    /**
     * @brief 数据存储
     */
    void setSaveRetain(const QString &key, const QVariant &value);

    /**
     * @brief 读取保存的数据
     * @return QVariant
     */
    QVariant getReadRetain(const QString &key) const;
    QVariant getReadRetain(const QString &key, const QVariant &defaultValue) const;
    /**
     * @brief 清除所有数据
     */
    void clearRetain();

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

    Alson_apiPrivate *d = nullptr;
};

#endif // ALSON_API_H
