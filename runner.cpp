#include "runner.h"

#include <QSqlDatabase>
#include <QString>
#include <QDebug>
#include <QSqlQuery>
#include <QStringList>
#include <QProcess>
#include <QTextStream>
#include <QTimer>

Runner::Runner(QCoreApplication* app) : fileMapPath("/home/saman/.local/share/baloo/file/fileMap.sqlite3"),
                                        fileMetaDataPath("/home/saman/.local/share/baloo/file/fileMetaData.sqlite3"),
                                        qCout(stdout),
                                        setfattr(this),
                                        totalTasks(0),
                                        completedTasks(0)
{
    this->app = app;
    init();
}

void Runner::init()
{
    fileMapDb = QSqlDatabase::addDatabase("QSQLITE", "fileMapDb");
    fileMapDb.setDatabaseName(fileMapPath);
    if (!fileMapDb.open())
    {
        qDebug() << "An error occured";
        app->exit(1);
    }

    fileMetaData = QSqlDatabase::addDatabase("QSQLITE", "fileMetaData");
    fileMetaData.setDatabaseName(fileMetaDataPath);
    if (!fileMetaData.open())
    {
        qDebug() << "Cannot open metadata";
        app->exit(1);
    }
    connect(&setfattr, SIGNAL(finished(int,QProcess::ExitStatus)),SLOT(commandFinished(int,QProcess::ExitStatus)));
}

void Runner::run()
{
    QSqlQuery queryFile(fileMapDb);
    queryFile.exec("select * from files");
    while (queryFile.next())
    {
        int fileId = queryFile.value("id").toInt();
        QString filePath = queryFile.value("url").toString();
        QSqlQuery queryMetaData(fileMetaData);
        QString query = QString("select * from files where id = %1").arg(fileId);
        queryMetaData.exec(query);
        while (queryMetaData.next())
        {
            QString propertyStr = queryMetaData.value("property").toString();
            QString valueStr = queryMetaData.value("value").toString();
            if (propertyStr.isEmpty() || valueStr.isEmpty() || filePath.isEmpty())
            {
                qDebug() << "error";
                app->exit(2);
            }
            QStringList args;
            args << "-n" << propertyStr << "-v" << valueStr << filePath;
            //qDebug() << args;
            addToQueue(args);
        }
    }
    if (commandArgs.isEmpty())
        QTimer::singleShot(0, this, SIGNAL(finished()));
}

void Runner::addToQueue(QStringList& stringList)
{
    if (commandArgs.isEmpty())
        QTimer::singleShot(0, this, SLOT(runNextCommand()));
    commandArgs.enqueue(stringList);
    ++totalTasks;
}

void Runner::runNextCommand()
{
    if (commandArgs.isEmpty())
    {
        QString message("%1/%2 tasks completed successfully");
        message = message.arg(completedTasks).arg(totalTasks);
        qCout << message << endl;
        emit finished();
        return;
    }
    QStringList stringList = commandArgs.dequeue();
    qDebug() << stringList;
    setfattr.start("setfattr", stringList);
}

void Runner::commandFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus != QProcess::NormalExit)
    {
        QString message("There is a problems. exit code is: %1");
        message = message.arg(exitCode);
        qDebug() << message;
        app->exit(exitCode);
    }
    else
        ++completedTasks;
    runNextCommand();
}
