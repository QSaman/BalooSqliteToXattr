#ifndef RUNNER_H
#define RUNNER_H

#include <QObject>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QProcess>
#include <QStringList>
#include <QQueue>
#include <QTextStream>

class Runner : public QObject
{
    Q_OBJECT
public:
    Runner(QCoreApplication* app);
    void run();
signals:
    void finished();
private slots:
    void runNextCommand();
    void commandFinished(int exitCode, QProcess::ExitStatus exitStatus);
private:
    void init();
    void addToQueue(QStringList& stringList);

    QCoreApplication * app;
    const QString fileMapPath;
    const QString fileMetaDataPath;
    QTextStream qCout;
    QSqlDatabase fileMapDb;
    QSqlDatabase fileMetaData;
    QProcess setfattr;
    QQueue<QStringList> commandArgs;
    int totalTasks;
    int completedTasks;
};

#endif // RUNNER_H
