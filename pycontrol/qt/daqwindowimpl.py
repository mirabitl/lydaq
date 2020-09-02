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
from dbwindowimpl import DbWindowImpl
from difwindowimpl import DifWindowImpl
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
        self.laCMD.setText(self.ctrl.state)

        return True

    def make_connections(self):
        self.pbMDCC.clicked.connect(self.action_pbMDCC)
        self.pbFEB.clicked.connect(self.action_pbFEB)
        self.pbDB.clicked.connect(self.action_pbDB)
        self.pbDIF.clicked.connect(self.action_pbDIF)
        
        self.pbGENESYS.clicked.connect(self.action_pbGENESYS)
        self.pbINITIALISE.clicked.connect(self.action_pbINITIALISE)
        self.pbCONFIGURE.clicked.connect(self.action_pbCONFIGURE)
        self.pbSTART.clicked.connect(self.action_pbSTART)
        self.pbSTOP.clicked.connect(self.action_pbSTOP)
        self.pbDESTROY.clicked.connect(self.action_pbDESTROY)
        self.pbUpdate.clicked.connect(self.action_pbUpdate)


    def action_pbMDCC(self):
        if ( not "MDCCSERVER" in self.ctrl.appMap ):
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Critical)
            msg.setText("No MDCCSERVER application")
            msg.setWindowTitle("Error")
            msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
            retval = msg.exec_()
        else:
            self.window_mdccc=MdccWindowImpl(self,controller=self.ctrl)
            self.window_mdccc.show()

    def action_pbFEB(self):
        if ( not "TDCSERVER" in self.ctrl.appMap ):
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Critical)
            msg.setText("No TDCSERVER application")
            msg.setWindowTitle("Error")
            msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
            retval = msg.exec_()
        else:
            self.window_feb=FebWindowImpl(self,controller=self.ctrl)
            self.window_feb.show()
            
    def action_pbDB(self):
        self.window_db=DbWindowImpl(self,controller=self.ctrl)
        self.window_db.show()

    def action_pbDIF(self):
        self.window_dif=DifWindowImpl(self,controller=self.ctrl)
        self.window_dif.show()
        
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
        srep=False
        try:
            srep=self.ctrl.initialise()
        except:
            print("Unexpected error:", sys.exc_info()[0])
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Warning)
            msg.setText("Cannot initialise (FSM error)")
            msg.setWindowTitle("Error")
            msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
            retval = msg.exec_() 
  

        if (not srep):
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Warning)
            msg.setText("Cannot make tranistion Process are not configured")
            msg.setWindowTitle("Error")
            msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
            retval = msg.exec_() 
  
        self.laCMD.setText(self.ctrl.state)
        
    def action_pbCONFIGURE(self):
        srep=False
        try:
            srep=self.ctrl.configure()
        except:
            print("Unexpected error:", sys.exc_info()[0])
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Warning)
            msg.setText("Cannot configure (FSM error)")
            msg.setWindowTitle("Error")
            msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
            retval = msg.exec_() 
  

        if (not srep):
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Warning)
            msg.setText("Cannot make tranistion Process are not configured")
            msg.setWindowTitle("Error")
            msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
            retval = msg.exec_() 
  
        self.laCMD.setText(self.ctrl.state)
        
    def action_pbDESTROY(self):
        srep=False
        try:
            srep=self.ctrl.destroy()
        except:
            print("Unexpected error:", sys.exc_info()[0])
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Warning)
            msg.setText("Cannot destroy (FSM error)")
            msg.setWindowTitle("Error")
            msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
            retval = msg.exec_() 
  

        if (not srep):
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Warning)
            msg.setText("Cannot make tranistion Process are not configured")
            msg.setWindowTitle("Error")
            msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
            retval = msg.exec_() 
  
        self.laCMD.setText(self.ctrl.state)
        
    def action_pbSTART(self):
        srep=False
        self.ctrl.comment=self.leCOMMENT.text()
        try:
            srep=self.ctrl.start()
        except:
            print("Unexpected error:", sys.exc_info()[0])
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Warning)
            msg.setText("Cannot start (FSM error)")
            msg.setWindowTitle("Error")
            msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
            retval = msg.exec_() 
  

        if (not srep):
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Warning)
            msg.setText("Cannot make tranistion Process are not configured")
            msg.setWindowTitle("Error")
            msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
            retval = msg.exec_() 
  
        self.laCMD.setText(self.ctrl.state)

    def action_pbSTOP(self):
        srep=False
        try:
            srep=self.ctrl.stop()
        except:
            print("Unexpected error:", sys.exc_info()[0])
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Warning)
            msg.setText("Cannot stop (FSM error)")
            msg.setWindowTitle("Error")
            msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
            retval = msg.exec_() 
  

        if (not srep):
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Warning)
            msg.setText("Cannot make tranistion Process are not configured")
            msg.setWindowTitle("Error")
            msg.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
            retval = msg.exec_() 
  
        self.laCMD.setText(self.ctrl.state)
        
    def action_pbUpdate(self):
        srep=self.ctrl.SourceStatus()
        jsrep=json.loads(srep)
        print(jsrep)
        
        s="<h1> Data Sources Status </h1>"
        s=s+json2html.json2html.convert(json=srep)
        srep=self.ctrl.BuilderStatus()
        jsrep=json.loads(srep)
        print(jsrep)
        run=0
        evt=0
        for x,y in jsrep.items():
            print("ITEM ",x," VAL",y)
            run=y["run"]
            evt=y["event"]
            break
        self.lcdRUN.display(run)
        self.lcdEvent.display(evt)
        
        
        s=s+"<h1> Builder Status </h1>"
        s=s+json2html.json2html.convert(json=srep)
        srep=self.ctrl.TriggerStatus()
        jsrep=json.loads(srep)
        print(jsrep)
        s=s+"<h1> Trigger Status </h1>"
        s=s+json2html.json2html.convert(json=srep)
        s=s+"</P></br>"
        self.tbSTATUS.setText(s)

        
