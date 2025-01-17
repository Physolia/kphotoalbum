// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef MEMBERMAP_H
#define MEMBERMAP_H
#include <kpabase/StringSet.h>

#include <qmap.h>
#include <qobject.h>
#include <qstringlist.h>

namespace DB
{
using Utilities::StringSet;

class Category;

class MemberMap : public QObject
{
    Q_OBJECT
public:
    MemberMap();
    MemberMap(const MemberMap &);
    virtual MemberMap &operator=(const MemberMap &);

    // TODO: this should return a StringSet
    virtual QStringList groups(const QString &category) const;
    virtual void deleteGroup(const QString &category, const QString &name);

    // TODO: this should return a StringSet
    virtual QStringList members(const QString &category, const QString &memberGroup, bool closure) const;
    virtual void setMembers(const QString &category, const QString &memberGroup, const QStringList &members);
    virtual bool isEmpty() const;
    virtual bool isGroup(const QString &category, const QString &memberGroup) const;
    virtual QMap<QString, StringSet> groupMap(const QString &category) const;
    virtual QMap<QString, StringSet> inverseMap(const QString &category) const;
    virtual void renameGroup(const QString &category, const QString &oldName, const QString &newName);
    virtual void renameCategory(const QString &oldName, const QString &newName);

    virtual void addGroup(const QString &category, const QString &group);
    bool canAddMemberToGroup(const QString &category, const QString &group, const QString &item) const
    {
        // If there already is a path from item to group then adding the
        // item to group would create a cycle, which we don't want.
        return !hasPath(category, item, group);
    }
    virtual void addMemberToGroup(const QString &category, const QString &group, const QString &item);
    virtual void removeMemberFromGroup(const QString &category, const QString &group, const QString &item);

    virtual const QMap<QString, QMap<QString, StringSet>> &memberMap() const { return m_members; }

    virtual bool hasPath(const QString &category, const QString &from, const QString &to) const;
    virtual bool contains(const QString &category, const QString &item) const;

protected:
    void calculate() const;
    QStringList calculateClosure(QMap<QString, StringSet> &resultSoFar, const QString &category, const QString &group) const;

public Q_SLOTS:
    virtual void deleteCategory(const QString &category);
    virtual void deleteItem(DB::Category *category, const QString &name);
    virtual void renameItem(DB::Category *category, const QString &oldName, const QString &newName);
    void setLoading(bool b);

Q_SIGNALS:
    void dirty();

private:
    void markDirty(const QString &category);
    void regenerateFlatList(const QString &category);
    // This is the primary data structure
    // { category |-> { group |-> [ member ] } } <- VDM syntax ;-)
    QMap<QString, QMap<QString, StringSet>> m_members;
    mutable QMap<QString, QSet<QString>> m_flatMembers;

    // These are the data structures used to develop closures, they are only
    // needed to speed up the program *SIGNIFICANTLY* ;-)
    mutable bool m_dirty;
    mutable QMap<QString, QMap<QString, StringSet>> m_closureMembers;

    bool m_loading;
};
}

#endif /* MEMBERMAP_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
