#include <QCoreApplication>
#include "commandserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    CommandServer server;

    return app.exec();
}
