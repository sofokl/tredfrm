#ifndef TRANSPORTPROXY_H
#define TRANSPORTPROXY_H

#include <QObject>
#include "transport_global.h"
#include "wsdl_service.h"

#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QSslSocket>

QT_BEGIN_NAMESPACE
class QSslSocket;
QT_END_NAMESPACE

typedef QMap<QString, QVariant> TData;
typedef QList<TData> TDataList;
typedef QHash<QString, TData> TList;

using namespace ClientServise;

const QString WS_PATH="/tredifarm/ws/service";

class TRANSPORT_EXPORT TransportProxy : public QObject
{
    Q_OBJECT
public:
    explicit TransportProxy(QObject *parent = 0);
    bool useSsl;
    bool ignoreSslErrors;
    
signals:
    void dataSalepoinstIsReady(const TList);
    void priceDataIsReady(TList providers, TList products, TList synons, TList prices);
    void priceDataIsGetted(int count);

    void orderIsSended(int id, QString number);
    void transportError(QString, QString, bool);
    
public slots:
    void setUserName(const QString &);
    void setPassword(const QString &);
    void setEndPoint(const QString &, int port);

    bool getSalePoints();
    void getPriceList();
    void sendOrders(const TDataList &data);


private slots:
    void operationError(QString method, const KDSoapMessage &error);

private:
    QString m_endPoint;
    QString m_username;
    QString m_password;
    ClientServiseSoapBinding m_service;
    bool shouldRemove(TData first, TData second);
    
};

#endif // TRANSPORTPROXY_H
