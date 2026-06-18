#include "devicesinfo.h"

DevicesInfo::DevicesInfo(QObject *parent) : QObject(parent)
{
    readCpuUsage();
}

QString DevicesInfo::readDeviceInfo(const QString &type)
{
    if (type == "板卡型号") {
        return readFileContent();
    } else if (type == "CPU核心数") {
        return readCpuCoreCount();
    } else if (type == "CPU架构") {
        return executeShellCommand("uname -m");
    } else if (type == "CPU频率") {
        return readCpuFrequency();
    } else if (type == "DDR容量") {
        return readMemorySize();
    } else if (type == "eMMC容量") {
        return readEmmcSize();
    } else if (type == "系统版本") {
        return readOsVersion();
    } else if (type == "设备序列号") {
        return readSerialNumber();
    } else if (type == "MAC地址") {
        return readMacAddress();
    } else if (type == "网络节点状态") {
        return readNetworkStatus();
    } else if (type == "CPU负载率") {
        return readCpuUsage();
    }
    return QString("unknown");
}

QString DevicesInfo::readFileContent()
{
    QFile file("/proc/device-tree/model");
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QByteArray data = file.readAll();
    file.close();

    QString result = QString::fromUtf8(data);
    result.remove(QChar('\0'));
    return result.trimmed();
}

QString DevicesInfo::readCpuCoreCount()
{
    QFile file("/proc/cpuinfo");
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QByteArray data = file.readAll();
    file.close();

    QString content = QString::fromUtf8(data);
    int coreCount = content.count("processor");
    return QString::number(coreCount);
}

QString DevicesInfo::executeShellCommand(const QString &command)
{
    QProcess process;
    process.start("sh", {"-c", command});

    if (!process.waitForFinished(3000)) {
        return QString();
    }

    if (process.exitCode() != 0) {
        return QString();
    }

    return process.readAllStandardOutput().trimmed();
}

QString DevicesInfo::readCpuFrequency()
{
    QFile file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QString freq = file.readAll().trimmed();
    file.close();

    bool ok;
    int freqValue = freq.toInt(&ok);
    if (ok && freqValue > 0) {
        return QString::number(freqValue / 1000) + " MHz";
    }
    return QString();
}

QString DevicesInfo::readMemorySize()
{
    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QString content = file.readAll();
    file.close();

    // 查找 MemTotal 行
    QRegularExpression re("^MemTotal:\\s+(\\d+) kB$", QRegularExpression::MultilineOption);
    QRegularExpressionMatch match = re.match(content);

    if (match.hasMatch()) {
        int memKB = match.captured(1).toInt();
        int memMB = memKB / 1024;
        return QString::number(memMB) + " MB";
    }
    return QString();
}

QString DevicesInfo::readEmmcSize()
{
    QProcess process;
    process.start("sh", {"-c", "for d in /sys/block/mmcblk[0-9]*; do [ \"$(cat $d/device/type 2>/dev/null)\" = \"MMC\" ] && awk '{printf \"%.0f GB\\n\", $1*512/1024/1024/1024}' $d/size; done"});

    if (!process.waitForFinished(3000)) {
        return QString();
    }

    return process.readAllStandardOutput().trimmed();
}

QString DevicesInfo::readOsVersion()
{
    QFile file("/etc/os-release");
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QString content = file.readAll();
    file.close();

    // 查找 PRETTY_NAME
    QRegularExpression re("^PRETTY_NAME=\"?([^\"]*)\"?$", QRegularExpression::MultilineOption);
    QRegularExpressionMatch match = re.match(content);

    if (match.hasMatch()) {
        return match.captured(1).trimmed();
    }
    return QString();
}

QString DevicesInfo::readSerialNumber()
{
    QFile file("/sys/class/dmi/id/product_serial");
    if (file.open(QIODevice::ReadOnly)) {
        QString serial = file.readAll().trimmed();
        file.close();
        if (!serial.isEmpty()) {
            return serial;
        }
    }

    // 备选：从 cpuinfo 读取
    QFile cpuFile("/proc/cpuinfo");
    if (!cpuFile.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QString content = cpuFile.readAll();
    cpuFile.close();

    QRegularExpression re("^Serial\\s*:\\s*(.+)$", QRegularExpression::MultilineOption);
    QRegularExpressionMatch match = re.match(content);

    if (match.hasMatch()) {
        return match.captured(1).trimmed();
    }
    return QString();
}

QString DevicesInfo::readMacAddress()
{
    QDir dir("/sys/class/net");
    QStringList interfaces = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &iface : interfaces) {
        if (iface == "lo") continue;

        QFile addrFile(dir.absolutePath() + "/" + iface + "/address");
        if (addrFile.open(QIODevice::ReadOnly)) {
            QString mac = addrFile.readAll().trimmed();
            addrFile.close();
            if (!mac.isEmpty() && mac != "00:00:00:00:00:00") {
                return mac;
            }
        }
    }
    return QString();
}

QString DevicesInfo::readNetworkStatus()
{
    QDir dir("/sys/class/net");
    QStringList interfaces = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    QStringList results;
    for (const QString &iface : interfaces) {
        QFile stateFile(dir.absolutePath() + "/" + iface + "/operstate");
        QString state = "unknown";
        if (stateFile.open(QIODevice::ReadOnly)) {
            state = stateFile.readAll().trimmed();
            stateFile.close();
        }
        results << iface + ": " + state;
    }
    return results.join("\n");
}

QString DevicesInfo::readCpuUsage()
{
    static qlonglong lastTotal = 0;
    static qlonglong lastIdle = 0;
    static bool firstRun = true;

    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QString line = file.readLine();
    file.close();

    if (!line.startsWith("cpu ")) {
        return QString();
    }

    QStringList parts = line.mid(4).simplified().split(' ');
    if (parts.size() < 8) {
        return QString();
    }

    // 提取各项时间
    qlonglong user = parts[0].toLongLong();
    qlonglong nice = parts[1].toLongLong();
    qlonglong system = parts[2].toLongLong();
    qlonglong idle = parts[3].toLongLong();
    qlonglong iowait = parts[4].toLongLong();
    qlonglong irq = parts[5].toLongLong();
    qlonglong softirq = parts[6].toLongLong();
    qlonglong steal = parts[7].toLongLong();

    // 计算总时间和空闲时间
    qlonglong total = user + nice + system + idle + iowait + irq + softirq + steal;
    qlonglong idleTime = idle + iowait;

    // 第一次运行，只记录数据不计算
    if (firstRun) {
        lastTotal = total;
        lastIdle = idleTime;
        firstRun = false;
        return QString(); // 第一次返回空，下次调用才有值
    }

    // 计算差值
    qlonglong totalDiff = total - lastTotal;
    qlonglong idleDiff = idleTime - lastIdle;

    // 更新上次的值
    lastTotal = total;
    lastIdle = idleTime;

    // 计算使用率
    if (totalDiff <= 0) {
        return QString();
    }

    double usage = 100.0 * (totalDiff - idleDiff) / totalDiff;

    // 格式化输出，保留一位小数
    return QString::number(usage, 'f', 1) + " %";
}

