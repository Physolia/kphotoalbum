/* Copyright (C) 2018 Tobias Leupold <tobias.leupold@gmx.de>

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

#ifndef GEOCOORDINATES_H
#define GEOCOORDINATES_H

#include <QPair>
#include <QMetaType>
#include <QString>

namespace Map
{

class GeoCoordinates
{

public:
    bool hasCoordinates() const;
    double lon() const;
    double lat() const;
    double alt() const;
    bool hasAltitude() const;
    void setLatLon(const double lat, const double lon);
    void setAlt(const double alt);

    typedef QPair<GeoCoordinates, GeoCoordinates> Pair;
    static Pair makePair(const double lat1, const double lon1,
                         const double lat2, const double lon2);

    operator QString() const;
private: // Variables
    double m_lat;
    double m_lon;
    double m_alt;
    bool m_hasCoordinates;
    bool m_hasAlt;
};

}

Q_DECLARE_METATYPE(Map::GeoCoordinates::Pair)

#endif // GEOCOORDINATES_H

// vi:expandtab:tabstop=4 shiftwidth=4:
