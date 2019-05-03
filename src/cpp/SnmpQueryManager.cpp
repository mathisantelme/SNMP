#include "SnmpQueryManager.hpp"

SnmpQueryManager::SnmpQueryManager(
    QString p_IP_address,            // the ip address of the target
    QString p_settings_filename,     // the path to the config file
    int p_retries,                   // the number of retries allowed
    long p_timeout,                  // the allowed response delay
    long p_timer_timeout,            // the timer delay
    ConfigurationConnection p_config // a config object used to connect to the message broker
)
{
    IP_address = p_IP_address;               // setting the address of the target
    settings_filename = p_settings_filename; // setting the path of the config file
    retries = p_retries;                     // setting the number of retries
    timeout = p_timeout;                     // setting the response timeout
    _config = p_config;                      // setting the timer timeout

    // TIMER ===============
    // connecting the timer with the refresh event
    QObject::connect(&_timer, &QTimer::timeout, this, &SnmpQueryManager::event);

    // starting timer
    _timer.start(p_timer_timeout);
    qDebug() << "Timer delay =" << _timer.remainingTime() << "ms";
    // =====================

    // MQTT CONNECTION =====
    // creating an object used to connect to a message broker
    _mqtt = new MQTTConnection(_config, SETTINGS::INI::MQTT::SECTION + "/" + SETTINGS::MQTT::ID);
    qDebug() << "Connecting to the Broker...";

    // displaying informations about the broker
    qDebug() << "Broker infos: ";
    qDebug() << "\t-> Destination ip :" << _config.getAddress();
    qDebug() << "\t-> Port :" << _config.getPort();

    // connecting broker events to corresponding functions
    QObject::connect(_mqtt, &MQTTConnection::connected, this, &SnmpQueryManager::MqttConnected);
    QObject::connect(_mqtt, &MQTTConnection::disconnected, this, &SnmpQueryManager::MqttDisconnected);

    // connecting to the broker
    _mqtt->connect();
    // =====================

    // SNMP ================
    // reading OIDs from the file and creating corresponding requests
    iniRead(p_settings_filename);

    _received = -1;     // setting the number of received responses
    _sentFromStart = 0; // setting the number of requests send since the start of the object
    // =====================
}

void SnmpQueryManager::iniRead(const QString &file)
{
    // we check if the file exists
    QFileInfo checkFile(file);

    // if the file doesn't exist we display a warning
    if (!(checkFile.exists() && checkFile.isFile()))
        qWarning() << "Error:\n\t-> Ini file " << file << " doesn't exist";

    // reading the values from the configuration file and storing them in the settings object
    QSettings settings(file, QSettings::IniFormat);

    // catching OIDs list
    QStringList OIDList(settings.value(SETTINGS::INI::SNMP::OID_LIST_NAME).toStringList());

    // catching list size
    int size = OIDList.size();

    // clearing _pack of potential remaining SNMPobjects
    _pack.clear_obj();

    // for each OID
    for (int i = 0; i < size; i++)
    {
        //------ INI READING ------//
        QString const idStr = OIDList.at(i);                                                                                                                                  // catching OID value
        QString const nameStr(settings.value(idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::NAME_NAME, "null").toString());                                                     // catching name value
        QString const descStr(settings.value(idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::DESC_NAME, idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::DESC_DEFAULT).toString());  // catching description value
        QString const walkStr = settings.value(idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::WALK_NAME, idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::WALK_DEFAULT).toString(); // catching walk value (if the current OID requires a multirequest)
        int maxInt = settings.value(idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::MAX_NAME, idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::MAX_DEFAULT).toInt();                 // catching the maximum number of request (for multirequest)
        //-------------------------//

        //------ PROTOBUF ------//
        // creating a new SNMPobject and adding it to the pack
        SnmpObject *const obj = _pack.add_obj();

        qDebug() << "=======================";
        qDebug() << "OID has been set to " << idStr;
        obj->set_oid(idStr.toUtf8().constData()); // setting object OID value

        qDebug() << "Name has been set to " << nameStr;
        obj->set_name(nameStr.toUtf8().constData()); // setting object name value

        qDebug() << "Description has been set to " << descStr;
        obj->set_description(descStr.toUtf8().constData()); // setting object description value

        // if the current OID requires a multirequest
        if (walkStr == "true")
        {
            qDebug() << "Walk value has been set to " << maxInt;
            obj->set_walk(maxInt); // setting object walk value
        }

        qDebug() << "=======================";
        //----------------------//
    }

    qDebug() << "Done reading config file";

    //------ SNMP ------//
    // for each object in the pack
    for (int i = 0; i < _pack.obj_size(); ++i)
    {
        // setting max value
        int max = 1;

        // if the walk value of the current object has been set, then this is a multirequest one
        if (_pack.mutable_obj(i)->has_walk())
            max = _pack.mutable_obj(i)->walk(); // updating current max value

        // creating a new SNMPrequest object
        SnmpRequest *req = Q_NULLPTR;

        if (max > 1) // if we need a SNMP walk request
        {
            // catching OID
            QString idStr = OIDList.at(i);

            // catching number of request group
            int group = settings.value(idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::GROUP_NAME, idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::GROUP_DEFAULT).toInt();

            // creating new SNMPMultiRequest out of current obj info
            req = new SnmpMultiRequest(QString::fromStdString(_pack.mutable_obj(i)->oid()), max, group, IP_address, retries, timeout);

            // displaying confirmation message
            qDebug() << "Creating SNMP bulk request ...";
        }
        else // if we need a SNMP get request
        {
            // creating SNMP request out of current object info
            req = new SnmpRequest(QString::fromStdString(_pack.mutable_obj(i)->oid()), IP_address, retries, timeout);

            // displaying confirmation message
            qDebug() << "Creating SNMP get request ...";
        }

        // adding the new request to the list of requests
        _oidList.append(req);

        // connecting the request event gotResponse to the Manager event requestsDone
        QObject::connect(req, &SnmpRequest::gotResponse, this, &SnmpQueryManager::requestsDone);
    }

    //------------------//
}

void SnmpQueryManager::event()
{
    // if the number of response received if different of the number of requests sent or no response were received
    if (_received != _oidList.size() && _received != -1)
    {
        // display warnings
        qWarning() << "Not enough time to get all the requests !";
        qWarning() << "Check the SNMP network connection";

        // leaving the program
        qFatal("Your SNMP network speed is too slow to handle all the SNMP requests");
    }

    // reset the received var
    _received = 0;

    // [DEBUG] - measuring time
    long utime = getMicrotime();

    makeRequests(); // on lance les requetes asynchronnes

    // [DEBUG] - measuring time
    qDebug() << "Requests sent ! Took: " << (getMicrotime() - utime) / 1000 << "ms";
}

void SnmpQueryManager::makeRequests()
{
    // for each request in the list
    for (int i = 0; i < _oidList.size(); ++i)
    {
        _oidList[i]->makeRequest(); // we call the request own makerequest function

        // displaying confirmation message
        qDebug() << "Sending request for OID" << _oidList[i]->getOid();
    }
}

void SnmpQueryManager::serialize()
{
    // serializing the content of the pack (SNMPobject)
    QString serialized = QString::fromStdString(_pack.SerializeAsString());

    // displaying the serialized value
    qDebug() << "Message serialized :";
    qDebug() << serialized;

    // setting _serialized
    _serialized = serialized;
}

void SnmpQueryManager::requestsDone()
{
    // incrementing the number of responses received
    _received++;

    // displaying a confirmation message
    qDebug() << "Received" << _received << "out of" << _oidList.size() << "\n";

    // if we got as many responses as we have request to make
    if (_received == _oidList.size())
    {
        // displaying a confirmation message
        qDebug() << "All SNMP values received !";

        // for each OIDs
        for (int i = 0; i < _oidList.size(); ++i)
        {
            // catching value from request
            QString valueStr = _oidList[i]->getValue();

            // setting the value of the current object
            _pack.mutable_obj(i)->set_value(valueStr.toUtf8().constData());

            // dsiplaying confirmation message
            qDebug() << "Value for" << QString::fromStdString(_pack.mutable_obj(i)->name()) << "has been set to :" << QString::fromStdString(_pack.mutable_obj(i)->value());
        }

        // [DEBUG] - measuring time
        long utime = getMicrotime();

        // serializing data
        serialize();

        // [DEBUG] - measuring time
        qDebug() << "Serialization done ! Took :" << (float)((getMicrotime() - utime) / 100 + 1) / 10. << "ms"
                 << " |  Size :" << (float)((_serialized.size() / 100) + 1) / 10. << "kB";

        //---- MQTT sending message ----
        // converting serialized message to bytearray
        QByteArray byteArray = _serialized.toUtf8();

        if (_mqtt->isConnected()) // if we are connected to the broker
        {
            qDebug() << "Sending a message to the broker";

            // pushing message to the broker
            if (_mqtt->publish(SETTINGS::MQTT::TOPIC, byteArray) >= 0) // if no error then  confirmation message
                qDebug() << "Success !";
            else // else display warning
                qWarning() << "Failure !";
        }
        else // if we are not connected to the broker we display a warning
            qWarning() << "No connection with the broker...";
        //------------------------------
    }
}

// returns the number of request sent from start
long SnmpQueryManager::getSentFromStart()
{
    return _sentFromStart;
}

// event called when we are connected to the broker
void SnmpQueryManager::MqttConnected()
{
    qDebug() << "Broker connected";
}

// event called when we are disconnected from the broker
void SnmpQueryManager::MqttDisconnected()
{
    qDebug() << "Broker disconnected";
}
