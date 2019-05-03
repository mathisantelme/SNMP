#include <QDebug>

#ifndef BNOS_DEVICES_SNMP_HPP
#define BNOS_DEVICES_SNMP_HPP

namespace SETTINGS
{

//CONFIGURATION OF FILE
namespace FILE
{
const QString PATH = "config/settings.ini"; // le fichier de configuration par défaut
const QString ENCODING = "UTF-8";           // l'encodage du fichier par défaut
} // namespace FILE

//INI FILE READING (ini sections, names and default values)
namespace INI
{
namespace MQTT
{
const QString SECTION = "mqtt";           // section par défaut
const QString ADDR_NAME = "ip";           // le nom d'addresse par défaut
const QString ADDR_DEFAULT = "127.0.0.1"; // l'addresse IP par défaut
const QString PORT_NAME = "port";         // le nom de port par défaut
const QString PORT_DEFAULT = "1883";      // le port par défaut
} // namespace MQTT

// on définit les valeurs par défaut des objets SNMP
namespace SNMP
{
const QString SECTION = "snmp";                         // la section par défaut
const QString ADDR_NAME = "ip";                         // le nom d'addresse par défaut
const QString ADDR_DEFAULT = "127.0.0.1";               // l'addresse par défaut
const QString TIMEOUT_NAME = "timeout";                 // le nom du timeout par défaut
const QString TIMEOUT_DEFAULT = "3000";                 // la valeur de timeout par défaut
const QString OID_FILE_PATH_NAME = "filename";          // le nom du fichier de configuration OID par défaut
const QString OID_FILE_PATH_DEFAULT = "config/OID.ini"; // le fichier de configuration OID par défaut
const QString OID_LIST_NAME = "OIDs";
const QString RETRY_NAME = "autoretries";
const QString RETRY_DEFAULT = "10";

// le fichier de description des OID
namespace OID_FILE
{
const QString NAME_NAME = "name";              // le nom par défaut
const QString DESC_NAME = "desc";              // le nom de description par défaut
const QString DESC_DEFAULT = "No description"; // la description par défaut
const QString WALK_NAME = "walk";              // le nom de la valeur de walk par défaut
const QString WALK_DEFAULT = "false";          // la valeur de walk par défaut
const QString MAX_NAME = "max";                // le nom du nombre de répétitions par défaut
const QString MAX_DEFAULT = "20";              //  le nombre de répétitions par défaut
const QString GROUP_NAME = "group";            // le nom de la valeur de groupe par défaut
const QString GROUP_DEFAULT = "10";            // la valeur de groupe par défaut
} // namespace OID_FILE

} // namespace SNMP
//TIMER
namespace TIMER
{
const QString SECTION = "timer";      // le nom de la section
const QString DELAY_NAME = "delay";   // le nom de la valeur de delai par defaut
const QString DELAY_DEFAULT = "5000"; // la valeur de delai (timeout) par defaut
} // namespace TIMER
} // namespace INI

//SNMP
namespace SNMP
{
const QString COMMUNITY = "public";  // la valeur de "community" par defaut
const QString CALLBACKDELAY = "100"; // la valeur du delai de callback par défaut
const QString BULKMAX = "10";        // la valeur maximale de requete BULK par défaut
const QString SEPARATOR = ";";       // le séparateur par défaut
} // namespace SNMP

//MQTT
namespace MQTT
{
const QString ID = "5864";                   // l'id du broker par défaut
const QString TOPIC = "dev/5864/monitoring"; // le topic du borker par défaut
} // namespace MQTT
} // namespace SETTINGS

#endif // BNOS_DEVICES_SNMP_HPP
