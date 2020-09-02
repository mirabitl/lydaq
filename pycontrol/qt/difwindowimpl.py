#!/usr/bin/python3
import json2html 
import MongoJob as mg
import json
import sys
import os
from PyQt5 import QtWidgets, uic





from DifWindow import Ui_DifWindow



class DifWindowImpl(QtWidgets.QMainWindow, Ui_DifWindow):
    def __init__(self, *args, obj=None,controller=None, **kwargs):
        super(DifWindowImpl, self).__init__(*args, **kwargs)
        self.setupUi(self)

        self.ctrg=None
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
        self.pbSET.clicked.connect(self.action_SET)

    def action_SET(self):
        mode=self.setTriggerMode(self.cbPOWERPULSED.isChecked(),self.cbILC.isChecked(),self.cbTEMPERATURE.isChecked(),
                                 self.cbANALOG.isChecked(),self.cbDIGITAL.isChecked())
        self.laMODE.setText("0X%x" % mode)
        self.ctrl.setControlRegister("0X%x" % mode)
        #par={}
        #par["state"]=self.config.split(":")[0]
        #par["version"]=int(self.config.split(":")[1])
        #print(process,par)
        #print(self.ctrl.processCommand("DOWNLOADDB",process,par))
        
    def setTriggerMode(self,powerpulsing=True,ilc=True,temperature=True,analog=False,digital=True):
        """
        //       0x895A0040 2304376896 SDCC, BT, PP et bits 8-12 =0 dans ctrl reg  temperature digital          OK
        //       0x89180000 2300051456 SDCC, ILC PP et bits 8-12 =0 dans ctrl reg                                                                                               OK
        //       0x89580000 2304245760 SDCC ILC PP pwn on lecture digitale                                                                                                                              OK
        //       0x895A0000 2304376832 SDCC ILC PP pwn on lecture digitale      et temperatures                                                         OK
        //       0x81981B40 2174229312 SDCC BT  lecture analogique                                                                                      OK
        //       0x819A1B40 2174360384 SDCC BT  lecture analogique      temperature                                                             OK
        //                       0x81DA1B40 2178554688 SDCC BT  lecture digitale et analogique  temperature                                                     OK
        //                       0x815A1B00 2170166016 SDCC ILC  lecture digitale       et temperatures                                                         
        //                       0x815A1B40 2170166080 SDCC BT  lecture digitale        et temperatures         
        Set the tigger Mode : 
        self explicit        


        """
        #  BT, PP et bits 8-12 =0 dans ctrl reg  temperature digital       
        if ( (not ilc) and powerpulsing and temperature and (not analog) and digital):
            mode= 0x895A0040
        # ILC PP et bits 8-12 =0 dans ctrl reg 
        if ( ilc  and powerpulsing and (not temperature) and (not analog) and digital):
            mode= 0x89580000
        #  ILC PP pwn on lecture digitale  et temperatures
        if ( ilc  and powerpulsing and (temperature) and (not analog) and digital):
            mode= 0x895A0000
        # BT  lecture analogique
        if ( (not ilc)  and (not powerpulsing) and (not temperature) and (analog) and (not digital)):
            mode= 0x81981B40
        # BT  lecture analogique   temperature
        if ( (not ilc)  and (not powerpulsing) and (temperature) and (analog) and (not digital)):
            mode=0x819A1B40
        # BT  lecture digitale et analogique       temperature
        if ( (not ilc)  and (not powerpulsing) and (temperature) and (analog) and (digital)):
            mode=0x81DA1B40
        # ILC  lecture digitale    et temperatures 
        if ( (ilc)  and (not powerpulsing) and (temperature) and (not analog) and (digital)):
            mode=0x815A1B00
        # BT  lecture digitale     et temperatures
        if ( (not ilc)  and (not powerpulsing) and (temperature) and (not analog) and (digital)):
            mode=0x815A1B40
  
        return mode
