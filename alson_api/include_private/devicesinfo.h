#ifndef DEVICESINFO_H
#define DEVICESINFO_H

#include <QObject>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QDir>

class DevicesInfo : public QObject
{
    Q_OBJECT
public:
    explicit DevicesInfo(QObject *parent = nullptr);

    static QString readDeviceInfo(const QString &type);

private:
    static QString readFileContent();                               // 板卡型号
    static QString readCpuCoreCount();                              // CPU核心数
    static QString executeShellCommand(const QString &command);     // CPU架构
    static QString readCpuFrequency();                              // CPU频率
    static QString readMemorySize();                                // DDR容量
    static QString readEmmcSize();                                  // eMMC容量
    static QString readOsVersion();                                 // 系统版本
    static QString readSerialNumber();                              // 读取序列号
    static QString readMacAddress();                                // 读取 MAC 地址
    static QString readNetworkStatus();                             // 读取网络状态
    static QString readCpuUsage();                                  // CPU负载率

};

#endif // DEVICESINFO_H
