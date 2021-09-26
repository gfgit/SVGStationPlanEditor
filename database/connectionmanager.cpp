#include "connectionmanager.h"

#include <pqxx/connection>

#include <QTimerEvent>
#include <QCoreApplication>

#include <QTime>

ConnectionManager::ConnectionManager(QObject *parent) :
    QObject(parent),
    cleanUpTimerId(0),
    connectionPool{}
{

}

ConnectionManager::~ConnectionManager()
{
    cleanupConnections(true);

    QMutexLocker locker(&mutex);
    for(int i = 0; i < MaxFreeConnections; i++)
    {
        connectionPool[i].conn = nullptr;
    }
    for(auto it = activeConnections.begin(); it != activeConnections.end(); it++)
    {
        delete *it;
    }
    activeConnections.clear();
}

void ConnectionManager::cleanupConnections(bool force)
{
    int pending = 0;

    int timeMs = QTime::currentTime().msecsSinceStartOfDay();

    QMutexLocker locker(&mutex);
    for(int i = 0; i < MaxFreeConnections; i++)
    {
        if(connectionPool[i].conn)
        {
            if(force || timeMs - connectionPool[i].timeMs >= kCleanUpAfter_ms)
            {
                for(auto it = activeConnections.begin(); it != activeConnections.end(); it++)
                {
                    if(*it == connectionPool[i].conn)
                    {
                        //Remove from activeConnections
                        activeConnections.erase(it);
                        break;
                    }
                }

                delete connectionPool[i].conn;
                connectionPool[i].conn = nullptr;
                continue;
            }

            pending++;
        }
    }

    if(pending == 0) //No more connection waiting to be cleaned or reused
        stopCleanUpTimer();
}

void ConnectionManager::setConnectionOptions(const QByteArray &options)
{
    QMutexLocker locker(&mutex);
    connectionOptions = options;
}

ConnectionPtr ConnectionManager::getConnection()
{
    QMutexLocker locker(&mutex);

    ConnectionPtr ptr;
    ptr.mgr = this;
    ptr.conn = nullptr;
    ptr.ref = new int(1);

    //Try to get a free connection
    for(int i = 0; i < MaxFreeConnections; i++)
    {
        if(connectionPool[i].conn)
        {
            ptr.conn = connectionPool[i].conn;
            connectionPool[i].conn = nullptr;
            break;
        }
    }

    if(!ptr.conn)
    {
        //Create a new connection
        try
        {
            ptr.conn = new pqxx::connection(connectionOptions.constData());
            activeConnections.emplace_back(ptr.conn);
        }
        catch (std::exception &e)
        {
            throw e;
        }

    }

    return ptr;
}

bool ConnectionManager::event(QEvent *e)
{
    if(e->type() == QEvent::Timer)
    {
        QTimerEvent *ev = static_cast<QTimerEvent *>(e);
        if(ev->timerId() == cleanUpTimerId)
            cleanupConnections();
        return true;
    }
    else if(e->type() == QEvent::User)
    {
        QMutexLocker locker(&mutex);
        if(!cleanUpTimerId)
        {
            //Start timer
            //Qt doesn't support starting timers from different thread
            //This object lives in main thread so start timer here
            cleanUpTimerId = startTimer(kCleanUpTimerInteval_ms);
        }
        return true;
    }

    return QObject::event(e);
}

void ConnectionManager::releaseConnection(ConnectionPtr_ &c)
{
    if(!c.isVald())
        return;

    QMutexLocker locker(&mutex);

    //Try to store it in free connections
    for(int i = 0; i < MaxFreeConnections; i++)
    {
        if(!connectionPool[i].conn)
        {
            connectionPool[i].conn = c.conn;
            connectionPool[i].timeMs = QTime::currentTime().msecsSinceStartOfDay();
            c.conn = nullptr;

            //Send 'start timer' event, it must be started in main thread
            if(!cleanUpTimerId)
                qApp->postEvent(this, new QEvent(QEvent::User));
            break;
        }
    }

    //If not, delete it
    if(c.conn)
    {
        for(auto it = activeConnections.begin(); it != activeConnections.end(); it++)
        {
            if(*it == c.conn)
            {
                //Remove from activeConnections
                activeConnections.erase(it);
                break;
            }
        }

        delete c.conn;
        c.conn = nullptr;
    }
}

ConnectionPtr_::ConnectionPtr_(ConnectionPtr_ &other)
{
    mgr = other.mgr;
    conn = other.conn;
    ref = other.ref;
    incref();
}

ConnectionPtr_& ConnectionPtr_::operator=(ConnectionPtr_ &other)
{
    deref();
    mgr = other.mgr;
    conn = other.conn;
    ref = other.ref;
    incref();
    return *this;
}

ConnectionPtr_::~ConnectionPtr_()
{
    deref();
}

void ConnectionPtr_::deref()
{
    if(ref)
    {
        int &r = *ref;
        r--;
        if(!r)
        {
            mgr->releaseConnection(*this);
            delete ref;
            ref = nullptr;
            mgr = nullptr;
        }
    }
}
