#include "book.h"

#include "tcpclient.h"
#include "opewidget.h"

Book::Book(QWidget *parent)
        : QWidget{parent} {

    m_pTimer = new QTimer;
    m_bDownload = false;

    m_pBookListW = new QListWidget;
    m_pReturnPB = new QPushButton("返回");
    m_pCreateDirPB = new QPushButton("创建文件夹");
    m_pDeleteDirPB = new QPushButton("删除文件夹");
    m_pReNamePB = new QPushButton("重命名");
    m_pFlushFilePB = new QPushButton("刷新文件夹");
    m_pUpLoadPB = new QPushButton("上传文件");
    m_pDownLoadPB = new QPushButton("下载文件");
    m_pDeleteFilePB = new QPushButton("删除文件");
    m_pShareFilePB = new QPushButton("分享文件");
    m_pMoveFilePB = new QPushButton("移动文件");
    m_pSelectDirPB = new QPushButton("目标目录");
    m_pSelectDirPB->setEnabled(false);
    m_pFileLableLE = new QLabel("文件栏");
    m_pDoLableLE = new QLabel("操作栏");

    QVBoxLayout *pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDeleteDirPB);
    pDirVBL->addWidget(m_pReNamePB);
    pDirVBL->addWidget(m_pFlushFilePB);

    QVBoxLayout *pFileVBL = new QVBoxLayout;
    pFileVBL->addWidget(m_pUpLoadPB);
    pFileVBL->addWidget(m_pDownLoadPB);
    pFileVBL->addWidget(m_pDeleteFilePB);
    pFileVBL->addWidget(m_pShareFilePB);
    pFileVBL->addWidget(m_pMoveFilePB);
    pFileVBL->addWidget(m_pSelectDirPB);

    QHBoxLayout *pMiddleVBL = new QHBoxLayout;
    pMiddleVBL->addLayout(pDirVBL);
    pMiddleVBL->addLayout(pFileVBL);

    QVBoxLayout *pUpVBL = new QVBoxLayout;
    pUpVBL->addWidget(m_pDoLableLE);
    pUpVBL->addLayout(pMiddleVBL);

    QVBoxLayout *pLeftVBL = new QVBoxLayout;
    pLeftVBL->addWidget(m_pFileLableLE);
    pLeftVBL->addWidget(m_pBookListW);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addLayout(pLeftVBL);
    pMain->addLayout(pUpVBL);


    setLayout(pMain);//设置pmain为窗口总布局

    connect(m_pCreateDirPB, SIGNAL(clicked(bool)), this, SLOT(createDirPb()));
    connect(m_pFlushFilePB, SIGNAL(clicked(bool)), this, SLOT(flushFile()));
    connect(m_pDeleteDirPB, SIGNAL(clicked(bool)), this, SLOT(delDir()));
    connect(m_pReNamePB, SIGNAL(clicked(bool)), this, SLOT(reSetName()));
    connect(m_pBookListW, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(enterDir(QModelIndex)));
    connect(m_pReturnPB, SIGNAL(clicked(bool)), this, SLOT(returnDir()));
    connect(m_pDeleteFilePB, SIGNAL(clicked(bool)), this, SLOT(deleteFile()));
    connect(m_pUpLoadPB, SIGNAL(clicked(bool)), this, SLOT(upLoadFile()));
    //使用定时器，防止上传发送请求和上传文件内容粘连一起
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(upLoadFileData()));
    connect(m_pDownLoadPB, SIGNAL(clicked(bool)), this, SLOT(downLoadFile()));
    connect(m_pShareFilePB, SIGNAL(clicked(bool)), this, SLOT(shareFile()));
    connect(m_pMoveFilePB, SIGNAL(clicked(bool)), this, SLOT(moveFile()));
    connect(m_pSelectDirPB, SIGNAL(clicked(bool)), this, SLOT(selectDestDir()));

}

void Book::createDirPb() {
    QString strNewDir = QInputDialog::getText(this, "新建文件夹", "新建文件夹名字");
    if (!strNewDir.isEmpty()) {
        if (strNewDir.size() > 32) {
            QMessageBox::warning(this, "新建文件夹", "文件夹名字不可超出32个字符");
        } else {

            QString strName = TcpClient::getInstance().loginName();
            QString strCurPath = TcpClient::getInstance().curPath();

            PDU *pdu = mkPDU(strCurPath.size() + 1);
            pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            strncpy(pdu->caData, strName.toStdString().c_str(), strName.size());
            strncpy(pdu->caData + 32, strNewDir.toStdString().c_str(), strNewDir.size());

            memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());

            TcpClient::getInstance().getTcpSocket().write((char *) pdu, pdu->uiPDULen);
            free(pdu);
            pdu = nullptr;

        }
    } else {
        QMessageBox::warning(this, "新建文件夹", "新建文件夹名字不可为空");
    }


}

void Book::flushFile() {
    m_strEnterDir.clear();
    QString strCurPath = TcpClient::getInstance().curPath();
    PDU *pdu = mkPDU(strCurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    strncpy((char *) pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char *) pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;

}

void Book::upDateFileList(const PDU *pdu) {
    if (pdu == nullptr)
        return;
    m_pBookListW->clear();
    FileInfo *pFileInfo = nullptr;
    int iCount = pdu->uiMsgLen / sizeof(FileInfo);
    for (int j = 0; j < iCount; ++j) {
        pFileInfo = (FileInfo *) (pdu->caMsg) + j;
        QListWidgetItem *pItem = new QListWidgetItem;
        if (pFileInfo->iFileType == 0)
            pItem->setIcon(QIcon(QPixmap(":/map/dir.jpg")));
        else if (pFileInfo->iFileType == 1)
            pItem->setIcon(QIcon(QPixmap(":/map/file.jpg")));
        pItem->setText(pFileInfo->caFileName);
        m_pBookListW->addItem(pItem);
    }
}

void Book::delDir() {
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (pItem == nullptr)
        QMessageBox::warning(this, "删除文件", "请选择要删除的文件");
    else {
        QString strDelName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;
        strncpy(pdu->caData, strDelName.toStdString().c_str(), strDelName.size());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char *) pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
//    flushFile();

}

void Book::reSetName() {
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (pItem == nullptr)
        QMessageBox::warning(this, "重命名", "请选择文件");
    else {
        QString strOldName = pItem->text();
        QString strNewName = QInputDialog::getText(this, "重命名文件", "请输入新的文件名");
        if (strNewName.isEmpty())
            QMessageBox::warning(this, "重命名文件", "名字不能为空");
        else {
            PDU *pdu = mkPDU(strCurPath.size() + 1);
            pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
            strncpy(pdu->caData, strOldName.toStdString().c_str(), strOldName.size());
            strncpy(pdu->caData + 32, strNewName.toStdString().c_str(), strNewName.size());
            memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
            TcpClient::getInstance().getTcpSocket().write((char *) pdu, pdu->uiPDULen);
            free(pdu);
            pdu = nullptr;
        }
    }
//    flushFile();
}

void Book::enterDir(const QModelIndex &index) {

    QString strDirName = index.data().toString();
    m_strEnterDir = strDirName;
    QString strCurPath = TcpClient::getInstance().curPath();
    if (strDirName == ".." || strDirName == ".") {
        QMessageBox::warning(this, "进入文件夹", "此文件夹不可选中");
    } else {
        PDU *pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
        strncpy(pdu->caData, strDirName.toStdString().c_str(), strDirName.size());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());

        TcpClient::getInstance().getTcpSocket().write((char *) pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
//    flushFile();
}

void Book::clearEnterDir() {
    m_strEnterDir.clear();
}

QString Book::enterDir() {
    return m_strEnterDir;
}

void Book::returnDir() {
    QString strCurPath = TcpClient::getInstance().curPath();
    QString strRootPath = "./" + TcpClient::getInstance().loginName();
    if (strCurPath == strRootPath)
        QMessageBox::warning(this, "返回", "已经在最开始目录了");
    else {
        int index = strCurPath.lastIndexOf('/');
        strCurPath.remove(index, strCurPath.size() - index);
        TcpClient::getInstance().setCurPath(strCurPath);
//        clearEnterDir();
        flushFile();
    }
}

void Book::deleteFile() {
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (pItem == nullptr)
        QMessageBox::warning(this, "删除文件", "请选择要删除的文件");
    else {
        QString strDelName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_REQUEST;
        strncpy(pdu->caData, strDelName.toStdString().c_str(), strDelName.size());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char *) pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
//    flushFile();
}

void Book::upLoadFile() {
    QString strCurPath = TcpClient::getInstance().curPath();
    m_strUploadFilePath = QFileDialog::getOpenFileName();
    if (m_strUploadFilePath.isEmpty())
        QMessageBox::warning(this, "上传文件", "路径不能为空");
    else {
        int index = m_strUploadFilePath.lastIndexOf('/');
        QString strFileName = m_strUploadFilePath.right(m_strUploadFilePath.size() - index);
        QFile file(m_strUploadFilePath);
        qint64 fileSize = file.size();//获取文件大小

        PDU *pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        sprintf(pdu->caData, "%s %lld", strFileName.toStdString().c_str(), fileSize);

        TcpClient::getInstance().getTcpSocket().write((char *) pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
        m_pTimer->start(500);

    }
}

void Book::upLoadFileData() {
    m_pTimer->stop();
    QFile file(m_strUploadFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "上传文件", "打开文件失败");
        return;
    }
    char *pBuffer = new char[4096];//每次读写数据4096字节，效率最大;
    qint64 ret = 0;
    while (true) {
        ret = file.read(pBuffer, 4096);
        if (ret > 0 && ret <= 4096) {
            TcpClient::getInstance().getTcpSocket().write(pBuffer, ret);
        } else if (ret == 0)
            break;
        else {
            QMessageBox::warning(this, "上传文件", "上传文件失败：读取文件失败");
            break;
        }
    }
    file.close();
    delete[]pBuffer;
    pBuffer = nullptr;
}

void Book::downLoadFile() {

    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (pItem == nullptr)
        QMessageBox::warning(this, "下载文件", "请选择要下载的文件");
    else {
        QString strSaveFilePath = QFileDialog::getSaveFileName();
        if (strSaveFilePath == nullptr) {
            QMessageBox::warning(this, "下载文件", "请指定保存位置");
            m_strSaveFilePath.clear();
        } else {
            m_strSaveFilePath = strSaveFilePath;
//            m_bDownload = true;
        }
        QString strCurPath = TcpClient::getInstance().curPath();
        PDU *pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
        QString strFileName = pItem->text();
        memcpy(pdu->caData, strFileName.toStdString().c_str(), strFileName.size());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char *) pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
}

void Book::setDownloadFlag(bool status) {
    m_bDownload = status;
}

bool Book::getDownloadStatus() {
    return m_bDownload;
}

QString Book::getSaveFilePath() {
    return m_strSaveFilePath;
}

void Book::shareFile() {
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (pItem == nullptr) {
        QMessageBox::warning(this, "分享文件", "请选择分享文件");
        return;
    } else {
        QString strFileName = pItem->text();
        m_strShareFileName = strFileName;
    }
    Friend *pFriend = OpeWidget::getInstance().getFriend();
    QListWidget *pFriendList = pFriend->getFriendList();
    for (int i = 0; i < pFriendList->count(); ++i) {
        qDebug() << pFriendList->item(i)->text();
    }
    ShareFile::getInstance().upDateFriend(pFriendList);
    if (ShareFile::getInstance().isHidden())
        ShareFile::getInstance().show();
}

QString Book::getShareFileName() {
    return m_strShareFileName;
}

void Book::moveFile() {
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if (pCurItem == nullptr) {
        QMessageBox::warning(this, "移动文件", "请选择要移动的文件");
    } else {
        m_strMoveFileName = pCurItem->text();
        QString strCurPath = TcpClient::getInstance().curPath();
        m_strMoveFilePath = strCurPath + '/' + m_strMoveFileName;
        m_pSelectDirPB->setEnabled(true);
    }
}

void Book::selectDestDir() {
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if (pCurItem == nullptr) {
        QMessageBox::warning(this, "移动文件", "请选择要移动的文件");
    } else {
        QString strDestDir = pCurItem->text();
        QString strCurPath = TcpClient::getInstance().curPath();
        m_strDestDir = strCurPath + '/' + strDestDir;
        int srcLen = m_strMoveFilePath.size();
        int destLen = m_strDestDir.size();
        PDU *pdu = mkPDU(srcLen + destLen + 2);
        pdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
        sprintf(pdu->caData, "%d %d %s", srcLen, destLen, m_strMoveFileName.toStdString().c_str());

        memcpy(pdu->caMsg, m_strMoveFilePath.toStdString().c_str(), srcLen);
        memcpy((char *) (pdu->caMsg) + srcLen + 1, m_strDestDir.toStdString().c_str(), destLen);
        TcpClient::getInstance().getTcpSocket().write((char *) pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
    m_pSelectDirPB->setEnabled(false);
}

