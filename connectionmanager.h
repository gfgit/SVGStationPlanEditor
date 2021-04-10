#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QObject>
#include <list>
#include <QMutex>

namespace pqxx {
class connection;
} // namespace pqxx

class ConnectionManager;

typedef struct ConnectionPtr_
{
public:
    inline bool isVald() const { return conn; }
    inline operator pqxx::connection& () { return *conn; }

public:
    inline ConnectionPtr_() :mgr(nullptr), conn(nullptr), ref(nullptr) {};
    ConnectionPtr_(ConnectionPtr_ &other);
    ~ConnectionPtr_();
    ConnectionPtr_ &operator =(ConnectionPtr_& other);

private:
    void deref();
    inline void incref() { if(ref) (*ref)++; }

    friend class ConnectionManager;
    ConnectionManager *mgr;
    pqxx::connection *conn;
    int *ref;
} ConnectionPtr;

class ConnectionManager : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionManager(QObject *parent = nullptr);
    ~ConnectionManager();

    void cleanupConnections(bool force = false);

    void setConnectionOptions(const QByteArray& options);

    ConnectionPtr getConnection();

    bool event(QEvent *e) override;

private:
    friend struct ConnectionPtr_;
    static constexpr int kCleanUpTimerInteval_ms = 5000; //Every 5 seconds
    static constexpr int kCleanUpAfter_ms = 3000; //Every 5 seconds
    static constexpr int MaxFreeConnections = 3;

    inline void stopCleanUpTimer()
    {
        if(cleanUpTimerId)
        {
            killTimer(cleanUpTimerId);
            cleanUpTimerId = 0;
        }
    }

    void releaseConnection(ConnectionPtr_ &c);

private:
    int cleanUpTimerId;
    QByteArray connectionOptions;
    QMutex mutex;

    struct ConnectionRef
    {
        pqxx::connection *conn = nullptr;
        int timeMs;
    };

    //Contains released connections waiting to be cleaned or reused
    ConnectionRef connectionPool[MaxFreeConnections];

    std::list<pqxx::connection *> activeConnections;
};

#endif // CONNECTIONMANAGER_H
