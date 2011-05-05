/* Copyright (C) 2003-2011 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef CANCELEVENT_H
#define CANCELEVENT_H

#include <QEvent>

namespace ImageManager {

class ImageRequest;

const int CANCELEVENTID = 1002;

class CancelEvent : public QEvent
{
public:
    CancelEvent( ImageRequest* request );
    ~CancelEvent();
    ImageRequest* request() const;

private:
    ImageRequest* m_request;
};

}
#endif // CANCELEVENT_H