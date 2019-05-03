#include "SNMPRequest.hpp"

const Snmp_pp::CTarget *SnmpRequest::_ctarget(const QString addr, int retries, long timeout)
{
    // creating UDP address from target address
    Snmp_pp::UdpAddress address(addr.toStdString().c_str());

    // creating CTarget object out of UDP address
    Snmp_pp::CTarget *ctarget = new Snmp_pp::CTarget(address);

    ctarget->set_version(Snmp_pp::version2c);                                        // setting version
    ctarget->set_readcommunity(SETTINGS::SNMP::COMMUNITY.toLocal8Bit().constData()); // setting community name
    ctarget->set_retry(retries);                                                     // setting retries
    ctarget->set_timeout(timeout);                                                   // setting timeout delay

    return ctarget;
}

/*
 * CONSTRUCTOR
 */
SnmpRequest::SnmpRequest(const QString OID, const QString inetaddr, int retries, long timeout) : _OID(OID)
{

    _IPaddr = inetaddr; // setting IP address of target
    _retries = retries; // setting retries
    _timeout = timeout; // setting timeout delay
    _value = "null";    // setting default value of the request to "null"

    // creating SNMP session
    int status;
    _snmp = new Snmp_pp::Snmp(status);

    // if the session created wasn't valid, we display a warning message
    if (status != SNMP_CLASS_SUCCESS)
        qWarning() << "Could not create SNMP++ session:";

    QObject::connect(&_timer, &QTimer::timeout, this, &SnmpRequest::timerExpired); // connecting the timeout event to the refresh method
    _timer.start(SETTINGS::SNMP::CALLBACKDELAY.toInt());                           // starting timer
}

// C Callback function for snmp++
void callback(int reason, Snmp_pp::Snmp *snmp, Snmp_pp::Pdu &pdu, Snmp_pp::SnmpTarget &target, void *cd)
{
    // calling the real callback member function
    if (cd)
        ((SnmpRequest *)cd)->async_callback(reason, snmp, pdu, target);
}

// Updates the values of the SNMP request
void SnmpRequest::makeRequest()
{
    Snmp_pp::Oid snmpOid(_OID.toLocal8Bit().constData()); // setting OID value
    Snmp_pp::Pdu pdu;                                     // PDU
    Snmp_pp::Vb vb;                                       // Variables Binding (VB)

    vb.set_oid(snmpOid); // setting VB
    pdu += vb;           // adding VB to PDU

    int status; // request status

    // launching request
    status = _snmp->get(pdu, (Snmp_pp::SnmpTarget &)*SnmpRequest::_ctarget(this->_IPaddr, this->_retries, this->_timeout), callback, this);

    // if an error happens, we display a warning message and set the value of the request to "error"
    if (status < 0)
    {
        qWarning() << "Error while trying to get the OID, status = " << status;
        _value = "error";
    }
}

void SnmpRequest::timerExpired()
{
    // When using async requests or if we are waiting for traps or
    // informs, we must call this member function periodically, as
    // snmp++ does not use an internal thread.
    _snmp->get_eventListHolder()->SNMPProcessPendingEvents();
}

void SnmpRequest::async_callback(int reason, Snmp_pp::Snmp *snmp, Snmp_pp::Pdu &pdu, Snmp_pp::SnmpTarget &target)
{
    // as snmp and target are parameters used to override the function, we need to inform the compiler that they are not used in this function
    Q_UNUSED(snmp);
    Q_UNUSED(target);

    // analysing the callback reason
    switch (reason)
    {
    case SNMP_CLASS_ASYNC_RESPONSE: // RESPONSE

        if (pdu.get_vb_count() == 0) // if PDU is empty, display warning message
            qWarning() << "\t-> Error : Pdu is empty";
        else // if PDU is not empty
        {
            // catching VB value and converting it to string
            Snmp_pp::Vb vbb;
            pdu.get_vb(vbb, 0);
            QString valueStr = vbb.get_printable_value();

            // if the value is not empty we set the value
            if (!valueStr.isEmpty())
                _value = valueStr;

            // displaying results
            qDebug() << "-> For this OID :" << _OID;
            qDebug() << "-> Found this value :" << _value;
        }

        // calling gotResponse event
        emit gotResponse();
        break; // RESPONSE

    case SNMP_CLASS_TIMEOUT: // TIMEOUT
        qWarning() << "Timeout  !";
        break; // TIMEOUT

    default: // DEFAULT
        qWarning() << "Did not receive any async response/trap.";
        break; // DEFAULT
    }
}

// returns the OID of the request
QString SnmpRequest::getOid() const
{
    return _OID;
}

// returns the request value
QString SnmpRequest::getValue() const
{
    return _value;
}
