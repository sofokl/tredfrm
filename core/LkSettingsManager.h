#ifndef LKSETTINGSMANAGER_H
#define LKSETTINGSMANAGER_H

#include "core_global.h"

#include <QObject>
#include <QSettings>

class CORESHARED_EXPORT LkSettingsManager : public QObject
{
    Q_OBJECT
public:
    explicit LkSettingsManager(QObject *parent = 0);

    //Getter
    QString getServerAddress();
    QString getUserName();
    QString getPassword();
    QString getProxyServer();
    QString getClientName();
    bool getUseSsl();
    bool getIgnoreSslErrors();
    int getPort();
    int getBuildNumber();
    bool getShowAllProvs();

    QByteArray getSplitterState(QString id);
    int getProxyPort();

signals:

public slots:
    void setServerAddress(QString value);
    void setUserName(QString value);
    void setPassword(QString value);
    void setProxyServer(QString value);
    void setProxyPort(int value);
    void setSplitterState(QString id, QByteArray data);
    void setPort(int value);
    void setUseSsl(bool);
    void setIgnoreSslErrors(bool);
    void setBuildNumber(int);
    void setShowAllProvs(bool);

private:
    QSettings *settings;
    
};

#endif // SSETTINGSMANAGER_H
