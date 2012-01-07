#!/usr/bin/env python
import sys

from PyQt4.QtCore import *
from PyQt4.QtGui import *

def main():
    app = QApplication(sys.argv)
    window = QMainWindow()
    for x in range(30):
        menu = window.menuBar().addMenu("Menu %d" % x)
        menu.addAction("Item")

    window.show()
    app.exec_()
    return 0

if __name__ == "__main__":
    sys.exit(main())
# vi: ts=4 sw=4 et