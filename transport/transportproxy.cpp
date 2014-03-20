#include "transportproxy.h"
#include <QMessageBox>
#include <QtCore>
#include <QHash>

TransportProxy::TransportProxy(QString apikkey, QObject *parent) : QObject(parent)
{

    m_service.setSoapVersion(KDSoapClientInterface::SOAP1_1);
    m_apikey = apikkey;

    connect(&m_service, SIGNAL(soapError(QString,KDSoapMessage)), this, SLOT(operationError(QString,KDSoapMessage)));

}

void TransportProxy::setUserName(const QString &name) {
    this->m_username = name;
}

void TransportProxy::setPassword(const QString &pass) {
    this->m_password = pass;
}

void TransportProxy::setEndPoint(const QString &point, int port) {

    QString protocol;
    if(this->useSsl ==true)
        protocol = "https";
    else
        protocol = "http";

    this->m_endPoint = QString(protocol+"://%1:%2"+WSP).arg(point.trimmed(), QString::number(port));
    m_service.setEndPoint(this->m_endPoint);

}

bool TransportProxy::getSalePoints() {

    if(this->useSsl && this->ignoreSslErrors)
        m_service.ignoreSslErrors();

    KDSoapAuthentication *auth = new KDSoapAuthentication();
    auth->setUser(m_username);
    auth->setPassword(m_password);

    m_service.clientInterface()->setAuthentication(*auth);

    TNS__GetSalePoints params;
    params.setApiKey(m_apikey);


    TNS__GetSalePointsResponse response;

    response = m_service.getSalePoints(params);

    if(!m_service.lastError().isEmpty()){

        QString text = m_service.lastError();
        text = text.replace(m_endPoint, "$END_POINT$");
        emit transportError(tr("Ошибка при получении данных [#1]\t\t"), text, true);
        return false;
    }

    TList data;

    foreach (TNS__SalePoint item, response.return_().items())
    {
        TData props;
        QString key = item.code().value();
        props["name"] = item.name();
        props["is_deleted"] = item.isDeleted();
        data.insert(key, props);
    }

    emit dataSalepoinstIsReady(data);
    //FIXME: delete data;
    return true;

}

bool TransportProxy::getProviders() {

    if(this->useSsl && this->ignoreSslErrors)
        m_service.ignoreSslErrors();

    KDSoapAuthentication *auth = new KDSoapAuthentication();
    auth->setUser(m_username);
    auth->setPassword(m_password);

    m_service.clientInterface()->setAuthentication(*auth);

    TNS__GetProviders params;
    params.setApiKey(m_apikey);

    TNS__GetProvidersResponse response;

    response = m_service.getProviders(params);

    if(!m_service.lastError().isEmpty()){

        QString text = m_service.lastError();
        text = text.replace(m_endPoint, "$END_POINT$");
        emit transportError(tr("Ошибка при получении данных [#1]\t\t"), text, true);
        return false;
    }

    TList data;

    foreach (TNS__Provider item, response.return_().items())
    {
        TData props;
        QString key = item.id().value();
        props["name"] = item.name();
        props["is_deleted"] = item.isDeleted();
        props["is_unavailable"] = item.isUnavailable();
        props["price_date"] = item.datePrice().toString("dd.MM - hh:mm");
        data.insert(key, props);
    }

    emit dataProvidersIsReady(data);
    return true;

}

void TransportProxy::operationError(QString method, const KDSoapMessage &error)
{
    QString err=error.faultAsString().replace(m_endPoint,"$END_POINT$");
    emit transportError(tr("Ошибка метода [$%1]").arg(method), err, true);
}

void TransportProxy::getPriceList(QList<QString> providerList) {

    bool ok = this->getSalePoints();
    if(!ok)
        return;

    if(this->useSsl && this->ignoreSslErrors)
        m_service.ignoreSslErrors();

    KDSoapAuthentication *auth = new KDSoapAuthentication();
    auth->setUser(m_username);
    auth->setPassword(m_password);

    m_service.clientInterface()->setAuthentication(*auth);

    TNS__ProviderList provs;
    QList<TNS__Provider> provList;
    for(int i=0; i < providerList.size(); i++){


        QString p_key = providerList.at(i);
        TNS__UUID uuid;
        uuid.setValue(p_key);

        TNS__Provider prov;
        prov.setId(uuid);
       // prov.setName("asdas");
       // prov.setIsUnavailable(false);
       // prov.setDatePrice(KDDateTime());
       // prov.setIsDeleted(false);

       provList << prov;
    }
    provs.setItems(provList);
    qDebug() << provs.items().count();

    TNS__GetPriceList params;
    params.setApiKey(m_apikey);
    params.setProviders(provs);

    TNS__GetPriceListResponse response = m_service.getPriceList(params);

    if(!m_service.lastError().isEmpty()){
        QString text = m_service.lastError();
        text = text.replace(m_endPoint, "$END_POINT$");
        emit transportError(tr("Ошибка при получении данных [#2]"),text, true);
        return;
    }

    int all_counts = response.return_().items().count();
    emit priceDataIsGetted(all_counts);

    TList providers;
    TList products;
    TList synonims;
    TList pricelist;
    int item_counter = 0;
    foreach (TNS__Price item,  response.return_().items()){

        TNS__ProviderProduct provProduct = item.providerProduct();

//        TNS__Provider provider = provProduct.provider();
//        TData provider_data;
//        provider_data["name"]           = provider.name();
//        provider_data["price_update"]   = item.date().toString("dd.MM - hh:mm");
//        provider_data["is_deleted"]     = provider.isDeleted();
//        QString key = provider.id().value();
//        providers.insert(key, provider_data);

        TNS__Product product = provProduct.product();
        TData product_data;
        QString key_prod            = product.id().value();
        product_data["name"]        = product.name();
        product_data["is_vital"]    = product.isVital();
        product_data["is_deleted"]  = product.isDeleted();
        product_data["groupName"]   = product.groupName();
        products.insert(key_prod, product_data);

        TData synonim_data;
        QString key_syn                 = provProduct.id().value();
        synonim_data["code"]            = provProduct.partCode();
        synonim_data["provider_code"]   = provProduct.provider_id().value();
        synonim_data["product_code"]    = provProduct.product().id().value();
        synonim_data["name"]            = provProduct.name();
        synonim_data["is_deleted"]      = provProduct.isDeleted();
        synonim_data["is_cutePrice"]    = provProduct.isCutPrice();
        synonims.insert(key_syn, synonim_data);

        TData price_row;
        QString key_row                 = item.providerProduct().id().value();
        //QDate price_date                = item.date();
        price_row["key"]                = key_row;
        price_row["manufacturer"]       = item.manufacturer();
        price_row["priceValue"]         = item.priceValue();
        price_row["priceVital"]         = item.priceVital();
        price_row["countPricePack"]     = item.countPricePack();
        price_row["expirationDate"]     = item.expirationDate().toString("dd.MM.yyyy");
        price_row["balance"]            = item.balance();
        price_row["is_urgent"]          = item.isUrgent();
        price_row["key"]                = key_row;

        pricelist.insert(QString::number(item_counter++), price_row);

    }

    qDebug() << pricelist.count();
    emit priceDataIsReady(providers, products, synonims, pricelist);

    providers.clear();
    products.clear();
    synonims.clear();
    pricelist.clear();

    //delete &providers;
}

void TransportProxy::sendOrders(const TDataList &data)
{
    if(this->useSsl && this->ignoreSslErrors)
        m_service.ignoreSslErrors();

    KDSoapAuthentication *auth = new KDSoapAuthentication();
    auth->setUser(m_username);
    auth->setPassword(m_password);

    m_service.clientInterface()->setAuthentication(*auth);

    foreach (TData order_data, data)
    {
        int order_id = order_data["id"].toInt();
        TNS__Order order;
        order.setProvider_id(TNS__UUID(order_data["prov_code"].toString()));
        order.setSalepoint_id(TNS__UUID(order_data["sp_code"].toString()));

        QList<TNS__OrderRow> rows;
        TDataList data_rows = qvariant_cast<TDataList>(order_data["rows"]);

        foreach (TData row_data, data_rows)
        {
            TNS__OrderRow row;
            row.setSynonim_id(TNS__UUID(row_data["syn_code"].toString()));
            row.setCount_value(row_data["count"].toInt());
            row.setPrice_value(row_data["price"].toDouble());
            rows << row;
        }

        order.setRows(rows);
        TNS__SendOrder params;
        params.setParams(order);
        TNS__SendOrderResponse response = m_service.sendOrder(params);

        if(!m_service.lastError().isEmpty()){

             transportError(tr("Ошибка при отправке заказа"), m_service.lastError(), true);
             return;
        }

        emit orderIsSended(order_id, response.return_());

    }

    return;
}
