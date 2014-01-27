TEMPLATE = subdirs
SUBDIRS += \
        transport \
        core \
        app \

app.depends = transport core


