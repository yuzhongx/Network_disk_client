#include "online.h"
#include "ui_online.h"
#include "tcpclient.h"

Online::Online(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

void Online::ShowUsr(PDU *pdu)
{
    if(pdu == nullptr)
        return;
    ui->online_listwidget->clear();
    uint uiSize = pdu->uiMsgLen/32;
    char caImp[32];

    for (int var = 0; var < uiSize; ++var) {
        memcpy(caImp,(char*)(pdu->caMsg)+var*32,32);
        ui->online_listwidget->addItem(caImp);
    }

}

void Online::on_addFriend_clicked()
{
    QListWidgetItem *pItem = ui->online_listwidget->currentItem();
    QString strPerUsrName = pItem->text();
    QString strLoginName = TcpClient::getInstance().loginName();
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    memcpy(pdu->caData,strPerUsrName.toStdString().c_str(),strPerUsrName.size());
    qDebug()<<strPerUsrName.size()<<"add";
    qDebug()<<strLoginName.size()<<"add";
    memcpy(pdu->caData+32,strLoginName.toStdString().c_str(),strLoginName.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;


}

