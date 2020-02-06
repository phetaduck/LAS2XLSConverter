#ifndef MQTT_IMPL_H
#define MQTT_IMPL_H

#include <QObject>
#include "mosquitto.hpp"

class QThread;

class MQTT_Impl : public QObject, public Mosquitto
{
    Q_OBJECT
public:
    MQTT_Impl(QObject* parent = nullptr);

    virtual void start(bool tryReconnect = true);

    virtual void subscribe(const QString& topic) override;
    virtual void publish(const QString &topic, const std::string &message) override;

    virtual void connectToBroker(const QString& remote, const int port = 1883, const int aliveDelay = 30) override;

    virtual void set_tls(const char* cacert, const char* certfile, const char* keyfile) override;
    virtual void set_tls_insecure(bool enabled) override;
    virtual void set_username_password(const char* username, const char* password) override;

signals:
    void MessageRecieved(std::string msg);
    void connectedSuccessfully();

    void exceptionThrown(QString what);

    void publishException(QString what);
    void connectException(QString what);

    void set_tlsException(QString what);
    void set_tls_insecureException(QString what);
    void set_username_passwordException(QString what);

    void error(QString what);

private slots:
    void workerThreadFinished();

private:
    QThread* m_messageLoop = nullptr;
    QString m_topic;

    virtual void onConnected() override;
    virtual void onMessage(QString topic, std::string message) override;
    virtual void onError(const QString &msg) override;

    void detachFromWorkerThread();
};

#endif // MQTT_IMPL_H
