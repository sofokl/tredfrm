#include "LkUpdateHelper.h"

LkUpdateHelper::LkUpdateHelper(LkSettingsManager *sman, int cbuild, QObject *parent) :
    QObject(parent)
{

    s_manager = sman;
    int last_build = s_manager->getBuildNumber();
    if(last_build != cbuild)
        updateLocalData(last_build, cbuild);

}

void LkUpdateHelper::updateLocalData(int l_number, int n_number)
{
    switch(l_number) {
        case 201: update_from_201(n_number); break;
    }

    return;
}

void LkUpdateHelper::update_from_201(int n_number)
{
    //qDebug() << tr("Номер текущей сборки: %1").arg(n_number);

    if(n_number == 202) {
        qDebug() << "Выполняется обновление локальных данных до версии 1.2.0";
        s_manager->setSplitterState("0x14", QByteArray());
        s_manager->setSplitterState("0x12", QByteArray());

        qDebug() << "TODO: " << "Set new build number";
        //s_manager->setBuildNumber(202);
    }
}
