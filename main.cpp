#include <QCoreApplication>
#include <QTimer>

#include "runner.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Runner runner(&a);
    runner.run();
    QObject::connect(&runner, SIGNAL(finished()), &a, SLOT(quit()));
    return a.exec();
}
