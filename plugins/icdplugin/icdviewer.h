/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2010 by Eric MAEKER, MD (France) <eric.maeker@free.fr>        *
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
 *   Main Developper : Eric MAEKER, <eric.maeker@free.fr>                  *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#ifndef ICDVIEWER_H
#define ICDVIEWER_H

#include <QWidget>


namespace ICD {
namespace Internal {
class IcdViewerPrivate;
}  // End namespace Internal

namespace Ui {
    class IcdViewer;
}

class IcdViewer : public QWidget
{
    Q_OBJECT

public:
    explicit IcdViewer(QWidget *parent = 0);
    ~IcdViewer();

public Q_SLOTS:
    void setCodeSid(const QVariant &sid);

protected:
    void changeEvent(QEvent *e);

private:
    Internal::IcdViewerPrivate *d;
};


}  // End namespace ICD


#endif // ICDVIEWER_H
