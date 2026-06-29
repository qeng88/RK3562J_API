#include "retain.h"
#include <QDebug>
#include <QTextCodec>

Retain::Retain(QObject *parent) : QObject(parent)
{
    m_settings = new QSettings("/opt/alson/retain.ini",QSettings::IniFormat,this);
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    m_settings->setIniCodec(codec);
}

void Retain::setSaveRetain(const QString &key, const QVariant &value)
{
    m_settings->setValue(key,value);
    m_settings->sync();
}

QVariant Retain::getReadRetain(const QString &key) const
{
    return m_settings->value(key);
}

QVariant Retain::getReadRetain(const QString &key, const QVariant &defaultValue) const
{
    return m_settings->value(key, defaultValue);
}

void Retain::clear()
{
    m_settings->clear();
    m_settings->sync();
}
