#include "SnmpMultiRequest.hpp"

/*
 * CONSTRUCTOR
 */
SnmpMultiRequest::SnmpMultiRequest(const QString OID, const int MAXI, const int GROUP) : SnmpRequest(OID), _MAX(MAXI), _GROUP(GROUP)
{
     _remain = _MAX; // on initialise le nombre d'envoi de requete maximum tant qu'on a pas recu de réponse
     _lastOid = OID; // on initialise le dernier OID
}

// C Callback function for snmp++
void bulkCallback(int reason, Snmp_pp::Snmp *snmp, Snmp_pp::Pdu &pdu, Snmp_pp::SnmpTarget &target, void* cd)
{
  if (cd)
  {
    // just call the real callback member function...
    ((SnmpMultiRequest*)cd)->SnmpMultiRequest::async_callback(reason, snmp, pdu, target);
  }
}

// fonction qui permet d'effectuer une requete
void SnmpMultiRequest::makeRequest(){
    // si il ne reste aucun envoi de requete à effectuer
    if(_remain <= 0){
        // on réinitialise les valeurs du nombre d'envoi et de l'OID
        _remain = _MAX;
        _lastOid = _OID;
    }

    // on récupère l'OID
    Snmp_pp::Oid snmpOid(_lastOid.toLocal8Bit().constData());

    Snmp_pp::Pdu pdu; // le type de requete
    Snmp_pp::Vb vb; // le variable binding

    // on affiche un message de confirmation
    qDebug() << "Bulk request processing...";

    // on définit le VB
    vb.set_oid(snmpOid);
    // on ajoute le VB dans le PDU
    pdu += vb;

    // on récupère le nombre de requete à effectuer
    int num = _GROUP;

    // si le nombre de requete qu'il reste à effectuer est inférieur au nombre de requete total, on change la valeur du nombre de requete
    if(_remain <= _GROUP)
        num = _remain;

    // on lance la requete et on vérifie qu'on a pas d'erreur
    int status = _snmp->get_bulk(pdu,(Snmp_pp::SnmpTarget&)* SnmpRequest::_ctarget, 0, num, bulkCallback, this);

    // si on a une erreur alors on affiche un warning et on change la valeur à null
    if (status < 0) {
        qWarning() << "Error while trying to get the OID, status = " << status;
        _value = "error";
    }
}

// fonction de callback asynchronne
void SnmpMultiRequest::async_callback(int reason, Snmp_pp::Snmp * snmp, Snmp_pp::Pdu &pdu, Snmp_pp::SnmpTarget &target)
{
    // on indique au compilateur que l'on ne sert pas des parametres suivants
    Q_UNUSED(snmp);
    Q_UNUSED(target);

    // on vérifie les raisons de la réponse
    // si c'est une réponse d'une requete asynchronne
    if (reason == SNMP_CLASS_ASYNC_RESPONSE)
    {
        // si il reste des requetes à effectuer
        if(_remain >= 0){
            // on affiche le nombre de réponse recues
            qDebug() << "Bulk !";
            qDebug() << "Received" << pdu.get_vb_count() <<" response(s)";

            // si le PDU est vide on affiche un warning
            if (pdu.get_vb_count() == 0)
                qWarning() << "Error : Pdu is empty";
            // si le PDU contient au moins un VB
            else {
                // pour chaque VB dans le PDU
                for(int i=0; i<pdu.get_vb_count(); i++){

                    // on récupère le VB courant
                    Snmp_pp::Vb vbb;
                    pdu.get_vb(vbb,i);

                    // on vérifie si il s'agit d'un noeud fils de l'OID courant
                    if(QString::fromStdString(vbb.get_printable_oid()).prepend(".").contains(_OID)){

                        // on récupère la valeur lisible du VB courant
                        QString valueStr = vbb.get_printable_value();

                        // si la valeur n'est pas nulle
                        if(!valueStr.isEmpty()){
                            // si on est pas à la premiere reponse, on ajoute un séparateur avant de concaténer
                            if (_remain != _MAX)
                                _value += SETTINGS::SNMP::SEPARATOR;

                            // si la valeur est nulle, on supprime la valeur par défaut avant de concaténer
                            else
                                _value = "";


                            // on affiche la valeur obtenue
                            qDebug() << "Adding this :" << valueStr;

                            // on concatène la valeur courante avec les valeurs précédentes
                            _value += valueStr;
                        }
                        // si la valeur est nulle alors on affiche un warning
                        else
                            qWarning() << "Empty response !";

                        // on affiche l'OID et la valeur trouvée
                        qDebug() << "For this OID :" << _OID;
                        qDebug() << "Found this value :" << _value;
                    }
                    // si on est pas dans un noeud fils (on a terminé)
                    else {
                        // on affiche des messages
                        qDebug() << "This OID is not a child :" << vbb.get_printable_oid();
                        qDebug() << "Ending SNMP walk request !";
                        _remain = 1; // on met à jour le nombre de requetes restantes à pour quitter la boucle
                    }

                    _remain --; // on décrémente le nombre de requetes restantes

                    // on affiche le progres effectué
                    qDebug() << "Bulk progress :" << _MAX-_remain << "/" << _MAX << "    (" << (_MAX-_remain)*100/_MAX << "% )";

                    // si il ne reste aucune requete
                    if (_remain <= 0) {
                        emit gotResponse(); // on appelle le signal gotResponse
                        break; // on quitte la boucle
                    }
                    // si on est rendu à l'avant-dernier VB
                    else if (i == pdu.get_vb_count()-1){
                        // on affiche tout les OIDs recus
                        qDebug() << "Received this OIDs with bulk :";

                        // pour chacun des VB
                        for(int i=0; i<pdu.get_vb_count(); i++){
                            // on récupère l'OID du VB et on l'affiche
                            pdu.get_vb(vbb,i);
                            qDebug() << "-->" << vbb.get_printable_oid();
                        }

                        // on informe l'utilisateur qu'on lance une nouvelle requete en BULK
                        qDebug() << "Launching next bulk request";

                        // on récupère le nouvel OID (le dernier des VB)
                        _lastOid = vbb.get_printable_oid();

                        // on lance les requetes
                        makeRequest();
                    }
                }
            }
        }
    }
    // si c'est un timeout, on affiche un warning
    else if (reason == SNMP_CLASS_TIMEOUT)
        qWarning() << "Timeout  !";
    // si la raison n'est pas reconnue, on affiche un warning
    else
        qWarning() << "Did not receive any async response/trap.";
}
