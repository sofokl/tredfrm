#include "LkSettingsManager.h"
#include <QStandardPaths>
#include <QDebug>

LkSettingsManager::LkSettingsManager(QObject *parent) :
    QObject(parent)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);

    settings = new QSettings(path+"/config.ini", QSettings::IniFormat);
}

QString LkSettingsManager::getServerAddress()
{
    return settings->value("Connections/server", "127.0.0.1").toString();
}

void LkSettingsManager::setServerAddress(QString value)
{
    settings->setValue("Connections/server", value);
}

QString LkSettingsManager::getUserName()
{
    return QByteArray::fromBase64(settings->value("Connections/username").toString().toUtf8());
}

void LkSettingsManager::setUserName(QString value)
{
    settings->setValue("Connections/username", QString(value.toUtf8().toBase64()));
}

QString LkSettingsManager::getPassword()
{
    return QByteArray::fromBase64(settings->value("Connections/password").toString().toUtf8());
}

void LkSettingsManager::setPassword(QString value)
{
    settings->setValue("Connections/password", QString(value.toUtf8().toBase64()));
}

QString LkSettingsManager::getProxyServer()
{
    return settings->value("Connections/proxy").toString();
}

void LkSettingsManager::setProxyServer(QString value)
{
    settings->setValue("Connections/proxy", value);
}

int LkSettingsManager::getProxyPort()
{
    return settings->value("Connections/proxy_port").toInt();
}

int LkSettingsManager::getPort()
{
    return settings->value("Connections/port", 80).toInt();
}

void LkSettingsManager::setPort(int value)
{
    settings->setValue("Connections/port", value);
}

void LkSettingsManager::setProxyPort(int value)
{
    settings->setValue("Connections/proxy_port", value);
}

void LkSettingsManager::setSplitterState(QString id, QByteArray data)
{
    settings->setValue("Geometry/"+id, data);
    qDebug() << id << data;
}

void LkSettingsManager::setBuildNumber(int value)
{
    settings->setValue("Main/buildn", value);
}

QByteArray LkSettingsManager::getSplitterState(QString id) {

    return settings->value("Geometry/"+id).toByteArray();

}

QString LkSettingsManager::getClientName()
{
    return QByteArray::fromBase64(settings->value("Main/name").toString().toUtf8());
}

void LkSettingsManager::setIgnoreSslErrors(bool value)
{
    settings->setValue("Connections/ignore_ssl_errors", value);
}

bool LkSettingsManager::getIgnoreSslErrors()
{
    return settings->value("Connections/ignore_ssl_errors").toBool();
}

bool LkSettingsManager::getShowAllProvs()
{
    return settings->value("Main/showallprovs").toBool();
}

void LkSettingsManager::setShowAllProvs(bool value)
{
    settings->setValue("Main/showallprovs", value);
}

void LkSettingsManager::setUseSsl(bool value)
{
    settings->setValue("Connections/ssl", value);
}

bool LkSettingsManager::getUseSsl()
{
    return settings->value("Connections/ssl").toBool();
}

int LkSettingsManager::getBuildNumber()
{
    return settings->value("Main/buildn", 201).toInt();
}
