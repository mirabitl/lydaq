# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'daqwindow.ui'
#
# Created by: PyQt5 UI code generator 5.10.1
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_DaqWindow(object):
    def setupUi(self, DaqWindow):
        DaqWindow.setObjectName("DaqWindow")
        DaqWindow.resize(800, 600)
        self.centralwidget = QtWidgets.QWidget(DaqWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.verticalLayout_2 = QtWidgets.QVBoxLayout(self.centralwidget)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.groupBox = QtWidgets.QGroupBox(self.centralwidget)
        self.groupBox.setAutoFillBackground(True)
        self.groupBox.setObjectName("groupBox")
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.groupBox)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.pbMDCC = QtWidgets.QPushButton(self.groupBox)
        self.pbMDCC.setObjectName("pbMDCC")
        self.horizontalLayout.addWidget(self.pbMDCC)
        spacerItem = QtWidgets.QSpacerItem(18, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.pbDIF = QtWidgets.QPushButton(self.groupBox)
        self.pbDIF.setObjectName("pbDIF")
        self.horizontalLayout.addWidget(self.pbDIF)
        spacerItem1 = QtWidgets.QSpacerItem(18, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem1)
        self.pbDB = QtWidgets.QPushButton(self.groupBox)
        self.pbDB.setObjectName("pbDB")
        self.horizontalLayout.addWidget(self.pbDB)
        spacerItem2 = QtWidgets.QSpacerItem(18, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem2)
        self.pbFEB = QtWidgets.QPushButton(self.groupBox)
        self.pbFEB.setObjectName("pbFEB")
        self.horizontalLayout.addWidget(self.pbFEB)
        spacerItem3 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem3)
        self.pbGENESYS = QtWidgets.QPushButton(self.groupBox)
        self.pbGENESYS.setObjectName("pbGENESYS")
        self.horizontalLayout.addWidget(self.pbGENESYS)
        self.verticalLayout_2.addWidget(self.groupBox)
        self.groupBox_2 = QtWidgets.QGroupBox(self.centralwidget)
        self.groupBox_2.setAutoFillBackground(True)
        self.groupBox_2.setObjectName("groupBox_2")
        self.gridLayout = QtWidgets.QGridLayout(self.groupBox_2)
        self.gridLayout.setObjectName("gridLayout")
        self.pbINITIALISE = QtWidgets.QPushButton(self.groupBox_2)
        self.pbINITIALISE.setObjectName("pbINITIALISE")
        self.gridLayout.addWidget(self.pbINITIALISE, 0, 0, 1, 1)
        spacerItem4 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout.addItem(spacerItem4, 0, 1, 1, 2)
        self.pbCONFIGURE = QtWidgets.QPushButton(self.groupBox_2)
        self.pbCONFIGURE.setObjectName("pbCONFIGURE")
        self.gridLayout.addWidget(self.pbCONFIGURE, 0, 3, 1, 1)
        spacerItem5 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout.addItem(spacerItem5, 0, 4, 1, 1)
        self.pbSTART = QtWidgets.QPushButton(self.groupBox_2)
        self.pbSTART.setObjectName("pbSTART")
        self.gridLayout.addWidget(self.pbSTART, 0, 5, 1, 1)
        spacerItem6 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout.addItem(spacerItem6, 0, 6, 1, 1)
        self.pbSTOP = QtWidgets.QPushButton(self.groupBox_2)
        self.pbSTOP.setObjectName("pbSTOP")
        self.gridLayout.addWidget(self.pbSTOP, 0, 7, 1, 1)
        spacerItem7 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout.addItem(spacerItem7, 0, 8, 1, 1)
        self.pbDESTROY = QtWidgets.QPushButton(self.groupBox_2)
        self.pbDESTROY.setObjectName("pbDESTROY")
        self.gridLayout.addWidget(self.pbDESTROY, 0, 9, 1, 1)
        self.label_3 = QtWidgets.QLabel(self.groupBox_2)
        self.label_3.setObjectName("label_3")
        self.gridLayout.addWidget(self.label_3, 1, 0, 1, 2)
        self.leCOMMENT = QtWidgets.QLineEdit(self.groupBox_2)
        self.leCOMMENT.setObjectName("leCOMMENT")
        self.gridLayout.addWidget(self.leCOMMENT, 1, 2, 1, 8)
        self.verticalLayout_2.addWidget(self.groupBox_2)
        self.groupBox_3 = QtWidgets.QGroupBox(self.centralwidget)
        self.groupBox_3.setAutoFillBackground(True)
        self.groupBox_3.setObjectName("groupBox_3")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.groupBox_3)
        self.verticalLayout.setObjectName("verticalLayout")
        self.groupBox_4 = QtWidgets.QGroupBox(self.groupBox_3)
        self.groupBox_4.setAutoFillBackground(True)
        self.groupBox_4.setObjectName("groupBox_4")
        self.horizontalLayout_3 = QtWidgets.QHBoxLayout(self.groupBox_4)
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        self.laCMD = QtWidgets.QLabel(self.groupBox_4)
        font = QtGui.QFont()
        font.setPointSize(12)
        font.setBold(True)
        font.setItalic(True)
        font.setWeight(75)
        self.laCMD.setFont(font)
        self.laCMD.setObjectName("laCMD")
        self.horizontalLayout_3.addWidget(self.laCMD)
        spacerItem8 = QtWidgets.QSpacerItem(201, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_3.addItem(spacerItem8)
        self.label = QtWidgets.QLabel(self.groupBox_4)
        self.label.setObjectName("label")
        self.horizontalLayout_3.addWidget(self.label)
        self.lcdRUN = QtWidgets.QLCDNumber(self.groupBox_4)
        self.lcdRUN.setObjectName("lcdRUN")
        self.horizontalLayout_3.addWidget(self.lcdRUN)
        spacerItem9 = QtWidgets.QSpacerItem(201, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_3.addItem(spacerItem9)
        self.label_2 = QtWidgets.QLabel(self.groupBox_4)
        self.label_2.setObjectName("label_2")
        self.horizontalLayout_3.addWidget(self.label_2)
        self.lcdEvent = QtWidgets.QLCDNumber(self.groupBox_4)
        self.lcdEvent.setObjectName("lcdEvent")
        self.horizontalLayout_3.addWidget(self.lcdEvent)
        self.pbUpdate = QtWidgets.QPushButton(self.groupBox_4)
        self.pbUpdate.setObjectName("pbUpdate")
        self.horizontalLayout_3.addWidget(self.pbUpdate)
        self.verticalLayout.addWidget(self.groupBox_4)
        self.tbSTATUS = QtWidgets.QTextBrowser(self.groupBox_3)
        self.tbSTATUS.setObjectName("tbSTATUS")
        self.verticalLayout.addWidget(self.tbSTATUS)
        self.verticalLayout_2.addWidget(self.groupBox_3)
        DaqWindow.setCentralWidget(self.centralwidget)
        self.statusbar = QtWidgets.QStatusBar(DaqWindow)
        self.statusbar.setObjectName("statusbar")
        DaqWindow.setStatusBar(self.statusbar)

        self.retranslateUi(DaqWindow)
        QtCore.QMetaObject.connectSlotsByName(DaqWindow)

    def retranslateUi(self, DaqWindow):
        _translate = QtCore.QCoreApplication.translate
        DaqWindow.setWindowTitle(_translate("DaqWindow", "Daq Access"))
        self.groupBox.setTitle(_translate("DaqWindow", "Settings"))
        self.pbMDCC.setText(_translate("DaqWindow", "MDCC..."))
        self.pbDIF.setText(_translate("DaqWindow", "DIF..."))
        self.pbDB.setText(_translate("DaqWindow", "Database ASICs"))
        self.pbFEB.setText(_translate("DaqWindow", "FEB..."))
        self.pbGENESYS.setText(_translate("DaqWindow", "Genesys.."))
        self.groupBox_2.setTitle(_translate("DaqWindow", "State Machine "))
        self.pbINITIALISE.setText(_translate("DaqWindow", "Initialise"))
        self.pbCONFIGURE.setText(_translate("DaqWindow", "Configure"))
        self.pbSTART.setText(_translate("DaqWindow", "Start"))
        self.pbSTOP.setText(_translate("DaqWindow", "Stop"))
        self.pbDESTROY.setText(_translate("DaqWindow", "Destroy"))
        self.label_3.setText(_translate("DaqWindow", "Run comment "))
        self.leCOMMENT.setText(_translate("DaqWindow", "Not set"))
        self.groupBox_3.setTitle(_translate("DaqWindow", "Status"))
        self.groupBox_4.setTitle(_translate("DaqWindow", "FSM"))
        self.laCMD.setText(_translate("DaqWindow", "Last Command"))
        self.label.setText(_translate("DaqWindow", "Run"))
        self.label_2.setText(_translate("DaqWindow", "Event"))
        self.pbUpdate.setText(_translate("DaqWindow", "Update Status"))

