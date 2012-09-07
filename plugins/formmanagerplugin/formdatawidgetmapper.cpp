/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2012 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
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
 *   Main developers : Eric MAEKER, <eric.maeker@gmail.com>                *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
/**
 * \class Form::Internal::FormDataWidgetMapper
 * This mapper acts on forms (Form::FormMain) like a QDataWidgetMapper.\n
 * This mapper manages the UI's creation (according to the current form) and the
 * mapping ui <-> FormItemData.\n
 * It is connected to the central Form::FormTreeModel and the Form::EpisodeModel accessible from
 * the form manager.
 * \sa Form::FormManager, Form::FormManager::formTreeModel(), Form::FormManager::episodeModel()
 */

#include "formdatawidgetmapper.h"
#include <formmanagerplugin/formmanager.h>
#include <formmanagerplugin/iformitem.h>
#include <formmanagerplugin/iformitemdata.h>
#include <formmanagerplugin/iformwidgetfactory.h>
#include <formmanagerplugin/episodemodel.h>
#include <formmanagerplugin/constants_db.h>
#include <formmanagerplugin/constants_settings.h>

#include <coreplugin/icore.h>
#include <coreplugin/ipatient.h>
#include <coreplugin/iuser.h>

#include <utils/log.h>
#include <utils/global.h>

#include <QScrollArea>
#include <QStackedLayout>
#include <QTextBrowser>

using namespace Form;
using namespace Internal;

enum {WarnLogChronos=true};

static inline Form::FormManager *formManager() { return Form::FormManager::instance(); }
static inline Core::IUser *user() {return Core::ICore::instance()->user();}
static inline Core::IPatient *patient() {return Core::ICore::instance()->patient();}

namespace Form {
namespace Internal {
class FormDataWidgetMapperPrivate
{
public:
    FormDataWidgetMapperPrivate(FormDataWidgetMapper *parent) :
        _stack(0),
        _formMain(0),
        _episodeModel(0),
        q(parent)
    {
    }

    ~FormDataWidgetMapperPrivate()
    {}

    void clearStackLayout()
    {
        if (_stack)
            delete _stack;
        _stack = new QStackedLayout(q);
        q->setLayout(_stack);
    }

    void populateStack(Form::FormMain *rootForm)
    {
        Q_ASSERT(_stack);
        if (!_stack)
            return;
        clearStackLayout();
        _formMain = rootForm;

        // Add the synthesis form
//        QScrollArea *sa = new QScrollArea(q);
//        sa->setWidgetResizable(true);
//        QWidget *w = new QWidget(sa);
//        sa->setWidget(w);
//        QVBoxLayout *vl = new QVBoxLayout(w);
//        vl->setSpacing(0);
//        vl->setMargin(0);
//        QTextBrowser *t = new QTextBrowser(w);
//        t->setReadOnly(true);
//        t->setEnabled(true);
//        vl->addWidget(t);
//        int id = _stack->addWidget(sa);
//        _stackId_FormUuid.insert(id, Constants::PATIENTLASTEPISODES_UUID);

        // add all form's widgets
        if (!rootForm)
            return;

        QList<Form::FormMain *> forms;
        forms << _formMain;
        forms << _formMain->flattenFormMainChildren();

        foreach(FormMain *form, forms) {
            if (form->formWidget()) {
                QScrollArea *sa = new QScrollArea(_stack->parentWidget());
                sa->setWidgetResizable(true);
                QWidget *w = new QWidget(sa);
                sa->setWidget(w);
                QVBoxLayout *vl = new QVBoxLayout(w);
                vl->setSpacing(0);
                vl->setMargin(0);
                vl->addWidget(form->formWidget());
                int id = _stack->addWidget(sa);
                _stackId_FormUuid.insert(id, form->uuid());
//                form->formWidget()->setEnabled(false);
            }
        }
    }

    void useEpisodeModel(Form::FormMain *rootForm)
    {
        // EpisodeModel from the FormManager must not be deleted
        if (_episodeModel) {
            _episodeModel = 0;
        }
        _episodeModel = formManager()->episodeModel(rootForm);
    }

    QString getCurrentXmlEpisode()
    {
        if (!_formMain)
            return QString::null;

        QHash<QString, FormItem *> items;
        foreach(FormItem *it, _formMain->flattenFormItemChildren()) {
            if (it->itemData()) {
                items.insert(it->uuid(), it);
            }
        }
        // create the XML episode file
        QHash<QString, QString> xmlData;
        foreach(FormItem *it, items) {
            xmlData.insert(it->uuid(), it->itemData()->storableData().toString());
        }
        return Utils::createXml(Form::Constants::XML_FORM_GENERAL_TAG, xmlData, 2, false);
    }

    // Set the current episode into the form and populate the patientmodel too
    void setCurrentEpisode(const QModelIndex &index)
    {
        if (!_episodeModel) {
            if (_formMain)
                LOG_ERROR_FOR(q, "No episode model. FormUid: " + _formMain->uuid());
            else
                LOG_ERROR_FOR(q, "No episode model. FormUid: (0x0)");
            return;
        }
        _currentEpisode = index;

        if (!index.isValid())
            return;

        // show the form widgets
        int stackIndex;
        (_formMain) ? stackIndex = _stackId_FormUuid.key(_formMain->uuid()) : stackIndex = 0;
        _stack->setCurrentIndex(stackIndex);

        QTime chrono;
        if (WarnLogChronos)
            chrono.start();

        _formMain->clear();
        _formMain->formWidget()->setEnabled(false);

        QModelIndex xmlIndex = _episodeModel->index(index.row(), EpisodeModel::XmlContent);
        const QString &xml = _episodeModel->data(xmlIndex).toString();

        QHash<QString, FormItem *> items;
        QHash<QString, QString> datas;
        if (!xml.isEmpty()) {
            // read the xml'd content
            if (!Utils::readXml(xml, Form::Constants::XML_FORM_GENERAL_TAG, datas, false)) {
                QModelIndex uid = _episodeModel->index(index.row(), EpisodeModel::Uuid);
                LOG_ERROR_FOR(q, QString("Error while reading episode content (%1)").arg(_episodeModel->data(uid).toString()));
                return;
            }

            // put datas into the FormItems of the form
            foreach(FormItem *it, _formMain->flattenFormItemChildren()) {
                items.insert(it->uuid(), it);
            }
        }

        // Populate the FormMain item data (username, userdate, label)
        QModelIndex userName = _episodeModel->index(index.row(), EpisodeModel::UserCreatorName);
        QModelIndex userDate = _episodeModel->index(index.row(), EpisodeModel::UserDate);
        QModelIndex label = _episodeModel->index(index.row(), EpisodeModel::Label);
        _formMain->itemData()->setData(IFormItemData::ID_EpisodeDate, _episodeModel->data(userDate));
        _formMain->itemData()->setData(IFormItemData::ID_EpisodeLabel, _episodeModel->data(label));
        _formMain->itemData()->setData(IFormItemData::ID_UserName, _episodeModel->data(userName));
        _formMain->itemData()->setStorableData(false); // equal: _formMain->itemData()->setModified(false)

        // Populate FormItem data and the patientmodel
        foreach(FormItem *it, items.values()) {
            if (!it) {
                LOG_ERROR_FOR(q, "activateForm :: ERROR: no item: " + items.key(it));
                continue;
            }
            if (!it->itemData())
                continue;

            it->itemData()->setStorableData(datas.value(it->uuid()));
//            if (feedPatientModel && it->patientDataRepresentation() >= 0)
            if (it->patientDataRepresentation() >= 0)
                    patient()->setValue(it->patientDataRepresentation(), it->itemData()->data(it->patientDataRepresentation(), IFormItemData::PatientModelRole));
        }

        _formMain->formWidget()->setEnabled(true);
        if (WarnLogChronos)
            Utils::Log::logTimeElapsed(chrono, q->objectName(), "feedFormWithEpisodeContent");
    }

public:
    QStackedLayout *_stack;
    QHash<int, QString>_stackId_FormUuid;
    Form::FormMain *_formMain;
    EpisodeModel *_episodeModel;
    QPersistentModelIndex _currentEpisode;

private:
    FormDataWidgetMapper *q;
};
}  // namespace Internal
}  // namespace Form

FormDataWidgetMapper::FormDataWidgetMapper(QWidget *parent) :
    QWidget(parent),
    d(new FormDataWidgetMapperPrivate(this))
{
    setObjectName("FormDataWidgetMapper");
}

FormDataWidgetMapper::~FormDataWidgetMapper()
{
}

bool FormDataWidgetMapper::initialize()
{
    return true;
}

bool FormDataWidgetMapper::isDirty() const
{
    if (!d->_formMain)
        return false;

    // form isModified() (using storableData)
    if (d->_formMain->itemData() && d->_formMain->itemData()->isModified()) {
        qWarning() << "FormDataWidgetMapper::isDirty" << d->_formMain->uuid() << d->_formMain->itemData()->isModified();
        return true;
    }
    // ask all current form item data
    foreach(FormItem *it, d->_formMain->flattenFormItemChildren()) {
        if (it->itemData() && it->itemData()->isModified()) {
            qWarning() << "FormDataWidgetMapper::isDirty" << it->uuid() << it->itemData()->isModified();
            return true;
        }
    }
    return false;
}

void FormDataWidgetMapper::setCurrentForm(const QString &formUid)
{
    setCurrentForm(formManager()->form(formUid));
}

void FormDataWidgetMapper::setCurrentForm(Form::FormMain *form)
{
    d->clearStackLayout();
    if (!form)
        return;
    qWarning() << "FormDataWidgetMapper::setCurrentForm" << form->uuid();
    d->populateStack(form);
    d->useEpisodeModel(form);
    if (d->_formMain->itemData())
        d->_formMain->itemData()->setStorableData(false);  // equal == form->setModified(false);
}

void FormDataWidgetMapper::setCurrentEpisode(const QVariant &uid)
{
}

void FormDataWidgetMapper::setCurrentEpisode(const QModelIndex &index)
{
    WARN_FUNC;
    d->setCurrentEpisode(index);
}

bool FormDataWidgetMapper::submit()
{
    qWarning() << "FormDataWidgetMapper::submit";
    const QString &xml = d->getCurrentXmlEpisode();
    QModelIndex xmlIndex = d->_episodeModel->index(d->_currentEpisode.row(), EpisodeModel::XmlContent);
    if (!d->_episodeModel->setData(xmlIndex, xml))
        return false;

    QModelIndex userName = d->_episodeModel->index(d->_currentEpisode.row(), EpisodeModel::UserCreatorName);
    QModelIndex userDate = d->_episodeModel->index(d->_currentEpisode.row(), EpisodeModel::UserDate);
    QModelIndex label = d->_episodeModel->index(d->_currentEpisode.row(), EpisodeModel::Label);

    d->_episodeModel->setData(label, d->_formMain->itemData()->data(IFormItemData::ID_EpisodeLabel));
    d->_episodeModel->setData(userName, d->_formMain->itemData()->data(IFormItemData::ID_UserName));
    d->_episodeModel->setData(userDate, d->_formMain->itemData()->data(IFormItemData::ID_EpisodeDate));

    return true;
//    return d->_episodeModel->submit();
}
