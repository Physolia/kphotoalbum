/* Copyright (C) 2016 Tobias Leupold <tobias.leupold@web.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef AREATAGSELECTLINEEDIT_H
#define AREATAGSELECTLINEEDIT_H

#include <QLineEdit>

class QKeyEvent;

namespace AnnotationDialog {

class CompletableLineEdit;
class ResizableFrame;

class AreaTagSelectLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit AreaTagSelectLineEdit(ResizableFrame* area,
                                   CompletableLineEdit* categoryLineEdit);

private: // Functions
    void keyPressEvent(QKeyEvent* event);

private: // Variables
    CompletableLineEdit* m_categoryLineEdit;
    ResizableFrame* m_area;
};

}

#endif // AREATAGSELECTLINEEDIT_H

// vi:expandtab:tabstop=4 shiftwidth=4:
