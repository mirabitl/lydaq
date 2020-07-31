#!/usr/bin/python3
import json2html 
import MongoJob as mg
import json
import sys
import os
from PyQt5 import QtWidgets, uic

from PyQt5.QtWidgets import QMessageBox

from GenesysWindow import Ui_GenesysWindow


class GenesysWindowImpl(QtWidgets.QMainWindow, Ui_GenesysWindow):
    def __init__(self, *args, obj=None,controller=None, **kwargs):
        super(GenesysWindowImpl, self).__init__(*args, **kwargs)
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
        self.pbDEVICE.clicked.connect(self.action_genesys_device)
        self.pbOPEN.clicked.connect(self.action_genesys_open)
        self.pbCLOSE.clicked.connect(self.action_genesys_close)
        self.pbOFF.clicked.connect(self.action_genesys_off)
        self.pbON.clicked.connect(self.action_genesys_on)
        self.pbSTATUS.clicked.connect(self.action_genesys_status)


        
    def action_genesys_device(self):
        par={}
        par["device"]=self.sbDEVICE.value()
        par["address"]=self.sbADDRESS.value()
        self.ctrl.appMap["GENESYS"][0].sendCommand("DEVICE",par)
        self.action_genesys_status()
    def action_genesys_open(self):
        self.ctrl.appMap["GENESYS"][0].sendTransition("OPEN",{})
        self.action_genesys_status()
    def action_genesys_close(self):
        self.ctrl.appMap["GENESYS"][0].sendTransition("CLOSE",{})
        self.action_genesys_status()
        
    def action_genesys_on(self):
        self.ctrl.appMap["GENESYS"][0].sendCommand("ON",{})
        self.action_genesys_status()

    def action_genesys_off(self):
        self.ctrl.appMap["GENESYS"][0].sendCommand("OFF",{})
        self.action_genesys_status()
        
    def action_genesys_status(self):
        srep=self.ctrl.appMap["GENESYS"][0].sendCommand("STATUS",{})
        if (type(srep) is bytes):
            srep=srep.decode("utf-8")
        s="<h1>Status</h1>"
        s=s+"<P> <b>Parameters:</b>"
        s=s+json2html.json2html.convert(json=srep)
        s=s+"</P></br>"
        self.tbSTATUS.setText(s)

