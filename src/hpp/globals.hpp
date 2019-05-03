#include <QDebug>

#ifndef BNOS_DEVICES_SNMP_HPP
#define BNOS_DEVICES_SNMP_HPP

/**
 * \brief A header file used to store default values for object and configuration files
 * 
 * \authors Emilien BARBAUD, ANTELME Mathis
 *
 * \date 2019
 */

namespace SETTINGS
{
    namespace FILE // configuration file values
    {
        const QString PATH = "config/settings.ini"; // default configuration file path
        const QString ENCODING = "UTF-8";           // default config file encoding
    } // namespace FILE

    namespace INI // default values used to read configuration files
    {
        namespace MQTT // MQTT object default configuration values
        {
            const QString SECTION = "mqtt";           // MQTT default section name
            const QString ADDR_NAME = "ip";           // address default identifier
            const QString ADDR_DEFAULT = "127.0.0.1"; // address default value
            const QString PORT_NAME = "port";         // port default identifier
            const QString PORT_DEFAULT = "1883";      // port default value
        } // namespace MQTT

        namespace SNMP // SNMP objects default configuration values
        {
            const QString TIMEOUT_NAME = "timeout";                 // timeout default identifier
            const QString TIMEOUT_DEFAULT = "3000";                 // timeout default value
            const QString RETRY_NAME = "autoretries";               // retry default identifier
            const QString RETRY_DEFAULT = "10";                     // retry default value
            const QString OID_FILE_PATH_NAME = "filename";          // OID filename default identifier
            const QString OID_FILE_PATH_DEFAULT = "config/OID.ini"; // OID filename default value
            const QString IP_LIST_NAME = "IPs";                     // IP addresses list default identifier
            const QString OID_LIST_NAME = "OIDs";                   // OID list default identifier

        namespace OID_FILE // OID objects default configuration values
        {
            const QString NAME_NAME = "name";              // name default identifier
            const QString DESC_NAME = "desc";              // description default identifier
            const QString DESC_DEFAULT = "No description"; // description default value
            const QString WALK_NAME = "walk";              // walk default identifier
            const QString WALK_DEFAULT = "false";          // walk default value
            const QString MAX_NAME = "max";                // max default identifier
            const QString MAX_DEFAULT = "20";              // max default value
            const QString GROUP_NAME = "group";            // group default identifier
            const QString GROUP_DEFAULT = "10";            // group default value
        } // namespace OID_FILE

        } // namespace SNMP

        namespace TIMER // TIMER default configuration values
        {
            const QString SECTION = "timer";      // timer default identifier
            const QString DELAY_NAME = "delay";   // delay default identifier
            const QString DELAY_DEFAULT = "5000"; // delay default value
        } // namespace TIMER
    } // namespace INI

    namespace SNMP // SNMP session default configuration values
    {
        const QString COMMUNITY = "public";  // readcomunity default value
        const QString CALLBACKDELAY = "100"; // callback delay default value
        const QString BULKMAX = "10";        // max bulk request default value
        const QString SEPARATOR = ";";       // default speparator
    } // namespace SNMP

    namespace MQTT // MQTT default configuration values
    {
        const QString ID = "5864";                   // broker ID default value
        const QString TOPIC = "dev/5864/monitoring"; // broker topic default value
    } // namespace MQTT
} // namespace SETTINGS

#endif // BNOS_DEVICES_SNMP_HPP
