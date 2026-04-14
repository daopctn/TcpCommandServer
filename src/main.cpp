#include <QCoreApplication>
#include "streamserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    StreamServer server;

    return app.exec();
}
