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
    void start(bool tryReconnect);
    virtual void subscribe(const QString& topic) override;
signals:
    void MessageRecieved(std::string msg);
private:
    QThread* m_messageLoop = nullptr;
    QString m_topic;
    virtual void onMessage(QString topic, std::string message) override;
};

#endif // MQTT_IMPL_H
