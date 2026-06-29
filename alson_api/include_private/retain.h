#ifndef RETAIN_H
#define RETAIN_H

#include <QObject>
#include <QSettings>

class Retain : public QObject
{
    Q_OBJECT
public:
    explicit Retain(QObject *parent = nullptr);

    void setSaveRetain(const QString &key, const QVariant &value);
    QVariant getReadRetain(const QString &key) const;
    QVariant getReadRetain(const QString &key, const QVariant &defaultValue) const;
    void clear();

private:
    QSettings * m_settings = nullptr;
};

#endif // RETAIN_H
