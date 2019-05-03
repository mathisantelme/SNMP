#ifndef SNMPREQUEST_H
#define SNMPREQUEST_H

#include <QDebug>
#include <QTimer>
#include <QtCore>
#include <snmp_pp/snmp_pp.h> // bibliothèque introuvable
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
 * \author Emilien BARBAUD
 *
 * \date 2018
 */

class SnmpRequest : public QObject
{
    Q_OBJECT

public:
    SnmpRequest(const QString OID);

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
     * @brief Getter for the value
     *
     * Is empty when instanciating the object, will take the right value after a makeRequest().
     *
     * @return Returns the first OID value on the list (for SNMP Get)
     */
    QString getValue() const;

private slots:
    void timerExpired();

private:
    //The timer to make asynchronous event
    QTimer _timer;

protected:
    //The target for SNMP Request
    const Snmp_pp::CTarget *_ctarget;

    const QString _OID;

    QString _value;
    Snmp_pp::Snmp *_snmp;

signals:
    void gotResponse();
};

#endif // SNMPREQUEST_H
