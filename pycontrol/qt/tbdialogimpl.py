#!/usr/bin/python3


from PyQt5 import QtWidgets, uic

from PyQt5.QtWidgets import QMessageBox


from TbDialog import Ui_TbDialog

class TbDialogImpl(QtWidgets.QDialog, Ui_TbDialog):
    def __init__(self, *args, obj=None, **kwargs):
        super(TbDialogImpl, self).__init__(*args, **kwargs)
        self.setupUi(self)
