#ifndef SNMPPLUGIN_H
#define SNMPPLUGIN_H

#include <QDebug>
#include "Protobuf.pb.h"
#include "SnmpQueryManager.hpp"
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
 * This class makes the following operations :
 * -> Reading a .ini file
 * -> Creating SNMPQueryManager out of the informations contained in the setting file
 * -> Connecting to a message broker
 * 
 * \author Emilien BARBAUD, ANTELME Mathis
 *
 * \date 2019
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
   * @brief Event called when the connection with the Broker has been established
   */
  void MqttConnected() const;

  /**
   * @brief Event called when a disconnection with the Broker occurs
   */
  void MqttDisconnected() const;

private:
  QTimer _timer; // Timer used to launch async SNMP request

  MQTTConnection *_mqtt; // a pointer to a message broker

  QSettings _settings; // an object used to store all of the configuration information

  QList<SnmpQueryManager *> _managers; // a list of SNMPQueryManagers

  long _sentFromStart; // the number of request sent from all of the managers
};

#endif // SNMPPLUGIN_H
