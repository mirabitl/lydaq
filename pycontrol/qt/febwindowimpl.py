#!/usr/bin/python3
import json2html 
import MongoJob as mg
import json
import sys
import os
from PyQt5 import QtWidgets, uic

from PyQt5.QtWidgets import QMessageBox

from FebWindow import Ui_FebWindow


class FebWindowImpl(QtWidgets.QMainWindow, Ui_FebWindow):
    def __init__(self, *args, obj=None,controller=None, **kwargs):
        super(FebWindowImpl, self).__init__(*args, **kwargs)
        self.setupUi(self)
