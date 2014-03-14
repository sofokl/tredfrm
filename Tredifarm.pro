TEMPLATE = subdirs
SUBDIRS += \
        sqlcipher3 \
        qsqlcipher \
        transport \
        core \
        app \
        updater

app.depends = qsqlcipher transport core
qsqlcipher.depends = sqlcipher3




