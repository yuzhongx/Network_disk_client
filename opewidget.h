#ifndef OPEWIDGET_H
#define OPEWIDGET_H

#include <QWidget>
#include <QListWidget>
#include "friend.h"
#include "book.h"
//堆栈窗口，每次只显示一个,默认显示第一个
#include <QStackedWidget>


class OpeWidget : public QWidget {
Q_OBJECT
public:
    explicit OpeWidget(QWidget *parent = nullptr);

    static OpeWidget &getInstance();

    Friend *getFriend();

    Book *getBook();

signals:

public slots:

private:
    QListWidget *m_pListWidget;//存放界面内容的列表；
    Friend *m_pFriend;
    Book *m_pBook;

    QStackedWidget *m_pSW;

};

#endif // OPEWIDGET_H
