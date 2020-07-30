# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'logwindow.ui'
#
# Created by: PyQt5 UI code generator 5.7
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_LogWindow(object):
    def setupUi(self, LogWindow):
        LogWindow.setObjectName("LogWindow")
        LogWindow.resize(800, 600)
        self.centralwidget = QtWidgets.QWidget(LogWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.centralwidget)
        self.verticalLayout.setObjectName("verticalLayout")
        self.gbPROC = QtWidgets.QGroupBox(self.centralwidget)
        self.gbPROC.setMinimumSize(QtCore.QSize(0, 80))
        self.gbPROC.setObjectName("gbPROC")
        self.verticalLayout.addWidget(self.gbPROC)
        LogWindow.setCentralWidget(self.centralwidget)
        self.statusbar = QtWidgets.QStatusBar(LogWindow)
        self.statusbar.setObjectName("statusbar")
        LogWindow.setStatusBar(self.statusbar)

        self.retranslateUi(LogWindow)
        QtCore.QMetaObject.connectSlotsByName(LogWindow)

    def retranslateUi(self, LogWindow):
        _translate = QtCore.QCoreApplication.translate
        LogWindow.setWindowTitle(_translate("LogWindow", "Logs of processes"))
        self.gbPROC.setTitle(_translate("LogWindow", "Processes"))

