// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COOPERATIONPROXY_H
#define COOPERATIONPROXY_H

#include <QObject>
#include <QMap>

class FrontendService;

namespace cooperation_core {

class CooperationTransDialog;
class CooperationProxy : public QObject
{
    Q_OBJECT

public:
    struct TransferInfo
    {
        int64_t totalSize = 0;   // 总量
        int64_t transferSize = 0;   // 当前传输量
        int32_t maxTimeSec = 0;   // 耗时

        void clear()
        {
            totalSize = 0;
            transferSize = 0;
            maxTimeSec = 0;
        }
    };

    static CooperationProxy *instance();

public Q_SLOTS:
    void waitForConfirm(const QString &name);
    void onTransJobStatusChanged(int id, int result, const QString &msg);
    void onFileTransStatusChanged(const QString &status);

    void onAccepted();
    void onRejected();
    void onCanceled();
    void onCompleted();
    void onViewed();

private:
    explicit CooperationProxy(QObject *parent = nullptr);
    ~CooperationProxy();

    bool pingBackend();
    void localIPCStart();
    void replyTransRequest(int type);
    CooperationTransDialog *cooperationDialog();

    void showTransResult(bool success, const QString &msg);
    void updateProgress(int value, const QString &msg);

private:
    FrontendService *frontendIpcSer { nullptr };
    QString sessionId;
    bool backendOk { false };
    bool thisDestruct { false };

    TransferInfo transferInfo;
    QString recvFilesSavePath;
    QMap<int, int64_t> fileIds;   // <file_id, last_current_size> 统计正在传输的文件量<文件id，上次已传输量>
    QString fromWho;
    bool isTransTimeout { false };
    CooperationTransDialog *cooperationDlg { nullptr };
};

}   // namespace cooperation_core

#endif   // COOPERATIONPROXY_H
