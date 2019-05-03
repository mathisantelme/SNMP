#include "SnmpMultiRequest.hpp"

/*
 * CONSTRUCTOR
 */
SnmpMultiRequest::SnmpMultiRequest(const QString OID, const int MAXI, const int GROUP, const QString inetaddr, int retries, long timeout) : SnmpRequest(OID, inetaddr, retries, timeout), _MAX(MAXI), _GROUP(GROUP)
{
    _remain = _MAX; // setting the number of remaining request as the maximum amount of requests to send
    _lastOid = OID; // setting the last OID value as the current OID
}

// C Callback function for snmp++
void bulkCallback(int reason, Snmp_pp::Snmp *snmp, Snmp_pp::Pdu &pdu, Snmp_pp::SnmpTarget &target, void *cd)
{
    // calling the real callback member function
    if (cd)
        ((SnmpMultiRequest *)cd)->async_callback(reason, snmp, pdu, target);
}

// fonction qui permet d'effectuer une requete
void SnmpMultiRequest::makeRequest()
{
    // if no request are remaining
    if (_remain <= 0)
    {
        _remain = _MAX;  // setting remaining request value to max request value
        _lastOid = _OID; // setting lastOID value as current OID value
    }

    Snmp_pp::Oid snmpOid(_lastOid.toLocal8Bit().constData()); // catching current OID
    Snmp_pp::Pdu pdu;                                         // PDU
    Snmp_pp::Vb vb;                                           // variables binding (VB)

    // displaying confirmation message
    qDebug() << "Bulk request processing...";

    vb.set_oid(snmpOid); // setting VB
    pdu += vb;           // adding VB to PDU
    int num = _GROUP;    // setting number of requests groups

    // if the number of request remaining is equal or less than the number of request group,
    // we make the last request as a whole group
    // ex:  say 3 request are remaining and the group size is 4.
    //  We launch the 3 remaining request as a group
    if (_remain <= _GROUP)
        num = _remain;

    // launching request
    int status = _snmp->get_bulk(pdu, (Snmp_pp::SnmpTarget &)*SnmpRequest::_ctarget(this->_IPaddr, this->_retries, this->_timeout), 0, num, bulkCallback, this);

    // if an error happens, we display a warning message and set the value of the request to "error"
    if (status < 0)
    {
        qWarning() << "Error while trying to get the OID, status = " << status;
        _value = "error";
    }
}

void SnmpMultiRequest::async_callback(int reason, Snmp_pp::Snmp *snmp, Snmp_pp::Pdu &pdu, Snmp_pp::SnmpTarget &target)
{

    // as snmp and target are parameters used to override the function, we need to inform the compiler that they are not used in this function
    Q_UNUSED(snmp);
    Q_UNUSED(target);

    // analysing the callback reason
    switch (reason)
    {
    case SNMP_CLASS_ASYNC_RESPONSE: // RESPONSE
        // if there are request remaining
        if (_remain >= 0)
        {
            // displaying the current number of response catched
            qDebug() << "Bulk !";
            qDebug() << "Received" << pdu.get_vb_count() << " response(s)";

            if (pdu.get_vb_count() == 0) // if PDU is empty we display a warining message
                qWarning() << "Error : Pdu is empty";
            else // if PDU contains at least one VB
            {
                // pfor each VB in PDU
                for (int i = 0; i < pdu.get_vb_count(); i++)
                {
                    // catching current VB
                    Snmp_pp::Vb vbb;
                    pdu.get_vb(vbb, i);

                    // checking if it is child of the request OID
                    if (QString::fromStdString(vbb.get_printable_oid()).prepend(".").contains(_OID))
                    {
                        // converting current VB to string
                        QString valueStr = vbb.get_printable_value();

                        // if value is not empty
                        if (!valueStr.isEmpty())
                        {
                            if (_remain != _MAX) // if it's not the first response, we add a separator before concatenation
                                _value += SETTINGS::SNMP::SEPARATOR;
                            else
                                _value = "";

                            // displaying response catched
                            qDebug() << "Adding this :" << valueStr;

                            // adding value catched to the global value of the request
                            _value += valueStr;
                        }
                        // if value is empty we display a warning
                        else
                            qWarning() << "Empty response !";

                        // displaying OID and request value
                        qDebug() << "For this OID :" << _OID;
                        qDebug() << "Found this value :" << _value;
                    }
                    // if we are not in the child of the request OID (end of the bulk request)
                    else
                    {
                        // displaying messages
                        qDebug() << "This OID is not a child :" << vbb.get_printable_oid();
                        qDebug() << "Ending SNMP walk request !";
                        _remain = 1; // resetting the remain value to leave the if condition
                    }

                    _remain--;

                    // displaying progress
                    qDebug() << "Bulk progress :" << _MAX - _remain << "/" << _MAX << "    (" << (_MAX - _remain) * 100 / _MAX << "% )";

                    if (_remain <= 0) // if there is no more requests to be done
                    {
                        emit gotResponse(); // calling signal gotResponse
                        break;              // leaving the loop
                    }
                    else if (i == pdu.get_vb_count() - 1) // if the current VB is the before last one
                    {
                        // displaying all of the catched OID
                        qDebug() << "Received thoses OIDs with bulk :";

                        // for each VB
                        for (int i = 0; i < pdu.get_vb_count(); i++)
                        {
                            // displaying value and OID
                            pdu.get_vb(vbb, i);
                            qDebug() << "-->" << vbb.get_printable_oid();
                        }

                        // setting the last OID
                        _lastOid = vbb.get_printable_oid();

                        // launching next request
                        qDebug() << "Launching next bulk request";
                        makeRequest();
                    } // END - if _remain <= 0
                }     // END - for each VB in PDU
            }         // END - if PDU not empty
        }             // END - if _remain >= 0
        break;        // RESPONSE

    case SNMP_CLASS_TIMEOUT: // TIMEOUT
        qWarning() << "Timeout  !";
        break; // TIMEOUT

    default: // DEFAULT
        qWarning() << "Did not receive any async response/trap.";
        break; // DEFAULT
    }
}
