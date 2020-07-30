#!/usr/bin/python3
import json2html 
import MongoJob as mg
import json
import sys
import os
from PyQt5 import QtWidgets, uic

from PyQt5.QtWidgets import QMessageBox

from DaqWindow import Ui_DaqWindow
from mdccwindowimpl import MdccWindowImpl

class DaqWindowImpl(QtWidgets.QMainWindow, Ui_DaqWindow):
    def __init__(self, *args, obj=None,controller=None, **kwargs):
        super(DaqWindowImpl, self).__init__(*args, **kwargs)
        self.setupUi(self)

        self.ctrl=None
        if (controller!=None):
            self.ctrl=controller
            print(self.ctrl)
        self.make_connections()
        
    def setController(self,ctrl):
        print(ctrl)
        
        if (ctrl == None):
            return False
        self.ctrl=ctrl
        return True

    def make_connections(self):
        self.pbMDCC.clicked.connect(self.action_pbMDCC)
        self.pbINITIALISE.clicked.connect(self.action_pbINITIALISE)

    def action_pbMDCC(self):
        self.window_mdccc=MdccWindowImpl(self,controller=self.ctrl)
        self.window_mdccc.show()

    def action_pbINITIALISE(self):
        self.ctrl.daq_initialise()
