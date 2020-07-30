#!/usr/bin/python3

import MongoJob as mg
import combrc
import sys
import os
import json
from PyQt5 import QtWidgets, uic

from PyQt5.QtWidgets import QMessageBox
from PyQt5.QtWidgets import QTextBrowser
from PyQt5.QtWidgets import QVBoxLayout

from PyQt5.QtWidgets import QListWidgetItem

from MainWindow import Ui_MainWindow

from jobdialogimpl import JobDialogImpl
from tbdialogimpl import TbDialogImpl
from daqwindowimpl import DaqWindowImpl

class MainWindow(QtWidgets.QMainWindow, Ui_MainWindow):
    def __init__(self, *args, obj=None, **kwargs):
        super(MainWindow, self).__init__(*args, **kwargs)
        self.setupUi(self)
        account = os.getenv("MGDBLOGIN", "NONE")
        if (account == "NONE"):
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Critical)
            msg.setText("Fatal the MGDBLOGIN value should be set")
            msg.setWindowTitle("Fatal")
            msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
            retval = msg.exec_()
            print("The ENV varaible MGDBLOGIN=user/pwd@host:port@dbname mut be set")
            exit(0)
        self.account=account
        self.laMONGOACCOUNT.setText(account)
        self.lwMONGO.clear()
        self.db=mg.instance()
        self.lwMONGO.setMouseTracking(True)
        for x,y,z in self.db.configurations():
            print(x,y,z)
            itc=QListWidgetItem("%s:%s" % (x,y))
            itc.setToolTip(z)

            self.lwMONGO.addItem(itc)
        daqsetup = os.getenv("DAQSETUP", "NONE")

        if (daqsetup == "NONE"):
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Critical)
            msg.setText("Fatal the DAQSETUP value should be set")
            msg.setWindowTitle("Fatal")
            msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
            retval = msg.exec_()
            #print("The ENV varaible DAQSETUP must be set")
            exit(0)
        self.laDAQSETUP.setText(daqsetup)
        self.setup=daqsetup
        daqmongo = os.getenv("DAQMONGO", "NONE")
        self.laDAQMONGO.setText(daqmongo)
        self.config=self.laDAQMONGO.text()
        print(self.config)
        sockport = None
        sp = os.getenv("SOCKPORT", "Not Found")
        if (sp != "Not Found"):
            sockport = int(sp)
        if (sockport != None):
            self.lcdSOCKPORT.display(sockport)

        # window
        self.window_jobcontrol = JobDialogImpl(self)
        self.window_daq = DaqWindowImpl(self)
        self.make_connections()

        #
        self.fdc=None
        
    def make_connections(self):
         self.pbJOBCONTROL.clicked.connect(self.action_pbJOBCONTROL)
         self.pbDAQ.clicked.connect(self.action_pbDAQ)
         self.lwMONGO.doubleClicked.connect(self.action_lwMONGO)
         self.pbCREATEACCESS.clicked.connect(self.action_pbCREATEACCESS)
         self.pbSHOWCONFIG.clicked.connect(self.action_pbSHOWCONFIG)

         
    def action_pbJOBCONTROL(self):
        if (self.window_jobcontrol.setController(self.fdc)):
            self.window_jobcontrol.action_pbJCINFO()
            self.window_jobcontrol.show()
        else:
           msg = QMessageBox()
           msg.setIcon(QMessageBox.Warning)
           msg.setText("The DAQ access should be created first")
           msg.setWindowTitle("Error")
           msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
           retval = msg.exec_() 

    def action_pbDAQ(self):
        if (self.window_daq.setController(self.fdc)):
            self.window_daq.show()
        else:
           msg = QMessageBox()
           msg.setIcon(QMessageBox.Warning)
           msg.setText("The DAQ access should be created first")
           msg.setWindowTitle("Error")
           msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
           retval = msg.exec_() 
            
    def action_lwMONGO(self):
        #print()
        self.laDAQMONGO.setText(self.lwMONGO.currentItem().text())
        self.config=self.laDAQMONGO.text()

    def action_pbSHOWCONFIG(self):
        #print()
        config=self.lwMONGO.currentItem().text()
        self.db.downloadConfig(config.split(":")[0],int(config.split(":")[1]),True)
        daq_file="/dev/shm/mgjob/"+config.split(":")[0]+"_"+config.split(":")[1]+".json"
        with open(daq_file) as data_file:    
            mgConfig = json.load(data_file)
            tb=TbDialogImpl(self)

            # setting the minimum size 
            tb.tbTEXT.setText(json.dumps(mgConfig, indent=4, sort_keys=True))
            tb.show()
            
    def action_pbCREATEACCESS(self):
        self.fdc = combrc.combRC(self.account, self.config)
        self.fdc.Connect()
        msg = QMessageBox()
        msg.setIcon(QMessageBox.Information)
        msg.setText("DIF acquisition created")
        msg.setWindowTitle("Information")
        msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
        retval = msg.exec_()
app = QtWidgets.QApplication(sys.argv)

window = MainWindow()

window.show()
app.exec()

