#ifndef SNMPQUERYMANAGER_H
#define SNMPQUERYMANAGER_H

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

using namespace proto::monitoring;

/**
 * \class SnmpQueryManager
 *
 * \brief A class used to create and handle SNMP request for one target
 * 
 * \author ANTELME Mathis
 *
 * \date 2019
 */

class SnmpQueryManager : public QObject
{
	Q_OBJECT

public:
	/**
	 * @brief Used to create a SNMPQueryManager out of parameters list
	 * 
	 * @param p_IP_address	the ip address of the target
	 * @param p_settings_filename 	the path to the config file
	 * @param p_retires	the number of retries allowed
	 * @param p_timeout	the allowed response delay 
	 * @param p_timer_timeout	the timer delay
	 * @param p_config	a config object used to connect to the message broker
	 */
	SnmpQueryManager(QString p_IP_address, QString p_settings_filename, int p_retires, long p_timeout, long p_timer_timeout, ConfigurationConnection p_config); // constructor

	/**
     * @brief Getter for sentFromStart
     * @return returns the number of Protobuf message sent from start for this object 
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

private:
	// members
	QString IP_address;		   // target IP address
	QString settings_filename; // path to config file
	int retries;			   // the number of retries
	long timeout;			   // the response delay

	ConfigurationConnection _config; // a configuration object used to connect to the message broker
	MQTTConnection *_mqtt;			 // a pointer to a message broker
	QTimer _timer;					 // timer used to make async request
	SnmpPackage _pack;				 // a collection of OIDs
	QString _serialized;			 // used to hold the serialized values of request
	QList<SnmpRequest *> _oidList;   // OID list
	QSettings _settings;			 // settings object used to store the values readed in the config file

	long _sentFromStart; // the number of request sent
	int _received;		 // the current number of responses received

	// functions ===========
	void iniRead(const QString &file); // read a configuration file and creates SNMP Objects
	void makeRequests();			   // updates the values of SNMP request
	void serialize();				   // serialize the results of the requests
	void MqttConnected();			   // event called when connection with the broker has been made
	void MqttDisconnected();		   // event called when the connection with the broker has been lost
};

#endif // SNMPQUERYMANAGER_H
