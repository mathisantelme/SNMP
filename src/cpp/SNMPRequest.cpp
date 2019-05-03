#include "SNMPRequest.hpp"

const Snmp_pp::CTarget *SnmpRequest::_ctarget()
{
    // c'est dans cette section qu'il va falloir charger les différentes addresse  pour pouvoir s'addresser aux différents objets
    // en gros créer un objet ctarget pour chaque objet.
    // on va récupérer un objet en paramètre qui contient l'addresse, le nom de communauté, le délai de timeout et le nombre d'essais
    // on peut essayer avec un msg protobuff
    // ou avec une liste de strings

    // on se chargera de créer l'objet ctarget correspondant aux valeurs passées par SNMPplugin

    // on charge l'objet settings avec les valeurs du fichier de description de configuration
    QSettings settings(SETTINGS::FILE::PATH, QSettings::IniFormat);

    // on définit l'addresse de la requete depuis l'objet settings
    Snmp_pp::UdpAddress address(settings.value(SETTINGS::INI::SNMP::SECTION + "/" + SETTINGS::INI::SNMP::ADDR_NAME, SETTINGS::INI::SNMP::ADDR_DEFAULT).toByteArray().toStdString().c_str());

    //----Defining target----
    // pour les version 1 et 2 de SNMP, on a besoin d'une classe CTarget
    // on instancie un objet CTarget avec l'addresse définie plus haut
    Snmp_pp::CTarget *ctarget = new Snmp_pp::CTarget(address);

    ctarget->set_version(Snmp_pp::version2c);                                                                                                                        // on initialise la version de SNMP
    ctarget->set_readcommunity(SETTINGS::SNMP::COMMUNITY.toLocal8Bit().constData());                                                                                 // on initialise la valeur de community avec la valeur présente dans globals.hpp
    ctarget->set_retry(settings.value(SETTINGS::INI::SNMP::SECTION + "/" + SETTINGS::INI::SNMP::RETRY_NAME, SETTINGS::INI::SNMP::RETRY_DEFAULT).toInt());            // on initialise le nombre de nouvel envoi de la requete apres un timeout
    ctarget->set_timeout(settings.value(SETTINGS::INI::SNMP::SECTION + "/" + SETTINGS::INI::SNMP::TIMEOUT_NAME, SETTINGS::INI::SNMP::TIMEOUT_DEFAULT).toLongLong()); // on initialise le delai de timeout
    //-------------------------

    // on retourne l'objet créé
    return ctarget;
};

/*
 * CONSTRUCTOR
 */
SnmpRequest::SnmpRequest(const QString OID) : _OID(OID)
{
    // on définit la valeur par défaut de la requete comme etant nulle
    _value = "null";

    // on créé une session SNMP++
    int status;
    _snmp = new Snmp_pp::Snmp(status);

    // si on y arrive pas on créé un warning
    if (status != SNMP_CLASS_SUCCESS)
        qWarning() << "Could not create SNMP++ session:";

    // on connect le timeout du timer à la methode de rafraichissement
    QObject::connect(&_timer, &QTimer::timeout, this, &SnmpRequest::timerExpired);
    _timer.start(SETTINGS::SNMP::CALLBACKDELAY.toInt()); // on lance le timer avec le delai de callback
}

// C Callback function for snmp++
void callback(int reason, Snmp_pp::Snmp *snmp, Snmp_pp::Pdu &pdu, Snmp_pp::SnmpTarget &target, void *cd)
{
    if (cd)
    {
        // just call the real callback member function...
        ((SnmpRequest *)cd)->async_callback(reason, snmp, pdu, target);
    }
}

// fonction qui permet d'effectuer les requetes (on y passe l'addresse de la cible de la requete)
void SnmpRequest::makeRequest()
{

    // on récupère la valeur de l'OID
    Snmp_pp::Oid snmpOid(_OID.toLocal8Bit().constData());

    Snmp_pp::Pdu pdu; // le type de requete
    Snmp_pp::Vb vb;   // variable bindings

    // on définit l'OID du VB
    vb.set_oid(snmpOid);
    // on ajoute le VB au PDU
    pdu += vb;

    // on lance la requete et on vérifie que l'obtient pas d'erreur
    int status = _snmp->get(pdu, (Snmp_pp::SnmpTarget &)*SnmpRequest::_ctarget, callback, this);

    // si on obtient une erreur alors on affiche un warning et on définit la valeur à "error"
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

// fonction de callback asynchrone
void SnmpRequest::async_callback(int reason, Snmp_pp::Snmp *snmp, Snmp_pp::Pdu &pdu, Snmp_pp::SnmpTarget &target)
{
    // on indique au compilateur qu'on utilise pas les parametres suivants
    Q_UNUSED(snmp);
    Q_UNUSED(target);

    // on analyse la raison du callback
    if (reason == SNMP_CLASS_ASYNC_RESPONSE) // si il s'agit d'une réponse à une requete asynchronne
    {
        // on affiche un message
        qDebug() << "Response !";

        // si le PDU ne contient aucun VB alors on affiche un warning
        if (pdu.get_vb_count() == 0)
            qWarning() << "Error : Pdu is empty";
        // si le PDU contient un VB
        else
        {
            // on récupère la valeur du VB dans une variable
            Snmp_pp::Vb vbb;
            pdu.get_vb(vbb, 0);

            // on convertit la valeur du VB en str
            QString valueStr = vbb.get_printable_value();

            // si la str n'est pas vide on modifie la valeur de la requete
            if (!valueStr.isEmpty())
                _value = valueStr;

            // On affiche l'OID et la valeur recue
            qDebug() << "For this OID :" << _OID;
            qDebug() << "Found this value :" << _value;
        }

        // on appelle le signal gotResponse
        emit gotResponse();
    }

    // si il s'agit d'un timeout, on affiche un warning
    else if (reason == SNMP_CLASS_TIMEOUT)
        qWarning() << "Timeout  !";

    // si il s'agit d'une raison différente, on affiche un warning
    else
        qWarning() << "Did not receive any async response/trap.";
}

// permet de retourner l'OID de la requete
QString SnmpRequest::getOid() const
{
    return _OID;
}

// permet de retourner la valeur de la requete
QString SnmpRequest::getValue() const
{
    return _value;
}
