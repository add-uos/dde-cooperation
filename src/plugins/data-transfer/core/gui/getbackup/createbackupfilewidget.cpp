﻿#include "createbackupfilewidget.h"
#include "../select/item.h"
#include "../type_defines.h"
#include "./zipworker.h"
#include "../select/userselectfilesize.h"
#include "../select/calculatefilesize.h"
#include "../win/devicelistener.h"

#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLineEdit>
#include <QStackedWidget>
#include <QListView>

#include <QStandardItemModel>
#include <QStorageInfo>

#include <QStandardPaths>
#include <gui/connect/choosewidget.h>
#include <gui/mainwindow_p.h>
#include <utils/optionsmanager.h>
#include <utils/transferhepler.h>

CreateBackupFileWidget::CreateBackupFileWidget(QWidget *parent) : QFrame(parent)
{
    initUI();
    QObject::connect(DeviceListener::instance(), &DeviceListener::updateDevice, this,
                     &CreateBackupFileWidget::getUpdateDeviceSingla);
}

CreateBackupFileWidget::~CreateBackupFileWidget() { }

void CreateBackupFileWidget::sendOptions()
{
    QStringList savePath;
    QAbstractItemModel *model = diskListView->model();
    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex index = model->index(row, 0);
        QVariant checkboxData = model->data(index, Qt::CheckStateRole);
        Qt::CheckState checkState = static_cast<Qt::CheckState>(checkboxData.toInt());
        if (checkState == Qt::Checked) {
            QString selectDecive = model->data(index, Qt::UserRole).toString();
            QString documentsPath =
                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
            if (selectDecive == QDir(documentsPath).rootPath()) {
                savePath << documentsPath;
            } else {
                savePath << selectDecive;
            }
            break;
        }
    }
    OptionsManager::instance()->addUserOption(Options::kBackupFileSavePath, savePath);

    QStringList saveName;
    saveName << fileNameInput->text();
    OptionsManager::instance()->addUserOption(Options::kBackupFileName, saveName);
    qInfo() << "backup file save path:" << savePath;
    qInfo() << "backup file name:" << saveName;
}

void CreateBackupFileWidget::clear()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(diskListView->model());
    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex itemIndex = model->index(row, 0);
        model->setData(itemIndex, Qt::Unchecked, Qt::CheckStateRole);
    }
    fileNameInput->clear();
}

void CreateBackupFileWidget::initUI()
{
    setStyleSheet("background-color: white; border-radius: 10px;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    QLabel *titileLabel = new QLabel(tr("Create data backup"), this);
    titileLabel->setFixedHeight(30);
    QFont font;
    font.setPointSize(16);
    font.setWeight(QFont::DemiBold);
    titileLabel->setFont(font);
    titileLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QLabel *fileNameLabel = new QLabel(tr("File information"), this);
    fileNameLabel->setStyleSheet("opacity: 1;"
                                 "color: rgba(0,26,46,1);"
                                 "font-family: \"SourceHanSansSC-Bold\";"
                                 "font-size: 16px;font-weight: 700;"
                                 "font-style: normal;"
                                 "text-align: left;");
    QHBoxLayout *fileNameLayout = new QHBoxLayout();
    fileNameLayout->addSpacing(140);
    fileNameLayout->addWidget(fileNameLabel);
    fileNameLayout->setAlignment(Qt::AlignBottom);

    QFrame *fileName = new QFrame(this);
    QHBoxLayout *fileNameEditLayout = new QHBoxLayout();
    fileName->setLayout(fileNameEditLayout);
    fileName->setFixedSize(459, 36);
    fileName->setStyleSheet("border-radius: 8px;"
                            "opacity: 1;"
                            "background-color: rgba(0,0,0, 0.08);");

    QLabel *fileNameInputLabel1 = new QLabel(QString(tr("Name")), fileName);
    fileNameInputLabel1->setFixedWidth(56);
    fileNameInputLabel1->setStyleSheet("opacity: 1;"
                                       "background-color: rgba(0,0,0,0);"
                                       "color: rgba(65,77,104,1);"
                                       "font-family: \"SourceHanSansSC-Medium\";"
                                       "font-size: 14px;font-weight: 500;"
                                       "font-style: normal;"
                                       "text-align: left;");

    fileNameInput = new QLineEdit(fileName);
    fileNameInput->setFixedWidth(211);
    fileNameInput->setPlaceholderText(tr("Default File Name"));
    fileNameInput->setStyleSheet("QLineEdit {"
                                 "opacity: 1;"
                                 "background-color: rgba(0,0,0,0.01);"
                                 "color: rgba(82,106,127,1);"
                                 "font-family:\"SourceHanSansSC-Normal\";"
                                 "font-size: 12px;"
                                 "font-weight: 400;"
                                 "font-style: normal;"
                                 "text-align: left;"
                                 "background-image: url(\":/icon/edit.svg\");"
                                 "background-repeat: no-repeat;"
                                 "background-position: center;}");
    connect(fileNameInput, &QLineEdit::textChanged, [=]() {
        bool isEmpty = fileNameInput->text().isEmpty();
        if (!isEmpty) {
            fileNameInput->setStyleSheet("QLineEdit { "
                                         "background-color: rgba(0,0,0,0.01);"
                                         "background-image: none; }");
        } else {
            fileNameInput->setStyleSheet("QLineEdit { "
                                         "background-color: rgba(0,0,0,0.01);"
                                         "background-image: url(\":/icon/edit.svg\");"
                                         "background-repeat: no-repeat; "
                                         "background-position: center;}");
        }
    });

    QHBoxLayout *fileNameInputLabel1Layout = new QHBoxLayout();
    fileNameInputLabel1Layout->addSpacing(10);
    fileNameInputLabel1Layout->addWidget(fileNameInputLabel1);
    fileNameInputLabel1Layout->addSpacing(10);
    fileNameInputLabel1Layout->addWidget(fileNameInput);

    backupFileSizeLabel = new QLabel(QString("大小:  0B"), fileName);
    backupFileSizeLabel->setStyleSheet("opacity: 1;"
                                       "background-color: rgba(0,0,0,0);"
                                       "color: rgba(65,77,104,1);"
                                       "font-family: \"SourceHanSansSC-Medium\";"
                                       "font-size: 14px;font-weight: 500;"
                                       "font-style: normal;"
                                       "text-align: left;");
    QHBoxLayout *fileNameInputLabel2Layout = new QHBoxLayout();
    fileNameInputLabel2Layout->addSpacing(20);
    fileNameInputLabel2Layout->addWidget(backupFileSizeLabel);

    fileNameEditLayout->addLayout(fileNameInputLabel1Layout);
    fileNameEditLayout->addLayout(fileNameInputLabel2Layout);

    QHBoxLayout *layout1 = new QHBoxLayout();
    layout1->addSpacing(140);
    layout1->addWidget(fileName);
    layout1->addSpacing(140);
    layout1->setAlignment(Qt::AlignTop);

    QLabel *savePathLabel1 = new QLabel(tr("Location"), this);
    savePathLabel1->setFixedWidth(65);
    savePathLabel1->setStyleSheet("opacity: 1;"
                                  "color: rgba(0,26,46,1);"
                                  "font-family: \"SourceHanSansSC-Bold\";"
                                  "font-size: 16px;"
                                  "font-weight: 700;"
                                  "font-style: normal;"
                                  "text-align: left;");

    QLabel *savePathLabel2 = new QLabel(tr("(Select Backup Disk)"), this);
    savePathLabel2->setStyleSheet("opacity: 1;"
                                  "color: rgba(82,106,127,1);"
                                  "font-family: \"SourceHanSansSC-Normal\";"
                                  "font-size: 12px; "
                                  "font-weight: 400; "
                                  "font-style: normal; "
                                  "text-align: left; ");
    QHBoxLayout *savePathLayout = new QHBoxLayout();
    savePathLayout->setAlignment(Qt::AlignTop);
    savePathLayout->addSpacing(140);
    savePathLayout->addWidget(savePathLabel1);
    savePathLayout->addWidget(savePathLabel2);

    initDiskListView();

    QHBoxLayout *diskListViewLayout = new QHBoxLayout();
    diskListViewLayout->setAlignment(Qt::AlignTop);
    diskListViewLayout->addSpacing(140);
    diskListViewLayout->addWidget(diskListView);

    promptLabel = new QLabel(this);

    promptLabel->setText(
            QString("<font size='3' color='#FF5736'>%1</font>")
                    .arg(tr("Insufficient space in the selected disk, please clean the space")));
    promptLabel->setAlignment(Qt::AlignCenter);
    promptLabel->setVisible(false);

    determineButton = new QToolButton(this);
    determineButton->setText(tr("Backup"));
    determineButton->setFixedSize(120, 35);
    setDetermineButtonEnable(false);

    QObject::connect(determineButton, &QToolButton::clicked, this, [this]() {
        nextPage();
        ZipWork *worker = new ZipWork(this);
        worker->start();
    });

    QToolButton *cancelButton = new QToolButton(this);
    cancelButton->setText(tr("Cancel"));
    cancelButton->setFixedSize(120, 35);
    cancelButton->setStyleSheet(".QToolButton{border-radius: 8px;"
                                "border: 1px solid rgba(0,0,0, 0.03);"
                                "opacity: 1;"
                                "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 "
                                "rgba(230, 230, 230, 1), stop:1 rgba(227, 227, 227, 1));"
                                "font-family: \"SourceHanSansSC-Medium\";"
                                "font-size: 14px;"
                                "font-weight: 500;"
                                "color: rgba(65,77,104,1);"
                                "font-style: normal;"
                                "text-align: center;"
                                ";}");
    QObject::connect(cancelButton, &QToolButton::clicked, this, &CreateBackupFileWidget::backPage);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addSpacing(15);
    buttonLayout->addWidget(determineButton);
    buttonLayout->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

    IndexLabel *indelabel = new IndexLabel(2, this);
    indelabel->setAlignment(Qt::AlignCenter);
    QHBoxLayout *indexLayout = new QHBoxLayout();
    indexLayout->addWidget(indelabel, Qt::AlignCenter);

    mainLayout->addSpacing(30);
    mainLayout->addWidget(titileLabel);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(fileNameLayout);
    mainLayout->addLayout(layout1);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(savePathLayout);
    mainLayout->addLayout(diskListViewLayout);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(promptLabel);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(indexLayout);
    QObject::connect(diskListView, &QListView::clicked, this, [this](const QModelIndex &index) {
        if (index.data(Qt::CheckStateRole) == Qt::Unchecked) {
            setDetermineButtonEnable(false);
        } else {
            setDetermineButtonEnable(true);
        }
    });

    QObject::connect(UserSelectFileSize::instance(), &UserSelectFileSize::updateUserFileSelectSize,
                     this, &CreateBackupFileWidget::updateuserSelectFileSize);
}

void CreateBackupFileWidget::initDiskListView()
{
    SaveItemDelegate *saveDelegate = new SaveItemDelegate();
    QStandardItemModel *model = new QStandardItemModel(this);
    diskListView = new QListView(this);
    diskListView->setStyleSheet(".QListView{"
                                "border-radius:8px;"
                                "opacity:1;"
                                "background-color:rgba(0,0,0,0.08);"
                                "padding-left: 10px; padding-right: 10px; padding-top: 10px; }");

    diskListView->setItemDelegate(saveDelegate);
    diskListView->setFixedSize(460, 179);

    diskListView->setModel(model);
    diskListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    diskListView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    diskListView->setSelectionMode(QAbstractItemView::SingleSelection);

    QObject::connect(diskListView, &QListView::clicked, [model](const QModelIndex &index) {
        for (int row = 0; row < model->rowCount(); ++row) {
            QModelIndex itemIndex = model->index(row, 0);
            if (itemIndex != index) {
                model->setData(itemIndex, Qt::Unchecked, Qt::CheckStateRole);
            }
        }
    });
}

void CreateBackupFileWidget::checkDisk()
{

    bool isValid = false;
    for (auto iterator = diskCapacity.begin(); iterator != diskCapacity.end(); ++iterator) {
        QStandardItem *item = iterator.key();
        quint64 size = iterator.value();
        if (size < allSize) {
            item->setCheckable(false);
            item->setData(true, Qt::BackgroundRole);
        } else {
            isValid = true;
            item->setCheckable(true);
            item->setData(false, Qt::BackgroundRole);
        }
    }
    if (!isValid)
        promptLabel->setVisible(true);
    else
        promptLabel->setVisible(false);
}

void CreateBackupFileWidget::setDetermineButtonEnable(bool enable)
{
    if (enable) {
        determineButton->setStyleSheet(".QToolButton{border-radius: 8px;"
                                       "border: 1px solid rgba(0,0,0, 0.03);"
                                       "opacity: 1;"
                                       "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 "
                                       "rgba(230, 230, 230, 1), stop:1 rgba(227, 227, 227, 1));"
                                       "font-family: \"SourceHanSansSC-Medium\";"
                                       "font-size: 14px;"
                                       "font-weight: 500;"
                                       "color: rgba(65,77,104,1);"
                                       "font-style: normal;"
                                       "text-align: center;"
                                       ";}");
        determineButton->setEnabled(true);
    } else {
        determineButton->setStyleSheet(".QToolButton{border-radius: 8px;"
                                       "border: 1px solid rgba(0,0,0, 0.03);"
                                       "opacity: 1;"
                                       "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 "
                                       "rgba(230, 230, 230, 0.6), stop:1 rgba(227, 227, 227, 0.6));"
                                       "font-family: \"SourceHanSansSC-Medium\";"
                                       "font-size: 14px;"
                                       "font-weight: 500;"
                                       "color: rgba(65,77,104,0.6);"
                                       "font-style: normal;"
                                       "text-align: center;"
                                       ";}");
        determineButton->setEnabled(false);
    }
}

void CreateBackupFileWidget::nextPage()
{
    // send useroptions
    sendOptions();
    // nextpage
    emit TransferHelper::instance()->changeWidget(PageName::zipfileprocesswidget);
}
void CreateBackupFileWidget::backPage()
{
    clear();
    emit TransferHelper::instance()->changeWidget(PageName::selectmainwidget);
}

void CreateBackupFileWidget::updateuserSelectFileSize(const QString &sizeStr)
{
    userSelectFileSize = fromQstringToByte(sizeStr);
}

void CreateBackupFileWidget::updaeBackupFileSize()
{
    QStringList filePathList = OptionsManager::instance()->getUserOption(Options::kFile);
    QStringList appList = OptionsManager::instance()->getUserOption(Options::kApp);
    QStringList browserList = OptionsManager::instance()->getUserOption(Options::kBrowserBookmarks);
    QStringList configList = OptionsManager::instance()->getUserOption(Options::kConfig);

    TransferHelper::instance()->getTransferFilePath(filePathList,appList,browserList,configList);

    QStringList userDataInfoJsonPath =
            OptionsManager::instance()->getUserOption(Options::KUserDataInfoJsonPath);
    QStringList wallpaperPath = OptionsManager::instance()->getUserOption(Options::KWallpaperPath);
    QStringList bookmarkJsonPath =
            OptionsManager::instance()->getUserOption(Options::KBookmarksJsonPath);
    quint64 userDataInfoJsonSize = 0;
    quint64 wallpaperSize = 0;
    quint64 bookmarkJsonSize = 0;
    if (!userDataInfoJsonPath.isEmpty()) {
        userDataInfoJsonSize = QFileInfo(userDataInfoJsonPath[0]).size();
    }
    if (!wallpaperPath.isEmpty()) {
        wallpaperSize = QFileInfo(wallpaperPath[0]).size();
    }
    if (!bookmarkJsonPath.isEmpty()) {
        bookmarkJsonSize = QFileInfo(bookmarkJsonPath[0]).size();
    }

    allSize = userSelectFileSize + userDataInfoJsonSize + wallpaperSize + bookmarkJsonSize;
    OptionsManager::instance()->addUserOption(Options::KBackupFileSize,QStringList{QString::number(allSize)});
    backupFileSizeLabel->setText(QString(tr("Size:%1")).arg(fromByteToQstring(allSize)));

    checkDisk();
}

void CreateBackupFileWidget::getUpdateDeviceSingla()
{
    QString rootPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString cPath = QDir(rootPath).rootPath();

    QList<QStorageInfo> devices = QStorageInfo::mountedVolumes();

    for (const QStorageInfo &device : devices) {
        // Exclude read-only devices
        if (device.isReadOnly() || !device.isReady()) {
            devices.removeOne(device);
            continue;
        }
        QString rootPath = device.rootPath();
        if (deviceList.contains(device)) {
            deviceList.removeOne(device);
            continue;
        }
        // add device
        updateDevice(device, true);
    }

    for (const QStorageInfo &device : deviceList) {
        // del device
        updateDevice(device, false);
    }
    deviceList = devices;

    checkDisk();
}

void CreateBackupFileWidget::updateDevice(const QStorageInfo &device, const bool &isAdd)
{
    if (isAdd) {
        QStandardItemModel *model = qobject_cast<QStandardItemModel *>(diskListView->model());
        QString rootPath = device.rootPath();
        QString displayName = (device.name().isEmpty() ? tr("local disk") : device.name()) + "("
                + rootPath.at(0) + ":)";

        QStandardItem *item = new QStandardItem();
        item->setData(displayName, Qt::DisplayRole);
        item->setData(rootPath, Qt::UserRole);
        item->setData(tr("%1/%2 available")
                              .arg(fromByteToQstring(device.bytesAvailable()))
                              .arg(fromByteToQstring(device.bytesTotal())),
                      Qt::ToolTipRole);
        if (device.name().isEmpty()) {
            item->setIcon(QIcon(":/icon/drive-harddisk-32px.svg"));
        } else {
            item->setIcon(QIcon(":/icon/drive-harddisk-usb-32px.svg"));
        }
        diskCapacity[item] = device.bytesAvailable();
        item->setCheckable(true);
        model->appendRow(item);
    } else {
        QString rootPath = device.rootPath();
        QStandardItemModel *model = qobject_cast<QStandardItemModel *>(diskListView->model());
        for (int row = 0; row < model->rowCount(); ++row) {
            QModelIndex itemIndex = model->index(row, 0);
            if (rootPath == model->data(itemIndex, Qt::UserRole)) {
                model->removeRow(itemIndex.row());
            }
        }
        for (auto iterator = diskCapacity.begin(); iterator != diskCapacity.end(); ++iterator) {
            QString path = iterator.key()->data(Qt::UserRole).toString();
            if (path == rootPath) {
                diskCapacity.erase(iterator);
                break;
            }
        }
    }
}
