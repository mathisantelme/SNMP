#include "SnmpPlugin.hpp"

// constructeur
// on charge la configuration qui est située dans le fichier settings.ini (spécifié dans globals.hpp) en utilisant le format .ini de qsettings
SnmpPlugin::SnmpPlugin() : _settings(SETTINGS::FILE::PATH, QSettings::IniFormat)
{
    // on définit l'encodage des settings en UTF-8
    _settings.setIniCodec(SETTINGS::FILE::ENCODING.toLocal8Bit().constData());

    // on connecte notre timer avec l'evènement de requete
    QObject::connect(&_timer, &QTimer::timeout, this, &SnmpPlugin::event);

    // TIMER
    // on récupère la valeur du timeout présente dans l'objet settings
    _timer.start(_settings.value(SETTINGS::INI::TIMER::SECTION + "/" + SETTINGS::INI::TIMER::DELAY_NAME, SETTINGS::INI::TIMER::SECTION + "/" + SETTINGS::INI::TIMER::DELAY_DEFAULT).toInt());

    //on démarre le timer
    qDebug() << "Timer delay =" << _timer.remainingTime() << "ms";

    //MQTT CONNECTION
    // on se connecte au broker de messages en utilisant les informations présentes dans l'objet settingss
    ConfigurationConnection config(_settings.value(SETTINGS::INI::MQTT::SECTION + "/" + SETTINGS::INI::MQTT::ADDR_NAME, SETTINGS::INI::MQTT::SECTION + "/" + SETTINGS::INI::MQTT::ADDR_DEFAULT).toString(), _settings.value(SETTINGS::INI::MQTT::SECTION + "/" + SETTINGS::INI::MQTT::PORT_NAME, SETTINGS::INI::MQTT::SECTION + "/" + SETTINGS::INI::MQTT::PORT_DEFAULT).toInt());
    _mqtt = new MQTTConnection(config, SETTINGS::INI::MQTT::SECTION + "/" + SETTINGS::MQTT::ID);
    qDebug() << "Connecting to the Broker...";

    // on connecte la connexion au broker à l'event correspondant
    QObject::connect(_mqtt, &MQTTConnection::connected, this, &SnmpPlugin::MqttConnected);

    // on connecte la déconnexion du borker à l'event correspondant
    QObject::connect(_mqtt, &MQTTConnection::disconnected, this, &SnmpPlugin::MqttDisconnected);

    // on affiche l'addresse de destination ainsi que le port
    qDebug() << "Destination ip :" << config.getAddress();
    qDebug() << "Port :" << config.getPort();

    // on se connecte au broker
    _mqtt->connect();

    //SNMP
    // on définit un chemin de fichier contenant les OIDs / ou on prends le chemin par défaut
    QString iniPath(_settings.value(SETTINGS::INI::SNMP::SECTION + "/" + SETTINGS::INI::SNMP::OID_FILE_PATH_NAME, SETTINGS::INI::SNMP::SECTION + "/" + SETTINGS::INI::SNMP::OID_FILE_PATH_DEFAULT).toString());

    // pour chaque objet listé dans le fichier settings.ini on lit le fichier common.ini (ou un autre spécifié)


    // on récupère les OID et on construit les requetes correspondantes
    iniRead(iniPath);

    // on initialise nos compteurs
    _received = -1;     // le nombre de réponses recues
    _sentFromStart = 0; // le nombre de requetes envoyées
}

// une fonction évènement appellée à chaque timeout du timer
void SnmpPlugin::event()
{
    // si on est connecté au broker
    if (_mqtt->isConnected())
    {
        // si le nombre de réponses recues est null ou différent du nombre de requete envoyées
        if (_received != _oidList.size() && _received != -1)
        {
            // on affiche des warnings
            qWarning() << "Not enough time to get all the requests !";
            qWarning() << "Check the SNMP network connection";
            qFatal("Your SNMP network speed is too slow to handle all the SNMP requests");
        }

        // on réinitialise notre compteur de réponses
        _received = 0;

        // on calcule le temps mit pour lancer les requetes asynchronnes
        long utime = getMicrotime();
        makeRequests(); // on lance les requetes asynchronnes
        qDebug() << "Requests sent ! Took :" << (getMicrotime() - utime) / 1000 << "ms";
    }
    // si on est pas connecté au broker
    else
    {
        qDebug() << "No connection with the broker";
    }
}

// fonction qui permet de créer des objets SNMP à partir d'un fichier de configuration .ini
void SnmpPlugin::iniRead(const QString &file)
{

    // on ouvre le fichier .ini
    QFileInfo checkFile(file);

    // si le fichier n'existe pas on affiche un warning
    if (!(checkFile.exists() && checkFile.isFile()))
        qWarning() << "Error ! Ini file does not exist :" << file;

    // on créé un objet settings qui contient les informations du fichier
    QSettings settings(file, QSettings::IniFormat);



    // on récupère les OID depuis l'objet settings que l'on transforme en liste
    QStringList OIDList(settings.value(SETTINGS::INI::SNMP::OID_LIST_NAME).toStringList());

    // on récupère la taille de la liste
    int size = OIDList.size();

    // Protobuf
    // on créé un objet qui permet de stocker les informations liées à un requete SNMP

    // on nettoie le pack de tout les potentiels objets résiduels
    _pack.clear_obj();

    // on récupère les valeurs de la liste d'OID
    for (int i = 0; i < size; i++)
    {
        //------INI READING------//
        QString const idStr = OIDList.at(i);                                                                                                                                  // on récupère la valeur de l'OID depuis la liste
        QString const nameStr(settings.value(idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::NAME_NAME, "null").toString());                                                     // on récupère le nom correspondant depuis l'objet settings
        QString const descStr(settings.value(idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::DESC_NAME, idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::DESC_DEFAULT).toString());  // on récupère la description correspondante depuis l'objet settings
        QString const walkStr = settings.value(idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::WALK_NAME, idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::WALK_DEFAULT).toString(); // on récupère la valeur de walk (si la requete doit etre exécutée plusieurs fois afin d'obtenir une réponse)
        int maxInt = settings.value(idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::MAX_NAME, idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::MAX_DEFAULT).toInt();                 // on récupère le nombre d'envois de la requete avant l'obtention d'une réponse
        //-----------------------//

        //------PROTOBUF------//
        // on ajoute l'objet nouvellement créé au pack
        SnmpObject *const obj = _pack.add_obj();

        qDebug() << "=======================";
        qDebug() << "OID has been set to " << idStr;
        obj->set_oid(idStr.toUtf8().constData()); // on définit la valeur de l'OID de l'objet courant

        qDebug() << "Name has been set to " << nameStr;
        obj->set_name(nameStr.toUtf8().constData()); // on définit la valeur du nom de l'objet courant

        qDebug() << "Description has been set to " << descStr;
        obj->set_description(descStr.toUtf8().constData()); // on définit la valeur de la description de l'objet courant

        // si on a besoin de faire plusieurs requetes alors on définit la valeur walk de notre objet
        if (walkStr == "true") {
            qDebug() << "Walk value has been set to " << maxInt;
            obj->set_walk(maxInt);
        }

        qDebug() << "=======================";
    }

    // on créé les requetes SNMP à partir des objets créé précédements
    // pour chaque objet du pack
    for (int i = 0; i < _pack.obj_size(); ++i)
    {
        // on définit un nombre de répétiton maximum par défaut pour chaque requete
        int max = 1;

        // si le champ walk de l'objet est définit c'est qu'on a besoin de faire plusieurs envois
        if (_pack.mutable_obj(i)->has_walk())
            max = _pack.mutable_obj(i)->walk(); // on récupère la valeur du nombre d'envois

        // on définit un pointeur vers une requete SNMP
        SnmpRequest *req = Q_NULLPTR;

        // si on à un nombre d'envoi de requete supérieur à 1
        if (max > 1)
        {
            // on récupère l'OID
            QString idStr = OIDList.at(i);

            // on créé une valeur de groupe
            int group = settings.value(idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::GROUP_NAME, idStr + "/" + SETTINGS::INI::SNMP::OID_FILE::GROUP_DEFAULT).toInt();

            // on créé une nouvelle requete multiple
            req = new SnmpMultiRequest(QString::fromStdString(_pack.mutable_obj(i)->oid()), max, group);

            // on affiche un message de confirmation
            qDebug() << "Creating SNMP bulk request ...";
        }
        // si le nombre d'envoi de requete est de 1
        else
        {
            // on créé une requete simple avec l'OID correspondant
            req = new SnmpRequest(QString::fromStdString(_pack.mutable_obj(i)->oid()));
            qDebug() << "Creating SNMP get request ...";
        }

        // on ajoute la requete nouvellement créé à la liste des requetes à effectuer
        _oidList.append(req);

        // on connecte notre requete aux events de callback
        QObject::connect(req, &SnmpRequest::gotResponse, this, &SnmpPlugin::requestsDone);
    }
}

// fonction qui permet d'effectuer les requetes présentes dans la liste
void SnmpPlugin::makeRequests()
{
    // on lance chaque requete présente dans la liste
    for (int i = 0; i < _oidList.size(); ++i)
    {
        _oidList[i]->makeRequest();

        // on affiche un message de confirmation
        qDebug() << "Sending request for OID" << _oidList[i]->getOid();
    }
}

// fonction qui permet de serialiser le contenu du pack
void SnmpPlugin::serialize()
{
    // on sérilize le contenu du pack (utilise Protobuf ?)
    QString serialized = QString::fromStdString(_pack.SerializeAsString());

    // on affiche un message de confiramtion et la valeur obtenue
    qDebug() << "Message serialized :";
    qDebug() << serialized;

    //
    _serialized = serialized;
}

// fonction appellée lorsqu'une requete est terminée
void SnmpPlugin::requestsDone()
{
    // on incrémente notre compteur de callback
    _received++;

    // on affiche un message
    qDebug() << "Received" << _received << "out of" << _oidList.size();

    // si on a recu autant de réponse qu'on a envoyé de requete
    if (_received == _oidList.size())
    {
        // on affiche un message de confirmation
        qDebug() << "All SNMP values received !";

        // pour chacun des OID de la liste
        for (int i = 0; i < _oidList.size(); ++i)
        {
            // on récupère les valeurs de l'OID
            QString valueStr = _oidList[i]->getValue();

            // on change la valeur de l'objet correspondant
            _pack.mutable_obj(i)->set_value(valueStr.toUtf8().constData());

            // on affiche un message de confirmation
            qDebug() << "Value for" << QString::fromStdString(_pack.mutable_obj(i)->name()) << "has been set to :" << QString::fromStdString(_pack.mutable_obj(i)->value());
        }

        // on calcule et on affiche le temps mit pour la sérialization
        long utime = getMicrotime();
        serialize(); // on sérialize les données
        qDebug() << "Serialization done ! Took :" << (float)((getMicrotime() - utime) / 100 + 1) / 10. << "ms"
                 << " |  Size :" << (float)((_serialized.size() / 100) + 1) / 10. << "kB";

        //---- MQTT sending message ----
        // on convertit notre valeur sérializé en tableau de bits
        QByteArray byteArray = _serialized.toUtf8();

        // si on est connecté au Broker
        if (_mqtt->isConnected())
        {
            // on essaie de push le message sur le broker
            qDebug() << "Sending a message to the broker";

            // si on y arrive alors on affiche un message de confirmation
            if (_mqtt->publish(SETTINGS::MQTT::TOPIC, byteArray) >= 0)
                qDebug() << "Success !";
            // si on arrive pas a push le message on affiche un message d'erreur
            else
                qDebug() << "Failure !";
        }
        // si on est pas connecté au broker on affiche un message d'erreur
        else
        {
            qDebug() << "No connection with the broker...";
        }
        //------------------------------
    }
}

// fonction qui retourne le nombre de requete envoyées depuis le début
long SnmpPlugin::getSentFromStart()
{
    return _sentFromStart;
}

// evenement appellé lorsque l'on est connecté au broker
void SnmpPlugin::MqttConnected() const
{
    qDebug() << "Connected to the MQTT Broker !";
}

// evenemtn appellé lorsque l'on est deconnecté du broker
void SnmpPlugin::MqttDisconnected() const
{
    qDebug() << "Disconnected from the MQTT Broker !";
}
