#include "tcpclient.h"
#include "opewidget.h"
#include "online.h"
#include "friend.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "TcpClient_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }

    }

    //使用单例模式进行展示
    TcpClient::getInstance().show();

//    TcpClient w;
//    w.show();


//    Online w;
//    w.show();


//    Friend w;
//    w.show();

//    OpeWidget w;
//    w.show();
    return a.exec();
}
