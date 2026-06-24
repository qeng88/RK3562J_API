#include "brightness.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

Brightness::Brightness(QObject *parent)
    : QObject(parent)
{

}

bool Brightness::writeBrightness(int value)
{
    if(brightPath.isEmpty()) return false;

    const int maxValue = readMaxBrightness();
    if(maxValue <= 0) return false;
    if(value < 0 || value > maxValue) return false;

    const int current =readBrightness();
    if(current == value) return true;

    QFile file(brightPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    QTextStream stream(&file);
    stream << value;
    file.close();
    return true;
}

int Brightness::readBrightness() const
{
    QFile file(brightPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return -1;
    }

    int curVal;
    QTextStream stream(&file);
    stream >> curVal;
    file.close();
    return curVal;
}

int Brightness::readMaxBrightness() const
{
     QFileInfo info(brightPath);
     const QString maxPath = info.absolutePath() + "/max_brightness";
     QFile file(maxPath);
     if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
         return -1;
     }

     const QByteArray data = file.readAll().trimmed();

     bool ok = false;
     const int value = data.toInt(&ok);

     return ok ? value : -1;
}
