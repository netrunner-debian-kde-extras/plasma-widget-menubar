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
// Self
#include "menubarapplet.h"

// Qt
#include <QKeyEvent>
#include <QMenu>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

// KDE
#include <KConfigDialog>
#include <KDebug>
#include <KIcon>
#include <KWindowSystem>
#include <Plasma/Containment>
#include <Plasma/ContainmentActions>
#include <Plasma/Corona>
#include <Plasma/View>

// dbusmenu-qt
#include <dbusmenuimporter.h>

// QJson
#include <qjson/serializer.h>

// Local
#include "menucloner.h"
#include "menuutils.h"
#include "menuwidget.h"
#include "registrar.h"
#include "rendereradaptor.h"
#include "windowmenumanager.h"

class MyDBusMenuImporter : public DBusMenuImporter
{
public:
    MyDBusMenuImporter(const QString &service, const QString &path, GtkIconTable *table, QObject *parent)
    : DBusMenuImporter(service, path, parent)
    , mService(service)
    , mPath(path)
    , mGtkIconTable(table)
    {}

    QString service() const { return mService; }
    QString path() const { return mPath; }

protected:
    virtual QIcon iconForName(const QString &name_)
    {
        QString name;
        if (name_.startsWith("gtk")) {
            name = mGtkIconTable->value(name_);
        } else {
            name = name_;
        }
        return KIcon(name);
    }

private:
    QString mService;
    QString mPath;
    GtkIconTable *mGtkIconTable;
};


K_EXPORT_PLASMA_APPLET(menubarapplet, MenuBarApplet)


MenuBarApplet::MenuBarApplet(QObject* parent, const QVariantList& args)
: Plasma::Applet(parent, args)
, mGtkIconTable("/usr/share/icons/gnome/16x16") // FIXME Do not hardcode path
, mLayout(new QGraphicsLinearLayout(this))
, mRegistrar(new Registrar(this))
, mDesktopMenu(new QMenu)
, mWindowMenuManager(new WindowMenuManager(this))
, mWindowMenu(new QMenu)
, mMenuCloner(new MenuCloner(this))
, mActiveWinId(0)
, mMenuWidget(0)
{
    mGtkIconTable.setRightToLeft(QApplication::isRightToLeft());
    setBackgroundHints(StandardBackground);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
}

MenuBarApplet::~MenuBarApplet()
{
    mDesktopMenu->deleteLater();
    mWindowMenu->deleteLater();
}

void MenuBarApplet::init()
{
    mLayout->setContentsMargins(0, 0, 0, 0);

    setupWindowMenu();
    setupDesktopMenu();
    setupDBusInterface();

    updateSizePolicy();

    if (!mRegistrar->connectToBus()) {
        kWarning() << "Could not connect registrar to DBus";
        return;
    }

    connect(mRegistrar, SIGNAL(WindowRegistered(WId, const QString&, const QDBusObjectPath&)),
        SLOT(slotWindowRegistered(WId, const QString&, const QDBusObjectPath&)));

    connect(mRegistrar, SIGNAL(WindowUnregistered(WId)),
        SLOT(slotWindowUnregistered(WId)));

    connect(this, SIGNAL(activate()), SLOT(slotActivated()));

    KWindowSystem* ws = KWindowSystem::self();
    connect(ws, SIGNAL(activeWindowChanged(WId)), SLOT(updateActiveWinId()));

    updateActiveWinId();
}

void MenuBarApplet::setupDBusInterface()
{
    static int id = 0;
    QString path = QString("/com/canonical/AppMenu/Renderer_%1").arg(id++);
    new RendererAdaptor(this);
    QDBusConnection::sessionBus().registerObject(path, this);
}

void MenuBarApplet::setupWindowMenu()
{
    QAction* action = mWindowMenu->addAction(i18n("File"));
    action->setMenu(mWindowMenuManager->menu());
}

void MenuBarApplet::setupDesktopMenu()
{
    QMenu* subMenu = mDesktopMenu->addMenu(i18n("Menu"));
    connect(subMenu, SIGNAL(aboutToShow()), SLOT(fillDesktopMenu()));
}

void MenuBarApplet::fillDesktopMenu()
{
    QMenu* subMenu = mDesktopMenu->actions().first()->menu();
    subMenu->clear();

    // Try to find the desktop containment. First attempt uses the current
    // desktop, but will only work when different containments are set per
    // desktop. Second attempt uses default containment.
    Plasma::Corona* corona = containment()->corona();
    int screen = containment()->screen();
    Plasma::Containment* desktop = corona->containmentForScreen(screen, KWindowSystem::currentDesktop() - 1);
    if (!desktop) {
        desktop = corona->containmentForScreen(screen);
    }
    if (!desktop) {
        kWarning() << "!desktop";
        desktop = containment();
    }
    Plasma::ContainmentActions* actions = Plasma::ContainmentActions::load(desktop, "contextmenu");
    actions->restore(desktop->config());
    if (actions) {
        Q_FOREACH(QAction* action, actions->contextualActions()) {
            subMenu->addAction(action);
        }
    } else {
        QAction* action = subMenu->addAction(i18n("No Items"));
        action->setEnabled(false);
    }
}

static bool isDesktopWinId(WId winId)
{
    KWindowInfo info = KWindowSystem::windowInfo(winId, NET::WMWindowType);
    return NET::typeMatchesMask(info.windowType(NET::DesktopMask), NET::DesktopMask);
}

void MenuBarApplet::createMenuBar()
{
    WId winId = mActiveWinId;
    QMenu* menu = menuForWinId(winId);

    if (!menu) {
        if (winId && !isDesktopWinId(winId)) {
            // We have an active window
            WId mainWinId = KWindowSystem::transientFor(winId);
            if (mainWinId) {
                // We have a parent window, use a disabled version of its
                // menubar if it has one.
                QMenu* mainMenu = menuForWinId(mainWinId);
                if (mainMenu) {
                    mMenuCloner->setOriginalMenu(mainMenu);
                    menu = mMenuCloner->clonedMenu();
                }
            }
            if (!menu) {
                // No suitable menubar but we have a window, use the
                // generic window menu
                mWindowMenuManager->setWinId(winId);
                menu = mWindowMenu;
            }
        } else {
            // No active window, show a desktop menubar
            menu = mDesktopMenu;
        }
    }
    if (useButtonFormFactor()) {
        createButtonsForButtonFormFactor(menu);
    } else {
        createButtonsForBarFormFactor(menu);
    }
}

void MenuBarApplet::slotActivated()
{
    if (mMenuWidget) {
        mMenuWidget->activate();
    }
}

void MenuBarApplet::createButtonsForButtonFormFactor(QMenu* menu)
{
    QMenu* mainMenu = new QMenu;
    QAction* action = mainMenu->addAction(i18n("Menu"));
    if (menu->actions().count() == 1 && menu->actions().first()->menu()) {
        // If there is only one top-level item, use it directly
        // This is useful for window and desktop menus, but is also more
        // efficient for applications with only one item in their menubar
        menu = menu->actions().first()->menu();
    }
    action->setMenu(menu);
    createButtonsForBarFormFactor(mainMenu);
}

void MenuBarApplet::createButtonsForBarFormFactor(QMenu* menu)
{
    delete mMenuWidget;
    mMenuWidget = new MenuWidget(this);
    mMenuWidget->setMenu(menu);
    mLayout->addItem(mMenuWidget);

    if (!useButtonFormFactor()) {
        QPropertyAnimation *anim = new QPropertyAnimation(mMenuWidget, "opacity");
        anim->setStartValue(0);
        anim->setEndValue(1);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void MenuBarApplet::createConfigurationInterface(KConfigDialog* parent)
{
    QWidget* widget = new QWidget;
    parent->addPage(widget, i18n("General"), icon());
    connect(parent, SIGNAL(applyClicked()), SLOT(applyConfig()));
    connect(parent, SIGNAL(okClicked()), SLOT(applyConfig()));

    mConfigButtonFormFactor = new QCheckBox;
    mConfigButtonFormFactor->setText(i18n("Use Button Form Factor"));
    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->addWidget(mConfigButtonFormFactor);
    layout->addStretch();

    KConfigGroup cg = config();
    mConfigButtonFormFactor->setChecked(useButtonFormFactor());
}

void MenuBarApplet::applyConfig()
{
    KConfigGroup cg = config();
    cg.writeEntry("useButtonFormFactor", mConfigButtonFormFactor->isChecked());
    updateSizePolicy();
    createMenuBar();
}

void MenuBarApplet::updateSizePolicy()
{
    if (useButtonFormFactor()) {
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    } else {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        setMaximumSize(INT_MAX, INT_MAX);
    }
}

bool MenuBarApplet::useButtonFormFactor() const
{
    return config().readEntry("useButtonFormFactor", false);
}

void MenuBarApplet::updateActiveWinId()
{
    WId id = KWindowSystem::activeWindow();
    if (id == mActiveWinId) {
        return;
    }
    if (view() && id == view()->window()->winId()) {
        // Do not update id if the active window is the one hosting this applet
        return;
    }
    mActiveWinId = id;
    createMenuBar();
}

void MenuBarApplet::slotWindowRegistered(WId wid, const QString& service, const QDBusObjectPath& menuObjectPath)
{
    MyDBusMenuImporter* importer = new MyDBusMenuImporter(service, menuObjectPath.path(), &mGtkIconTable, this);
    delete mImporters.take(wid);
    mImporters.insert(wid, importer);
    connect(importer, SIGNAL(actionActivationRequested(QAction*)), SLOT(slotActionActivationRequested(QAction*)));
    QMetaObject::invokeMethod(importer, "updateMenu", Qt::QueuedConnection);

    // If the window is already active, create the menubar (Happens with GTK apps)
    if (KWindowSystem::activeWindow() == wid) {
        createMenuBar();
    }
}

void MenuBarApplet::slotWindowUnregistered(WId wid)
{
    MyDBusMenuImporter* importer = mImporters.take(wid);
    if (importer) {
        importer->deleteLater();
    }
    if (wid == mActiveWinId) {
        mActiveWinId = 0;
        createMenuBar();
    }
}

void MenuBarApplet::slotActionActivationRequested(QAction* action)
{
    DBusMenuImporter* importer = static_cast<DBusMenuImporter*>(sender());

    if (mImporters.value(mActiveWinId) == importer) {
        if (!mMenuWidget) {
            kWarning() << "No mMenuWidget, should not happen!";
            return;
        }
        if (useButtonFormFactor()) {
            mMenuWidget->activateActionInMenu(action);
        } else {
            mMenuWidget->activateAction(action);
        }
    }
}

QMenu* MenuBarApplet::menuForWinId(WId wid) const
{
    MyDBusMenuImporter* importer = mImporters.value(wid);
    return importer ? importer->menu() : 0;
}


QString MenuBarApplet::GetCurrentMenu(QDBusObjectPath& path) const
{
    QString service;
    MyDBusMenuImporter* importer = mImporters.value(mActiveWinId);
    if (importer) {
        service = importer->service();
        path = QDBusObjectPath(importer->path());
    } else {
        path = QDBusObjectPath("/");
    }
    return service;
}

void MenuBarApplet::ActivateMenuItem(const QList<int>&)
{
}

QString MenuBarApplet::DumpCurrentMenu() const
{
    return DumpMenu(mActiveWinId);
}

QString MenuBarApplet::DumpMenu(WId id) const
{
    MyDBusMenuImporter* importer = mImporters.value(id);
    QVariant variant = MenuUtils::variantFromMenu(importer ? importer->menu() : 0);
    QJson::Serializer serializer;
    return QString::fromUtf8(serializer.serialize(variant));
}

#include "menubarapplet.moc"
