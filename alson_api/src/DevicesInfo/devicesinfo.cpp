#include "devicesinfo.h"
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QDir>
#include <QTextStream>

DevicesInfo::DevicesInfo(QObject *parent) : QObject(parent)
{
    readCpuUsage();
}

QString DevicesInfo::readDeviceInfo(const QString &type)
{
    if (type == "芯片型号") {
        return readFileContent();
    } else if (type == "CPU核心数") {
        return readCpuCoreCount();
    } else if (type == "CPU架构") {
        return executeShellCommand("uname -m");
    } else if (type == "CPU频率") {
        return readCpuFrequency();
    } else if (type == "内存容量") {
        return readMemorySize();
    } else if (type == "闪存容量") {
        return readEmmcSize();
    } else if (type == "系统版本") {
        return readOsVersion();
    //} else if (type == "设备序列号") {
        //return readSerialNumber();
    } else if (type == "MAC地址") {
        return readMacAddress();
    } else if (type == "网络节点状态") {
        return readNetworkStatus();
    } else if (type == "CPU负载率") {
        return readCpuUsage();

    } else if (type == "料号") {
        return readOsReleaseField("RK_PROJECT");
    } else if (type == "驱动版本") {
        return readOsReleaseField("SW_VERSION");
    } else if (type == "构建时间") {
        return readOsReleaseField("BUILD_TIME");
    } else if (type == "屏幕分辨率") {
        return readScreenResolution();
    } else if (type == "可用空间") {
        return readAvailableSpace();
    } else if (type == "BSP版本") {
        return readBspVersion();
    } else if (type == "CAN数量") {
        return readCanCount();
    } else if (type == "CAN状态") {
        return readCanStatus();
    } else if (type == "CPU温度") {
        return readCpuTemperature();
    }


    return QString("unknown");
}

QString DevicesInfo::readFileContent()
{
    QFile file("/proc/device-tree/model");
    if (!file.open(QIODevice::ReadOnly)) {
        return "unknown";
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
        return "unknown";
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
        return "unknown";
    }

    if (process.exitCode() != 0) {
        return "unknown";
    }

    return process.readAllStandardOutput().trimmed();
}

QString DevicesInfo::readCpuFrequency()
{
    QFile file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    if (!file.open(QIODevice::ReadOnly)) {
        return "unknown";
    }

    QString freq = file.readAll().trimmed();
    file.close();

    bool ok;
    int freqValue = freq.toInt(&ok);
    if (ok && freqValue > 0) {
        return QString::number(freqValue / 1000) + " MHz";
    }
    return "unknown";
}

QString DevicesInfo::readMemorySize()
{
    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly)) {
        return "unknown";
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
    return "unknown";
}

QString DevicesInfo::readEmmcSize()
{
    QProcess process;
    process.start("sh", {"-c", "for d in /sys/block/mmcblk[0-9]*; do [ \"$(cat $d/device/type 2>/dev/null)\" = \"MMC\" ] && awk '{printf \"%.0f GB\\n\", $1*512/1024/1024/1024}' $d/size; done"});

    if (!process.waitForFinished(3000)) {
        return "unknown";
    }

    return process.readAllStandardOutput().trimmed();
}

QString DevicesInfo::readOsVersion()
{
    QFile file("/etc/os-release");
    if (!file.open(QIODevice::ReadOnly)) {
        return "unknown";
    }

    QString content = file.readAll();
    file.close();

    // 查找 PRETTY_NAME
    QRegularExpression re("^PRETTY_NAME=\"?([^\"]*)\"?$", QRegularExpression::MultilineOption);
    QRegularExpressionMatch match = re.match(content);

    if (match.hasMatch()) {
        return match.captured(1).trimmed();
    }
    return "unknown";
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
        return "unknown";
    }

    QString content = cpuFile.readAll();
    cpuFile.close();

    QRegularExpression re("^Serial\\s*:\\s*(.+)$", QRegularExpression::MultilineOption);
    QRegularExpressionMatch match = re.match(content);

    if (match.hasMatch()) {
        return match.captured(1).trimmed();
    }
    return "unknown";
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
    return "unknown";
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
    return results.join(", ");
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
        return "unknown";
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
        return "unknown";
    }

    double usage = 100.0 * (totalDiff - idleDiff) / totalDiff;

    // 格式化输出，保留一位小数
    return QString::number(usage, 'f', 1) + " %";
}

QString DevicesInfo::readOsReleaseField(const QString &fieldName)
{
    QFile file("/etc/os-release");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "unknown";
    }

    QTextStream stream(&file);
    QRegularExpression regex(QString("^%1=\"([^\"]*)\"").arg(fieldName));

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        QRegularExpressionMatch match = regex.match(line);
        if (match.hasMatch()) {
            return match.captured(1);
        }
    }

    return "unknown";
}

QString DevicesInfo::readScreenResolution()
{
    QFile file("/sys/class/graphics/fb0/virtual_size");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "unknown";
    }

    QString sizeStr = file.readAll().trimmed();
    file.close();

    // 格式: "1280,800" -> "1280x800"
    QStringList parts = sizeStr.split(',');
    if (parts.size() == 2) {
        return parts[0] + "x" + parts[1];
    }

    return "unknown";
}

QString DevicesInfo::readAvailableSpace()
{
    QProcess process;
    process.start("df", QStringList() << "-h" << "/");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);

    if (lines.size() >= 2) {
        QStringList columns = lines[1].split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (columns.size() >= 4) {
            return columns[3]; // 第4列（索引3）是可用空间
        }
    }
    return "unknown";
}

QString DevicesInfo::readBspVersion()
{
    QString version = readOsReleaseField("VERSION");
    if (version != "unknown") return version;

    version = readOsReleaseField("VERSION_ID");
    if (version != "unknown") return version;

    version = readOsReleaseField("PRETTY_NAME");
    if (version != "unknown") return version;

    return "unknown";
}

QString DevicesInfo::readCanCount()
{
    QDir dir("/sys/class/net/");
    QStringList filters;
    filters << "can*";
    QStringList canDevices = dir.entryList(filters, QDir::Dirs);
    return QString::number(canDevices.count());
}

QString DevicesInfo::readCanStatus()
{
    QDir dir("/sys/class/net/");
    QStringList filters;
    filters << "can*";
    QStringList canDevices = dir.entryList(filters, QDir::Dirs);

    if (canDevices.isEmpty()) {
        return "No CAN devices found";
    }

    QStringList statusList;
    for (const QString &canDevice : canDevices) {
        QProcess process;
        process.start("ip", QStringList() << "-details" << "link" << "show" << canDevice);
        process.waitForFinished();
        QString output = process.readAllStandardOutput();

        if (output.isEmpty()) {
            statusList << canDevice + ": not found";
            continue;
        }

        // 提取 bitrate
        QString bitrate = "unknown";
        QRegularExpression bitrateRegex("bitrate (\\d+)");
        QRegularExpressionMatch match = bitrateRegex.match(output);
        if (match.hasMatch()) {
            bitrate = match.captured(1);
        }

        // 提取状态
        QString state = "unknown";
        if (output.contains("state DOWN")) {
            state = "DOWN";
        } else if (output.contains("state UP")) {
            state = "UP";
        } else if (output.contains("state ERROR-ACTIVE")) {
            state = "ERROR-ACTIVE";
        }

        statusList << QString("%1: %2, bitrate %3").arg(canDevice).arg(state).arg(bitrate);
    }

    return statusList.join(", ");
}

QString DevicesInfo::readCpuTemperature()
{
    QFile file("/sys/class/thermal/thermal_zone0/temp");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "unknown";
    }

    QString tempStr = file.readAll().trimmed();
    file.close();

    bool ok = false;
    int tempInt = tempStr.toInt(&ok);
    if (!ok) {
        return "unknown";
    }

    // 大于 1000 时除以 1000
    if (tempInt > 1000) {
        return QString::number(tempInt / 1000.0, 'f', 1) + " ℃";
    } else {
        return QString::number(tempInt) + " ℃";
    }
}

