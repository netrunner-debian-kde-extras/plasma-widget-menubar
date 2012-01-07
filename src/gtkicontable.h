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
#ifndef GTKICONTABLE_H
#define GTKICONTABLE_H

// Qt
#include <QHash>

/**
 * This class provides a mapping between gtk icon names and fdo standard icon
 * names.
 * It initializes itself when value() is called for the first time.
 * To create this mapping it reads the symlinks which can be found in the
 * "gnome" icon theme.
 */
class GtkIconTable
{
public:
    explicit GtkIconTable(const QString &gnomeThemeDirName);
    QString value(const QString &name) const;

    bool isRightToLeft() const { return mIsRightToLeft; }
    void setRightToLeft(bool);

private:
    QString mGnomeThemeDirName;
    bool mIsRightToLeft;
    QHash<QString, QString> mTable;
    void init();
    void readSymlinksInDir(const QString &);
};

#endif /* GTKICONTABLE_H */
