#ifndef SNMPMULTIREQUEST_H
#define SNMPMULTIREQUEST_H

/**
 * \class SnmpMultiRequest
 *
 * \brief The class to make the SNMP Walk multi requests
 *
 * To make an SNMP Walk request, you have to instanciate a SnmpRequest, then you have to call makeRequest() method to lauch an asynchronous request. A gotResponse() signal is emmited when all the values are fully gathered.
 *
 * \note async_callback() method sould not be called
 *
 * \author Emilien BARBAUD
 *
 * \date 2018
 */


#include "SNMPRequest.hpp"

class SnmpMultiRequest : public SnmpRequest
{
public:
    SnmpMultiRequest(const QString OID, const int MAXI, const int GROUP);

    /**
     * @brief Launch an asynchronous SNMP Walk request to the device.
     *
     * When all the results are received, a gotResponse() signal is emmited.
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
    virtual void async_callback(int reason, Snmp_pp::Snmp * snmp, Snmp_pp::Pdu &pdu, Snmp_pp::SnmpTarget &target);

private:
    const int _MAX;
    const int _GROUP;
    int _remain;
    QString _lastOid;
};

#endif // SNMPMULTIREQUEST_H
