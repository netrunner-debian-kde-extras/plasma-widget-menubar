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

// KDE
#include <qtest_kde.h>

// Local
#include "gtkicontable.h"

class GtkActionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testValue_data()
    {
        QTest::addColumn<QString>("src");
        QTest::addColumn<QString>("expected");

        #define ADD_ROW(src, expected) QTest::newRow(src) << src << expected
        ADD_ROW("gtk-quit", "application-exit");
        ADD_ROW("foo-custom-icon", "foo-custom-icon");
        #undef ADD_ROW
    }

    void testValue()
    {
        QFETCH(QString, src);
        QFETCH(QString, expected);
        GtkIconTable table("/usr/share/icons/gnome/16x16");

        QString result = table.value(src);
        QCOMPARE(result, expected);
    }

    void testRightToLeft()
    {
        GtkIconTable table("/usr/share/icons/gnome/16x16");

        QCOMPARE(table.value("gtk-goto-first"), QString("go-first"));
        table.setRightToLeft(true);
        QCOMPARE(table.value("gtk-goto-first"), QString("go-last"));
    }

};

QTEST_KDEMAIN(GtkActionTest, GUI)

#include "gtkactiontest.moc"
