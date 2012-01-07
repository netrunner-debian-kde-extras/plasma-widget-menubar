/*
 * Plasma applet to display application window menus
 *
 * Copyright 2010 Canonical Ltd.
 *
 * Authors:
 * - Aurélien Gâteau <aurelien.gateau@canonical.com>
 *
 * License: GPL v3
 */
#include "gtkicontable.h"

// Qt
#include <QDir>
#include <QFileInfo>

// KDE
#include <KDebug>

GtkIconTable::GtkIconTable(const QString &name)
: mGnomeThemeDirName(name)
, mIsRightToLeft(false)
{}

void GtkIconTable::setRightToLeft(bool value)
{
    if (mIsRightToLeft != value) {
        mIsRightToLeft = value;
        mTable.clear();
    }
}

QString GtkIconTable::value(const QString &name) const
{
    if (mTable.isEmpty()) {
        const_cast<GtkIconTable*>(this)->init();
    }
    return mTable.value(name, name);
}

void GtkIconTable::init()
{
    QDir gnomeDir(mGnomeThemeDirName);
    Q_FOREACH(const QString &type, gnomeDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        readSymlinksInDir(gnomeDir.filePath(type));
    }
}

void GtkIconTable::readSymlinksInDir(const QString &dirName)
{
    QDir dir(dirName);
    QString directionSuffix = mIsRightToLeft ? "-rtl" : "-ltr";
    QString skippedSuffix = mIsRightToLeft ? "-ltr" : "-rtl";
    Q_FOREACH(const QFileInfo &info, dir.entryInfoList(QDir::Files)) {
        QString target = info.symLinkTarget();
        if (!target.isEmpty()) {
            QString src = info.baseName();
            if (src.endsWith(skippedSuffix)) {
                continue;
            }
            if (src.endsWith(directionSuffix)) {
                src.chop(4);
            }
            mTable.insert(src, QFileInfo(target).baseName());
        }
    }
}
