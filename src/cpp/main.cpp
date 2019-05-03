#include <QDebug>
#include "SnmpPlugin.hpp"
#include "Protobuf.pb.h"
#include <QCoreApplication>

int main(int argc, char **argv)
{
    // starting new app
    QCoreApplication a(argc, argv);

    // verifying protobuf version
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    // laucnhing SNMPplugin
    SnmpPlugin *req = new SnmpPlugin();

    // notifying the compiler that req isn't used in this function body
    Q_UNUSED(req);

    // waiting for the end of the app
    return a.exec();
}
