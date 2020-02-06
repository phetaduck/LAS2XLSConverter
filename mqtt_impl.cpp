#include "mqtt_impl.h"

#include <QThread>

MQTT_Impl::MQTT_Impl(QObject* parent) :
    QObject(parent)
{}

void MQTT_Impl::detachFromWorkerThread()
{
    disconnect(m_messageLoop, &QThread::finished,
            this, &MQTT_Impl::workerThreadFinished);
    m_messageLoop->deleteLater();
    m_messageLoop = nullptr;
}

void MQTT_Impl::start(bool tryReconnect)
{
    if (m_messageLoop)
    {
        m_messageLoop->quit();
#pragma message("Determine if thread can be safely ignored with deleteLater")
        m_messageLoop->wait();
        detachFromWorkerThread();
    }
    auto func = std::bind(&MQTT_Impl::loop, this, tryReconnect );
    m_messageLoop = QThread::create(func);
    connect(m_messageLoop, &QThread::finished,
            this, &MQTT_Impl::workerThreadFinished);
    m_messageLoop->start();
}

void MQTT_Impl::subscribe(const QString &topic)
{
    try {
        Mosquitto::subscribe(topic);
        m_topic = topic;
    } catch (const std::exception& e) {
        emit exceptionThrown(QString::fromStdString(e.what()));
    }
}

void MQTT_Impl::publish(const QString &topic, const std::string &message)
{
    try {
        Mosquitto::publish(topic, message);
    } catch (const std::exception& e) {
        emit publishException(QString::fromStdString(e.what()));
        emit exceptionThrown(QString::fromStdString(e.what()));
    }
}

void MQTT_Impl::connectToBroker(const QString &remote, const int port, const int aliveDelay)
{
    try {
        Mosquitto::connectToBroker(remote, port, aliveDelay);
    } catch (const std::exception& e) {
        emit connectException(QString::fromStdString(e.what()));
        emit exceptionThrown(QString::fromStdString(e.what()));
    }
}

void MQTT_Impl::set_tls(const char *cacert, const char *certfile, const char *keyfile)
{
    try {
        Mosquitto::set_tls(cacert, certfile, keyfile);
    } catch (const std::exception& e) {
        emit set_tlsException(QString::fromStdString(e.what()));
        emit exceptionThrown(QString::fromStdString(e.what()));
    }
}

void MQTT_Impl::set_tls_insecure(bool enabled)
{
    try {
        Mosquitto::set_tls_insecure(enabled);
    } catch (const std::exception& e) {
        emit set_tls_insecureException(QString::fromStdString(e.what()));
        emit exceptionThrown(QString::fromStdString(e.what()));
    }
}

void MQTT_Impl::set_username_password(const char *username, const char *password)
{
    try {
        Mosquitto::set_username_password(username, password);
    } catch (const std::exception& e) {
        emit set_username_passwordException(QString::fromStdString(e.what()));
        emit exceptionThrown(QString::fromStdString(e.what()));
    }
}

void MQTT_Impl::workerThreadFinished()
{
    detachFromWorkerThread();
}

void MQTT_Impl::onConnected()
{
    Mosquitto::onConnected();
    emit connectedSuccessfully();
}

void MQTT_Impl::onMessage(QString topic, std::string message)
{
    (void)topic;
    emit MessageRecieved(message);
}

void MQTT_Impl::onError(const QString &msg)
{
    emit error(msg);
}
