#include "FarmApp.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    //a.setApplicationVersion(QString::number(APP_VERSION));

    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);
    MainWindow w;
    w.showMaximized();


    
    return a.exec();
}
