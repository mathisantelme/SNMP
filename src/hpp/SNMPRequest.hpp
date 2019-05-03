#ifndef SNMPREQUEST_H
#define SNMPREQUEST_H

#include <QDebug>
#include <QTimer>
#include <QtCore>
#include <snmp_pp/snmp_pp.h>
#include "globals.hpp"
#include "utime.hpp"

/**
 * \class SnmpRequest
 *
 * \brief The class to make the SNMP Requests
 *
 * To make an SNMP Request, you have to instanciate a SnmpRequest, then you have to call makeRequest() method to lauch an asynchronous request. A gotResponse() signal is emmited when the value is fully gathered.
 *
 * \note async_callback() method sould not be called
 *
 * \authors Emilien BARBAUD, ANTELME Mathis
 *
 * \date 2019
 */

class SnmpRequest : public QObject
{
  Q_OBJECT

public:
  /**
   * @brief Creates a SNMP GET request out of parameters list
   * 
   * @param OID The oid of the query
   * @param inetaddr The IP addresse of the target
   * @param retries The number of retries allowed (cTarget)
   * @param timeout The delay allowed for the target (cTarget)
   */
  SnmpRequest(const QString OID, const QString inetaddr, int retries, long timeout);

  /**
   * @brief Launch an asynchronous SNMP Request to the device.
   *
   * When the result is received, a gotResponse() signal is emmited.
   * Then you can use getValue().
   * The use of a connect() is highly recommended.
   */
  virtual void makeRequest();

  /**
   * @brief Should not be called by yourself, except you know exactly what you are doing.
   * @param reason The reason for this callback
   * @param snmp The Snmp object from SNMP++ lib.
   * @param pdu The Pdu object from SNMP++ lib.
   * @param target The target you created to define SNMP.
   */
  virtual void async_callback(int reason, Snmp_pp::Snmp *snmp, Snmp_pp::Pdu &pdu, Snmp_pp::SnmpTarget &target);

  /**
   * @brief Getter for the OID
   *
   * @return Returns the OID
   */
  QString getOid() const;

  /**
   * @brief Getter for the value of the request
   *
   * Is empty when instanciating the object, will take the right value after a makeRequest().
   *
   * @return Returns the first OID value on the list (for SNMP Get)
   */
  QString getValue() const;

private slots:
  /**
   * @brief Function called at each timer timeout
   */
  void timerExpired();

private:
  QTimer _timer; // the timer used to make asynchronous event

protected:
  /**
   * @brief Used to create a Snmp_pp::CTarget
   * 
   * @param addr The ip addresse of the target
   * @param retries The number of retries allowed
   * @param timeout The delay allowed for the target
   * 
   * @return a Snmp_pp::CTarget pointer
   */
  const Snmp_pp::CTarget *_ctarget(const QString addr, int retries, long timeout);

  // Parameters

  const QString _OID;   // the oid of the request
  QString _value;       // the value returned by the request
  Snmp_pp::Snmp *_snmp; // the snmp context of the request

  QString _IPaddr; // the ip address of the target
  int _retries;    // the number of retries allowed
  long _timeout;   // the timeout delay

signals:
  /**
   * @brief Function called when a request gets a response
   */
  void gotResponse();
};

#endif // SNMPREQUEST_H
