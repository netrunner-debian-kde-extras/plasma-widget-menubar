#!/usr/bin/env python
import sys

from PyQt4.QtCore import *
from PyQt4.QtGui import *

class Window(QMainWindow):
    def __init__(self):
        super(Window, self).__init__()
        self.createMenu("&File")
        self.updateMenu = self.createMenu("&Update")
        self.createMenu("&Help")

        QTimer.singleShot(500, self.connectUpdateMenu)

    def connectUpdateMenu(self):
        self.updateMenu.aboutToShow.connect(self.updateMenuBar)
        print "Ready"

    def createMenu(self, text):
        menu = self.menuBar().addMenu(text)
        menu.addAction("Item")
        return menu

    def updateMenuBar(self):
        self.updateMenu.aboutToShow.disconnect(self.updateMenuBar)
        self.createMenu("New")

def main():
    app = QApplication(sys.argv)
    window = Window()

    window.show()
    app.exec_()
    return 0

if __name__ == "__main__":
    sys.exit(main())
# vi: ts=4 sw=4 et
