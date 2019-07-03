#!/usr/bin/env python
import logging
from spyne import Application, srpc, ServiceBase, Iterable, UnsignedInteger,String,Float,Unicode
from spyne.protocol.json import JsonDocument
from spyne.protocol.xml import XmlDocument
from spyne.protocol.csv import Csv
from spyne.protocol.http import HttpRpc
from spyne.server.wsgi import WsgiApplication
from spyne.model.complex import ComplexModelMeta
import MongoRoc as mg
import os,sys,json

import time
def jdefault(o):
    return o.__dict__
_wdd=None

class wddService(ServiceBase):
    @srpc(String,UnsignedInteger,String,String,String, _returns=Iterable(String))
    def createAccess(host,port,dbname,username,passwd):    
        global _wdd
        _wdd=mg.MongoRoc(host,port,dbname,username,passwd)
        yield 'MongoRoc is created',host,port,dbname,username,passwd

    @srpc(String,UnsignedInteger, _returns=Iterable(String))
    def download(state,version):
        global _wdd
        s=_wdd.download(state,version);
        yield json.dumps(s)

    @srpc( _returns=Iterable(String))
    def startHost():
        global _wdd
        _wdd.host_start();
        yield 'Hosts are started'

    @srpc( _returns=Iterable(String))
    def stopRPI():
        global _wdd
        _wdd.rpi_stop();
        yield 'RPIs are stopped'

    @srpc( _returns=Iterable(String))
    def startRPI():
        global _wdd
        _wdd.rpi_start();
        yield 'RPI are started'

    @srpc( _returns=Iterable(String))
    def Discover():
        global _wdd
        _wdd.Discover();
        yield 'DIM DNS is browsed'

    @srpc( _returns=Iterable(String))
    def DiscoverDNS():
        global _wdd
        _wdd.DiscoverDNS();
        yield 'DIM DNS is browsed'

    @srpc( _returns=Iterable(String))
    def DownloadDB():
        global _wdd
        _wdd.DownloadDB();
        yield 'Oracle DB state %s is download' % (_wdd.state)

    @srpc( _returns=Iterable(String))
    def initialiseWriter():
        global _wdd
        _wdd.initialiseWriter();
        yield 'Data writer is open'

    @srpc( _returns=Iterable(String))
    def initialiseZup():
        global _wdd
        _wdd.initialiseZup();
        yield 'ZUP access is open'

    @srpc( String,_returns=Iterable(String))
    def ChangeState(state):
        global _wdd
        _wdd.ChangeState(state);
        yield 'Oracle DB state is set to %s' % (state)

    @srpc( UnsignedInteger,_returns=Iterable(String))
    def ChangeRegister(register):
        global _wdd
        _wdd.ChangeRegister(register);
        yield 'Trigger register is set to 0x%x' % (register)


    @srpc( _returns=Iterable(String))
    def scan():
       global _wdd
       _wdd.scan()
       yield 'SDHCAL is DIF scanned'

    @srpc( _returns=Iterable(String))
    def Print():
       global _wdd
       yield _wdd.Print()


    @srpc( _returns=Iterable(String))
    def Initialise():
       global _wdd
       _wdd.Initialise()
       yield 'SDHCAL is Initialised'

    @srpc( _returns=Iterable(String))
    def Configure():
       global _wdd
       _wdd.Configure()
       yield 'SDHCAL is configured'

    @srpc( _returns=Iterable(String))
    def Start():
       global _wdd
       _wdd.Start()
       yield 'Run is Started'


    @srpc( _returns=Iterable(String))
    def Stop():
       global _wdd
       _wdd.Stop()
       yield 'Run is stopped'


    @srpc( _returns=Iterable(String))
    def Destroy():
       global _wdd
       _wdd.Destroy()
       yield 'Daq is Destroyed'

    @srpc( _returns=Iterable(String))
    def Halt():
       global _wdd
       _wdd.Halt()
       yield 'Daq is Halted'


    @srpc( _returns=Iterable(String))
    def LVOff():
       global _wdd
       _wdd.LVOff()
       yield 'Lv is Off'


    @srpc( _returns=Iterable(String))
    def LVOn():
       global _wdd
       _wdd.LVOn()
       yield 'Lv is On'


    @srpc( _returns=Iterable(String))
    def zoff():
       global _wdd
       _wdd.zoff()
       yield 'Lv is Off'


    @srpc( _returns=Iterable(String))
    def zon():
       global _wdd
       _wdd.zon()
       yield 'Lv is On'


    @srpc( _returns=Iterable(String))
    def HVOff():
       global _wdd
       _wdd.HVOff()
       yield 'all HV channels Of'

    @srpc( _returns=Iterable(String))
    def HVOn():
       global _wdd
       _wdd.HVOn()
       yield 'all HV channels On'

    @srpc( _returns=Iterable(String))
    def DifStatus():
       global _wdd
       #_wdd.daq_.getDifInfo()
       v=[]
       for i in range(0,100):
           v.append([i,i,10*i,10000*i,100*i,1000*i,"lyosdhcal12","Dummy"])
#       for i in range(0,_wdd.daq_.seenNumberOfDif()):
#           d=DifinfoModel(ID=_wdd.daq_.seenId(i),SLC=_wdd.daq_.seenSlc(i),GTC= _wdd.daq_.seenGtc(i),
#                        BCID= _wdd.daq_.seenBcid(i),READBYTES=_wdd.daq_.seenBytes(i),HOST=_wdd.daq_.seenHost(i),
#                        STATE=_wdd.daq_.seenState(i))
#           v.append(d)
       for l in v:
           yield l 
#       yield v
    @srpc(Float,UnsignedInteger,UnsignedInteger, _returns=Iterable(String))
    def HVSetVoltage(voltage,firstchannel,lastchannel):
       global _wdd
       _wdd.HVSetVoltage(voltage,firstchannel,lastchannel)
       yield 'Channel %d to %d are set to %f ' % (firstchannel,lastchannel,voltage)

    @srpc(UnsignedInteger,UnsignedInteger, _returns=Iterable(String))
    def HVSwitchOn(firstchannel,lastchannel):
       global _wdd
       _wdd.HVSwicthOn(firstchannel,lastchannel)
       yield 'Channel %d to %d are switch On ' % (firstchannel,lastchannel)

    @srpc(UnsignedInteger,UnsignedInteger, _returns=Iterable(String))
    def HVSwitchOff(firstchannel,lastchannel):
       global _wdd
       _wdd.HVSwitchOff(firstchannel,lastchannel)
       yield 'Channel %d to %d are switch Off ' % (firstchannel,lastchannel)

    @srpc(UnsignedInteger,UnsignedInteger,  _returns=Iterable(String))
    def HVDump(firstchannel,lastchannel):
       global _wdd
       yield _wdd.HVDump(firstchannel,lastchannel)

    @srpc(UnsignedInteger,UnsignedInteger,   _returns=Iterable(String))
    def HVGet(firstchannel,lastchannel):
       global _wdd
       yield _wdd.HVGet(firstchannel,lastchannel)
 

  
if __name__=='__main__':
    # Python daemon boilerplate
    from wsgiref.simple_server import make_server
    logging.basicConfig(level=logging.DEBUG)

    # Instantiate the application by giving it:
    #   * The list of services it should wrap,
    #   * A namespace string.
    #   * An input protocol.
    #   * An output protocol.
    application = Application([wddService], 'spyne.examples.hello.http',
          # The input protocol is set as HttpRpc to make our service easy to
          # call. Input validation via the 'soft' engine is enabled. (which is
          # actually the the only validation method for HttpRpc.)
          in_protocol=HttpRpc(validator='soft'),

          # The ignore_wrappers parameter to JsonDocument simplifies the reponse
          # dict by skipping outer response structures that are redundant when
          # the client knows what object to expect.
          #out_protocol=XmlDocument()
          #out_protocol=YamlDocument(),
          out_protocol=JsonDocument(ignore_wrappers=False),
      )

    # Now that we have our application, we must wrap it inside a transport.
    # In this case, we use Spyne's standard Wsgi wrapper. Spyne supports 
    # popular Http wrappers like Twisted, Django, Pyramid, etc. as well as
    # a ZeroMQ (REQ/REP) wrapper.
    wsgi_application = WsgiApplication(application)

    # More daemon boilerplate
    server = make_server('134.158.137.101', 8100, wsgi_application)

    logging.info("listening to http://lyosdhcal15:8100")
    logging.info("wsdl is at: http://lyosdhcal15:8100/?wsdl")

    server.serve_forever()
