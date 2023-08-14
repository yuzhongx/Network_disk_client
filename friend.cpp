#include "friend.h"
#include "protocol.h"
#include "tcpclient.h"
#include "privatechat.h"
#include <QInputDialog>

Friend::Friend(QWidget *parent)
        : QWidget{parent} {
    m_pShowMsgTE = new QTextEdit;
    m_pFriendListWidget = new QListWidget;
    m_pInputMsgLE = new QLineEdit;
    m_pOnline = new Online;

    m_pDelFriendPB = new QPushButton("删除好友");
    m_pFlushFriendPB = new QPushButton("刷新好友");
    m_pShowOnlineUsrPB = new QPushButton("显示在线用户");
    m_pSearchUsrPB = new QPushButton("查找好友");
    m_pMsgSendPB = new QPushButton("发送");
    m_pPrivatechatPB = new QPushButton("私聊好友");


    //创建按钮 ,垂直布局
    QVBoxLayout *pRightPBVBL = new QVBoxLayout;
    pRightPBVBL->addWidget(m_pDelFriendPB);
    pRightPBVBL->addWidget(m_pFlushFriendPB);
    pRightPBVBL->addWidget(m_pShowOnlineUsrPB);
    pRightPBVBL->addWidget(m_pSearchUsrPB);
//    pRightPBVBL->addWidget(m_pMsgSendPB);
    pRightPBVBL->addWidget(m_pPrivatechatPB);

    //创建展示、好友列表框，并水平布局
    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pShowMsgTE);
    pTopHBL->addWidget(m_pFriendListWidget);
    pTopHBL->addLayout(pRightPBVBL);

    //创建输入和信息发送框，并垂直布局
    QVBoxLayout *pMsgHBL = new QVBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);


    QVBoxLayout *pMain = new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);

    pMain->addWidget(m_pOnline);
    m_pOnline->hide();//先不显示，点击显示在线用户后才显示


    setLayout(pMain);

    connect(m_pShowOnlineUsrPB, SIGNAL(clicked(bool)), this, SLOT(showOnline()));
    connect(m_pSearchUsrPB, SIGNAL(clicked(bool)), this, SLOT(searchUsr()));
    connect(m_pFlushFriendPB, SIGNAL(clicked(bool)), this, SLOT(flushFriend()));
    connect(m_pDelFriendPB, SIGNAL(clicked(bool)), this, SLOT(delFriend()));
    connect(m_pPrivatechatPB, SIGNAL(clicked(bool)), this, SLOT(privateChat()));
    connect(m_pMsgSendPB, SIGNAL(clicked(bool)),this, SLOT(groupChat()));
}

void Friend::showAllOnlineUsr(PDU *pdu) {
    if (pdu == nullptr)
        return;
    m_pOnline->ShowUsr(pdu);


}

void Friend::showOnline() {
    if (m_pOnline->isHidden()) {
        m_pOnline->show();
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char *) pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    } else {
        m_pOnline->hide();
    }
}

void Friend::searchUsr() {
    m_strSearchName = QInputDialog::getText(this, "搜索", "用户名：");
    if (!m_strSearchName.isEmpty()) {
        qDebug() << m_strSearchName;
        PDU *pdu = mkPDU(0);
        memcpy(pdu->caData, m_strSearchName.toStdString().c_str(), m_strSearchName.size());
        pdu->uiMsgType = ENUM_MSG_TYPE_SREARCH_USR_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char *) pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
}

void Friend::flushFriend() {
    QString strName = TcpClient::getInstance().loginName();
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    memcpy(pdu->caData, strName.toStdString().c_str(), strName.size());
    TcpClient::getInstance().getTcpSocket().write((char *) pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;

}

void Friend::updateFriendList(PDU *pdu) {
    if (pdu == nullptr)
        return;
    m_pFriendListWidget->clear();
    uint uiSize = pdu->uiMsgLen / 32;
    char caName[32] = {'\0'};
    for (int i = 0; i < uiSize; ++i) {
        memcpy(caName, (char *) (pdu->caMsg) + i * 32, 32);
        qDebug() << caName;
        m_pFriendListWidget->addItem(caName);

    }
}

void Friend::delFriend() {
    if (m_pFriendListWidget->currentItem() != nullptr) {
        QString strName = m_pFriendListWidget->currentItem()->text();
        QString strSelfName = TcpClient::getInstance().loginName();
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
        memcpy(pdu->caData, strSelfName.toStdString().c_str(), strSelfName.size());
        memcpy(pdu->caData + 32, strName.toStdString().c_str(), strName.size());
        TcpClient::getInstance().getTcpSocket().write((char *) pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;

    }

}

void Friend::privateChat() {
    if (m_pFriendListWidget->currentItem() != nullptr) {
        QString strChatName = m_pFriendListWidget->currentItem()->text();
        PrivateChat::getInstance().setChatName(strChatName);
        if (PrivateChat::getInstance().isHidden())
            PrivateChat::getInstance().show();
    } else {
        QMessageBox::warning(this, "私聊", "请选择私聊对象");
    }
}

void Friend::groupChat() {
    QString strMsg=m_pInputMsgLE->text();
    if (strMsg.isEmpty()){
        QMessageBox::warning(this,"群聊","信息不能为空");
    }
    else{
        PDU *pdu= mkPDU(strMsg.size()+1);
        pdu->uiMsgType=ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
        QString strName=TcpClient::getInstance().loginName();

        strncpy(pdu->caData,strName.toStdString().c_str(),strName.size());
        strncpy((char *)pdu->caMsg,strMsg.toStdString().c_str(),strMsg.size());

        TcpClient::getInstance().getTcpSocket().write((char *)pdu,pdu->uiPDULen);
//        m_pShowMsgTE->append(strMsg);
    }
}

void Friend::updateGroupMsg(PDU *pdu) {
    QString strMsg = QString("%1 :%2").arg(pdu->caData,(char *)pdu->caMsg);
    m_pShowMsgTE->append(strMsg);
    m_pInputMsgLE->clear();

}

QListWidget *Friend::getFriendList() {
    return m_pFriendListWidget;
}
