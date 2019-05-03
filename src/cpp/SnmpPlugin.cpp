#include "SnmpPlugin.hpp"

/*
 * CONSTRUCTOR
 */
SnmpPlugin::SnmpPlugin() : _settings(SETTINGS::FILE::PATH, QSettings::IniFormat) // reading .ini file and storing values in _settings object
{
    // defining ini codec as UTF-8
    _settings.setIniCodec(SETTINGS::FILE::ENCODING.toLocal8Bit().constData());

    // TIMER ===============
    // connecting timer to refresh function
    QObject::connect(&_timer, &QTimer::timeout, this, &SnmpPlugin::event);

    // starting timer with value found in _settings
    _timer.start(_settings.value(SETTINGS::INI::TIMER::SECTION + "/" + SETTINGS::INI::TIMER::DELAY_NAME, SETTINGS::INI::TIMER::SECTION + "/" + SETTINGS::INI::TIMER::DELAY_DEFAULT).toInt());
    qDebug() << "Timer delay =" << _timer.remainingTime() << "ms";
    // =====================

    //MQTT CONNECTION ======
    // creating config object used to connect to the message broker using values found in _settings
    ConfigurationConnection config(_settings.value(SETTINGS::INI::MQTT::SECTION + "/" + SETTINGS::INI::MQTT::ADDR_NAME, SETTINGS::INI::MQTT::SECTION + "/" + SETTINGS::INI::MQTT::ADDR_DEFAULT).toString(), _settings.value(SETTINGS::INI::MQTT::SECTION + "/" + SETTINGS::INI::MQTT::PORT_NAME, SETTINGS::INI::MQTT::SECTION + "/" + SETTINGS::INI::MQTT::PORT_DEFAULT).toInt());
    // =====================

    // SNMP ================
    // catching list of IP address to monitor
    QStringList IPlist(_settings.value(SETTINGS::INI::SNMP::IP_LIST_NAME).toStringList());

    // catching list size
    int size = IPlist.size();

    // checking if the list is not empty, else quitting the program
    if (size == 0)
    {
        qWarning() << "No IP addresses found in file: " << SETTINGS::FILE::PATH << "\n\t-> Quitting...";
        exit(EXIT_FAILURE);
    }

    // displaying the addresses found
    qDebug() << "Hosts to monitor: ";
    for (int i = 0; i < size; i++)
        qDebug()
            << "\t-> " << IPlist.at(i);

    // clearing the potential QueryManager left in the managers list
    _managers.clear();

    // for each address
    for (int i = 0; i < size; i++)
    {
        // on récupère les valeurs utiles pour créer nos managers
        QString const idStr = IPlist.at(i);                                                                                                                                     // catching current address
        QString const filenameStr(_settings.value(idStr + "/" + SETTINGS::INI::SNMP::OID_FILE_PATH_NAME, idStr + "/" + SETTINGS::INI::SNMP::OID_FILE_PATH_DEFAULT).toString()); // catching current OIDs file path
        int retries = _settings.value(idStr + "/" + SETTINGS::INI::SNMP::RETRY_NAME, idStr + "/" + SETTINGS::INI::SNMP::RETRY_DEFAULT).toInt();                                 // catching autoretries value
        long timeout = _settings.value(idStr + "/" + SETTINGS::INI::SNMP::TIMEOUT_NAME, idStr + "/" + SETTINGS::INI::SNMP::TIMEOUT_DEFAULT).toInt();                            // catching timeout delay

        // adding the new Manager to the list
        _managers.append(new SnmpQueryManager(idStr, filenameStr, retries, timeout, _settings.value(SETTINGS::INI::TIMER::SECTION + "/" + SETTINGS::INI::TIMER::DELAY_NAME, SETTINGS::INI::TIMER::SECTION + "/" + SETTINGS::INI::TIMER::DELAY_DEFAULT).toInt(), config));

        // TODO: can use _managers.last() to get the last reference of the list and then connect the timer of Plugin to the event of manager (if multi threaded)
    }
    // =====================
}

void SnmpPlugin::event()
{
    /* 
    TODO:   
        maybe connect the timer of the plugin to each of the manager event methods
        this would induce to launch each manager in a separate thread.
    */
}

// returns the number of request launched from the start for each of the managers
long SnmpPlugin::getSentFromStart()
{
    for (int i = 0; i < _managers.size(); i++)
        _sentFromStart += _managers.at(i)->getSentFromStart();

    return _sentFromStart;
}

// event called when we are connected to the broker
void SnmpPlugin::MqttConnected() const
{
    qDebug() << "Connected to the MQTT Broker !";
}

// event called when we are disconnected of the broker
void SnmpPlugin::MqttDisconnected() const
{
    qDebug() << "Disconnected from the MQTT Broker !";
}
