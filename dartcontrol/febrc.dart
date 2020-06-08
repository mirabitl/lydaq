import 'daqcontrol.dart';
//import 'rcbase.dart';
import 'dart:convert';

class febRC extends daqControl {
  /// Constructor
  febRC(String account, String config) : super(account, config) {}

  /// Connection
  Future<void> Connect() async {
    await this.parseMongo();
    await this.discover();
  }

  /// Commands
  ///
  ///  Builder Status
  ///
  Future<String> BuilderStatus() async {
    Map rep = new Map();
    for (var k in appMap.entries) {
      if (k.key != "BUILDER") continue;
      for (var s in k.value) {
        Map r = new Map();
        r["run"] = -1;
        r["event"] = -1;
        r["url"] = s.host_name;

        var mr = json.decode(await s.sendCommand("STATUS", new Map()));

        if (mr['status'] != "FAILED") {
          r["run"] = mr["answer"]["answer"]["run"];
          r["event"] = mr["answer"]["answer"]["event"];
          r["builder"] = mr["answer"]["answer"]["difs"];
          r["built"] = mr["answer"]["answer"]["build"];
          r["total"] = mr["answer"]["answer"]["total"];
          r["compressed"] = mr["answer"]["answer"]["compressed"];
          r["time"] = DateTime.now().millisecondsSinceEpoch / 1000;
        }
        rep["${s.host_name}_${s.infos['instance']}"] = r;
      }
    }
    return json.encode(rep);
  }

  /// TDCSERVER Status
  Future<String> SourceStatus(String name) async {
    Map rep = new Map();
    for (var k in appMap.entries) {
      if (k.key != name) continue;
      for (var s in k.value) {
        var mr = json.decode(await s.sendCommand("STATUS", new Map()));
        //print(mr);
        if (mr['status'] == "FAILED") {
          rep["${s.host_name}_${s.infos['instance']}"] = mr;
        } else
          rep["${s.host_name}_${s.infos['instance']}"] =
              mr["answer"]["TDCSTATUS"];
      }
    }
    return json.encode(rep);
  }

  /// Trigger Status
  Future<String> TriggerStatus(String name) async {
    Map rep = new Map();
    for (var k in appMap.entries) {
      if (k.key != name) continue;
      for (var s in k.value) {
        var mr = json.decode(await s.sendCommand("STATUS", new Map()));
        //print(mr);
        if (mr['status'] == "FAILED") {
          rep["${s.host_name}_${s.infos['instance']}"] = mr;
        } else
          rep["${s.host_name}_${s.infos['instance']}"] = mr["answer"];
      }
    }
    return json.encode(rep);
  }

  /// FEB specific
  /// Set6BDac
  ///
  Future<String> set6BDac(int dac) async {
    Map param = new Map();
    param["value"] = dac;
    return await processCommand("SET6BDAC", "TDCSERVER", param);
  }

  ///cal6BDac
  ///
  Future<String> cal6BDac(String mask, int shift) async {
    Map param = new Map();
    param["shift"] = shift;
    final number = int.parse(mask, radix: 16);
    param["mask"] = number;
    return await processCommand("CAL6BDAC", "TDCSERVER", param);
  }

  ///setVthTime
  ///
  Future<String> setVthTime(int Threshold) async {
    Map param = new Map();
    param["value"] = Threshold;
    return await processCommand("SETVTHTIME", "TDCSERVER", param);
  }

  ///setTdcMode
  ///
  Future<String> setTdcMode(int mode) async {
    Map param = new Map();
    param["value"] = mode;
    return await processCommand("SETMODE", "TDCSERVER", param);
  }

  ///setTdcDelays
  ///
  Future<String> setTdcDelays(int active, int dead) async {
    Map param = new Map();
    param["value"] = active;
    var sact = await processCommand("SETDELAY", "TDCSERVER", param);
    Map r = new Map();
    r["active"] = json.decode(sact);
    param["value"] = dead;
    var sdead = await processCommand("SETDURATION", "TDCSERVER", param);
    r["dead"] = json.decode(sdead);
    return json.encode(r);
  }

  ///setTdcMask
  ///
  Future<String> setTdcMask(int channelmask, int asicmask) async {
    Map param = new Map();
    param["value"] = channelmask;
    param["asic"] = asicmask;
    return await processCommand("SETMASK", "TDCSERVER", param);
  }

  /// tdcLUTCalib
  ///
  Future<String> tdcLUTCalib(int instance, int channel) async {
    if (!appMap.containsKey("TDCSERVER"))
      return '{"answer":"NOTDCSERVER","status":"FAILED"}';
    if (appMap["TDCSERVER"].length <= instance)
      return '{"answer":"InvalidInstance","status":"FAILED"}';
    var tdc = appMap["TDCSERVER"][instance];

    int n = (1 << channel);
    Map param = new Map();
    param["value"] = n.toRadixString(16);
    Map r = new Map();
    r["cal_mask"] = json.decode(await tdc.sendCommand("CALIBMASK", param));
    r["cal_status"] = json.decode(await tdc.sendCommand("CALIBSTATUS", param));
    return json.encode(r);
  }
  
  /// tdcLUTMask
  ///
  Future<String> tdcLUTMask(int instance, int channel) async {
    if (!appMap.containsKey("TDCSERVER"))
      return '{"answer":"NOTDCSERVER","status":"FAILED"}';
    if (appMap["TDCSERVER"].length <= instance)
      return '{"answer":"InvalidInstance","status":"FAILED"}';
    var tdc = appMap["TDCSERVER"][instance];

    int n = (1 << channel);
    Map param = new Map();
    param["value"] = n.toRadixString(16);
    Map r = new Map();
    r["test_mask"] = json.decode(await tdc.sendCommand("TESTMASK", param));
    r["cal_status"] = json.decode(await tdc.sendCommand("CALIBSTATUS", param));
    return json.encode(r);
  }


  /// febSCurve
  /// 
  Future<String> febSCurve(int ntrg,int ncon,int ncoff,int thmin,int thmax,int mask,int  step) async {
    mdcc_Pause();
    mdcc_setSpillOn(ncon);
    print(" Clock On ${ncon} Off ${ncoff}"); 
      mdcc_setSpillOff(ncoff);
      mdcc_setSpillRegister(4);
      mdcc_CalibOn(1);
      mdcc_setCalibCount(ntrg);
      mdcc_Status();
      int thrange=(thmax-thmin+1)~/step;
      for (int vth=0;vth<=thrange;vth++) {
        mdcc_Pause();
        setVthTime(thmax-vth*step);
        builder_setHeader(2, thmax-vth*step, 0xFF);
        /// Check Lat built event
        /// Resume Calibration
        mdcc_ReloadCalibCount();
          mdcc_Resume();
          mdcc_Status();
        /// Wait for ntrg events capture
      /// End of point
        mdcc_Calibon(0);
      mdcc_Pause();
      }
          if ( not self.scurve_running):
              break;

  
         
          #self.feb_setmask(mask)
          #self.daq_setrunheader(2,(thmax-vth*step))
          self.daq_setrunheader(2,xi)
          # check current evb status
          sr=self.daq_evbstatus()
          sj=json.loads(sr)
          ssj=sj["answer"]
          firstEvent=int(ssj["event"])
          time.sleep(0.1)
          
          
          lastEvent=firstEvent
          nloop=0;
          while (lastEvent<(firstEvent+ntrg-20)):
              sr=self.daq_evbstatus()
              sj=json.loads(sr)
              ssj=sj["answer"]
              lastEvent=int(ssj["event"])
              print firstEvent,lastEvent,xi
              time.sleep(0.5)
              nloop=nloop+1
              if (nloop>20):
                  break
      
  }
  /// DAQ
  ///
  /// Initialise
  ///
  Future<String> daq_initialise() async {
    Map r = new Map();

    /// Initilise MDCC
    ///
    var s = json.decode(
        await appMap['MDCCSERVER'][0].sendTransition("OPEN", new Map()));
    r["MDCCSERVER"] = s;

    /// Configure Builder
    for (var x in appMap['BUILDER']) {
      var s = json.decode(await x.sendTransition("CONFIGURE", new Map()));
      r["BUILDER_${x.appInstance}"] = s;
    }

    /// Initialise TDCSERVER
    for (var x in appMap['TDCSERVER']) {
      var s = json.decode(await x.sendTransition("INITIALISE", new Map()));
      r["TDCSERVER_${x.appInstance}"] = s;
    }

    return json.encode(r);
  }

  ///
  /// Configure
  ///
  Future<String> daq_configure() async {
    Map r = new Map();

    /// Configure TDCSERVER
    for (var x in appMap['TDCSERVER']) {
      var s = json.decode(await x.sendTransition("CONFIGURE", new Map()));
      r["TDCSERVER_${x.appInstance}"] = s;
    }

    return json.encode(r);
  }

  ///
  /// Stop
  ///
  Future<String> daq_stop() async {
    Map r = new Map();

    /// Stop trigger
    var s = json.decode(
        await appMap['MDCCSERVER'][0].sendTransition("PAUSE", new Map()));
    r["TRIGGER"] = s;

    /// Stop TDCSERVER
    for (var x in appMap['TDCSERVER']) {
      s = json.decode(await x.sendTransition("STOP", new Map()));
      r["TDCSERVER_${x.appInstance}"] = s;
    }

    /// Stop Builder
    for (var x in appMap['BUILDER']) {
      s = json.decode(await x.sendTransition("STOP", new Map()));
      r["BUILDER_${x.appInstance}"] = s;
    }
    return json.encode(r);
  }

  ///
  /// DESTROY
  ///
  Future<String> daq_destroy() async {
    Map r = new Map();

    /// Stop TDCSERVER
    for (var x in appMap['TDCSERVER']) {
      var s = json.decode(await x.sendTransition("DESTROY", new Map()));
      r["TDCSERVER_${x.appInstance}"] = s;
    }

    return json.encode(r);
  }

  /// Start
  Future<String> daq_start(int run,
      {String location = "UNKNOWN", String comment = "Not set"}) async {
    /// Run from DB
    var nrun = run;
    if (nrun == 0) {
      await mongoAccess.open();
      //await m.listConfigurations();
      nrun = await mongoAccess.getRun(location, comment);
      await mongoAccess.close();
    } else
      nrun = run;

    ///
    Map r = new Map();

    /// Stop Builder
    for (var x in appMap['BUILDER']) {
      Map param = new Map();
      param['run'] = nrun;
      var s = json.decode(await x.sendTransition("START", param));
      r["BUILDER_${x.appInstance}"] = s;
      print(s);
    }

    /// Start TDCSERVER
    for (var x in appMap['TDCSERVER']) {
      var s = json.decode(await x.sendTransition("START", new Map()));
      r["TDCSERVER_${x.appInstance}"] = s;
      print(s);
    }

    /// Start trigger
    var s = json.decode(
        await appMap['MDCCSERVER'][0].sendTransition("RESET", new Map()));
    s = json.decode(
        await appMap['MDCCSERVER'][0].sendTransition("ECALRESUME", new Map()));
    r["TRIGGER"] = s;
    print(s);

    return json.encode(r);
  }
}
