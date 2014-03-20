#include "LkStoreHelper.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStringList>
#include <QDateTime>

LkStoreHelper::LkStoreHelper(QObject *parent, QSqlDatabase db) : QObject(parent)
{
    this->m_db = db;
}

QSqlError LkStoreHelper::startDataBase()
{
    QSqlQuery q(m_db);


    q.exec("PRAGMA KEY='passfor1c'");
    q.exec("PRAGMA FOREIGN_KEYS=ON;");
    q.exec("PRAGMA synchronous=OFF");

    return q.lastError();
}

void LkStoreHelper::writeNewDbSchema(const QByteArray &data)
{
    QString text = data;

    if(!m_db.isOpen()){
        qDebug() << "База данных не открыта.";
    }
    QSqlQuery query(m_db);
    QStringList scriptQueries = text.split('@');
    foreach (QString queryTxt, scriptQueries)
    {
        if (queryTxt.trimmed().isEmpty()) {
            continue;
        }
        if (!query.exec(queryTxt+";"))
        {
            qDebug() << "Error query:" <<query.lastError().text();
        }
        query.finish();
    }
}

void LkStoreHelper::writeDataPrice(TList provs, TList prods, TList syns, TList prices){

    if(!m_db.isOpen()){
        qDebug() << "База данных не открыта.";
        return;
    }

    m_db.transaction();
        qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << "Write Providers";
        writeProviders(provs);
    m_db.commit();
    emit dataIsWriten(QString("Providers"));
    //delete provs;

    m_db.transaction();
        qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << "Write Product";
        writeProducts(prods);
    m_db.commit();
    emit dataIsWriten(QString("Product"));
    //delete &prods;

    m_db.transaction();
        qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << "Write Synonims";
        writeSynonims(syns);
    m_db.commit();
    emit dataIsWriten(QString("Synonims"));
    //delete &syns;

    m_db.transaction();
        qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << "Write Prices";
        writePriceList(prices);
    m_db.commit();
    emit dataIsWriten(QString("Finish"));
    //delete &prices;

}

void LkStoreHelper::writeProviders(const TList &dataList)
{

    foreach (QString key, dataList.keys()) {
        TData data = dataList.value(key);

        QSqlQuery query(m_db);
        query.prepare("INSERT OR IGNORE INTO PROVIDERS (code, name, price_update, is_used, is_deleted, is_unavailable) values (?,?,?,?,?,?)");
        query.bindValue(0, key);
        query.bindValue(1, data["name"]);
        query.bindValue(2, data["price_date"]);
        query.bindValue(3, false);
        query.bindValue(4, data["is_deleted"]);
        query.bindValue(5, data["is_unavailable"]);
        query.exec();

        query.prepare("UPDATE PROVIDERS SET name=?, price_update=?, is_deleted=?, is_unavailable=? WHERE code=?");
        query.bindValue(0, data["name"]);
        query.bindValue(1, data["price_date"]);
        query.bindValue(2, data["is_deleted"]);
        query.bindValue(3, data["is_unavailable"]);
        query.bindValue(4, key);

        if(!query.exec()){
            qDebug() << "Error write provider:" << query.lastError().text();
            return;
        }

    }
    emit dataIsWriten(QString("Providers"));
}

void LkStoreHelper::writeProducts(const TList &dataList)
{

    foreach (QString key, dataList.keys()) {
        TData data = dataList.value(key);

        //m_db.transaction();
        QSqlQuery query(m_db);
        query.prepare("INSERT OR IGNORE INTO PRODUCTS (code, name, groupName, is_vital, is_deleted) values (?,?,?,?,?)");
        query.bindValue(0, key);
        query.bindValue(1, data["name"]);
        query.bindValue(2, data["groupName"]);
        query.bindValue(3, data["is_vital"]);
        query.bindValue(4, data["is_deleted"]);
        query.exec();

        query.prepare("UPDATE PRODUCTS SET name=?, groupName=?, is_vital=?, is_deleted=? WHERE code=?");
        query.bindValue(0, data["name"]);
        query.bindValue(1, data["groupName"]);
        query.bindValue(2, data["is_vital"]);
        query.bindValue(3, data["is_deleted"]);
        query.bindValue(4, key);

        if(!query.exec()){
            qDebug() << "Error write product:" << query.lastError().text();
            m_db.rollback();
            return;
        }
        //m_db.commit();

    }

}

void LkStoreHelper::writeSynonims(const TList &dataList)
{

    foreach (QString key, dataList.keys()) {
        TData data = dataList.value(key);

        QSqlQuery query(m_db);
        query.prepare("INSERT OR IGNORE INTO SYNONIMS (id, code, provider_code, product_code, name, is_cutePrice, is_deleted) "
                      "values (?,?,?,?,?,?,?)");
        query.bindValue(0, key);
        query.bindValue(1, data["code"]);
        query.bindValue(2, data["provider_code"]);
        query.bindValue(3, data["product_code"]);
        query.bindValue(4, data["name"]);
        query.bindValue(5, data["is_cutePrice"]);
        query.bindValue(6, data["is_deleted"]);
        query.exec();

        query.prepare("UPDATE SYNONIMS SET code=?, provider_code=?, product_code=?, name=?, is_cutePrice=?, is_deleted=? "
                      "WHERE id=?");
        query.bindValue(0, data["code"]);
        query.bindValue(1, data["provider_code"]);
        query.bindValue(2, data["product_code"]);
        query.bindValue(3, data["name"]);
        query.bindValue(4, data["is_cutePrice"]);
        query.bindValue(5, data["is_deleted"]);
        query.bindValue(6, key);

        if(!query.exec()){
            qDebug() << "Error write synonims:" << query.lastError().text();
            m_db.rollback();
            continue;
        }

    }

}

void LkStoreHelper::writePriceList(const TList &dataList)
{
    qDebug() << "Price List count: " << dataList.keys().count();
    QSqlQuery query(m_db);

    if(!query.exec("DELETE FROM PRICELIST;")){
        qDebug() << "Error clear current pricelist:" << query.lastError().text();
        return;
    }
    int counter = 0;
    foreach (QString rowkey, dataList.keys()) {
        counter ++;
        TData row = dataList.value(rowkey);

        query.clear();
        query.prepare("INSERT INTO PRICELIST "
                      "(synonim_id, manufacturer, priceValue, priceVital, countPricePack, expirationDate, balance, is_urgent) "
                      "values (?,?,?,?,?,?,?,?)");
        query.bindValue(0, row["key"]);
        query.bindValue(1, row["manufacturer"]);
        query.bindValue(2, row["priceValue"]);
        query.bindValue(3, row["priceVital"]);
        query.bindValue(4, row["countPricePack"]);
        query.bindValue(5, row["expirationDate"]);
        query.bindValue(6, row["balance"]);
        query.bindValue(7, row["is_urgent"]);

        if(!query.exec()){
            qDebug() << counter << "- Error write current price:" << query.lastError().text() << rowkey;
            continue;
        }
        emit dataIsWriten(QString("PriceList"), counter-1);
    }
    emit dataIsWriten(QString("PriceList"), counter);
}

void LkStoreHelper::writeSalePoints(const TList &dataList)
{

    if(!m_db.isOpen())
        return;

    foreach (QString key, dataList.keys()) {
        TData item = dataList.value(key);
        QSqlQuery query(m_db);
        m_db.transaction();
        query.prepare("INSERT OR IGNORE INTO SALEPOINTS (code, name, is_deleted) "
                      "values (?,?,?);");
        query.bindValue(0, key);
        query.bindValue(1, item["name"]);
        query.bindValue(2, item["is_deleted"]);
        query.exec();

        query.prepare("UPDATE SALEPOINTS SET name=?, is_deleted=? WHERE code=?");
        query.bindValue(0, item["name"]);
        query.bindValue(1, item["is_deleted"]);
        query.bindValue(2, key);

        if(!query.exec()){
            qDebug() << "Error write salepoint:" << query.lastError().text();
            m_db.rollback();
            return;
        }
        m_db.commit();
    }

    emit dataIsWriten(QString("SalePoints"));
}

void LkStoreHelper::setCurrentSpCode(QString &code)
{
    m_currentSalepointCode = code;

    QSqlQuery query(m_db);
    query.prepare("UPDATE Salepoints SET is_default=1 where code = ?");
    query.bindValue(0, code);

    if(!query.exec()){
        qDebug() << "Error update salepoint:" << query.lastError().text();
        return;
    }
}

QString LkStoreHelper::countValueChanged(QString &synonim_id, int new_value, double price, QString &expDate, QString other_sp)
{
    QString salepoint = m_currentSalepointCode;
    if(!other_sp.isEmpty())
        salepoint = other_sp;


    QSqlQuery query(m_db);
    QString text("select VOR.id as ID, vor.count_val as VAL from order_products as VOR "
                 "INNER JOIN orders ON orders.id = vor.order_id "
                 "WHERE vor.synonim_id = ? AND orders.sended = 0 AND vor.expirationDate = ? "
                 "AND orders.salepoint_code = ? LIMIT 1");

    query.prepare(text);
    query.bindValue(0, synonim_id);
    query.bindValue(1, expDate);
    query.bindValue(2, salepoint);

    if(!query.exec())
        return query.lastError().text().prepend("Get last value");

    query.next();

    int old_value = query.record().value(1).toInt();
    if(new_value == old_value) //No Change ? if price updated
        return QString();

    int order_id = getOrderBySynonim(synonim_id, salepoint);

    int order_products_row = query.record().value(0).toInt();

    if(new_value == 0 && order_products_row !=0) //DELETE FROM order
    {
        query.clear();
        query.prepare("DELETE FROM order_products WHERE id = ?");
        query.bindValue(0, order_products_row);


        if(!query.exec())
            return query.lastError().text().prepend("Delete from order");

        emit orderIsChanged(order_id);
        return QString();
    }

    if(order_products_row !=0)  //UPDATING ? if price updated
    {
        query.clear();
        query.prepare("UPDATE order_products SET count_val = ? where id = ?");
        query.bindValue(0, new_value);
        query.bindValue(1, order_products_row);
        if(!query.exec())
            return query.lastError().text().prepend("Update row order");

        emit orderIsChanged(order_id);
        return QString();
    }

    //INSERTING TO ORDER
    query.clear();
    query.prepare("INSERT into order_products (synonim_id, expirationDate, order_id, count_val, price) "
                  "VALUES (?,?, ?, ?, ?)");
    query.bindValue(0, synonim_id);
     query.bindValue(1, expDate);
    query.bindValue(2, order_id);
    query.bindValue(3, new_value);
    query.bindValue(4, price);

    if(!query.exec())
        return query.lastError().text().prepend("Insert to order: ");

    emit orderIsChanged(order_id);
    return QString();

}

int LkStoreHelper::getOrderBySynonim(const QString &synonim_id, const QString sp_code)
{

    QString text = "select orders.id as order_id from orders "
                   "LEFT JOIN SYNONIMS ON SYNONIMS.provider_code = orders.provider_code "
                   "WHERE orders.sended = 0 AND synonims.id = ? AND orders.salepoint_code = ? LIMIT 1";

    QSqlQuery query(m_db);
    query.prepare(text);
    query.bindValue(0, synonim_id);
    query.bindValue(1, sp_code);
    query.exec();
    query.next();

    int order_id = query.record().value("order_id").toInt();

    if(order_id == 0) //CREATE NEW ORDER
    {
        query.clear();
        query.prepare("INSERT INTO orders (salepoint_code, provider_code) "
                      "values(?, (select syn.provider_code from synonims as syn "
                      "where syn.id = ?))");
        query.bindValue(0, sp_code);
        query.bindValue(1, synonim_id);
        query.exec();

        order_id = query.lastInsertId().toInt();
    }

    return order_id;

}

QSqlError LkStoreHelper::deleteOrder(QString &id) {

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM ORDERS WHERE id= ?");
    query.bindValue(0, id);
    query.exec();

    return query.lastError();

}

QSqlError LkStoreHelper::clearOrder(QString &id) {

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM ORDER_PRODUCTS WHERE order_id= ?");
    query.bindValue(0, id);
    query.exec();

    return query.lastError();

}

TDataList LkStoreHelper::getOrdersData(const QVector<int> &order_ids, bool current_sp)
{
    TDataList data;
    QVector<int> vector = order_ids;

    if(vector.isEmpty()) {
        QSqlQuery query(m_db);
        QString text;
        text = "SELECT id FROM ORDERS WHERE sended = 0 ";
        if(current_sp){
            text += " AND salepoint_code = ?";
            query.prepare(text);
            query.bindValue(0, m_currentSalepointCode);
        }
        else {
            query.prepare(text);
        }

        query.exec();

        while(query.next())
        {
            vector << query.record().value(0).toInt();
        }

    }

    foreach (int id, vector)
    {
        qDebug() << "Prepare data order" << id;
        QSqlQuery query(m_db);
        query.prepare("SELECT salepoint_code as spc, provider_code as provc, "
                      "synonim_id as syn_id, count_val as val, price "
                      "FROM order_products "
                      "INNER join ORDERS ON ORDERS.id = order_id "
                      "WHERE order_id =?");
        query.bindValue(0, id);

        query.exec();

        TData order;
        TDataList rows;

        while(query.next()){

            TData product;

            order["id"] = id;
            order["sp_code"] = query.record().value("spc").toString();
            order["prov_code"] = query.record().value("provc").toString();
            product["syn_code"] = query.record().value("syn_id").toString();
            product["count"] = query.record().value("val").toInt();
            product["price"] = query.record().value("price").toDouble();

            rows << product;
        }
        if(!order.isEmpty()){
            order["rows"] = QVariant::fromValue(rows);
            data << order;
        }
    }

    return data;
}

void LkStoreHelper::writeOrderAsSended(int id, QString number) {

    QSqlQuery query(m_db);
    query.prepare("UPDATE ORDERS SET sended=1, remote_number=? where id = ?;");
    query.bindValue(0, number);
    query.bindValue(1, id);


    if(!query.exec()) {
        qDebug() << "Error Set ORder as sended " << query.lastError().text();
        return;
    }

    emit orderIsChanged(id, true);
}

QList<QString> LkStoreHelper::getUsedProviderList() {

    QList<QString> providers;

    QSqlQuery query(m_db);
    QString text("SELECT code, name FROM PROVIDERS WHERE %1");

    text  = text.arg("is_deleted=0 AND is_used=1");

    query.prepare(text);
    if(!query.exec()){
        qDebug() << "Error get providers " << query.lastError().text();
        return providers;
    }

    while(query.next()){
        QString code = query.record().value(0).toString();
        providers << code;
    }

    return providers;

}

QHash<QString, QString> LkStoreHelper::getLocalProviders() {

    QHash<QString, QString> providers;

    QSqlQuery query(m_db);
    QString text("SELECT code, name FROM PROVIDERS WHERE %1");

    text  = text.arg("is_deleted=0 AND is_used=1");

    query.prepare(text);
    if(!query.exec()){
        qDebug() << "Error get providers " << query.lastError().text();
        return providers;
    }

    while(query.next()){
        QString code = query.record().value(0).toString();
        QString name = query.record().value(1).toString();
        providers.insert(code, name);
    }

    return providers;

}

QHash<QString, QString> LkStoreHelper::getSalepoints() {

    QHash<QString, QString> salepoints;

    QSqlQuery query(m_db);
    query.prepare("SELECT code, name FROM SALEPOINTS WHERE is_deleted = 0");
    if(!query.exec()){
        qDebug() << "Error get salepoint " << query.lastError().text();
        return salepoints;
    }

    while(query.next()){
        QString code = query.record().value(0).toString();
        QString name = query.record().value(1).toString();
        salepoints.insert(code, name);
    }

    return salepoints;

}

bool LkStoreHelper::chechState(QString sn) {

    qDebug() << "TODO: check activate";
    return false;
}

QString LkStoreHelper::addToOrderByCode(TData params){

    QString salepoint   = params["sale_code"].toString();
    QString provider    = params["prov_code"].toString();
    QString syn_code    = params["syn_code"].toString();
    int count           = params["count"].toInt();
    if(count <= 0)
        return tr("Неверное количество (значение в файле '%1')").arg(params["count"].toString());

    QSqlQuery query(m_db);
    query.prepare("Select id as SynId, priceValue as price FROM synonims "
                  "INNER JOIN PRICELIST ON synonim_id = SynId "
                  "WHERE synonims.code=? AND synonims.provider_code = ?");
    query.bindValue(0, syn_code);
    query.bindValue(1, provider);

    if(!query.exec()){
        return query.lastError().text();
    }

    if(!query.next())
        return tr("Не удалось сопоставить товары в базе и в файле (неверный код или поставщик)");

    QString syn_id = query.record().value(0).toString();
    double price = query.record().value(1).toDouble();

    QString er = countValueChanged(syn_id, count, price, salepoint);
    return er;

}

QString LkStoreHelper::getTextPriceView(){

    QString textQuery("SELECT PRD.Code as ProdCode, FUL.SID as SynID, Prov.NAme as ProvName, "
                       "PROV.price_update, PRD.NAME as ProdName, FUL.SynCode, FUL.SynName, "
                       "PRICELIST.manufacturer, PRICELIST.priceVital, PRD.groupName, "
                       "PRICELIST.CountPricePack, PRICELIST.expirationDate, "
                       "PRICELIST.PriceValue * Prov.coef as PriceCoef, PRICELIST.PriceValue, "
                       "PRICELIST.balance, ifnull(VOR.Count, 0) as ORDER_COUNT, "
                       "PRICELIST.PriceValue * (1 + FUL.markups / 100) as SalePrice, "
                       "ifnull(FUl.SPCODE, 0) as salepoint_code, PRICELIST.is_urgent, FUL.is_cutePrice "
                       "FROM PRICELIST "
                       "LEFT JOIN (SELECT id as SID, synonims.code as SynCode, "
                       "synonims.name as SynName, product_code as ProdCODE, "
                       "provider_code as ProvCode, sp.code as SPCODE, sp.markups, synonims.is_cutePrice from synonims "
                       "LEFT JOIN SAlepoints as sp WHERE sp.is_default = 1 AND "
                       "product_code = ?) as FUL ON FUL.SID = PRICELIST.synonim_id "
                       "LEFT JOIN V04 as VOR ON VOR.synonim_id = FUL.SID AND "
                       "VOR.salepoint_code = FUl.SPCODE AND sended=0 AND VOR.expDate = PRICELIST.expirationDate "
                       "LEFT JOIN PRODUCTS as PRD ON PRD.code = FUL.ProdCODE "
                       "LEFT JOIN PROVIDERS as PROV ON PROV.CODE = FUL.ProvCode AND PROV.is_used = 1 "
                       "WHERE PRD.code  = ?");
    return textQuery;

}
