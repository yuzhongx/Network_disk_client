//
// Created by oi on 2023/8/12.
//
#ifndef TCPCLIENT_SHAREFILE_H
#define TCPCLIENT_SHAREFILE_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QScrollArea>
#include <QCheckBox>
#include <QListWidget>

class ShareFile : public QWidget {
Q_OBJECT
public:
    explicit ShareFile(QWidget *parent = nullptr);

    static ShareFile &getInstance();
    void upDateFriend(QListWidget *pFriendList);
signals:

public slots:
    void cancelSelect();
    void selectAll();
    void okShow();
    void cancelShow();

private:
    QPushButton *m_pSelectAllPB;
    QPushButton *m_pCancelSelectPB;

    QPushButton *m_pOKPB;
    QPushButton *m_pCancelPB;

    QScrollArea *m_pSA;
    QWidget *m_pFriendW;
    QVBoxLayout *m_FriendWVBL;
    QButtonGroup *m_PButtonGroupPB;
};


#endif //TCPCLIENT_SHAREFILE_H
