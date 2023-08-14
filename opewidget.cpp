#include "opewidget.h"

OpeWidget::OpeWidget(QWidget *parent)
    : QWidget{parent}
{
    m_pListWidget = new QListWidget(this);
    m_pListWidget->addItem("好友");
    m_pListWidget->addItem("图书");

    m_pFriend = new Friend;
    m_pBook = new Book;

    //堆栈窗口显示，默认显示第一个
    m_pSW = new QStackedWidget;
    m_pSW->addWidget(m_pFriend);
    m_pSW->addWidget(m_pBook);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pListWidget);
    pMain->addWidget(m_pSW);

    setLayout(pMain);

    //根据list的编号来进行选择展示的窗口
    connect(m_pListWidget,SIGNAL(currentRowChanged(int))
            ,m_pSW,SLOT(setCurrentIndex(int)));
}

OpeWidget &OpeWidget::getInstance()
{
    static OpeWidget instance;
    return instance;

}

Friend *OpeWidget::getFriend()
{
    return m_pFriend;

}

Book *OpeWidget::getBook() {
    return m_pBook;
}
