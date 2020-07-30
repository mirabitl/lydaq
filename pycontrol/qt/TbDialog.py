# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'tbdialog.ui'
#
# Created by: PyQt5 UI code generator 5.7
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_TbDialog(object):
    def setupUi(self, TbDialog):
        TbDialog.setObjectName("TbDialog")
        TbDialog.resize(575, 464)
        self.gridLayout = QtWidgets.QGridLayout(TbDialog)
        self.gridLayout.setObjectName("gridLayout")
        self.tbTEXT = QtWidgets.QTextBrowser(TbDialog)
        self.tbTEXT.setObjectName("tbTEXT")
        self.gridLayout.addWidget(self.tbTEXT, 0, 0, 1, 1)
        self.buttonBox = QtWidgets.QDialogButtonBox(TbDialog)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtWidgets.QDialogButtonBox.Cancel|QtWidgets.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.gridLayout.addWidget(self.buttonBox, 1, 0, 1, 1)

        self.retranslateUi(TbDialog)
        self.buttonBox.rejected.connect(TbDialog.reject)
        self.buttonBox.accepted.connect(TbDialog.accept)
        QtCore.QMetaObject.connectSlotsByName(TbDialog)

    def retranslateUi(self, TbDialog):
        _translate = QtCore.QCoreApplication.translate
        TbDialog.setWindowTitle(_translate("TbDialog", "Dialog"))

