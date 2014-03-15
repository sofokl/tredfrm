CREATE TABLE INF (systemid VARCHAR UNIQUE PRIMARY KEY, is_enabled BOOLEAN not null default(0))@
CREATE TABLE ORDERS (
        id INTEGER UNIQUE PRIMARY KEY AUTOINCREMENT,
        salepoint_code VARCHAR NOT NULL,
        provider_code VARCHAR NOT NULL ,
        creation_date VARCHAR DEFAULT (strftime('%d.%m.%Y %H:%M:%S', 'now', 'localtime')),
        remote_number VARCHAR DEFAULT (0),
        sended BOOLEAN NOT NULL DEFAULT (0),
        is_deleted BOOLEAN NOT NULL DEFAULT(0),
        FOREIGN KEY (salepoint_code) REFERENCES SALEPOINTS(code),
        FOREIGN KEY (provider_code) REFERENCES PROVIDERS (code)
)@
CREATE TABLE ORDER_PRODUCTS (
        id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
        synonim_id INTEGER NOT NULL REFERENCES "SYNONIMS" (id) ON DELETE CASCADE ON UPDATE CASCADE,
        order_id INTEGER NOT NULL REFERENCES "ORDERS" (id) ON DELETE CASCADE ON UPDATE CASCADE,
        count_val INTEGER DEFAULT 0.0,
        price DOUBLE DEFAULT 0.0
)@
CREATE TABLE PRICELIST (
    "synonim_id" VARCHAR NOT NULL,
    "manufacturer" VARCHAR NOT NULL,
    "priceValue" FLOAT DEFAULT (0.0),
    "priceVital" FLOAT DEFAULT (0.0),
    "countPricePack" INTEGER DEFAULT (0),
    "expirationDate" DATE DEFAULT (0),
    "balance" INTEGER DEFAULT (0),
    FOREIGN KEY (synonim_id) REFERENCES SYNONIMS(id)
)@
CREATE TABLE PRODUCTS (
    code VARCHAR UNIQUE PRIMARY KEY,
    name VARCHAR NOT NULL,
    groupName TEXT,
    is_vital BOOLEAN not null DEFAULT(0),
    is_deleted BOOLEAN not null DEFAULT(0)
)@
CREATE TABLE PROVIDERS (
        code VARCHAR UNIQUE PRIMARY KEY,
        name VARCHAR NOT NULL,
        price_update DATE TIME DEFAULT 0,
        is_deleted BOOLEAN NOT NULL DEFAULT (0),
        coef FLOAT Default(0.0),
        is_used BOOLEAN NOT NULL DEFAULT(0)
)@
CREATE TABLE SALEPOINTS (
    code VARCHAR UNIQUE PRIMARY KEY,
    name VARCHAR NOT NULL,
    is_default BOOLEAN not null default(0),
    is_deleted BOOLEAN not null default (0)
)@
CREATE TABLE SYNONIMS (
    "id" VARCHAR,
    "code" VARCHAR NOT NULL,
    "provider_code" VARCHAR NOT NULL,
    "product_code" VARCHAR NOT NULL,
    "name" VARCHAR NOT NULL,
    "is_deleted" BOOLEAN NOT NULL DEFAULT (0),
    FOREIGN KEY (provider_code) REFERENCES PROVIDERS(code),
    FOREIGN KEY (product_code) REFERENCES PRODUCTS(code)
)@
CREATE VIEW "V01" AS    SELECT
        PRODUCTS.code,
        SALEPOINTS.code as salepoint_code,
        PRODUCTS.name,
        is_vital,
        (CASE WHEN ifnull((SELECT count(ORDER_PRODUCTS.id)
        from ORDER_PRODUCTS
        INNER JOIN ORDERS ON ORDERS.id = order_id
        INNER JOIN SYNONIMS ON SYNONIMS.id = synonim_id
        where orders.is_deleted = 0
        AND orders.sended = 0
        AND product_code = products.code
        AND orders.salepoint_code = SALEPOINTS.code),0) > 0 THEN 1 ELSE 0 END )
        as InOrder
        FROM SALEPOINTS
        LEFT JOIN PRODUCTS Where PRODUCTS.is_deleted = 0 AND SALEPOINTs.is_deleted = 0 AND SALEPOINTS.is_default = 1@
CREATE VIEW "V03" AS SELECT
        O.id,
        O.provider_code,
        SP.code as salepoint_code,
        O.Sended as Sended,
        O.remote_number as Number,
        O.creation_date as "Date",
        PROV.name,
        SP.name,
        (SELECT count(id) FROM ORDER_PRODUCTS as OP where OP.order_id =O. id ) as Pos,
        (SELECT CAST(ifnull(sum(count_val*price),0) as REAL)
        FROM ORDER_PRODUCTS as OS where OS.order_id = O.id ) as Summ
        FROM orders as O
        INNER JOIN PROVIDERS as PROV ON PROV.code= O.provider_code
        INNER JOIN SALEPOINTS as SP ON SP.code = O.salepoint_code
        LEFT OUTER JOIN ORDER_PRODUCTS as OP ON OP.order_id = O.id
        GROUP BY O.id
        ORDER BY SENDED, "Date"@
CREATE VIEW "v02" AS SELECT
        PRD.Code as ProdCode,
        FUL.SID as SynID,
        Prov.NAme as ProvName,
        PROV.price_update,
        PRD.NAME as ProdName,
        FUL.SynCode,
        FUL.SynName,
        PRICELIST.manufacturer,
        PRICELIST.priceVital,
        PRD.groupName,
        PRICELIST.CountPricePack,
        PRICELIST.expirationDate,
        PRICELIST.PriceValue,
        PRICELIST.balance,
        ifnull(VOR.Count, 0) as ORDER_COUNT,
        ifnull(FUl.SPCODE, 0) as salepoint_code
        FROM PRICELIST
        LEFT JOIN (SELECT id as SID, synonims.code as SynCode,
        synonims.name as SynName,
        product_code as ProdCODE,
        provider_code as ProvCode,
        sp.code as SPCODE from synonims LEFT JOIN SAlepoints as sp WHERE sp.is_default = 1)
        as FUL ON FUL.SID = PRICELIST.synonim_id
        LEFT JOIN V04 as VOR
                               ON VOR.synonim_id = FUL.SID AND
                                      VOR.salepoint_code = FUl.SPCODE AND
                                      sended=0
        LEFT JOIN PRODUCTS as PRD ON PRD.code = FUL.ProdCODE
        LEFT JOIN PROVIDERS as PROV ON PROV.CODE = FUL.ProvCode@
CREATE VIEW "v04" AS
SELECT
        rows.id as id,
        rows.order_id as order_id,
        rows.synonim_id,
        products.name as Product,
        synonims.name as Synonim,
        providers.name as Provider,
        rows.count_val as "Count",
        rows.Price, (rows.count_val * rows.price) as Summ ,
        ORDERS.salepoint_code as salepoint_code,
        ORDERS.sended
        FROM ORDER_PRODUCTS as rows
        INNER JOIN SYNONIMS ON SYNONIMS.id = rows.synonim_id
        INNER JOIN PRODUCTS ON Products.code = synonims.product_code
        INNER JOIN PROVIDERS ON PRoviders.code = synonims.provider_code
        INNER JOIN ORDERS ON ORDERS.id = rows.order_id
        ORDER BY Product@
CREATE TRIGGER clear_products BEFORE DELETE ON ORDERS FOR EACH ROW
BEGIN DELETE FROM ORDER_PRODUCTS WHERE order_id = old.id; END@
CREATE TRIGGER "set_default_salepoint" BEFORE UPDATE OF "is_default" ON SALEPOINTS FOR EACH ROW WHEN "code" != new.code
BEGIN UPDATE SALEPOINTS SET "is_default"=0 where "is_default" != 0; END@
CREATE INDEX "Is_default_salepoint" ON "SALEPOINTS" ("is_default" ASC)@
CREATE INDEX ORDERS_deleted_index ON ORDERS(is_deleted)@
CREATE UNIQUE INDEX ORDERS_id_index_1 ON ORDERS (id)@
CREATE INDEX ORDERS_provider_index ON ORDERS (provider_code)@
CREATE INDEX ORDERS_salepoint_index ON ORDERS (salepoint_code)@
CREATE UNIQUE INDEX ORDER_PRODUCTS_id_index ON ORDER_PRODUCTS (id)@
CREATE INDEX ORDER_PRODUCTS_order_index ON ORDER_PRODUCTS (order_id)@
CREATE INDEX ORDER_PRODUCTS_synonim_index ON ORDER_PRODUCTS (synonim_id)@
CREATE INDEX PRICELIST_synonim_index ON PRICELIST(synonim_id, expirationDate)@
CREATE UNIQUE INDEX PRODUCTS_code_index on PRODUCTS(code)@
CREATE INDEX PRODUCTS_deleted_index ON PRODUCTS(is_deleted)@
CREATE INDEX PRODUCTS_name_index ON PRODUCTS(name ASC)@
CREATE UNIQUE INDEX PROVIDERS_code_index on providers(code)@
CREATE INDEX PROVIDERS_deleted_index ON PROVIDERS(is_deleted)@
CREATE INDEX SALEPOINTS_deleted_index ON SALEPOINTS(is_deleted)@
CREATE UNIQUE INDEX SALEPOINT_code_index on SALEPOINTS (code ASC)@
CREATE INDEX SYNONIMS_deleted_index ON SYNONIMS(is_deleted)@
CREATE UNIQUE INDEX SYNONIMS_id_index ON SYNONIMS (id)@
CREATE UNIQUE INDEX SYNONIMS_index_unique_in_product ON SYNONIMS(product_code, provider_code, code)@
CREATE UNIQUE INDEX SYNONIMS_index_unique_in_provider ON SYNONIMS(code, provider_code)@
