#include "mainwindow.h"

#include <QApplication>

#include <pqxx/pqxx>
#include "connectionmanager.h"

#include <QThreadPool>

#include <QDebug>

/* NOTE: to connect ssh tunnel to VirtualBox issue command
 * ssh -L 65530:localhost:5432 filippo@127.0.0.1 -p 2522
 *
 * 65530 is local port here
 * localhost:5432 is the end point in the other machine
 * filippo@127.0.0.1 is Ubunt user and host
 * 2522 is the SSH port mapping done by VirtualBox
 *
 * Here we connect to localhost:65530
 * The server see us as localhost:5432
 */

ConnectionManager *globalMgr;

void func()
{
    try
    {
        ConnectionPtr C = globalMgr->getConnection();

        pqxx::read_transaction W(C);
        pqxx::result res = W.exec("SELECT id,name FROM fremo.stations");
        for(const pqxx::row &row : res)
        {
            qDebug() << row[0].c_str() << row[1].c_str();
        }

        W.commit();
    }
    catch (std::exception const &e)
    {
        qDebug() << e.what();
    }

    try
    {
        ConnectionPtr C = globalMgr->getConnection();
    }
    catch (std::exception const &e)
    {
        qDebug() << e.what();
    }
};


int main(int argc, char *argv[])
{
    QApplication::setOrganizationName(QLatin1String("Filippo"));
    QApplication::setApplicationDisplayName(QLatin1String("SVG Station Plan Viewer"));
    QApplication a(argc, argv);

//    ConnectionManager mgr;
//    globalMgr = &mgr;
//    mgr.setConnectionOptions("host=localhost port=65530 dbname=fremo1 user=filippo password=Magritte.12");

//    for(int i = 0; i < 2; i++)
//        QThreadPool::globalInstance()->start(func);

//    ConnectionPtr keepAlive;
//    try
//    {
//        ConnectionPtr C = mgr.getConnection();
//        keepAlive = C;

//        pqxx::nontransaction W(C);
//        pqxx::result res = W.exec("SELECT id,name FROM fremo.stations");
//        for(const pqxx::row &row : res)
//        {
//            qDebug() << row[0].c_str() << row[1].c_str();
//        }
//        W.exec("UPDATE fremo.stations SET name='Bologna' WHERE id=6");

//        res = W.exec("DROP SCHEMA public");
//        for(const pqxx::row &row : res)
//        {
//            qDebug() << row[0].c_str() << row[1].c_str();
//        }
//        W.commit();
//    }
//    catch (std::exception const &e)
//    {
//        qDebug() << e.what();
//    }

    MainWindow w;
    w.show();
    int ret = a.exec();
    QThreadPool::globalInstance()->waitForDone();
    return ret;
}
