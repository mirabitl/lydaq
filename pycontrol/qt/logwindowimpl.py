#!/usr/bin/python3
import json2html 
import MongoJob as mg
import json
import sys
import os
from PyQt5 import QtWidgets, uic

from PyQt5.QtWidgets import QMessageBox

from LogWindow import Ui_LogWindow


class LogWindowImpl(QtWidgets.QMainWindow, Ui_LogWindow):
    def __init__(self, *args, obj=None,controller=None, **kwargs):
        super(LogWindowImpl, self).__init__(*args, **kwargs)
        self.setupUi(self)
