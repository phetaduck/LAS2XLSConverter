#include "mqtt_impl.h"

#include <QThread>

MQTT_Impl::MQTT_Impl(QObject* parent) :
    QObject(parent)
{}

void MQTT_Impl::start(bool tryReconnect)
{
    auto func = std::bind(&MQTT_Impl::loop, this, tryReconnect );
    m_messageLoop = QThread::create(func);
    m_messageLoop->start();
}

void MQTT_Impl::subscribe(const QString &topic)
{
    Mosquitto::subscribe(topic);
    m_topic = topic;
}

void MQTT_Impl::onMessage(QString topic, QString message)
{
    if (topic == m_topic) {
        emit MessageRecieved(message);
    }
}
