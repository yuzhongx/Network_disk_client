#include "tcpclient.h"
#include "./ui_tcpclient.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include <utility>
//#include"protocol.h"
#include "opewidget.h"
#include "privatechat.h"

TcpClient::TcpClient(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::TcpClient) {
    ui->setupUi(this);
    resize(600, 600);
    loadConfig();

    connect(&m_tcpSocket, SIGNAL(connected()), this, SLOT(showConnect()));
    connect(&m_tcpSocket, SIGNAL(readyRead()), this, SLOT(recvMsg()));

    //连接服务器
    m_tcpSocket.connectToHost(QHostAddress(m_strIp), m_usPort);
}

TcpClient::~TcpClient() {
    delete ui;
}

void TcpClient::loadConfig() {
    QFile file(":/client.config");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray baData = file.readAll();
        QString strData = baData.toStdString().c_str();
//        qDebug()<< strData;
        file.close();
        strData.replace("\n", " ");
//        qDebug()<< strData;
        QStringList strList = strData.split(" ");
//        for(int i=0;i<strList.size();++i)
//        {
//            qDebug()<<strList[i];
//        }
        m_strIp = strList.at(0);
        m_usPort = strList.at(1).toUShort();
        qDebug() << "ip:" << m_strIp << "port:" << m_usPort;
    } else {
        QMessageBox::critical(this, "open config", "open config failed");
    }

}

QString TcpClient::loginName() {
    return m_strLoginName;
}

TcpClient &TcpClient::getInstance() {
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSocket() {
    return m_tcpSocket;
}

void TcpClient::showConnect() {
    QMessageBox::information(this, "连接服务器", "连接服务器成功");
}

void TcpClient::recvMsg() {
    if (!OpeWidget::getInstance().getBook()->getDownloadStatus()) {
        qDebug() << m_tcpSocket.bytesAvailable();
        uint uiPDULen = 0;


        m_tcpSocket.read((char *) &uiPDULen, sizeof(uint));

        uint uiMsgLen = uiPDULen - sizeof(PDU);
        PDU *pdu = mkPDU(uiMsgLen);
        m_tcpSocket.read((char *) pdu + sizeof(uint), uiPDULen - sizeof(uint));
        switch (pdu->uiMsgType) {
            case ENUM_MSG_TYPE_REGIST_RESPOND: {

                if (strcmp(pdu->caData, REGIST_OK) == 0) {
                    QMessageBox::information(this, "注册", "注册成功");

                } else if (strcmp(pdu->caData, REGIST_FAILED) == 0) {
                    QMessageBox::warning(this, "注册", "注册失败");
                }
                break;
            }
            case ENUM_MSG_TYPE_LOGIN_RESPOND: {
                if (strcmp(pdu->caData, LOGIN_OK) == 0) {
                    QMessageBox::information(this, "登陆", LOGIN_OK);
                    m_strCurPath = QString("./%1").arg(m_strLoginName);
                    OpeWidget::getInstance().show();
                    this->hide();

                } else if (strcmp(pdu->caData, LOGIN_FAILED) == 0) {
                    QMessageBox::warning(this, "登陆", LOGIN_FAILED);
                }
                break;
            }
            case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND: {
                OpeWidget::getInstance().getFriend()->showAllOnlineUsr(pdu);

                break;
            }
            case ENUM_MSG_TYPE_SREARCH_USR_RESPOND: {
                qDebug() << OpeWidget::getInstance().getFriend()->m_strSearchName;
                qDebug() << pdu->caData;
                if (strcmp(SEARCH_USR_NO, pdu->caData) == 0) {
                    QMessageBox::information(this, "搜索", QString("%1:not exist").arg(
                            OpeWidget::getInstance().getFriend()->m_strSearchName));
                } else if (strcmp(SEARCH_USR_OFFLINE, pdu->caData) == 0) {
                    QMessageBox::information(this, "搜索", QString("%1:offline").arg(
                            OpeWidget::getInstance().getFriend()->m_strSearchName));

                } else if (strcmp(SEARCH_USR_ONLINE, pdu->caData) == 0) {
                    QMessageBox::information(this, "搜索", QString("%1:online").arg(
                            OpeWidget::getInstance().getFriend()->m_strSearchName));

                }
                break;
            }
            case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: {
//            char  caMyself[32] = {'\0'};
                char caName[32] = {'\0'};
                strncpy(caName, pdu->caData + 32, 32);
                int ret = QMessageBox::information(this, "添加好友",
                                                   QString("%1 wang to add you as friend").arg(caName),
                                                   QMessageBox::Yes, QMessageBox::No);
                PDU *respdu = mkPDU(0);
                memcpy(respdu->caData, pdu->caData, 32);
                memcpy(respdu->caData + 32, pdu->caData + 32, 32);
                if (ret == QMessageBox::Yes) {
                    respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;


                } else if (ret == QMessageBox::No) {
                    respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;

                }
                m_tcpSocket.write((char *) respdu, respdu->uiPDULen);
                free(respdu);
                respdu = nullptr;

                break;
            }
            case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND: {

                QMessageBox::information(this, "添加好友", pdu->caData);

                break;
            }
//        case ENUM_MSG_TYPE_ADD_FRIEND_AGREE: {
//
//            break;
//        }
//        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE: {
//
//            break;
//        }
            case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND: {
                OpeWidget::getInstance().getFriend()->updateFriendList(pdu);

                break;
            }
            case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST: {
                char caName[32] = {'\0'};
                memcpy(caName, pdu->caData, 32);
                QMessageBox::information(this, "删除好友", QString("好友%1删除了你").arg(caName));

                break;
            }
            case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND: {
                QMessageBox::information(this, "删除好友", "删除好友成功");
                break;
            }
            case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST: {
                if (PrivateChat::getInstance().isHidden())
                    PrivateChat::getInstance().show();
                char caSendName[32] = {'\0'};
                memcpy(caSendName, pdu->caData, 32);
                QString strSendName = caSendName;
                PrivateChat::getInstance().setChatName(strSendName);
                PrivateChat::getInstance().updateMsg(pdu);
                break;

            }
            case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST: {
                OpeWidget::getInstance().getFriend()->updateGroupMsg(pdu);

                break;
            }
            case ENUM_MSG_TYPE_CREATE_DIR_RESPOND: {
                QMessageBox::information(this, "创建文件", pdu->caData);
                break;
            }
            case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND: {

                OpeWidget::getInstance().getBook()->upDateFileList(pdu);
                QString strEnterDirName = OpeWidget::getInstance().getBook()->enterDir();
                if (!strEnterDirName.isEmpty())
                    m_strCurPath = m_strCurPath + "/" + strEnterDirName;
                break;
            }
            case ENUM_MSG_TYPE_DEL_DIR_RESPOND: {
                QMessageBox::information(this, "删除文件夹", pdu->caData);
                break;
            }
            case ENUM_MSG_TYPE_RENAME_FILE_RESPOND: {
                QMessageBox::information(this, "重命名文件", pdu->caData);
                break;
            }
            case ENUM_MSG_TYPE_ENTER_DIR_RESPOND: {
                OpeWidget::getInstance().getBook()->clearEnterDir();
                QMessageBox::information(this, "进入文件夹", pdu->caData);
                break;
            }
            case ENUM_MSG_TYPE_DELETE_FILE_RESPOND: {
                QMessageBox::information(this, "删除文件", pdu->caData);
                break;
            }
            case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND: {
                QMessageBox::information(this, "上传文件", pdu->caData);
//            qDebug()<<"123";
                break;
            }
            case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND: {
                qDebug() << "接收下载回复";
                char caFileName[32] = {'\0'};
                qDebug() << "文件大小" << OpeWidget::getInstance().getBook()->m_iTotal;
                sscanf(pdu->caData, "%s %lld", caFileName, &(OpeWidget::getInstance().getBook()->m_iTotal));
                if (strlen(caFileName) > 0 && OpeWidget::getInstance().getBook()->m_iTotal >= 0) {
                    OpeWidget::getInstance().getBook()->setDownloadFlag(true);

                    QString path_1 = OpeWidget::getInstance().getBook()->getSaveFilePath();


                    m_file.setFileName(OpeWidget::getInstance().getBook()->getSaveFilePath());
                    qDebug() << path_1 << "保存路径" << m_file.fileName();
                    if (!m_file.open(QIODevice::WriteOnly)) {
                        QMessageBox::warning(this, "下载文件", "获得保存路径失败");
                    }
                }
                break;
            }
            case ENUM_MSG_TYPE_SHARE_FILE_RESPOND: {
                QMessageBox::information(this, "分享文件", "分享文件成功");
                break;
            }
            case ENUM_MSG_TYPE_SHARE_FILE_NOTE: {
                char *pPath = new char[pdu->uiMsgLen];
                memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
                char *pos = strrchr(pPath, '/');
                if (pos != nullptr) {
                    ++pos;
                    QString strNote = QString("%1 share file->%2").arg(pdu->caData, pos);
                    int ret = QMessageBox::question(this, "共享文件", strNote);
                    if (ret == QMessageBox::Yes) {
                        PDU *respdu = mkPDU(pdu->uiMsgLen);
                        respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
                        memcpy(respdu->caMsg, pdu->caMsg, pdu->uiMsgLen);
                        QString strName = TcpClient::getInstance().loginName();
                        memcpy(respdu->caData, strName.toStdString().c_str(), strName.size());
                        m_tcpSocket.write((char*)respdu,respdu->uiPDULen);
                    }
                }
                break;
            }
            case ENUM_MSG_TYPE_MOVE_FILE_RESPOND:{
                QMessageBox::information(this,"移动文件",pdu->caData);
                break;
            }

            default:
                break;
        }
        free(pdu);
        pdu = nullptr;
    } else {
        QByteArray buffer = m_tcpSocket.readAll();

        m_file.write(buffer);
        Book *pBook = OpeWidget::getInstance().getBook();
        pBook->m_iReceived += buffer.size();
        qDebug() << "上传文件------------>2" << pBook->m_iReceived;

        if (pBook->m_iReceived == pBook->m_iTotal) {
            m_file.close();
            pBook->m_iTotal = 0;
            pBook->m_iReceived = 0;
            pBook->setDownloadFlag(false);
            QMessageBox::information(this, "下载文件", "下载文件成功");
//            QMessageBox::critical(this,"下载文件","下载文件失败");

        } else if (pBook->m_iTotal < pBook->m_iReceived) {
            m_file.close();
            pBook->m_iTotal = 0;
            pBook->m_iReceived = 0;
            pBook->setDownloadFlag(false);
            QMessageBox::critical(this, "下载文件", "下载文件失败");
        }

    }

}

/*
void TcpClient::on_send_db_clicked()
{
    QString strMsg=ui->lineEdit->text();
    if(!strMsg.isEmpty())
    {
        PDU *pdu=mkPDU(strMsg.size());
        //测试
        pdu->uiMsgType=8888;

        memcpy(pdu->caMsg,strMsg.toStdString().c_str(),strMsg.size());

        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=nullptr;
    }
    else {
        QMessageBox::warning(this,"信息发送","发送的信息不能为空");
    }
}
*/

void TcpClient::on_login_pb_clicked() {
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if (!strName.isEmpty() && !strPwd.isEmpty()) {
        m_strLoginName = strName;
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), 32);
        strncpy(pdu->caData + 32, strPwd.toStdString().c_str(), 32);
        m_tcpSocket.write((char *) pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;

    } else {
        QMessageBox::critical(this, "；登陆", "登陆失败：用户名或密码不可为空");

    }
}


void TcpClient::on_regist_pb_clicked() {
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if (!strName.isEmpty() && !strPwd.isEmpty()) {
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), 32);
        strncpy(pdu->caData + 32, strPwd.toStdString().c_str(), 32);
        m_tcpSocket.write((char *) pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;

    } else {
        QMessageBox::critical(this, "注册", "注册失败：用户名或密码不能为空");

    }
}


void TcpClient::on_cancel_pb_clicked() {

}

QString TcpClient::curPath() {
    return m_strCurPath;
}

void TcpClient::setCurPath(QString strCurpath) {
    m_strCurPath = std::move(strCurpath);
}

