/***************************************************************************
 *   FreeMedicalForms                                                      *
 *   Copyright (C) 2008-2009 by Eric MAEKER                                *
 *   eric.maeker@free.fr                                                   *
 *   All rights reserved.                                                  *
 *                                                                         *
 *   This program is a free and open source software.                      *
 *   It is released under the terms of the new BSD License.                *
 *                                                                         *
 *   Redistribution and use in source and binary forms, with or without    *
 *   modification, are permitted provided that the following conditions    *
 *   are met:                                                              *
 *   - Redistributions of source code must retain the above copyright      *
 *   notice, this list of conditions and the following disclaimer.         *
 *   - Redistributions in binary form must reproduce the above copyright   *
 *   notice, this list of conditions and the following disclaimer in the   *
 *   documentation and/or other materials provided with the distribution.  *
 *   - Neither the name of the FreeMedForms' organization nor the names of *
 *   its contributors may be used to endorse or promote products derived   *
 *   from this software without specific prior written permission.         *
 *                                                                         *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   *
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     *
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS     *
 *   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE        *
 *   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,  *
 *   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,  *
 *   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;      *
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER      *
 *   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT    *
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN     *
 *   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
 *   POSSIBILITY OF SUCH DAMAGE.                                           *
 ***************************************************************************/
/***************************************************************************
 *   Main Developper : Eric MAEKER, <eric.maeker@free.fr>                  *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#ifndef DATABASESELECTORWIDGET_H
#define DATABASESELECTORWIDGET_H

#include <coreplugin/ioptionspage.h>

#include <QPointer>
#include <QWidget>
#include <QStringListModel>

/**
 * \file databaseselectorwidget.h
 * \author Eric MAEKER <eric.maeker@free.fr>
 * \version 0.4.0
 * \date 02 Mar 2010
*/


namespace DrugsDB {
class DatabaseInfos;
}

namespace Core {
class ISettings;
}

namespace DrugsWidget {
namespace Internal {
class DatabaseSelectorWidgetPrivate;
namespace Ui {
class DatabaseSelectorWidget;
}

class DatabaseSelectorWidget : public QWidget
{
    Q_OBJECT
public:
    DatabaseSelectorWidget(QWidget *parent = 0);
    ~DatabaseSelectorWidget();

    void setDatasToUi();
    static void writeDefaultSettings(Core::ISettings *s = 0);

public Q_SLOTS:
    void saveToSettings(Core::ISettings *s = 0);

protected:
    void changeEvent(QEvent *e);

private Q_SLOTS:
    void getAllAvailableDatabases();
    void updateDatabaseInfos(int row);
    void addPath();
    void removePath();
    void tooglePaths();

private:
    Ui::DatabaseSelectorWidget *ui;
    DatabaseSelectorWidgetPrivate *d;
};

}  // End namespace Internal


class DrugsDatabaseSelectorPage : public Core::IOptionsPage
{
public:
    DrugsDatabaseSelectorPage(QObject *parent = 0);
    ~DrugsDatabaseSelectorPage();

    QString id() const;
    QString name() const;
    QString category() const;

    void resetToDefaults();
    void checkSettingsValidity();
    void applyChanges();
    void finish();

    QString helpPage() {return "multiple_drugs_databases.html";}

    static void writeDefaultSettings(Core::ISettings *s) {Internal::DatabaseSelectorWidget::writeDefaultSettings(s);}

    QWidget *createPage(QWidget *parent = 0);

private:
    QPointer<Internal::DatabaseSelectorWidget> m_Widget;
};

}  // End namespace DrugsWidget


#endif // DATABASESELECTORWIDGET_H
