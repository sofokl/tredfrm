#ifndef LKUPDATEHELPER_H
#define LKUPDATEHELPER_H

#include <QObject>
#include "LkSettingsManager.h"

#include "core_global.h"

class CORESHARED_EXPORT LkUpdateHelper : public QObject
{
    Q_OBJECT
public:
    explicit LkUpdateHelper(LkSettingsManager *sman, int cbuild, QObject *parent = 0);

signals:

public slots:

private:
    LkSettingsManager *s_manager;
    void updateLocalData(int, int);
    void update_from_201(int);

};

#endif // LKUPDATEHELPER_H
