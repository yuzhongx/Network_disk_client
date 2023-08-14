#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>
#include <QIcon>
#include <QFileDialog>
#include <QTimer>
#include "protocol.h"
//#include "opewidget.h"
#include "ShareFile.h"
#include "friend.h"

class Book : public QWidget {
Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);

    void upDateFileList(const PDU *pdu);

    void setDownloadFlag(bool status);

    QString getShareFileName();

    qint64 m_iTotal;
    qint64 m_iReceived;

    bool getDownloadStatus();

    QString getSaveFilePath();

signals:

public slots:

    void createDirPb();

    void flushFile();

    void delDir();

    void reSetName();

    void enterDir(const QModelIndex &index);

    void clearEnterDir();

    QString enterDir();

    void returnDir();

    void deleteFile();

    void upLoadFile();

    void upLoadFileData();

    void downLoadFile();

    void shareFile();

    void moveFile();

    void selectDestDir();

private:
    QListWidget *m_pBookListW;
    QPushButton *m_pReturnPB;
    QPushButton *m_pCreateDirPB;
    QPushButton *m_pDeleteDirPB;
    QPushButton *m_pReNamePB;
    QPushButton *m_pFlushFilePB;
    QPushButton *m_pUpLoadPB;
    QPushButton *m_pDownLoadPB;
    QPushButton *m_pDeleteFilePB;
    QPushButton *m_pShareFilePB;
    QPushButton *m_pMoveFilePB;
    QLabel *m_pFileLableLE;
    QLabel *m_pDoLableLE;
    QPushButton *m_pSelectDirPB;

    QString m_strEnterDir;
    QString m_strUploadFilePath;
    QTimer *m_pTimer;
    QString m_strSaveFilePath;
    bool m_bDownload;
    QString m_strShareFileName;
    QString m_strMoveFileName;
    QString m_strMoveFilePath;
    QString m_strDestDir;

};

#endif // BOOK_H
