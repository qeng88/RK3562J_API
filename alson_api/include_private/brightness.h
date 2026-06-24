#ifndef BRIGHTNESS_H
#define BRIGHTNESS_H

#include <QObject>

class Brightness : public QObject
{
    Q_OBJECT
public:
    explicit Brightness(QObject *parent = nullptr);

    bool writeBrightness(int value);
    int readBrightness() const;
    int readMaxBrightness() const;

private:
    const QString brightPath = "/sys/class/backlight/backlight/brightness";
};

#endif // BRIGHTNESS_H
