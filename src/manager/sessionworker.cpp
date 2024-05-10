// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionworker.h"

#include "sessionmanager.h"

#include "common/logger.h"

#include <QHostInfo>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QStorageInfo>

SessionWorker::SessionWorker(const std::shared_ptr<AsioService> &service, QObject *parent)
    : QObject(parent)
    , _service(service)
{
}

void SessionWorker::onReceivedMessage(const proto::OriginMessage &request, proto::OriginMessage *response)
{
    if (request.json_msg.empty()) {
        DLOG << "empty json message: ";
        return;
    }

    // Step 2: 解析响应数据
    picojson::value v;
    std::string err = picojson::parse(v, request.json_msg);
    if (!err.empty()) {
        DLOG << "Failed to parse JSON data: " << err;
        return;
    }

    // if these exten mssages are handled, return
    if (_extMsghandler) {
        std::string res_json;
        bool handled = _extMsghandler(request.mask, v, &res_json);
        if (handled) {
            response->id = request.id;
            response->mask = 1;
            response->json_msg = res_json;
            return;
        }
    }

//    picojson::array files = v.get("files").get<picojson::array>();
    int type = request.mask;
    switch (type) {
    case REQ_LOGIN: {
        LoginMessage req, res;
        req.from_json(v);
        DLOG << "Login: " << req.name << " " << req.auth;
        std::hash<std::string> hasher;
        uint32_t pinHash = (uint32_t)hasher(_savedPin.toStdString());
        uint32_t pwdnumber = std::stoul(req.auth);
        if (pinHash == pwdnumber) {
            res.auth = "thatsgood";
            response->mask = LOGIN_SUCCESS;
        } else {
            // return empty auth token.
            res.auth = "";
            response->mask = LOGIN_DENIED;
        }
        res.name = QHostInfo::localHostName().toStdString();
        response->json_msg = res.as_json().serialize();
    }
    break;
    case REQ_FREE_SPACE: {
        FreeSpaceMessage req, res;
        req.from_json(v);

        int remainSpace = 0;//TransferHelper::getRemainSize(); // xx G
        res.free = remainSpace;
        response->json_msg = res.as_json().serialize();
    }
    break;
    case REQ_TRANS_DATAS: {
        TransDataMessage req, res;
        req.from_json(v);

//        bool isdir = req.flag;
//        DLOG << "trans data: " << req.token << " " << isdir;
//        DLOG << "Names: ";
//        for (const auto &name : req.names) {
//            DLOG << name << " ";
//        }

        res.id = req.id;
        res.names = req.names;
        res.flag = true;
        res.size = 0;//TransferHelper::getRemainSize();
        response->json_msg = res.as_json().serialize();

        emit onTransData(req.names);
    }
    break;
    case REQ_TRANS_CANCLE: {
        TransCancelMessage req, res;
        req.from_json(v);

        DLOG << "cancel name: " << req.name << " " << req.reason;

        res.id = req.id;
        res.name = req.name;
        res.reason = "";
        response->json_msg = res.as_json().serialize();

        emit onCancelJob(req.id);
    }
    break;
    case CAST_INFO: {
    }
    break;
    default:
        DLOG << "unkown type: " << type;
        break;
    }

    response->id = request.id;
    response->mask = 1;
}

void SessionWorker::onStateChanged(int state, std::string msg)
{
//    RPC_ERROR = -2,
//    RPC_DISCONNECTED = -1,
//    RPC_DISCONNECTING = 0,
//    RPC_CONNECTING = 1,
//    RPC_CONNECTED = 2,
    bool result = false;
    switch (state) {
    case RPC_CONNECTED: {
        _connectedAddress = QString(msg.c_str());
        DLOG << "connected remote: " << msg;
        result = true;
        // update save dir
//        _saveDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + QDir::separator() + _connectedAddress;
    }
    break;
    case RPC_DISCONNECTED: {
        DLOG << "disconnected remote: " << msg;
    }
    break;
    case RPC_ERROR: {
        DLOG << "error remote code: " << msg;
        int code = std::stoi(msg);
        if (asio::error::host_unreachable == code) {
            DLOG << "ping failed: " << msg;
        }
    }
    break;
    default:
        DLOG << "other handling CONNECTING or DISCONNECTING: " << msg;
        break;
    }

    emit onConnectChanged(state, QString(msg.c_str()));
}

void SessionWorker::setExtMessageHandler(ExtenMessageHandler cb)
{
    _extMsghandler = std::move(cb);
}

void SessionWorker::stop()
{
    if (_server) {
        // Stop the server
        _server->Stop();
    }

    if (_client) {
        _client->DisconnectAndStop();
    }
}

bool SessionWorker::startListen(int port)
{
    if (!listen(port)) {
        ELOG << "Fail to start local listen:" << port;
        return false;
    }

    return true;
}

bool SessionWorker::connectRemote(QString ip, int port, QString password)
{
    if (!connect(ip, port)) {
        ELOG << "Fail to connect remote:" << ip.toStdString();
        return false;
    }

    std::hash<std::string> hasher;
    uint32_t pinHash = (uint32_t)hasher(password.toStdString());

    LoginMessage req;
    req.name = qApp->applicationName().toStdString();
    req.auth = std::to_string(pinHash);
    proto::OriginMessage request;
    request.mask = REQ_LOGIN;
    request.json_msg = req.as_json().serialize();

    auto response = _client->request(request).get();
    return (LOGIN_SUCCESS == response.mask);
}

void SessionWorker::disconnectRemote()
{
    if (!_client) return;

    _client->DisconnectAsync();
}

proto::OriginMessage SessionWorker::sendRequest(const proto::OriginMessage &request)
{
//    _server->send(request);
    auto response = _client->request(request).get();
    return response;
}

void SessionWorker::updatePincode(QString code)
{
    _savedPin = code;
}

bool SessionWorker::listen(int port)
{
    auto asioService = _service.lock();
    if (!asioService) {
        WLOG << "weak asio service";
        return false;
    }

    // Create a new proto protocol server
    if (!_server) {
        _server = std::make_shared<ProtoServer>(asioService, port);
        _server->SetupReuseAddress(true);
        _server->SetupReusePort(true);

        auto self(this->shared_from_this());
        _server->setCallbacks(self);
    }

    // Start the server
    return _server->Start();
}

bool SessionWorker::connect(QString &address, int port)
{
    auto asioService = _service.lock();
    if (!asioService) {
        WLOG << "weak asio service";
        return false;
    }

    if (!_client) {
        _client = std::make_shared<ProtoClient>(asioService, address.toStdString(), port);

        auto self(this->shared_from_this());
        _client->setCallbacks(self);
    } else {
        if (_connectedAddress.compare(address) == 0) {
            LOG << "This target has been conntectd: " << address.toStdString();
        } else {
            // different target, create new connection.
            _client->DisconnectAndStop();
            _client = std::make_shared<ProtoClient>(asioService, address.toStdString(), port);

            auto self(this->shared_from_this());
            _client->setCallbacks(self);
        }
    }
    _client->ConnectAsync();

    return _client->IsConnected();
}
