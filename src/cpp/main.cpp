#include <QDebug>
#include "SnmpPlugin.hpp"
#include "Protobuf.pb.h"
#include <QCoreApplication>

int main(int argc, char **argv)
{
    // on lance une nouvelle application
    QCoreApplication a(argc, argv);

    // on vérifie la version de protobuf
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    // on lance notre plugin de SNMP (qui ne se connecte qu'a un seul "agent" pour le moment)
    // le but du projet est de permettre à un seul plugin de se connecter à N agents
    SnmpPlugin *req = new SnmpPlugin();

    // on informe le compilateur que "req" n'est pas utilisé
    Q_UNUSED(req);

    // on attent la fin de l'exécution de l'application
    return a.exec();
}
