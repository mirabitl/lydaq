#!/usr/bin/python
 
from PyQt4 import QtGui, QtCore
import sys
from ROOT import *
import sdhcalui
import daqcontrol
import json
from threading import Thread, RLock
import time
import os

import xml.etree.ElementTree as ET
from xml.etree.ElementTree import parse,fromstring, tostring

        
class AllStatus(QtCore.QThread):
    def __init__(self,daq):
        QtCore.QThread.__init__(self)
        self.daqui =daq
        self.state="UNKNOWN"
        self.stop = True
    def run(self):
        while self.stop == False:
            if (self.daqui.daq.daq_state() == "RUNNING"):
                self.emit(QtCore.SIGNAL("updateStatus()"))

            if (self.stop):
                break
            time.sleep(30)


class FdaqDialog(QtGui.QDialog, sdhcalui.Ui_Dialog):
    def __init__(self, parent=None):
        super(FdaqDialog, self).__init__(parent)
        self.setupUi(self)
        self.config=None
        self.login=None
        self.state="UNKNOWN"
        self.connectActions()
        self.daq=daqcontrol.fdaqClient()
        self.daq.curvth=0
        self.allstatus=AllStatus(self)
        self.LVHisto.setWindowTitle('Honey-Do List')
        self.LVHisto.setMinimumSize(600, 400)
 
        
        self.connect(self.allstatus, QtCore.SIGNAL("updateStatus()"), self.updateStatus)

        # Create an empty model for the list's data
        self.LVHmodel = QtGui.QStandardItemModel(self.LVHisto)
        self.LVHisto.clicked.connect(self.on_LVHmodel_changed)
        
    def connectActions(self):
        self.buttonBox.accepted.connect(exit)
        self.buttonBox.rejected.connect(exit)
        self.tabWidget.currentChanged.connect(self.onChange)
        # Configuration
        self.PBConfiguration.clicked.connect(self.action_PBConfiguration)
        # Job Control
        self.JCCreate.clicked.connect(self.action_JCCreate)
        self.JCStart.clicked.connect(self.action_JCStart)
        self.JCKill.clicked.connect(self.action_JCKill)
        self.JCDestroy.clicked.connect(self.action_JCDestroy)
        self.JCStatus.clicked.connect(self.action_JCStatus)
        # DAQ
        self.PBCreate.clicked.connect(self.action_daq_create)
        self.PBDiscover.clicked.connect(self.action_daq_discover)
        self.PBService.clicked.connect(self.action_daq_service)
        self.PBStartup.clicked.connect(self.action_daq_startup)
        self.PBInitialise.clicked.connect(self.action_daq_initialise)
        self.PBConfigure.clicked.connect(self.action_daq_configure)
        self.PBStart.clicked.connect(self.action_daq_start)
        self.PBStop.clicked.connect(self.action_daq_stop)
        self.PBDestroy.clicked.connect(self.action_daq_destroy)
        self.PBDaqStatus.clicked.connect(self.action_daq_status)
        #DB 
        self.PBDownload.clicked.connect(self.action_daq_download)
        # MDCC
        self.PBMdccSet.clicked.connect(self.action_mdcc_set_registers)
        self.PBMdccResetCounters.clicked.connect(self.action_mdcc_reset_counters)
        self.PBMdccResetTdc.clicked.connect(self.action_mdcc_reset_tdc)
        self.PBMdccPause.clicked.connect(self.action_mdcc_pause)
        self.PBMdccResume.clicked.connect(self.action_mdcc_resume)
        self.PBMdccPhysic.clicked.connect(self.action_mdcc_physic)

        # Status
        self.PBEvbStatus.clicked.connect(self.action_evb_status)
        self.PBDbStatus.clicked.connect(self.action_db_status)
        self.PBDifStatus.clicked.connect(self.action_dif_status)

        # Analysis
        self.PBMonitor.clicked.connect(self.action_monitor_start)
        self.PBProcStart.clicked.connect(self.action_proc_start)
        self.PBProcStop.clicked.connect(self.action_proc_stop)
        self.PBProcStatus.clicked.connect(self.action_proc_status)
        self.PBHistoList.clicked.connect(self.action_proc_histolist)

    
        
    def onChange(self,i):
        if (self.login == None):
            QtGui.QMessageBox.information(self,
                                          "Alert",
                                          "Configuration unknown \n Please set it first"  )
            self.tabWidget.setCurrentIndex(0)
            return
        self.state= self.daq.daq_state()
        if (self.state == "NO CONNECTION"):
            QtGui.QMessageBox.information(self,
                                          "Alert",
                                          "No FDAQ process responding/started \n start it first"  )
            self.tabWidget.setCurrentIndex(0)
            return
        if (i == 0):
            self.action_JCStatus()
        if (i == 1):
            self.action_daq_status()
        if ( i == 3):
            self.action_trig_status()
        if ( i == 2):
            self.updateStatus()
            
    def action_PBConfiguration(self):
        self.config="https://ilcconfdb.ipnl.in2p3.fr/config-content/"+str(self.LEConfiguration.text())
        self.login=str(self.LELogin.text())
        print self.config,self.login
        self.daq.loadConfig(self.config,self.login)
        self.canvas=TCanvas()
    def action_JCCreate(self):
        r=self.daq.jc_create()
        print r
        self.plainTextEdit.document().setPlainText(r)
    def action_JCStart(self):
        r=self.daq.jc_start()
        self.plainTextEdit.document().setPlainText(r)
    def action_JCKill(self):
        r=self.daq.jc_kill()
        self.plainTextEdit.document().setPlainText(r)
    def action_JCDestroy(self):
        r=self.daq.jc_destroy()
        self.plainTextEdit.document().setPlainText(r)
    def action_JCStatus(self):
        r=self.daq.jc_status()
        self.plainTextEdit.document().setPlainText(r)
    def action_daq_create(self):
        self.daq.daq_create()
        r= self.daq.daq_list()
        self.PTEDaq.document().setPlainText(r)
        l= self.daq.daq_state()
        self.state=l
        self.LADAQState.setText(l)
    def action_daq_discover(self):
        self.daq.daq_discover()
        r= self.daq.daq_list()
        self.PTEDaq.document().setPlainText(r)
        l= self.daq.daq_state()
        self.state=l
        self.LADAQState.setText(l)
    def action_daq_status(self):
        r= self.daq.daq_list()
        self.PTEDaq.document().setPlainText(r)
        l= self.daq.daq_state()
        self.state=l
        self.LADAQState.setText(l)    
    def action_daq_service(self):
        r1=self.daq.daq_services()
        sr=json.loads(r1)
        
        print sr
        self.LADAQStatus.setText(sr["status"])
        
        r= self.daq.daq_list()
        self.PTEDaq.document().setPlainText(r)
        l= self.daq.daq_state()
        self.state=l
        self.LADAQState.setText(l)
    def action_daq_startup(self):
        self.action_daq_create()
        self.action_daq_discover()
        self.action_daq_service()
    def action_daq_initialise(self):
        #self.daq.daq_resettdc()
        time.sleep(2)
        r1=self.daq.daq_initialise()
       
        srp=json.loads(r1)
        print srp
        print srp["status"]
        self.LADAQStatus.setText(srp["status"])
        r= self.daq.daq_list()
        self.PTEDaq.document().setPlainText(r)
        l= self.daq.daq_state()
        self.state=l
        self.LADAQState.setText(l)
        if (self.allstatus.stop):
            self.allstatus.stop=False
            self.allstatus.start()

    def action_daq_configure(self):
        r1=self.daq.daq_configure()
        sr=json.loads(r1)
        self.LADAQStatus.setText(sr["status"])


        if (self.CBTrigExt.isChecked()):
            self.daq.daq_settdcmode(1)
        else:
            self.daq.daq_settdcmode(0)
        self.daq.daq_settdcdelays(self.SBActive.value(),self.SBDead.value())
        r= self.daq.daq_list()
        self.PTEDaq.document().setPlainText(r)
        l= self.daq.daq_state()
        self.state=l
        self.LADAQState.setText(l)
        if (self.allstatus.stop):
            self.allstatus.stop=False
            self.allstatus.start()
    
    def action_daq_start(self):
        r1=self.daq.daq_start()
        sr=json.loads(r1)
        self.LADAQStatus.setText(sr["status"])
        print sr
        r= self.daq.daq_list()
        self.PTEDaq.document().setPlainText(r)
        l= self.daq.daq_state()
        self.state=l
        self.LADAQState.setText(l)
        if (self.allstatus.stop):
            self.allstatus.stop=False
            self.allstatus.start()

    def action_daq_stop(self):

        r1=self.daq.daq_stop()
        sr=json.loads(r1)
        self.LADAQStatus.setText(sr["status"])
        print sr
        r= self.daq.daq_list()
        self.PTEDaq.document().setPlainText(r)
        l= self.daq.daq_state()
        self.state=l
        self.LADAQState.setText(l)
    def action_daq_destroy(self):
        r1=self.daq.daq_destroy()
        print r1
        sr=json.loads(r1)
        self.LADAQStatus.setText(sr["status"])
        print sr
        r= self.daq.daq_list()
        self.PTEDaq.document().setPlainText(r)
        l= self.daq.daq_state()
        self.state=l
        self.LADAQState.setText(l)
    def action_daq_download(self):
        r1=self.daq.daq_downloaddb(str(self.LEDBState.text()))
        print r1
        sr=json.loads(r1)
        self.LADAQStatus.setText(sr["status"])
        print sr
        r= self.daq.daq_list()
        self.PTEDaq.document().setPlainText(r)
        l= self.daq.daq_state()
        self.state=l
        self.LADAQState.setText(l)

    def action_trig_status(self):
        r1=self.daq.trig_status()
        print r1
        self.TETrigStatus.document().setPlainText(r1)

        sr=json.loads(r1)
        if (sr["answer"]["STATUS"]=="DONE"):
            self.TEMdcc.document().setPlainText(json.dumps(sr["answer"]["COUNTERS"]))
            self.SBWindowMode.setValue(sr["answer"]["COUNTERS"]["spillreg"])
            self.LESpillOn.setText("%d" % sr["answer"]["COUNTERS"]["spillon"])
            self.LESpillOff.setText("%d" % sr["answer"]["COUNTERS"]["spilloff"])
            self.LEBeamOn.setText("%d" % sr["answer"]["COUNTERS"]["beam"])
        else:
            self.TEMdcc.document().setPlainText(sr["answer"]["STATUS"])
    def action_evb_status(self):
        r1=self.daq.daq_evbstatus()
        print r1
        self.TEEvbStatus.document().setPlainText(r1+"\n SCurve %d \n" % self.daq.curvth)
        sr=json.loads(r1)
        self.LARun.setText("%d %d " % (sr["answer"]["run"],sr["answer"]["event"])) 
    def action_db_status(self):
        r1=self.daq.daq_dbstatus()
        print r1
        self.TEDbStatus.document().setPlainText(r1)

    def action_dif_status(self):
        r1=self.daq.daq_status()
        print r1
        self.TEDifStatus.document().setPlainText(r1)
        
    def updateStatus(self):
        self.action_trig_status()
        self.action_evb_status()
        self.action_db_status()
        self.action_dif_status()
        
    def action_mdcc_set_registers(self):
        self.daq.trig_spillon(int(self.LESpillOn.text()))
        self.daq.trig_spilloff(int(self.LESpillOff.text()))
        self.daq.trig_beam(int(self.LEBeamOn.text()))
        self.daq.trig_spillregister(int(self.SBWindowMode.value()))
        self.daq.trig_setregister(32,int(self.SBBusy.value()))
        self.action_trig_status()
    def action_mdcc_physic(self):
        self.daq.trig_spillon(20000)
        self.daq.trig_spilloff(4000000)
        self.daq.trig_beam(int(self.LEBeamOn.text()))
        self.daq.trig_spillregister(64)
        self.daq.trig_setregister(32,int(self.SBBusy.value()))
        self.action_trig_status()
        
    def action_mdcc_reset_counters(self):
        self.daq.trig_reset()
        self.action_trig_status()
        
    def action_mdcc_reset_tdc(self):
        self.daq.trig_tdcreset()
        self.action_trig_status()
        
    def action_mdcc_pause(self):
        self.daq.trig_pause()
        self.action_trig_status()
        
    def action_mdcc_resume(self):
        self.daq.trig_resume()
        self.action_trig_status()

    def action_proc_start(self):
        r=self.daq.analysis_process(self.SBRun.value())
        print r
    def action_monitor_start(self):
        r=self.daq.analysis_monitor(self.SBRun.value())
        print r
    
    def action_proc_stop(self):
        r=self.daq.analysis_stop(self.SBRun.value())
        print r
    def action_proc_status(self):
        r=self.daq.analysis_status()
        sr=json.loads(r)
        print sr
        self.LAProcessRun.setText("%d" % sr["answer"]["run"])
        self.LAProcessEvent.setText("%d" % sr["answer"]["event"])
    def action_proc_histolist(self):
        r=self.daq.analysis_histolist()
        sr=json.loads(r)
        print sr["answer"]["list"]
        tree = ET.XML(sr["answer"]["list"])
        apps=tree.findall("Histo")
        flat =[]
        self.LVHmodel.clear()
        for i in range(len(apps)):
            #print apps[i].text
            #flat.append(apps[i].text)
            item = QtGui.QStandardItem(apps[i].text)
 
            # Add a checkbox to it
            item.setCheckable(False)
            item.setSelectable(True)
            item.setEnabled(True)
 
            # Add the item to the model
            self.LVHmodel.appendRow(item)
        #print flat
        self.LVHisto.setModel(self.LVHmodel)

    def on_LVHmodel_changed(self,index):
        # If the changed item is not checked, don't bother checking others
        print index.row(),self.LVHmodel.item(index.row()).text()
        r=self.daq.analysis_histo(self.LVHmodel.item(index.row()).text())
        sr=json.loads(r)
        #print sr["answer"]["histo"].replace('\\n','').replace('\"','"')
        h=TBufferJSON.ConvertFromJSON(sr["answer"]["histo"].replace('\\n','').replace('\"','"'))
        #tree = ET.XML(sr["answer"]["histo"])
        #h=TBufferXML.ConvertFromXML(ET.tostring(tree))
        #print ET.tostring(tree)
        self.canvas.cd()
        
        h.Draw()
        h.Print()
        #self.canvas.Draw()
        self.canvas.Modified()
        self.canvas.Update()
        
    def main(self):
        #gApplication.Run(0)
        gROOT.SetBatch(0)
        self.show()
 
if __name__=='__main__':
 
    app = QtGui.QApplication(sys.argv)
    imageViewer = FdaqDialog()
    imageViewer.main()
    #
    app.exec_()
    exit(0)
