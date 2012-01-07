#! /bin/sh
$EXTRACTRC `find src -name "*.ui" -o -name "*.rc" -o -name "*.kcfg" ` >> rc.cpp
$XGETTEXT `find src -name "*.cpp" -o -name "*.h"` -o $podir/plasma_applet_menubarapplet.pot
