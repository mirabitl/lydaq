#!/usr/bin/python3
import json2html 
import MongoJob as mg
import json
import sys
import os
from PyQt5 import QtWidgets, uic

from PyQt5.QtWidgets import QMessageBox, QPushButton,QVBoxLayout

from JobDialog import Ui_JobDialog

from tbdialogimpl import TbDialogImpl
from logwindowimpl import LogWindowImpl

class color:
   PURPLE = '\033[95m'
   CYAN = '\033[96m'
   DARKCYAN = '\033[36m'
   BLUE = '\033[94m'
   GREEN = '\033[92m'
   YELLOW = '\033[93m'
   RED = '\033[91m'
   BOLD = '\033[1m'
   UNDERLINE = '\033[4m'
   END = '\033[0m'

class JobDialogImpl(QtWidgets.QMainWindow, Ui_JobDialog):
    def __init__(self, *args, obj=None,controller=None, **kwargs):
        super(JobDialogImpl, self).__init__(*args, **kwargs)
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
        self.laCMD.setText(self.ctrl.jc.state)
        return True

    def make_connections(self):
        self.pbJCINFO.clicked.connect(self.action_pbJCINFO)
        self.pbJCCREATE.clicked.connect(self.action_pbJCCREATE)
        self.pbJCDESTROY.clicked.connect(self.action_pbJCDESTROY)
        self.pbJCKILL.clicked.connect(self.action_pbJCKILL)
        self.pbJCSTART.clicked.connect(self.action_pbJCSTART)
        self.pbJCAPPCREATE.clicked.connect(self.action_pbJCAPPCREATE)
        self.pbJOBLOGS.clicked.connect(self.action_pbJOBLOGS)


    def action_pbJCKILL(self):
       srep=False
       try:
          srep=self.ctrl.jc.kill()
       except:
          print("Unexpected error:", sys.exc_info()[0])

       self.laCMD.setText("KILL done")
       self.laCMD.setText(self.ctrl.jc.state)
       self.action_pbJCINFO()

    def action_pbJCSTART(self):

       srep=False
       try:
          srep=self.ctrl.jc.start()
       except:
          print("Unexpected error:", sys.exc_info()[0])

       self.laCMD.setText("START done")
       self.laCMD.setText(self.ctrl.jc.state)
       self.action_pbJCINFO()
        
    def action_pbJCAPPCREATE(self):
       srep=False
       try:
          srep=self.ctrl.jc.configure()
       except:
          print("Unexpected error:", sys.exc_info()[0])


       self.laCMD.setText("APPCREATE done")
       self.laCMD.setText(self.ctrl.jc.state)
       self.action_pbJCINFO()
        
    def action_pbJCCREATE(self):
       srep=False
       try:
          srep=self.ctrl.jc.initialise()
       except:
          print("Unexpected error:", sys.exc_info()[0])


       self.laCMD.setText("CREATE done")
       self.laCMD.setText(self.ctrl.jc.state)
       self.action_pbJCINFO()

    def action_pbJCDESTROY(self):
        srep=False
        try:
           srep=self.ctrl.jc.initialise()
        except:
           print("Unexpected error:", sys.exc_info()[0])

        self.laCMD.setText("DESTROY done")
        self.laCMD.setText(self.ctrl.jc.state)
        self.action_pbJCINFO()

    def action_pbJOBLOGS(self):
        swlog=LogWindowImpl(self)
        srep=self.ctrl.getAllInfos()
        jsrep=json.loads(srep)
        lay=QVBoxLayout()
        bhsize=0
        bvsize=0
        for x in jsrep:
            xi=x["infos"]
            xpr=x["process"]
            if (xpr["STATE"]=="DEAD"):
                continue;
            sbname="%s %d PID %d" % (xi["name"],xi["instance"],int(xpr["PID"]))
            sb=QPushButton(sbname)
            sb.setMinimumSize(100,50)
            bvsize=bvsize+50
            sb.clicked.connect(lambda z,x=xi["host"],y=xpr["PID"]: self.showLog(x,y))
            lay.addWidget(sb)
        lay.addStretch(1)
        swlog.gbPROC.setLayout(lay)
        
        swlog.gbPROC.resize(100,bvsize);
        #swlog.resize();
        swlog.show()
        
    def showLog(self,host,pid):
        print("showlog ", host,pid)
        srep=self.ctrl.getLog(host,pid)
        jrep=json.loads(srep)
        tb=TbDialogImpl(self)

        # setting the minimum size
        tb.setWindowTitle(jrep["answer"]["FILE"])
        tb.tbTEXT.setText(jrep["answer"]["LINES"])
        tb.show()
    def action_pbJCINFO(self):
        self.laCMD.setText(self.ctrl.jc.state)       
        srep=self.ctrl.getAllInfos()

        jsrep=json.loads(srep)
        print(jsrep)
        self.tbJCINFO.setOpenLinks(True)
        self.tbJCINFO.setOpenExternalLinks(True)


        s="<h1> Processus Status </h1>"

        #labelText .append("</font></i></b></P></br>");
        for x in jsrep:
            #print(x)
            xi=x["infos"]
            xpr=x["process"]
            if (xpr["STATE"]=="DEAD"):
                continue;
            print(xi)
            print(xpr)
            s=s+"<P><b><i><font color='blue' font_size=4>"
            s=s+'<a href="http://%s:%d">http://%s:%d</a>  %s %d %d %s  \n ' % (xi["host"],int(xi["port"]),xi["host"],int(xi["port"]),xi["name"],xi["instance"],int(xpr["PID"]),xpr["STATE"])
            s=s+"</font></i></b></P></br>";
            if (self.cbVERBOSE.isChecked()):
                if (xpr["STATE"] =="DEAD"):
                    continue
                s=s+"<P> <b>ALLOWED Transitions:</b></br> "
                for z in xpr["ALLOWED"]:
                    s=s+" %s" % z["name"]
                s=s+"</P></br>"
                s=s+"<P> <b>Available Commands:</b></br> "
                for z in xpr["CMD"]:
                    s=s+" %s" % z["name"]
                s=s+"</P></br>"
            if (self.cbPROCESSONLY.isChecked()):
                continue
            spar=json.dumps(x["params"])
            s=s+"<P> <b>Parameters:</b>"
            s=s+json2html.json2html.convert(json=spar)
            s=s+"</P></br>"
        self.tbJCINFO.setText(s)
