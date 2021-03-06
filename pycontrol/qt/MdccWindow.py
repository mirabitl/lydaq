# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'mdccwindow.ui'
#
# Created by: PyQt5 UI code generator 5.7
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_MdccWindow(object):
    def setupUi(self, MdccWindow):
        MdccWindow.setObjectName("MdccWindow")
        MdccWindow.resize(800, 600)
        self.centralwidget = QtWidgets.QWidget(MdccWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.centralwidget)
        self.verticalLayout.setObjectName("verticalLayout")
        self.groupBox_7 = QtWidgets.QGroupBox(self.centralwidget)
        self.groupBox_7.setAutoFillBackground(True)
        self.groupBox_7.setObjectName("groupBox_7")
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout(self.groupBox_7)
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.PBMdccPhysic = QtWidgets.QPushButton(self.groupBox_7)
        self.PBMdccPhysic.setObjectName("PBMdccPhysic")
        self.horizontalLayout_2.addWidget(self.PBMdccPhysic)
        self.PBMdccResume = QtWidgets.QPushButton(self.groupBox_7)
        self.PBMdccResume.setObjectName("PBMdccResume")
        self.horizontalLayout_2.addWidget(self.PBMdccResume)
        self.PBMdccPause = QtWidgets.QPushButton(self.groupBox_7)
        self.PBMdccPause.setObjectName("PBMdccPause")
        self.horizontalLayout_2.addWidget(self.PBMdccPause)
        self.PBMdccResetTdc = QtWidgets.QPushButton(self.groupBox_7)
        self.PBMdccResetTdc.setObjectName("PBMdccResetTdc")
        self.horizontalLayout_2.addWidget(self.PBMdccResetTdc)
        self.PBMdccResetCounters = QtWidgets.QPushButton(self.groupBox_7)
        self.PBMdccResetCounters.setObjectName("PBMdccResetCounters")
        self.horizontalLayout_2.addWidget(self.PBMdccResetCounters)
        self.PBMdccSet = QtWidgets.QPushButton(self.groupBox_7)
        self.PBMdccSet.setObjectName("PBMdccSet")
        self.horizontalLayout_2.addWidget(self.PBMdccSet)
        self.PBStatus = QtWidgets.QPushButton(self.groupBox_7)
        self.PBStatus.setObjectName("PBStatus")
        self.horizontalLayout_2.addWidget(self.PBStatus)
        self.verticalLayout.addWidget(self.groupBox_7)
        self.groupBox_6 = QtWidgets.QGroupBox(self.centralwidget)
        self.groupBox_6.setAutoFillBackground(True)
        self.groupBox_6.setObjectName("groupBox_6")
        self.gridLayout = QtWidgets.QGridLayout(self.groupBox_6)
        self.gridLayout.setObjectName("gridLayout")
        self.frame_5 = QtWidgets.QFrame(self.groupBox_6)
        self.frame_5.setMinimumSize(QtCore.QSize(181, 61))
        self.frame_5.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.frame_5.setFrameShadow(QtWidgets.QFrame.Raised)
        self.frame_5.setObjectName("frame_5")
        self.horizontalLayout_7 = QtWidgets.QHBoxLayout(self.frame_5)
        self.horizontalLayout_7.setObjectName("horizontalLayout_7")
        self.label_6 = QtWidgets.QLabel(self.frame_5)
        self.label_6.setObjectName("label_6")
        self.horizontalLayout_7.addWidget(self.label_6)
        self.SBWindowMode = QtWidgets.QSpinBox(self.frame_5)
        self.SBWindowMode.setMinimumSize(QtCore.QSize(57, 14))
        self.SBWindowMode.setToolTip("")
        self.SBWindowMode.setMinimum(1)
        self.SBWindowMode.setMaximum(256)
        self.SBWindowMode.setProperty("value", 64)
        self.SBWindowMode.setObjectName("SBWindowMode")
        self.horizontalLayout_7.addWidget(self.SBWindowMode)
        self.gridLayout.addWidget(self.frame_5, 0, 0, 1, 1)
        self.frame = QtWidgets.QFrame(self.groupBox_6)
        self.frame.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.frame.setFrameShadow(QtWidgets.QFrame.Raised)
        self.frame.setObjectName("frame")
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.frame)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.label_3 = QtWidgets.QLabel(self.frame)
        self.label_3.setObjectName("label_3")
        self.horizontalLayout.addWidget(self.label_3)
        self.LESpillOn = QtWidgets.QLineEdit(self.frame)
        self.LESpillOn.setObjectName("LESpillOn")
        self.horizontalLayout.addWidget(self.LESpillOn)
        self.gridLayout.addWidget(self.frame, 0, 1, 1, 1)
        self.frame_2 = QtWidgets.QFrame(self.groupBox_6)
        self.frame_2.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.frame_2.setFrameShadow(QtWidgets.QFrame.Raised)
        self.frame_2.setObjectName("frame_2")
        self.horizontalLayout_4 = QtWidgets.QHBoxLayout(self.frame_2)
        self.horizontalLayout_4.setObjectName("horizontalLayout_4")
        self.label_4 = QtWidgets.QLabel(self.frame_2)
        self.label_4.setObjectName("label_4")
        self.horizontalLayout_4.addWidget(self.label_4)
        self.LESpillOff = QtWidgets.QLineEdit(self.frame_2)
        self.LESpillOff.setObjectName("LESpillOff")
        self.horizontalLayout_4.addWidget(self.LESpillOff)
        self.gridLayout.addWidget(self.frame_2, 0, 2, 1, 1)
        self.frame_4 = QtWidgets.QFrame(self.groupBox_6)
        self.frame_4.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.frame_4.setFrameShadow(QtWidgets.QFrame.Raised)
        self.frame_4.setObjectName("frame_4")
        self.horizontalLayout_6 = QtWidgets.QHBoxLayout(self.frame_4)
        self.horizontalLayout_6.setObjectName("horizontalLayout_6")
        self.label_7 = QtWidgets.QLabel(self.frame_4)
        self.label_7.setObjectName("label_7")
        self.horizontalLayout_6.addWidget(self.label_7)
        self.SBBusy = QtWidgets.QSpinBox(self.frame_4)
        self.SBBusy.setMinimum(1)
        self.SBBusy.setObjectName("SBBusy")
        self.horizontalLayout_6.addWidget(self.SBBusy)
        self.gridLayout.addWidget(self.frame_4, 1, 0, 1, 1)
        self.frame_3 = QtWidgets.QFrame(self.groupBox_6)
        self.frame_3.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.frame_3.setFrameShadow(QtWidgets.QFrame.Raised)
        self.frame_3.setObjectName("frame_3")
        self.horizontalLayout_5 = QtWidgets.QHBoxLayout(self.frame_3)
        self.horizontalLayout_5.setObjectName("horizontalLayout_5")
        self.label_5 = QtWidgets.QLabel(self.frame_3)
        self.label_5.setObjectName("label_5")
        self.horizontalLayout_5.addWidget(self.label_5)
        self.LEBeamOn = QtWidgets.QLineEdit(self.frame_3)
        self.LEBeamOn.setObjectName("LEBeamOn")
        self.horizontalLayout_5.addWidget(self.LEBeamOn)
        self.gridLayout.addWidget(self.frame_3, 1, 1, 1, 1)
        spacerItem = QtWidgets.QSpacerItem(221, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout.addItem(spacerItem, 1, 2, 1, 1)
        self.verticalLayout.addWidget(self.groupBox_6)
        self.tbSTATUS = QtWidgets.QTextBrowser(self.centralwidget)
        self.tbSTATUS.setObjectName("tbSTATUS")
        self.verticalLayout.addWidget(self.tbSTATUS)
        MdccWindow.setCentralWidget(self.centralwidget)
        self.statusbar = QtWidgets.QStatusBar(MdccWindow)
        self.statusbar.setObjectName("statusbar")
        MdccWindow.setStatusBar(self.statusbar)

        self.retranslateUi(MdccWindow)
        QtCore.QMetaObject.connectSlotsByName(MdccWindow)

    def retranslateUi(self, MdccWindow):
        _translate = QtCore.QCoreApplication.translate
        MdccWindow.setWindowTitle(_translate("MdccWindow", "MDCC"))
        self.groupBox_7.setTitle(_translate("MdccWindow", "Commands"))
        self.PBMdccPhysic.setText(_translate("MdccWindow", "Physic"))
        self.PBMdccResume.setText(_translate("MdccWindow", "Resume"))
        self.PBMdccPause.setText(_translate("MdccWindow", "Pause"))
        self.PBMdccResetTdc.setText(_translate("MdccWindow", "Reset TDC"))
        self.PBMdccResetCounters.setText(_translate("MdccWindow", "Reset Counters"))
        self.PBMdccSet.setText(_translate("MdccWindow", "Set Registers"))
        self.PBStatus.setText(_translate("MdccWindow", "Status"))
        self.groupBox_6.setTitle(_translate("MdccWindow", "Registers"))
        self.label_6.setText(_translate("MdccWindow", "Window Mode"))
        self.label_3.setText(_translate("MdccWindow", "Spill On"))
        self.LESpillOn.setText(_translate("MdccWindow", "10000"))
        self.label_4.setText(_translate("MdccWindow", "Spill Off"))
        self.LESpillOff.setText(_translate("MdccWindow", "10000"))
        self.label_7.setText(_translate("MdccWindow", "Busy Latch"))
        self.label_5.setText(_translate("MdccWindow", "Beam"))
        self.LEBeamOn.setText(_translate("MdccWindow", "10000000"))

