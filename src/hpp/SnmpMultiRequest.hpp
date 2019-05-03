#ifndef SNMPMULTIREQUEST_H
#define SNMPMULTIREQUEST_H

/**
 * \class SnmpMultiRequest
 *
 * \brief The class to make the SNMP Walk multi requests
 *
 * To make an SNMP Walk request, you have to instanciate a SnmpRequest, then you have to call makeRequest() method to lauch an asynchronous request. A gotResponse() signal is emmited when all the values are fully gathered.
 *
 * \note async_callback() method should not be called
 *
 * \authors Emilien BARBAUD, ANTELME Mathis
 *
 * \date 2019
 */

#include "SNMPRequest.hpp"

class SnmpMultiRequest : public SnmpRequest
{
public:
	/**
	 * @brief Creates a SNMP BULK request out of parameters list
	 * 
	 * @param OID       The OID used by the request
	 * @param MAXI      The maximum numbers of requests sent until having a response
	 * @param GROUP     The number of groups of requests
	 * @param inetaddr  The IP address of the target 
	 * @param retries   The number of retries allowed (cTarget)
	 * @param timeout   The delay allowed (cTarget)
	 */
	SnmpMultiRequest(const QString OID, const int MAXI, const int GROUP, const QString inetaddr, int retries, long timeout);

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
	virtual void async_callback(int reason, Snmp_pp::Snmp *snmp, Snmp_pp::Pdu &pdu, Snmp_pp::SnmpTarget &target);

private:
	// Members
	const int _MAX;   // The maximum numbers of requests sent until having a response
	const int _GROUP; // The number of groups of requests
	int _remain;	  // The number of requests remaining to be launched
	QString _lastOid; // the last OID used
};

#endif // SNMPMULTIREQUEST_H
