//
// Created by oi on 2023/8/12.
//

#include "ShareFile.h"
#include "tcpclient.h"
#include "opewidget.h"

ShareFile::ShareFile(QWidget *parent) {
    m_pSelectAllPB = new QPushButton("全选");
    m_pCancelSelectPB = new QPushButton("取消选择");

    m_pOKPB = new QPushButton("确定");
    m_pCancelPB = new QPushButton("取消");

    m_pSA = new QScrollArea;
    m_pFriendW = new QWidget;
    m_FriendWVBL = new QVBoxLayout(m_pFriendW);
    m_PButtonGroupPB = new QButtonGroup(m_pFriendW);

    m_PButtonGroupPB->setExclusive(false);//设置为多选，true为单选

    auto *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_pCancelSelectPB);
    pTopHBL->addStretch();

    auto *pDownHBL = new QHBoxLayout;
    pDownHBL->addWidget(m_pOKPB);
    pDownHBL->addWidget(m_pCancelPB);

    auto *pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pSA);
    pMainVBL->addLayout(pDownHBL);

    setLayout(pMainVBL);

    connect(m_pCancelSelectPB, SIGNAL(clicked(bool)), this, SLOT(cancelSelect()));
    connect(m_pSelectAllPB, SIGNAL(clicked(bool)), this, SLOT(selectAll()));
    connect(m_pOKPB, SIGNAL(clicked(bool)), this, SLOT(okShow()));
    connect(m_pCancelPB, SIGNAL(clicked(bool)), this, SLOT(cancelShow()));

}

ShareFile &ShareFile::getInstance() {
    static ShareFile instance;
    return instance;
}

void ShareFile::upDateFriend(QListWidget *pFriendList) {
    if (pFriendList == nullptr)
        return;
    QAbstractButton *tmp = nullptr;
    QList<QAbstractButton *> preFriendList = m_PButtonGroupPB->buttons();

//    QVBoxLayout *p = new QVBoxLayout(m_pFriendW);
    QCheckBox *pCB = nullptr;
    for (int i = 0; i < preFriendList.size(); ++i) {
        tmp = preFriendList[i];
        m_FriendWVBL->removeWidget(preFriendList[i]);
        m_PButtonGroupPB->removeButton(preFriendList[i]);
        preFriendList.removeOne(tmp);
        delete tmp;
        tmp = nullptr;
    }
    for (int i = 0; i < pFriendList->count(); ++i) {
        pCB = new QCheckBox(pFriendList->item(i)->text());
        m_FriendWVBL->addWidget(pCB);
        m_PButtonGroupPB->addButton(pCB);
    }
    m_pSA->setWidget(m_pFriendW);
}

void ShareFile::cancelSelect() {
    QList<QAbstractButton *> cbList = m_PButtonGroupPB->buttons();
    for (int i = 0; i < cbList.count(); ++i) {
        if (cbList[i]->isChecked()) {
            cbList[i]->setChecked(false);
        }
    }
}

void ShareFile::selectAll() {
    QList<QAbstractButton *> cbList = m_PButtonGroupPB->buttons();
    for (int i = 0; i < cbList.count(); ++i) {
        if (!cbList[i]->isChecked()) {
            cbList[i]->setChecked(true);
        }
    }
}

void ShareFile::okShow() {
    QString strShareName = TcpClient::getInstance().loginName();
    QString strCurPath = TcpClient::getInstance().curPath();
    QString strShareFileName = OpeWidget::getInstance().getBook()->getShareFileName();

    QString strShareFilePath = strCurPath+"/"+strShareFileName;
    QList<QAbstractButton *> cbList = m_PButtonGroupPB->buttons();
    int num_1 = 0;
    for (auto & i : cbList) {
        if (i->isChecked()) {
            ++num_1;
        }
    }
    PDU *pdu = mkPDU(32*num_1+strShareFilePath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu->caData,"%s %d",strShareName.toStdString().c_str(),num_1);
    int  j=0;
    for (auto & i : cbList) {
        if (i->isChecked()){
            memcpy((char*)(pdu->caMsg)+j*32,i->text().toStdString().c_str(), i->text().size());
            ++j;
        }
    }
    memcpy((char *)(pdu->caMsg)+num_1*32,strShareFilePath.toStdString().c_str(),strShareFilePath.size());
    TcpClient::getInstance().getTcpSocket().write((char* )pdu,pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void ShareFile::cancelShow() {
    hide();
}
