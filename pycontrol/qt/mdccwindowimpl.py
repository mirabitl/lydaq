#!/usr/bin/python3
import json2html 
import MongoJob as mg
import json
import sys
import os
from PyQt5 import QtWidgets, uic

from PyQt5.QtWidgets import QMessageBox

from MdccWindow import Ui_MdccWindow


class MdccWindowImpl(QtWidgets.QMainWindow, Ui_MdccWindow):
    def __init__(self, *args, obj=None,controller=None, **kwargs):
        super(MdccWindowImpl, self).__init__(*args, **kwargs)
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
        self.PBMdccSet.clicked.connect(self.action_mdcc_set_registers)
        self.PBMdccResetCounters.clicked.connect(self.action_mdcc_reset_counters)
        self.PBMdccResetTdc.clicked.connect(self.action_mdcc_reset_tdc)
        self.PBMdccPause.clicked.connect(self.action_mdcc_pause)
        self.PBMdccResume.clicked.connect(self.action_mdcc_resume)
        self.PBMdccPhysic.clicked.connect(self.action_mdcc_physic)
        self.PBStatus.clicked.connect(self.action_mdcc_status)


        
    def action_mdcc_set_registers(self):
        self.ctrl.mdcc_setSpillOn(int(self.LESpillOn.text()))
        self.ctrl.mdcc_setSpillOff(int(self.LESpillOff.text()))
        self.ctrl.mdcc_setBeamOn(int(self.LEBeamOn.text()))
        self.ctrl.mdcc_setSpillRegister(int(self.SBWindowMode.value()))
        self.ctrl.mdcc_WriteRegister(32,int(self.SBBusy.value()))
        self.action_mdcc_status()
    def action_mdcc_physic(self):
        self.ctrl.mdcc_setSpillOn(20000)
        self.ctrl.mdcc_setSpillOff(4000000)
        self.ctrl.mdcc_setBeamOn(int(self.LEBeamOn.text()))
        self.ctrl.mdcc_setSpillRegister(64)
        self.ctrl.mdcc_setRegister(32,int(self.SBBusy.value()))
        self.action_mdcc_status()
        
    def action_mdcc_reset_counters(self):
        self.ctrl.mdcc_Reset()
        self.action_mdcc_status()
        
    def action_mdcc_reset_tdc(self):
        self.ctrl.mdcc_resetTdc()
        self.action_mdcc_status()
        
    def action_mdcc_pause(self):
        self.ctrl.mdcc_Pause()
        self.action_mdcc_status()
        
    def action_mdcc_resume(self):
        self.ctrl.mdcc_Resume()
        self.action_mdcc_status()

    def action_mdcc_status(self):
        srep=self.ctrl.mdcc_Status()
        if (type(srep) is bytes):
            srep=srep.decode("utf-8")
        s="<h1>Status</h1>"
        s=s+"<P> <b>Parameters:</b>"
        s=s+json2html.json2html.convert(json=srep)
        s=s+"</P></br>"
        self.tbSTATUS.setText(s)

