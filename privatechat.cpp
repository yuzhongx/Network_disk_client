#include "privatechat.h"
#include "ui_privatechat.h"
#include "tcpclient.h"


PrivateChat::PrivateChat(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::PrivateChat) {
    ui->setupUi(this);
}

PrivateChat::~PrivateChat() {
    delete ui;
}

PrivateChat &PrivateChat::getInstance() {
    static PrivateChat instance;
    return instance;
}

void PrivateChat::setChatName(QString strName) {
    m_strChatName = strName;
    m_strLoginName = TcpClient::getInstance().loginName();

}

void PrivateChat::on_sendMsg_pb_clicked() {
    QString strMsg = ui->inputMsg_le->text();
    ui->inputMsg_le->clear();
    if (strMsg.isEmpty())
        QMessageBox::warning(this, "私聊", "信息不能为空");
    else {
        PDU *pdu = mkPDU(strMsg.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
        memcpy(pdu->caData, m_strLoginName.toStdString().c_str(), m_strLoginName.size());
        memcpy(pdu->caData + 32, m_strChatName.toStdString().c_str(), m_strChatName.size());
        strcpy((char *) pdu->caMsg, strMsg.toStdString().c_str());
        TcpClient::getInstance().getTcpSocket().write((char *) pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
        QString strSelfMsg=QString("%1 : %2").arg(m_strLoginName,strMsg);
        ui->showMsg_te->append(strSelfMsg);
    }
}

void PrivateChat::updateMsg(const PDU *pdu) {
    if (pdu == nullptr)//客户端闪退问题在这,pdu一直为空，之前少写了=
        return;
    char caSendName[32] = {'\0'};
    memcpy(caSendName, pdu->caData, 32);
    QString strMsg = QString("%1 : %2").arg(caSendName, (char *) pdu->caMsg);
    ui->showMsg_te->append(strMsg);

}

