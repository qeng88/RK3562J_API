#ifndef DEVICESINFO_H
#define DEVICESINFO_H

#include <QObject>

class DevicesInfo : public QObject
{
    Q_OBJECT
public:
    explicit DevicesInfo(QObject *parent = nullptr);

    static QString readDeviceInfo(const QString &type);

private:
    static QString readFileContent();                               // 芯片型号
    static QString readCpuCoreCount();                              // CPU核心数
    static QString executeShellCommand(const QString &command);     // CPU架构
    static QString readCpuFrequency();                              // CPU频率
    static QString readMemorySize();                                // 内存容量
    static QString readEmmcSize();                                  // 闪存容量
    static QString readOsVersion();                                 // 系统版本
    static QString readSerialNumber();                              // 读取序列号
    static QString readMacAddress();                                // 读取 MAC 地址
    static QString readNetworkStatus();                             // 读取网络状态
    static QString readCpuUsage();                                  // CPU负载率

    static QString readOsReleaseField(const QString &fieldName);    // 料号 驱动版本 构建时间
    static QString readScreenResolution();                          // 屏幕分辨率
    static QString readAvailableSpace();                            // 可用空间
    static QString readBspVersion();                                // BSP版本
    static QString readCanCount();                                  // CAN数量
    static QString readCanStatus();                                 // CAN状态
    static QString readCpuTemperature();                            // CPU温度

};

#endif // DEVICESINFO_H
