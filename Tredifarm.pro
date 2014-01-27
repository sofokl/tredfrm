TEMPLATE = subdirs
SUBDIRS += \
        transport \
        core \
        app \
        updater

app.depends = transport core


