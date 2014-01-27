#ifndef LKSTOREHELPER_H
#define LKSTOREHELPER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QByteArray>

#include "core_global.h"


typedef QMap<QString, QVariant> TData;
typedef QList<TData> TDataList;
typedef QHash<QString, TData> TList;

class CORESHARED_EXPORT LkStoreHelper : public QObject
{
    Q_OBJECT
public:
    explicit LkStoreHelper(QObject *parent, QSqlDatabase db);
    QSqlError startDataBase();
    
signals:
    void dataIsWriten(QString object, int step=-1);
    void orderIsChanged(int id, bool show_progress=false);
    
public slots:
    void writeNewDbSchema(const QByteArray &data);
    void writeSalepoints(const TList &dataList);
    void writeOrderAsSended(int id, QString number);
    void writeDataPrice(TList provs, TList prods, TList syns, TList prices);

    void setCurrentSpCode(QString &code);
    QString countValueChanged(QString &synonim_id, int new_value, double price, QString other_sp="");

    QSqlError deleteOrder(QString &id);
    QSqlError clearOrder(QString &id);

    TDataList getOrdersData(const QVector<int> &order_ids, bool current_sp=true);

    QHash<QString, QString> getProviders();
    QHash<QString, QString> getSalepoints();

    QString addToOrderByCode(TData params);

private slots:
    int getOrderBySynonim(const QString &synonim_id, const QString sp_code);
    void writeProviders(const TList &dataList);
    void writeProducts(const TList &dataList);
    void writeSynonims(const TList &dataList);
    void writePriceList(const TList &dataList);


private:
    QSqlDatabase m_db;
    QString m_currentSalepointCode;
    
};

#endif // LKSTOREHELPER_H
