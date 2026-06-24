#include "rtc.h"
#include <QDateTime>
#include <QProcess>
#include <QLocale>

RTC::RTC(QObject *parent) : QObject(parent)
{

}

bool RTC::writeDateTime(const QString &time)
{
    if(time.isEmpty()) return false;

    const QDateTime dateTime = QDateTime::fromString(time, "yyyy-MM-dd HH:mm:ss");

    if(!dateTime.isValid()) return false;

    int ret = QProcess::execute("date", QStringList() << "-s" << time);
    if (ret != 0) {
        return false;
    }

    ret = QProcess::execute("hwclock", QStringList() << "-w");
    if (ret != 0) {
        return false;
    }

    return true;
}

QString RTC::readDateTime() const
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}
