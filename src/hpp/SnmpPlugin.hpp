#ifndef SNMPPLUGIN_H
#define SNMPPLUGIN_H

#include <QDebug>
#include "Protobuf.pb.h"
#include <libhasimov/messaging/configurationconnection.h>
#include <libhasimov/messaging/connection.h>
#include <libhasimov/messaging/willtestament.h>
#include <libhasimov/messaging/qos.h>
#include <libhasimov/messaging/mqttconnection.h>
#include <QSettings>
#include <QTimer>
#include <iostream>
#include "globals.hpp"
#include "utime.hpp"
#include <QFileInfo>
#include "SNMPRequest.hpp"
#include <QList>
#include "SnmpMultiRequest.hpp"

/**
 * \class SnmpPlugin
 *
 * \brief The class which handle all the main operations.
 *
 * This class makes all operations :
 * Ini file read
 * MQTT server connection
 * Data serialisation
 * SNMP requests
 * ...
 *
 * \author Emilien BARBAUD
 *
 * \date 2018
 */

using namespace proto::monitoring;

class SnmpPlugin : public QObject
{
    Q_OBJECT

public:
    SnmpPlugin();

    /**
     * @brief Getter for sentFromStart
     *
     * @return Returns the number of Protobuf messages (group of requests) sent from start.
     */
    long getSentFromStart();

public slots:
    /**
     * @brief Event called when timer is expired
     */
    void event();

    /**
     * @brief Event called when a SNMP async request foud value.
     *
     * @note This verify each time if all the requests have been gathered.
     */
    void requestsDone();

    /**
     * @brief Event called when the connection with the Broker has been established
     */
    void MqttConnected() const;

    /**
     * @brief Event called when a disconnection with the Broker occurs
     */
    void MqttDisconnected() const;

private:

    //The timer to make SNMP requests
    QTimer _timer;

    //The full package of OID(SnmpObjects) to send
    SnmpPackage _pack;

    //The MQTT connection
    MQTTConnection* _mqtt;

    //The serialized string to send
    QString _serialized;

    //List of all SNMP Requests
    QList<SnmpRequest*> _oidList;

    //Settings
    QSettings _settings;

    //Number of MQTT messages sent
    long _sentFromStart;

    //Number of received response of SNMP requests since last try
    int _received;

    //Function to read the ini file and fill the Protobuf serializer
    void iniRead(const QString & file);

    //Update the snmp values
    void makeRequests();

    //Serialize all the SNMP items/objects
    //Sould be made after updating snmp values
    void serialize();

};

#endif // SNMPPLUGIN_H
