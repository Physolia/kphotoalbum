/* Copyright (C) 2018 Robert Krawitz <rlk@alum.mit.edu>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef IMAGESCOUT_H
#define IMAGESCOUT_H
#include "DB/FileName.h"
#include <QList>
#include <QThread>
#include <QAtomicInteger>

typedef QList<DB::FileName> ImageScoutList;
namespace DB
{

class ImageScoutThread :public QThread {
public:
    ImageScoutThread( const ImageScoutList & );
    void incrementLoadedCount();
protected:
    virtual void run();
private:
    const ImageScoutList m_list;
    QAtomicInteger<int> m_loadedCount;
};
}

#endif /* IMAGESCOUT_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
