#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <QTextCodec>
#include <QDir>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextCodec* codec = QTextCodec::codecForName("CP1251");
    QTextCodec::setCodecForLocale(codec);

    if(argc == 6) {
        QString prog_name = argv[5];

        QString update_command = QString("%1 %2 %3%4 ").arg(argv[1]).arg(argv[2]).arg(argv[3]).arg(QDir::toNativeSeparators(argv[4]));

        QProcess *vec = new QProcess;

        qDebug() << "Update application...";

        int res = vec->execute(update_command);
        //TODO:
        //Message Box with warning of fault update process if res != 0
        // -2 = нет файла обновления

        qDebug() << "Restart programm...";
        vec->startDetached(prog_name);
        delete vec;

        return 0;
    }

    return a.exec();
}
