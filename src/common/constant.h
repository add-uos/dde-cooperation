// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSTANT_H
#define CONSTANT_H

#define UNI_RPC_PROTO 1.0
#define UNI_KEY "UOS-COOPERATION"
#define UNI_RPC_PORT_UDP  51595
#define UNI_RPC_PORT_BASE (UNI_RPC_PORT_UDP + 2)

// 前端应用独立配置的关键字
#define KEY_APP_LOGIN_CONFIRM "loginconfirm"
#define KEY_APP_STORAGE_DIR "storagedir"
#define KEY_APP_MODE "mode"
#define KEY_APP_NICK "appnick"
#define KEY_APP_OPTION_BACKRUN "option_backrun"
#define KEY_APP_OPTION_KEYBOARD "option_keyboard"
#define KEY_APP_OPTION_MOUSE "option_mouse"
#define KEY_APP_OPTION_FILETRANS "option_filetrans"


// 功能设置关键字
#define FEATURE_NAME_FILETRANS "file_transfer"
#define FEATURE_NAME_CLIPBOARD "share_clipboard"
#define FEATURE_NAME_KEYBOARD "share_keyboard"
#define FEATURE_NAME_MOUSE "share_mouse"


#define UNI_IPC_PROTO 1.0
#define UNI_IPC_BACKEND_PORT 7788
#define UNI_IPC_BACKEND_COOPER_TRAN_PORT (UNI_IPC_BACKEND_PORT - 2)
#define UNI_IPC_BACKEND_DATA_TRAN_PORT (UNI_IPC_BACKEND_PORT - 4)
#define UNI_IPC_FRONTEND_PORT (UNI_IPC_BACKEND_PORT + 2)
#define UNI_IPC_FRONTEND_COOPERATION_PORT (UNI_IPC_FRONTEND_PORT + 100)
#define UNI_IPC_FRONTEND_TRANSFER_PORT (UNI_IPC_FRONTEND_PORT + 200)

#define BLOCK_SIZE 1*15*1024

const int LOGIN_CONFIRM_TIMEOUT = 30000; // 5 minutes

typedef enum apply_trans_type_t {
    APPLY_TRANS_APPLY = 0, // 请求传输文件
    APPLY_TRANS_CONFIRM = 1, // 接受文件传输
    APPLY_TRANS_REFUSED = 2, // 拒绝文件传输
} ApplyTransType;

typedef enum whoami_t {
    ServiceDeamon = 0,
    AppDataTrans = 1,
    AppCooperation = 2,
    AppSendFile = 3,
} Whoami;

typedef enum device_os_t{
    OTHER = 0,
    UOS = 1,
    LINUX = 2,
    WINDOWS = 3,
    MACOS = 4,
    ANDROID = 5,
} DeviceOS;

typedef enum compositor_t {
    CPST_NONE = 0,
    CPST_X11 = 1,
    CPST_WAYLAND = 2,
} Compositor;

typedef enum app_run_t {
    DEEPIN = 0,
    WINE = 1,
} AppRunType;

typedef enum login_result_t {
    TIMEOUT = -2,
    DENY = -1,
    COMING = 0,
    AGREE = 1,
} LoginResult;

typedef enum net_status_t {
    UNKNOWN = -2,
    DISCONNECTED = -1,
    LOSTED = 0,
    CONNECTED = 1,
} NetStatus;

typedef enum peer_result_t {
    LOST= 0,
    ADD = 1,
} PeerResult;

typedef enum do_result_t {
    FAIL= 0,
    SUCCESS = 1,
    DONE = 2,
} DoResult;

typedef enum comm_type_t {
    LOGIN = 0, // result: LoginResult
    NET_STATUS = 1, // result: NetStatus
    PEER = 2, // result: PeerResult
    APP_INSTALL = 4, // result: DoResult
    WEB_IMPORT = 5, // result: DoResult
} CommType;

typedef enum fs_type_t {
    FILE_TRANS_IDLE = 0,
    FILE_TRANS_SPEED = 1,
    FILE_TRANS_END = 2,
    JOB_TRANS_FAILED = -1,
    JOB_TRANS_DOING = 11,
    JOB_TRANS_FINISHED = 12,
    TRANS_TYPE_SEND = 101,
    TRANS_TYPE_RECV = 102,
    ACTION_READ = 21,
    ACTION_REMOVE = 22,
    ACTION_CREATE = 23,
    ACTION_RENAME = 24,
    JOB_RESUME = 31,
    JOB_CANCEL = 32,
    JOB_DONE = 33,
} FSType;

typedef enum flow_type_t {
    TRANS_BLOCK = 0,
    TRANS_DIGEST = 1,
    TRANS_ERROR = 2,
    TRANS_DONE = 3,
} FSFlowType;

typedef enum rpc_result_t {
    HAVE_NO_EXECTOR = -3,
    PARAM_ERROR = -2,
    INVOKE_FAIL = -1,
    INVOKE_OK = 0,
    INVOKE_DONE = 1,
} RpcResult;
typedef enum rpc_send_status {
    APPLY_TRANS_FILE = 0,
    REMOTE_CLIENT_OFFLINE = 1, // tcp远端的tcpclient下线（服务调用方下线）
} RpcSendStatus;

enum FileType {
  DIR = 0,
  DIR_LINK = 1,
  FILE_B = 2,
  FILE_LINK =3,
};

enum FileTransRe {
  IO_ERROR = 0, // 出现读写错误
  OK = 1,  //无错误
  FINIASH = 2, // 完成
};

// use thread replace the coroutine
#if defined(DISABLE_GO)
    #define UNIGO(...) \
        do { \
            std::thread coThread(__VA_ARGS__); \
            coThread.detach(); \
        } while(0)
#else
    #define UNIGO go
#endif

#endif // CONSTANT_H
