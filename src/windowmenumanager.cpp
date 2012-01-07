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
#include <windowmenumanager.moc>

// Qt
#include <QX11Info>

// KDE
#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <KWindowSystem>
#include <netwm.h>

WindowMenuManager::WindowMenuManager(QObject* parent)
: QObject(parent)
, mMenu(new QMenu)
, mWid(0)
{
    connect(mMenu, SIGNAL(destroyed()), SLOT(deleteLater()));
    connect(mMenu, SIGNAL(aboutToShow()), SLOT(updateActions()));

    mCloseAction = mMenu->addAction(i18n("Close"), this, SLOT(closeWindow()));
    mCloseAction->setIcon(KIcon("window-close"));
}

WindowMenuManager::~WindowMenuManager()
{
    mMenu->deleteLater();
}

void WindowMenuManager::updateActions()
{
    if (!mWid) {
        kWarning() << "No winId!";
        return;
    }
    unsigned long properties[2] = {0, NET::WM2AllowedActions};
    NETWinInfo2 ni(QX11Info::display(), mWid, QX11Info::appRootWindow(), properties, 2);
    mCloseAction->setEnabled(ni.allowedActions() & NET::ActionClose);
}

void WindowMenuManager::setWinId(WId wid)
{
    mWid = wid;
}

void WindowMenuManager::closeWindow()
{
    NETRootInfo ri( QX11Info::display(), NET::CloseWindow );
    ri.closeWindowRequest(mWid);
}
