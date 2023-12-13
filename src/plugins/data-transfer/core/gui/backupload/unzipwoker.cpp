#include "unzipwoker.h"

#include <QProcess>
#include <QDebug>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <utils/transferhepler.h>

#include <QTimer>
#include <zip.h>

inline constexpr char datajson[] { "transfer.json" };

UnzipWorker::UnzipWorker(QString filepath)
    : filepath(filepath)
{
    QFileInfo fileInfo(filepath);
    targetDir = QDir::homePath() + "/" + fileInfo.baseName();
    while (QFile::exists(targetDir)) {
        targetDir = targetDir + "tmp";
    }
    count = getNumFiles(filepath);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        speed = currentTotal - previousTotal + 1;
        previousTotal = currentTotal;
    });

    //Calculate transmission speed
    timer->start(500);
}

UnzipWorker::~UnzipWorker()
{
}

void UnzipWorker::run()
{
    //decompression
    extract();

    //configuration
    TransferHelper::instance()->setting(targetDir);

    //remove temp dir
    QDir(targetDir).removeRecursively();
}

int UnzipWorker::getNumFiles(QString filepath)
{
    const char *zipFilePath = filepath.toLocal8Bit().constData();
    struct zip *archive = zip_open(zipFilePath, 0, NULL);

    if (archive) {
        int fileCount = zip_get_num_files(archive);
        LOG << "Number of files in ZIP file:" << fileCount;

        zip_close(archive);
        return fileCount;
    } else {
        LOG << "Unable to open ZIP file";
        return 0;
    }
}

bool UnzipWorker::isValid(QString filepath)
{
    const char *zipFilePath = filepath.toLocal8Bit().constData();
    struct zip *z = zip_open(zipFilePath, 0, nullptr);

    if (z) {
        int num_files = zip_get_num_files(z);
        LOG << "Number of files in ZIP file:" << num_files;

        for (int i = 0; i < num_files; i++) {
            struct zip_stat st;
            zip_stat_init(&st);
            zip_stat_index(z, static_cast<zip_uint64_t>(i), 0, &st);
            const char *jsonfile = "transfer.json";
            if (strcmp(st.name, jsonfile) == 0)
                return true;
        }
        zip_close(z);
        return false;
    } else {
        LOG << "Unable to open ZIP file";
        return false;
    }
}

bool UnzipWorker::extract()
{

    QStringList arguments;
    arguments << "-O"
              << "utf-8"
              << filepath
              << "-d"
              << targetDir;

    QProcess process;
    process.setReadChannel(QProcess::StandardOutput);
    process.setReadChannelMode(QProcess::SeparateChannels);
    process.start("unzip", arguments);

    //    LOG << process.arguments().toStdList();

    emit TransferHelper::instance()->transferContent(tr("Decompressing"), targetDir, 0, 0);

    while (process.waitForReadyRead(100000)) {
        QByteArray output = process.readAllStandardOutput();
        QString outputText = QString::fromLocal8Bit(output);
        if (outputText.startsWith("  inflating: ")) {
            outputText = outputText.mid(outputText.indexOf("inflating:") + QString("inflating:").length() + 1);
            currentTotal++;
            double value = static_cast<double>(currentTotal) / count;
            int progressbar = static_cast<int>(value * 100) - 1;
            int estimatedtime = (count - currentTotal) / speed / 2 + 1;
            emit TransferHelper::instance()->transferContent(tr("Decompressing"), outputText, progressbar, estimatedtime);
            LOG << value << outputText.toStdString();
        }
    }
    if (process.exitCode() != 0)
        LOG << "Error message:" << process.errorString().toStdString();
    process.waitForFinished();
    return true;
}

bool UnzipWorker::set()
{
    QFile file(targetDir + "/" + datajson);
    if (!file.open(QIODevice::ReadOnly)) {
        WLOG << "could not open datajson file";
        return false;
    }
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull()) {
        WLOG << "Parsing JSON data failed";
        return false;
    }
    QJsonObject jsonObj = jsonDoc.object();

    //Place the file in the corresponding user directory
    setUesrFile(jsonObj);

    return true;
}

bool UnzipWorker::setUesrFile(QJsonObject jsonObj)
{
    QJsonValue userFileValue = jsonObj["user_file"];
    if (userFileValue.isArray()) {
        const QJsonArray &userFileArray = userFileValue.toArray();
        for (const auto &value : userFileArray) {
            QString file = value.toString();
            QString targetFile = QDir::homePath() + "/" + file;
            QString filepath = targetDir + file.mid(file.indexOf('/'));
            bool success = QFile::rename(filepath, targetFile);
            LOG << filepath.toStdString() << success;
        }
    }
    LOG << jsonObj["user_file"].toString().toStdString();
    return true;
}
