// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRANSFERHELPER_H
#define TRANSFERHELPER_H

#include "global_defines.h"
#include "discover/deviceinfo.h"
#include "common/log.h"

namespace cooperation_core {

class TransferHelperPrivate;
class TransferHelper : public QObject
{
    Q_OBJECT

public:
    enum TransferStatus {
        Idle,
        Confirming,
        Connecting,
        Transfering
    };

    enum TransferType {
        Server,
        Client,
    };

    static TransferHelper *instance();

    void registBtn();
    static void buttonClicked(const QString &id, const DeviceInfoPointer info);
    static bool buttonVisible(const QString &id, const DeviceInfoPointer info);
    static bool buttonClickable(const QString &id, const DeviceInfoPointer info);

public Q_SLOTS:
    //common
    void transferResult(bool result, const QString &msg);
    void updateProgress(int value, const QString &remainTime);
    void onTransChanged(int status, const QString &path, quint64 size);

    //sever
    void sendFiles(const QString &ip, const QString &devName, const QStringList &fileList);
    TransferStatus transferStatus();
    void onConnectStatusChanged(int result, const QString &msg, const bool isself);
    void waitForConfirm();
    void accepted();
    void rejected();
    void cancelTransfer(bool sender);
    void onVerifyTimeout();

    //client
    void openFileLocation(const QString &path);
    void onActionTriggered(const QString &action);
    void notifyTransferRequest(const QString &info);
    void notifyTransferResult(bool result, const QString &msg);

private:
    explicit TransferHelper(QObject *parent = nullptr);
    ~TransferHelper();

    void updateTransProgress(uint64_t current);

private:
    QSharedPointer<TransferHelperPrivate> d { nullptr };
};

}   // namespace cooperation_core

#endif   // TRANSFERHELPER_H
