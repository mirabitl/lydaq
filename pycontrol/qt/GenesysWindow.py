# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'genesyswindow.ui'
#
# Created by: PyQt5 UI code generator 5.7
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_GenesysWindow(object):
    def setupUi(self, GenesysWindow):
        GenesysWindow.setObjectName("GenesysWindow")
        GenesysWindow.resize(499, 490)
        self.centralwidget = QtWidgets.QWidget(GenesysWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.centralwidget)
        self.verticalLayout.setObjectName("verticalLayout")
        self.groupBox = QtWidgets.QGroupBox(self.centralwidget)
        self.groupBox.setObjectName("groupBox")
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.groupBox)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.label = QtWidgets.QLabel(self.groupBox)
        self.label.setObjectName("label")
        self.horizontalLayout.addWidget(self.label)
        self.sbDEVICE = QtWidgets.QSpinBox(self.groupBox)
        self.sbDEVICE.setObjectName("sbDEVICE")
        self.horizontalLayout.addWidget(self.sbDEVICE)
        self.label_2 = QtWidgets.QLabel(self.groupBox)
        self.label_2.setObjectName("label_2")
        self.horizontalLayout.addWidget(self.label_2)
        self.sbADDRESS = QtWidgets.QSpinBox(self.groupBox)
        self.sbADDRESS.setObjectName("sbADDRESS")
        self.horizontalLayout.addWidget(self.sbADDRESS)
        self.pbDEVICE = QtWidgets.QPushButton(self.groupBox)
        self.pbDEVICE.setObjectName("pbDEVICE")
        self.horizontalLayout.addWidget(self.pbDEVICE)
        self.verticalLayout.addWidget(self.groupBox)
        self.groupBox_2 = QtWidgets.QGroupBox(self.centralwidget)
        self.groupBox_2.setObjectName("groupBox_2")
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout(self.groupBox_2)
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.pbOPEN = QtWidgets.QPushButton(self.groupBox_2)
        self.pbOPEN.setObjectName("pbOPEN")
        self.horizontalLayout_2.addWidget(self.pbOPEN)
        self.pbCLOSE = QtWidgets.QPushButton(self.groupBox_2)
        self.pbCLOSE.setObjectName("pbCLOSE")
        self.horizontalLayout_2.addWidget(self.pbCLOSE)
        self.pbON = QtWidgets.QPushButton(self.groupBox_2)
        self.pbON.setObjectName("pbON")
        self.horizontalLayout_2.addWidget(self.pbON)
        self.pbOFF = QtWidgets.QPushButton(self.groupBox_2)
        self.pbOFF.setObjectName("pbOFF")
        self.horizontalLayout_2.addWidget(self.pbOFF)
        self.pbSTATUS = QtWidgets.QPushButton(self.groupBox_2)
        self.pbSTATUS.setObjectName("pbSTATUS")
        self.horizontalLayout_2.addWidget(self.pbSTATUS)
        self.verticalLayout.addWidget(self.groupBox_2)
        self.tbSTATUS = QtWidgets.QTextBrowser(self.centralwidget)
        self.tbSTATUS.setObjectName("tbSTATUS")
        self.verticalLayout.addWidget(self.tbSTATUS)
        GenesysWindow.setCentralWidget(self.centralwidget)
        self.statusbar = QtWidgets.QStatusBar(GenesysWindow)
        self.statusbar.setObjectName("statusbar")
        GenesysWindow.setStatusBar(self.statusbar)

        self.retranslateUi(GenesysWindow)
        QtCore.QMetaObject.connectSlotsByName(GenesysWindow)

    def retranslateUi(self, GenesysWindow):
        _translate = QtCore.QCoreApplication.translate
        GenesysWindow.setWindowTitle(_translate("GenesysWindow", "Genesys LV Control"))
        self.groupBox.setTitle(_translate("GenesysWindow", "Device setting"))
        self.label.setText(_translate("GenesysWindow", "Serial  Port"))
        self.label_2.setText(_translate("GenesysWindow", "Address"))
        self.pbDEVICE.setText(_translate("GenesysWindow", "Set"))
        self.groupBox_2.setTitle(_translate("GenesysWindow", "Commands"))
        self.pbOPEN.setText(_translate("GenesysWindow", "OPEN"))
        self.pbCLOSE.setText(_translate("GenesysWindow", "CLOSE"))
        self.pbON.setText(_translate("GenesysWindow", "ON"))
        self.pbOFF.setText(_translate("GenesysWindow", "OFF"))
        self.pbSTATUS.setText(_translate("GenesysWindow", "STATUS"))

