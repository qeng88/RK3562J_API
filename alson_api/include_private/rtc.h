#ifndef RTC_H
#define RTC_H

#include <QObject>

class RTC : public QObject
{
    Q_OBJECT
public:
    explicit RTC(QObject *parent = nullptr);

    bool writeDateTime(const QString &time);
    QString readDateTime() const;

signals:

};

#endif // RTC_H
