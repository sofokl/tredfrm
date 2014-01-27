#include "FarmApp.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    //a.setApplicationVersion(QString::number(APP_VERSION));

    QTextCodec* codec = QTextCodec::codecForName("CP1251");
    QTextCodec::setCodecForLocale(codec);
    MainWindow w;
    w.showMaximized();


    
    return a.exec();
}
