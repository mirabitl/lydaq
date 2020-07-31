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
from febwindowimpl import FebWindowImpl
from genesyswindowimpl import GenesysWindowImpl

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
        self.pbFEB.clicked.connect(self.action_pbFEB)
        self.pbGENESYS.clicked.connect(self.action_pbGENESYS)
        self.pbINITIALISE.clicked.connect(self.action_pbINITIALISE)

    def action_pbMDCC(self):
        self.window_mdccc=MdccWindowImpl(self,controller=self.ctrl)
        self.window_mdccc.show()

    def action_pbFEB(self):
        self.window_feb=FebWindowImpl(self,controller=self.ctrl)
        self.window_feb.show()
        
    def action_pbGENESYS(self):
        if ( not "GENESYS" in self.ctrl.appMap ):
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Critical)
            msg.setText("No GENESYS application")
            msg.setWindowTitle("Error")
            msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
            retval = msg.exec_()
        else:
            self.window_genesys=GenesysWindowImpl(self,controller=self.ctrl)
            self.window_genesys.show()

    def action_pbINITIALISE(self):
        self.ctrl.daq_initialise()
