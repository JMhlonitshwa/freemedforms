/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2011 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  The FreeAccount plugins are free, open source FreeMedForms' plugins.   *
 *  (C) 2010-2011 by Pierre-Marie Desombre, MD <pm.desombre@medsyn.fr>     *
 *  and Eric Maeker, MD <eric.maeker@gmail.com>                            *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program (COPYING.FREEMEDFORMS file).                   *
 *  If not, see <http://www.gnu.org/licenses/>.                            *
 ***************************************************************************/
/***************************************************************************
 *  Main Developpers : Pierre-Marie DESOMBRE <pm.desombre@medsyn.fr>,      *
 *                     Eric MAEKER, <eric.maeker@gmail.com>                *
 *  Contributors :                                                         *
 *      NAME <MAIL@ADRESS>                                                 *
 ***************************************************************************/
#ifndef SITESPAGE_H
#define SITESPAGE_H
#include "ui_sitespage.h"
#include <coreplugin/ioptionspage.h>
#include <accountbaseplugin/workingplacesmodel.h>

#include <QPointer>
#include <QString>
#include <QDataWidgetMapper>
#include <QHash>
#include <QSpinBox>


namespace Core {
class ISettings;
}

namespace AccountDB {
class SitesModel;
}

namespace Account {
namespace Internal {

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////  SitesWidget  ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SitesWidget : public QWidget, private Ui::SitesWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SitesWidget)

public:
    explicit SitesWidget(QWidget *parent = 0);
    ~SitesWidget();
    void setDatasToUi();

    static void writeDefaultSettings( Core::ISettings *s );

public Q_SLOTS:
    void saveToSettings(Core::ISettings *s = 0);
    void on_wpComboBox_currentIndexChanged(int index);
    void on_addButton_clicked();
    void on_deleteButton_clicked();
    void findCityFromZipCode(const QString & zipCodeText);
   // void on_name_textChanged(const QString & text);

private:
    void changeEvent(QEvent *e);
    void showEvent(QShowEvent *event);
    void saveModel();
    QHash<QString,QString> parseZipcodeCsv();
    QStringList listOfCountries();
    QString findCityFromZipCode();
    int calcSitesUid();
    void fillHugeWidgets();

private:
    AccountDB::WorkingPlacesModel *m_Model;
    QSpinBox *m_siteUidLabel;
    QDataWidgetMapper *m_Mapper;
    QString m_user_uid;
    QString m_user_fullName;
    QHash<QString,QString> m_hashTownZip;
};

}  // End Internal


class SitesPage : public Core::IOptionsPage
{
public:
    SitesPage(QObject *parent = 0);
    ~SitesPage();

    QString id() const;
    QString name() const;
    QString category() const;
    QString title() const {return name();}

    void resetToDefaults();
    void checkSettingsValidity();
    void applyChanges();
    void finish();

    QString helpPage() {return QString();}

    static void writeDefaultSettings(Core::ISettings *s) {Internal::SitesWidget::writeDefaultSettings(s);}

    QWidget *createPage(QWidget *parent = 0);
private:
    QPointer<Internal::SitesWidget> m_Widget;
};

}  // End namespace Account

#endif // SitesSPAGE_H
