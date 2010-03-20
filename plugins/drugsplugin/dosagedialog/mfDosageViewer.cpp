/***************************************************************************
 *   FreeMedicalForms                                                      *
 *   (C) 2008-2010 by Eric MAEKER, MD                                     **
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

/**
  \class DosageViewer
  \brief QWidget for dosage creation / edition / modification. A dosage is a standard set of datas that will be used to help
  doctors when prescribing a drug.
  If you want to create a new dosage, you must create a new row onto the model BEFORE.\n
  If you want to edit or modify a dosage, you must inform the widget of the row and the CIS of the drug.\n

  Please always call done() when the dialog including the DosageViewer is done.

  \ingroup freediams drugswidget
*/

#include "mfDosageViewer.h"

#include <drugsbaseplugin/drugsdata.h>
#include <drugsbaseplugin/dosagemodel.h>
#include <drugsbaseplugin/drugsmodel.h>
#include <drugsbaseplugin/dailyschememodel.h>

#include <drugsplugin/constants.h>
#include <drugsplugin/drugswidgetmanager.h>

#include <utils/log.h>
#include <translationutils/constanttranslations.h>

#include <coreplugin/icore.h>
#include <coreplugin/isettings.h>
#include <coreplugin/itheme.h>
#include <coreplugin/dialogs/helpdialog.h>


// include Qt headers
#include <QHeaderView>
#include <QRadioButton>
#include <QCheckBox>
#include <QTableWidget>
#include <QSpinBox>
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QMenu>

using namespace DrugsWidget::Constants;
using namespace DrugsWidget::Internal;

inline static DrugsDB::DrugsModel *drugModel() { return DrugsWidget::DrugsWidgetManager::instance()->currentDrugsModel(); }

namespace DrugsWidget {
namespace Internal {

/**
  \brief Private part
  \internal
*/
class DosageViewerPrivate
{
public:
    DosageViewerPrivate(DosageViewer *parent) :
            m_Mapper(0), m_DosageModel(0), m_CIS(-1), m_SpinDelegate(0), m_Parent(parent) {}

    void setCheckBoxStateToModel(const int index, const int qtCheckState)
    {
        if (m_DosageModel) {
            m_DosageModel->setData(m_DosageModel->index(m_Mapper->currentIndex(), index), qtCheckState==Qt::Checked);
            qWarning() << "dosage" << Dosages::Constants::IntakesUsesFromTo << m_Mapper->currentIndex() <<  m_DosageModel->data(m_DosageModel->index(m_Mapper->currentIndex(), index));
        } else {
            qWarning() << "drug";
            drugModel()->setDrugData(m_CIS, index, qtCheckState==Qt::Checked);
        }
    }

    /** \brief Create and prepare mapper for drugsModel */
    void createDrugMapper()
    {
        using namespace Dosages::Constants;
        using namespace DrugsDB::Constants;
        if (!m_Mapper) {
            m_Mapper = new QDataWidgetMapper(m_Parent);
            m_Mapper->setModel(drugModel());
            m_Mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
            m_Mapper->addMapping(m_Parent->intakesFromSpin, Prescription::IntakesFrom, "value");
            m_Mapper->addMapping(m_Parent->intakesToSpin, Prescription::IntakesTo, "value");
            m_Mapper->addMapping(m_Parent->intakesCombo, Prescription::IntakesScheme, "currentText");
            m_Mapper->addMapping(m_Parent->periodSchemeCombo, Prescription::PeriodScheme, "currentText");
            m_Mapper->addMapping(m_Parent->periodSpin, Prescription::Period, "value");

            m_Mapper->addMapping(m_Parent->durationFromSpin, Prescription::DurationFrom);
            m_Mapper->addMapping(m_Parent->durationToSpin, Prescription::DurationTo);
            m_Mapper->addMapping(m_Parent->durationCombo, Prescription::DurationScheme, "currentText");

            m_Mapper->addMapping(m_Parent->minIntervalIntakesSpin, Prescription::IntakesIntervalOfTime, "value");
            m_Mapper->addMapping(m_Parent->intervalTimeSchemeCombo, Prescription::IntakesIntervalScheme, "currentIndex");
            m_Mapper->addMapping(m_Parent->mealTimeCombo, Prescription::MealTimeSchemeIndex, "currentIndex");
            m_Mapper->addMapping(m_Parent->noteTextEdit, Prescription::Note, "plainText");
            m_Parent->tabWidget->removeTab(6);
            m_Parent->tabWidget->removeTab(4);
            m_Parent->tabWidget->removeTab(3);
            m_Parent->tabWidget->removeTab(2);
        }
    }

    /** \brief If mapper does not already exists, create it */
    void createDosageMapper()
    {
        using namespace DrugsDB::Constants;
        Q_ASSERT(m_DosageModel);
        Q_ASSERT(m_Parent);
        if (!m_Mapper) {
            m_Mapper = new QDataWidgetMapper(m_Parent);
            m_Mapper->setModel(m_DosageModel);
            m_Mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
            m_Mapper->addMapping(m_Parent->labelLineEdit, Dosages::Constants::Label, "text");

//            m_Mapper->addMapping(m_Parent->fromToIntakesCheck, Dosages::Constants::IntakesUsesFromTo, "checked");

            m_Mapper->addMapping(m_Parent->intakesFromSpin, Dosages::Constants::IntakesFrom, "value");
            m_Mapper->addMapping(m_Parent->intakesToSpin, Dosages::Constants::IntakesTo, "value");
            m_Mapper->addMapping(m_Parent->intakesCombo, Dosages::Constants::IntakesScheme, "currentText");
            m_Mapper->addMapping(m_Parent->periodSchemeCombo, Dosages::Constants::PeriodScheme, "currentText");
            m_Mapper->addMapping(m_Parent->periodSpin, Dosages::Constants::Period, "value");

            m_Mapper->addMapping(m_Parent->durationFromSpin, Dosages::Constants::DurationFrom);
            m_Mapper->addMapping(m_Parent->durationToSpin, Dosages::Constants::DurationTo);
            m_Mapper->addMapping(m_Parent->durationCombo, Dosages::Constants::DurationScheme, "currentText");

            m_Mapper->addMapping(m_Parent->minIntervalIntakesSpin, Dosages::Constants::IntakesIntervalOfTime, "value");
            m_Mapper->addMapping(m_Parent->intervalTimeSchemeCombo, Dosages::Constants::IntakesIntervalScheme, "currentIndex");
            m_Mapper->addMapping(m_Parent->mealTimeCombo, Dosages::Constants::MealScheme, "currentIndex");
            m_Mapper->addMapping(m_Parent->noteTextEdit, Dosages::Constants::Note, "plainText");
            m_Mapper->addMapping(m_Parent->minAgeSpin, Dosages::Constants::MinAge, "value");
            m_Mapper->addMapping(m_Parent->maxAgeSpin, Dosages::Constants::MaxAge, "value");
            m_Mapper->addMapping(m_Parent->minAgeCombo, Dosages::Constants::MinAgeReferenceIndex, "currentIndex");
            m_Mapper->addMapping(m_Parent->maxAgeCombo, Dosages::Constants::MaxAgeReferenceIndex, "currentIndex");
            m_Mapper->addMapping(m_Parent->minWeightSpin, Dosages::Constants::MinWeight, "value");
            m_Mapper->addMapping(m_Parent->minClearanceSpin, Dosages::Constants::MinClearance, "value");
            m_Mapper->addMapping(m_Parent->maxClearanceSpin, Dosages::Constants::MaxClearance, "value");
            m_Mapper->addMapping(m_Parent->sexLimitCombo, Dosages::Constants::SexLimitedIndex, "currentIndex");
        }
    }

    /**
       \brief Manage non mapped datas from the model to the ui
       Manages Sex limitation, Age limitations, Clearance limitations
    */
    void changeNonMappedDataFromModelToUi(const int row)
    {
        Q_ASSERT(m_Parent);
        if (m_DosageModel) {
            // There is a bug with Editable QComboBoxes and the currentText property to be setted !!
            // Need to be filled by hand the comboboxes...
            // Label
            m_Parent->labelLineEdit->setText(m_DosageModel->index(row, Dosages::Constants::Label).data().toString());
            // Intakes
            m_Parent->intakesCombo->setCurrentIndex(-1);
            m_Parent->intakesCombo->setEditText(m_DosageModel->index(row, Dosages::Constants::IntakesScheme).data().toString());
            // Period
            m_Parent->periodSpin->setValue(m_DosageModel->index(row, Dosages::Constants::Period).data().toDouble());
            m_Parent->periodSchemeCombo->setEditText(m_DosageModel->index(row, Dosages::Constants::PeriodScheme).data().toString());
            // Duration
            m_Parent->durationCombo->setEditText(m_DosageModel->index(row, Dosages::Constants::DurationScheme).data().toString());
            // Interval
            m_Parent->minIntervalIntakesSpin->setValue(m_DosageModel->index(row, Dosages::Constants::IntakesIntervalOfTime).data().toDouble());

            bool intakeRange = m_DosageModel->index(row, Dosages::Constants::IntakesUsesFromTo).data().toBool();
            qWarning() << "intakeRange" << intakeRange;
            m_Parent->fromToIntakesCheck->setChecked(intakeRange);
            m_Parent->intakesToLabel->setVisible(intakeRange);
            m_Parent->intakesToSpin->setVisible(intakeRange);

            bool durationRange = m_DosageModel->index(row, Dosages::Constants::DurationUsesFromTo).data().toBool();
            m_Parent->fromToDurationCheck->setChecked(durationRange);
            m_Parent->durationToLabel->setVisible(durationRange);
            m_Parent->durationToSpin->setVisible(durationRange);

            // populate DailSchemeModel
            DrugsDB::DailySchemeModel *daily = m_Parent->dailyScheme->model();
            Q_ASSERT(daily);
            daily->setSerializedContent(m_DosageModel->index(row, Dosages::Constants::DailyScheme).data().toString());
//            m_Parent->dailySchemeView->resizeColumnsToContents();

            bool innPrescr = m_DosageModel->index(row, Dosages::Constants::INN_LK).data().toBool();
            m_Parent->dosageForAllInnCheck->setChecked(innPrescr);
            m_Parent->innCompositionLabel->setVisible(innPrescr);

            m_Parent->aldCheck->setChecked(m_DosageModel->index(row, Dosages::Constants::IsALD).data().toBool());
        } else {
            using namespace DrugsDB::Constants;
            m_Parent->labelLineEdit->hide();
            m_Parent->labelOfDosageLabel->hide();

            // Intakes
            m_Parent->intakesCombo->setCurrentIndex(-1);
            m_Parent->intakesCombo->setEditText(drugModel()->drugData(m_CIS, Prescription::IntakesScheme).toString());
            // Period
            m_Parent->periodSpin->setValue(drugModel()->drugData(m_CIS, Prescription::Period).toDouble());
            m_Parent->periodSchemeCombo->setEditText(drugModel()->drugData(m_CIS, Prescription::PeriodScheme).toString());
            // Duration
            m_Parent->durationCombo->setEditText(drugModel()->drugData(m_CIS, Prescription::DurationScheme).toString());
            // Interval
            m_Parent->minIntervalIntakesSpin->setValue(drugModel()->drugData(m_CIS, Prescription::IntakesIntervalOfTime).toDouble());

            bool intakeRange = m_DosageModel->index(row, Prescription::IntakesUsesFromTo).data().toBool();
            m_Parent->fromToIntakesCheck->setChecked(intakeRange);
            m_Parent->intakesToLabel->setVisible(intakeRange);
            m_Parent->intakesToSpin->setVisible(intakeRange);

            bool durationRange = m_DosageModel->index(row, Prescription::DurationUsesFromTo).data().toBool();
            m_Parent->fromToDurationCheck->setChecked(durationRange);
            m_Parent->durationToLabel->setVisible(durationRange);
            m_Parent->durationToSpin->setVisible(durationRange);

            m_Parent->aldCheck->setChecked(drugModel()->drugData(m_CIS, Prescription::IsALD).toBool());
            // populate DailSchemeModel
            DrugsDB::DailySchemeModel *daily = m_Parent->dailyScheme->model();
            Q_ASSERT(daily);
            daily->setSerializedContent(drugModel()->drugData(m_CIS, Prescription::DailyScheme).toString());
//            m_Parent->dailySchemeView->resizeColumnsToContents();
        }

        // Link to French RCP
        if (!drugModel()->drugData(m_CIS, DrugsDB::Constants::Drug::LinkToSCP).isNull()) {
            m_Parent->frenchRCPButton->setEnabled(true);
            m_Parent->frenchRCPButton->setToolTip(drugModel()->drugData(m_CIS, DrugsDB::Constants::Drug::LinkToSCP).toString());
        } else {
            m_Parent->frenchRCPButton->setEnabled(false);
        }
    }

    /** \brief Resize hourly table widget on view */
    void resizeTableWidget()
    {
        Q_ASSERT(m_Parent);
        int i = 0;
        int size = ((m_Parent->hourlyTableWidget->size().width() - m_Parent->style()->pixelMetric(QStyle::PM_DefaultFrameWidth)) / 8);
        for(i = 0; i < 8; i++)
            m_Parent->hourlyTableWidget->setColumnWidth(i, size);
    }

    /** \brief Update the Ui with the drug informations */
    void fillDrugsData()
    {
        Q_ASSERT(m_Parent);
        m_Parent->labelOfDosageLabel->setToolTip(drugModel()->drugData(m_CIS, DrugsDB::Constants::Drug::AvailableDosages).toString());
        m_Parent->drugNameLabel->setText(drugModel()->drugData(m_CIS, DrugsDB::Constants::Drug::Denomination).toString());
        m_Parent->drugNameLabel->setToolTip(drugModel()->drugData(m_CIS, DrugsDB::Constants::Drug::CompositionString).toString());
        m_Parent->interactionLabel->setPixmap(drugModel()->drugData(m_CIS, DrugsDB::Constants::Interaction::Icon).value<QIcon>().pixmap(16,16));
        m_Parent->interactionLabel->setToolTip(drugModel()->drugData(m_CIS, DrugsDB::Constants::Interaction::ToolTip).toString());
    }

    /**
      \brief Reset the Ui to the defaults.
      \li Clears all combo, refill them and set their currentIndex to the default mfDosageModel::periodDefault()
      \li Manage scored tablet
    */
    void resetUiToDefaults()
    {
        using namespace DrugsDB::Constants;
        Q_ASSERT(m_Parent);
        m_Parent->intakesToLabel->hide();
        m_Parent->intakesToSpin->hide();
        m_Parent->durationToLabel->hide();
        m_Parent->durationToSpin->hide();
        // Prepare some combos
        m_Parent->durationCombo->clear();
        m_Parent->durationCombo->addItems(Trans::ConstantTranslations::periods());
        m_Parent->durationCombo->setCurrentIndex(Trans::Constants::Time::Months);
        m_Parent->periodSchemeCombo->clear();
        m_Parent->periodSchemeCombo->addItems(Trans::ConstantTranslations::periods());
        m_Parent->periodSchemeCombo->setCurrentIndex(Trans::Constants::Time::Days);
        m_Parent->intervalTimeSchemeCombo->clear();
        m_Parent->intervalTimeSchemeCombo->addItems(Trans::ConstantTranslations::periods());
        m_Parent->intervalTimeSchemeCombo->setCurrentIndex(Trans::Constants::Time::Days);
        m_Parent->intakesCombo->addItems(drugModel()->drugData(m_CIS, Drug::AvailableForms).toStringList());
        m_Parent->intakesCombo->setCurrentIndex(0);
        m_Parent->mealTimeCombo->clear();
        m_Parent->mealTimeCombo->addItems(Trans::ConstantTranslations::mealTime());

        m_Parent->minAgeCombo->clear();
        m_Parent->minAgeCombo->addItems(Trans::ConstantTranslations::preDeterminedAges());
        m_Parent->maxAgeCombo->clear();
        m_Parent->maxAgeCombo->addItems(Trans::ConstantTranslations::preDeterminedAges());

        m_Parent->hourlyTableWidget->verticalHeader()->hide();
        m_Parent->hourlyTableWidget->horizontalHeader()->hide();
        m_Parent->hourlyTableWidget->resizeColumnsToContents();
        bool isScored = drugModel()->drugData(m_CIS, Drug::IsScoredTablet).toBool();
        if (isScored) {
            m_Parent->intakesToSpin->setDecimals(2);
            m_Parent->intakesFromSpin->setDecimals(2);
            m_Parent->intakesToSpin->setSingleStep(0.25);
            m_Parent->intakesFromSpin->setSingleStep(0.25);
            m_Parent->intakesToSpin->setMinimum(0.25);
            m_Parent->intakesFromSpin->setMinimum(0.25);
        } else {
            m_Parent->intakesToSpin->setDecimals(0);
            m_Parent->intakesFromSpin->setDecimals(0);
            m_Parent->intakesToSpin->setSingleStep(1);
            m_Parent->intakesFromSpin->setSingleStep(1);
            m_Parent->intakesToSpin->setMinimum(1);
            m_Parent->intakesFromSpin->setMinimum(1);
        }
        resizeTableWidget();

//        if (m_SpinDelegate) {
//            delete m_SpinDelegate;
//            m_SpinDelegate = 0;
//        }
//        if (isScored) {
//            m_SpinDelegate = new Utils::SpinBoxDelegate(m_Parent,0.0,1.0,true);
//        } else {
//            m_SpinDelegate = new Utils::SpinBoxDelegate(m_Parent,0,1,false);
//        }
//        m_Parent->dailySchemeView->setItemDelegateForColumn(DrugsDB::DailySchemeModel::Value, m_SpinDelegate);
        if (m_DosageModel)
            m_Parent->dosageForAllInnCheck->setEnabled(dosageCanLinkWithInn());
        else
            m_Parent->dosageForAllInnCheck->setVisible(dosageCanLinkWithInn());
    }

//    void setDailyMaximum(double max)
//    {
//        DrugsDB::DailySchemeModel *daily = static_cast<DrugsDB::DailySchemeModel*>(m_Parent->dailySchemeView->model());
//        DrugsDB::DailySchemeModel *daily = m_Parent->dailyScheme->model();
//        Q_ASSERT(daily);
//        if (daily) {
//            daily->setMaximumDay(max);
//        }
//        if (m_SpinDelegate) {
//            m_SpinDelegate->setMaximum(max-daily->sum());
//        }
//    }

    void recalculateDailySchemeMaximum()
    {
        if (!m_Parent->fromToIntakesCheck->isChecked()) {
            m_Parent->dailyScheme->setDailyMaximum(m_Parent->intakesFromSpin->value());
        } else {
            m_Parent->dailyScheme->setDailyMaximum(m_Parent->intakesToSpin->value());
        }
    }

    bool dosageCanLinkWithInn()
    {
        if (m_DosageModel) {
            return ((drugModel()->drugData(m_CIS, DrugsDB::Constants::Drug::MainInnCode).toInt()!=-1) &&
                    (drugModel()->drugData(m_CIS, DrugsDB::Constants::Drug::AllInnsKnown).toBool()));
        }
        return false;
    }

public:
    QDataWidgetMapper  *m_Mapper;
    DrugsDB::Internal::DosageModel *m_DosageModel;
    QString             m_ActualDosageUuid;
    int                 m_CIS;
    Utils::SpinBoxDelegate *m_SpinDelegate;
private:
    DosageViewer *m_Parent;
};

}  // End Internal
}  // End Drugs


/**
 \todo when showing dosage, make verification of limits +++  ==> for FMF only
 \todo use a QPersistentModelIndex instead of drugRow, dosageRow
*/
DosageViewer::DosageViewer(QWidget *parent)
    : QWidget(parent),
    d(0)
{
    // some initializations
    setObjectName("DosageViewer");
    d = new DosageViewerPrivate(this);

    // Ui initialization
    setupUi(this);
    setWindowTitle(tr("Drug Dosage Creator") + " - " + qApp->applicationName());
    userformsButton->setIcon(Core::ICore::instance()->theme()->icon(Core::Constants::ICONEDIT));
    // remove last page of tabWidget (TODO page)
    tabWidget->removeTab(tabWidget->count()-1);

    // define models
    DrugsDB::DailySchemeModel *model = new DrugsDB::DailySchemeModel(this);
    dailyScheme->setModel(model);
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(onDailySchemeModelDataChanged(QModelIndex)));

    // show first page of the tabwidget   ;;   Hide unused tables
    tabWidget->setCurrentIndex(0);
    this->hourlyTableWidget->hide();
}

/** \brief Use this function to define a drugsModel behavior. */
void DosageViewer::useDrugsModel(const int CIS, const int drugRow)
{
    Q_ASSERT(drugModel()->containsDrug(CIS));
    d->m_CIS = CIS;
    d->m_DosageModel = 0;
    d->resetUiToDefaults();
    d->fillDrugsData();
    d->createDrugMapper();
    changeCurrentRow(drugRow);
}

/** \brief Use this function to define a dosageModel behavior. */
void DosageViewer::setDosageModel(DrugsDB::Internal::DosageModel *model)
{
    Q_ASSERT(model);
    d->m_DosageModel = model;
    d->m_CIS = model->drugUID();
    d->resetUiToDefaults();
    d->fillDrugsData();
    d->createDosageMapper();

    if (model->rowCount()==0) {
        model->insertRow(0);
        changeCurrentRow(0);
    } else {
        changeCurrentRow(0);
    }
}

/** \brief Destructor */
DosageViewer::~DosageViewer()
{
    if (d) delete d;
    d=0;
}

/** \brief Changes the current editing dosage */
void DosageViewer::changeCurrentRow(const int dosageRow)
{
    if (dosageRow==d->m_Mapper->currentIndex())
        return;

    d->resetUiToDefaults();
    d->m_Mapper->setCurrentIndex(dosageRow);
    d->changeNonMappedDataFromModelToUi(dosageRow);
    d->recalculateDailySchemeMaximum();
    qWarning() << dosageRow << QString("%1 = %2,").arg(drugModel()->drugData(d->m_CIS,DrugsDB::Constants::Drug::MainInnName).toString().toUpper()).arg(d->m_CIS);
}

/** \brief Only provided because of focus bug */
void DosageViewer::commitToModel()
{
    d->m_Mapper->submit();
    // populate DailyShemeModel
    /** \todo Create and Use DailySchemeViewer Properties */
    DrugsDB::DailySchemeModel *daily = dailyScheme->model();
    Q_ASSERT(daily);
    if (d->m_DosageModel) {
        if (daily) {
            d->m_DosageModel->setData(d->m_DosageModel->index(d->m_Mapper->currentIndex(), Dosages::Constants::DailyScheme), daily->serializedContent());
        }
    } else {
        if (daily) {
            drugModel()->setDrugData(d->m_CIS, DrugsDB::Constants::Prescription::DailyScheme, daily->serializedContent());
        }
    }
}


/** \brief Changes the current editing dosage */
void DosageViewer::changeCurrentRow(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    changeCurrentRow(current.row());
}

/**
   \brief When dialog including mfDosageViewer is validate call this member with the result of the dialog.
   It stores some values inside the users settings.
*/
void DosageViewer::done(int r)
{
    if (r == QDialog::Accepted) {
        // match the user's form for the settings
        const QStringList &pre = DrugsDB::Internal::DosageModel::predeterminedForms();
        const QStringList &av  = drugModel()->drugData(d->m_CIS, DrugsDB::Constants::Drug::AvailableForms).toStringList();
        if ((pre.indexOf(intakesCombo->currentText()) == -1) &&
            (av.indexOf(intakesCombo->currentText()) == -1)) {
            Core::ICore::instance()->settings()->appendToValue(S_USERRECORDEDFORMS, intakesCombo->currentText());
        }
    }
}

/** \brief Used for hourly table widget resizing */
void DosageViewer::resizeEvent(QResizeEvent * event)
{
    d->resizeTableWidget();
    QWidget::resizeEvent(event);
}

void DosageViewer::on_fromToIntakesCheck_stateChanged(int state)
{
    if (d->m_DosageModel)
        d->setCheckBoxStateToModel(Dosages::Constants::IntakesUsesFromTo, state);
    else
        d->setCheckBoxStateToModel(DrugsDB::Constants::Prescription::IntakesUsesFromTo, state);
    d->recalculateDailySchemeMaximum();
}

void DosageViewer::on_fromToDurationCheck_stateChanged(int state)
{
    if (d->m_DosageModel)
        d->setCheckBoxStateToModel(Dosages::Constants::DurationUsesFromTo, state);
    else
        d->setCheckBoxStateToModel(DrugsDB::Constants::Prescription::DurationUsesFromTo, state);
}

/** \brief Redefine the minimum of the "to" intakes */
void DosageViewer::on_intakesFromSpin_valueChanged(double value)
{
    if (intakesToSpin->value() < value) {
        intakesToSpin->setValue(value);
    }
    intakesToSpin->setMinimum(value);
    d->recalculateDailySchemeMaximum();
}

/** \brief Redefine the minimum of the "to" duration */
void DosageViewer::on_durationFromSpin_valueChanged(double value)
{
    if (durationToSpin->value() < value) {
        durationToSpin->setValue(value);
    }
    durationToSpin->setMinimum(value);
    d->recalculateDailySchemeMaximum();
}

/** \brief Show a menu with the user recorded forms */
void DosageViewer::on_userformsButton_clicked()
{
    if (Core::ICore::instance()->settings()->value(S_USERRECORDEDFORMS, QVariant()).isNull())
        return;

    const QStringList &ulist = Core::ICore::instance()->settings()->value(S_USERRECORDEDFORMS).toStringList();
    QList<QAction*> list;
    foreach(const QString &form, ulist) {
        if (!form.isEmpty())
            list << new QAction(form, this);
    }
    QAction *aclear = new QAction(tr("Clear this list", "Clear the user's intakes recorded forms"), this);
    list << aclear;

    QAction *a = QMenu::exec(list, userformsButton->mapToGlobal(QPoint(0,20)));
    if (!a)
        return;
    if (a == aclear) {
        Core::ICore::instance()->settings()->setValue(S_USERRECORDEDFORMS, QString());
    } else {
        intakesCombo->setEditText(a->text());
        if (d->m_DosageModel)
            d->m_DosageModel->setData(d->m_DosageModel->index(d->m_Mapper->currentIndex(),Dosages::Constants::IntakesScheme),a->text());
        else
            drugModel()->setDrugData(d->m_CIS, DrugsDB::Constants::Prescription::IntakesScheme, a->text());
    }
}

/** \brief If INN linking is available, shows the inn name and the dosage used for the link */
void DosageViewer::on_dosageForAllInnCheck_stateChanged(int state)
{
    if (d->m_DosageModel) {
        // INN Prescription ?
        int row = d->m_Mapper->currentIndex();
            if ((dosageForAllInnCheck->isEnabled()) && (state==Qt::Checked)) {
                d->m_DosageModel->setData(d->m_DosageModel->index(row, Dosages::Constants::CIS_LK), d->m_CIS);
                d->m_DosageModel->setData(d->m_DosageModel->index(row, Dosages::Constants::CIP_LK), QVariant());
                d->m_DosageModel->setData(d->m_DosageModel->index(row, Dosages::Constants::INN_LK),
                                           drugModel()->drugData(d->m_CIS, DrugsDB::Constants::Drug::MainInnCode));
                d->m_DosageModel->setData(d->m_DosageModel->index(row, Dosages::Constants::InnLinkedDosage),
                                           drugModel()->drugData(d->m_CIS, DrugsDB::Constants::Drug::MainInnDosage));
            } else {
                d->m_DosageModel->setData(d->m_DosageModel->index(row, Dosages::Constants::CIS_LK), d->m_CIS);
                d->m_DosageModel->setData(d->m_DosageModel->index(row, Dosages::Constants::CIP_LK), QVariant());
                d->m_DosageModel->setData(d->m_DosageModel->index(row, Dosages::Constants::INN_LK), QVariant());
                d->m_DosageModel->setData(d->m_DosageModel->index(row, Dosages::Constants::InnLinkedDosage),
                                           drugModel()->drugData(d->m_CIS, DrugsDB::Constants::Drug::MainInnCode));
            }
        innCompositionLabel->show();
        innCompositionLabel->setText(tr("Linking to : ")
                                      + drugModel()->drugData(d->m_CIS, DrugsDB::Constants::Drug::MainInnName).toString() + " "
//                                      + tr("Dosage of molecule : ")
                                      + drugModel()->drugData(d->m_CIS, DrugsDB::Constants::Drug::MainInnDosage).toString());
    } else
        innCompositionLabel->hide();
}

void DosageViewer::on_aldCheck_stateChanged(int state)
{
    if (d->m_DosageModel)
        d->setCheckBoxStateToModel(Dosages::Constants::IsALD, state);
    else
        d->setCheckBoxStateToModel(DrugsDB::Constants::Prescription::IsALD, state);
}

void DosageViewer::on_frenchRCPButton_clicked()
{
    QDesktopServices::openUrl(QUrl(drugModel()->drugData(d->m_CIS, DrugsDB::Constants::Drug::LinkToSCP).toString()));
}

void DosageViewer::on_tabWidget_currentChanged(int)
{
    if (tabWidget->currentWidget()==tabSchemes)
        d->resizeTableWidget();
}

void DosageViewer::onDailySchemeModelDataChanged(const QModelIndex &index)
{
    Q_UNUSED(index);
    d->recalculateDailySchemeMaximum();
}

