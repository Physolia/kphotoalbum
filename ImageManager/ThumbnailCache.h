/* Copyright (C) 2003-2020 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef THUMBNAILCACHE_H
#define THUMBNAILCACHE_H
#include "CacheFileInfo.h"

#include <DB/FileNameList.h>

#include <QFile>
#include <QHash>
#include <QImage>
#include <QMutex>

template <class Key, class T>
class QCache;

namespace ImageManager
{

class ThumbnailMapping;

class ThumbnailCache : public QObject
{
    Q_OBJECT

public:
    static ThumbnailCache *instance();
    static void deleteInstance();
    ThumbnailCache(const QString &baseDirectory);
    void insert(const DB::FileName &name, const QImage &image);
    QPixmap lookup(const DB::FileName &name) const;
    QByteArray lookupRawData(const DB::FileName &name) const;
    bool contains(const DB::FileName &name) const;
    void removeThumbnail(const DB::FileName &);
    void removeThumbnails(const DB::FileNameList &);

public slots:
    void save() const;
    void flush();

signals:
    void doSave() const;

private:
    /**
     * @brief load the \c thumbnailindex file if possible.
     * This function populates the thumbnail hash, but does not
     * load any actual thumbnail data.
     * If the file does not exist, or if it is not compatible,
     * then it is discarded.
     */
    void load();
    ~ThumbnailCache() override;
    QString fileNameForIndex(int index) const;
    QString thumbnailPath(const QString &fileName) const;

    static ThumbnailCache *s_instance;
    const QString m_baseDir;
    QHash<DB::FileName, CacheFileInfo> m_hash;
    mutable QHash<DB::FileName, CacheFileInfo> m_unsavedHash;
    /* Protects accesses to the data (hash and unsaved hash) */
    mutable QMutex m_dataLock;
    /* Prevents multiple saves from happening simultaneously */
    mutable QMutex m_saveLock;
    /* Protects writing thumbnails to disk */
    mutable QMutex m_thumbnailWriterLock;
    int m_currentFile;
    int m_currentOffset;
    mutable QTimer *m_timer;
    mutable bool m_needsFullSave;
    mutable bool m_isDirty;
    void saveFull() const;
    void saveIncremental() const;
    void saveInternal() const;
    void saveImpl() const;

    /**
     * Holds an in-memory cache of thumbnail files.
     */
    mutable QCache<int, ThumbnailMapping> *m_memcache;
    mutable QFile *m_currentWriter;
};

}

#endif /* THUMBNAILCACHE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
