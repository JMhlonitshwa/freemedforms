TEMPLATE = lib
TARGET = Templates
PACKAGE_VERSION = 0.0.2
DEFINES += TEMPLATES_LIBRARY
include(../fmf_plugins.pri)
include( templatesplugin_dependencies.pri )
QT += sql
HEADERS = templatesplugin.h \
    templates_exporter.h \
    itemplates.h \
    itemplateprinter.h \
    templatebase.h \
    templatesmodel.h \
    constants.h \
    templatesview.h \
    templatesview_p.h \
    templateseditdialog.h \
    templatespreferencespages.h \
    templatescreationdialog.h
SOURCES = templatesplugin.cpp \
    templatebase.cpp \
    templatesmodel.cpp \
    templatesview.cpp \
    templateseditdialog.cpp \
    templatespreferencespages.cpp \
    templatescreationdialog.cpp \
    itemplates.cpp
OTHER_FILES = Templates.pluginspec
FORMS += templatesview.ui \
    templateseditdialog.ui \
    templatescontenteditor.ui \
    templatespreferenceswidget.ui \
    templatescreationdialog.ui

# Translators
TRANSLATIONS += $${SOURCES_TRANSLATIONS}/templatesplugin_fr.ts \
                $${SOURCES_TRANSLATIONS}/templatesplugin_de.ts \
                $${SOURCES_TRANSLATIONS}/templatesplugin_es.ts
