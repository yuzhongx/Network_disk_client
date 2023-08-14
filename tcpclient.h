#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QMainWindow>
#include <QFile>
#include <QTcpSocket>
#include "protocol.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TcpClient; }
QT_END_NAMESPACE

class TcpClient : public QMainWindow
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadConfig();
    QString loginName();
    QString curPath();
    void setCurPath(QString strCurpath);

    //单例模式
    static TcpClient &getInstance();

    QTcpSocket &getTcpSocket();



public slots:
    void showConnect();
    void recvMsg();

private slots:
//    void on_send_db_clicked();

    void on_login_pb_clicked();

    void on_regist_pb_clicked();

    void on_cancel_pb_clicked();

private:
    Ui::TcpClient *ui;
    QString m_strIp;
    quint16 m_usPort;

    QTcpSocket m_tcpSocket;
    QString m_strLoginName;
    QString m_strCurPath;
    QFile m_file;
};
#endif // TCPCLIENT_H
