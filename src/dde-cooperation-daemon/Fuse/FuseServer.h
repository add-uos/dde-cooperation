#ifndef FUSE_FUSESERVER_H
#define FUSE_FUSESERVER_H

#include <filesystem>
#include <thread>

#include "protocol/fs.pb.h"

#include <QObject>

namespace uvxx {
class Loop;
} // namespace uvxx

class QTcpServer;
class QTcpSocket;

class Machine;

class FuseServer : public QObject {
    Q_OBJECT

public:
    explicit FuseServer(const std::weak_ptr<Machine> &machine);
    ~FuseServer();

    uint16_t port() const;

private:
    std::weak_ptr<Machine> m_machine;

    std::shared_ptr<uvxx::Loop> m_uvLoop;
    QTcpServer *m_listen;
    QTcpSocket *m_conn;

    void handleNewConnection() noexcept;
    void handleDisconnected() noexcept;
    void handleRequest() noexcept;

    void methodGetattr(const FsMethodGetAttrRequest &req, FsMethodGetAttrResponse *resp);
    void methodOpen(const FsMethodOpenRequest &req, FsMethodOpenResponse *resp);
    void methodRead(const FsMethodReadRequest &req, FsMethodReadResponse *resp);
    void methodRelease(const FsMethodReleaseRequest &req, FsMethodReleaseResponse *resp);
    void methodReaddir(const FsMethodReadDirRequest &req, FsMethodReadDirResponse *resp);
};

#endif // !FUSE_FUSESERVER_H