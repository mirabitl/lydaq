#!/usr/bin/python3
import json2html 
import MongoJob as mg
import json
import sys
import os
from PyQt5 import QtWidgets, uic

from PyQt5.QtWidgets import QMessageBox
from PyQt5.QtWidgets import QTextBrowser
from PyQt5.QtWidgets import QVBoxLayout

from PyQt5.QtWidgets import QListWidgetItem
from tbdialogimpl import TbDialogImpl

from DbWindow import Ui_DbWindow
import MongoRoc as mg


class DbWindowImpl(QtWidgets.QMainWindow, Ui_DbWindow):
    def __init__(self, *args, obj=None,controller=None, **kwargs):
        super(DbWindowImpl, self).__init__(*args, **kwargs)
        self.setupUi(self)

        self.lwMONGO.clear()
        self.db=mg.instance()
        self.lwMONGO.setMouseTracking(True)
        for x,y,z in self.db.states():
            print(x,y,z)
            itc=QListWidgetItem("%s:%s" % (x,y))
            itc.setToolTip(z)
            
            self.lwMONGO.addItem(itc)

        self.config=None
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
        self.pbDOWNLOAD.clicked.connect(self.action_DOWNLOAD)
        self.lwMONGO.doubleClicked.connect(self.action_lwMONGO)
        self.pbSHOWCONFIG.clicked.connect(self.action_pbSHOWCONFIG)

    def action_lwMONGO(self):
        #print()
        self.laDBMONGO.setText(self.lwMONGO.currentItem().text())
        self.config=self.laDBMONGO.text()
    def action_pbSHOWCONFIG(self):
        #print()
        config=self.lwMONGO.currentItem().text()
        self.db.download(config.split(":")[0],int(config.split(":")[1]),True)
        daq_file="/dev/shm/mgroc/"+config.split(":")[0]+"_"+config.split(":")[1]+".json"
        with open(daq_file) as data_file:    
            mgConfig = json.load(data_file)
            tb=TbDialogImpl(self)

            # setting the minimum size 
            tb.tbTEXT.setText(json.dumps(mgConfig, indent=4, sort_keys=True))
            tb.show()
        
    def action_DOWNLOAD(self):
        process=self.lwPROCESS.currentItem().text()
        par={}
        par["state"]=self.config.split(":")[0]
        par["version"]=int(self.config.split(":")[1])
        print(process,par)
        print(self.ctrl.processCommand("DOWNLOADDB",process,par))
        
