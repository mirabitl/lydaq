import 'daqcontrol.dart';
//import 'rcbase.dart';
import 'dart:convert';
import 'dart:io';

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
  Future<String> tdcLUTMask(int instance, int mask) async {
    if (!appMap.containsKey("TDCSERVER"))
      return '{"answer":"NOTDCSERVER","status":"FAILED"}';
    if (appMap["TDCSERVER"].length <= instance)
      return '{"answer":"InvalidInstance","status":"FAILED"}';
    var tdc = appMap["TDCSERVER"][instance];

    Map param = new Map();
    param["value"] = mask.toRadixString(16);
    Map r = new Map();
    r["test_mask"] = json.decode(await tdc.sendCommand("TESTMASK", param));
    r["cal_status"] = json.decode(await tdc.sendCommand("CALIBSTATUS", param));
    return json.encode(r);
  }

  /// febSCurve
  ///
  Future<String> febSCurve(
      int ntrg, int ncon, int ncoff, int thmin, int thmax, int step) async {
    Map r = new Map();
    mdcc_Pause();
    mdcc_setSpillOn(ncon);
    print(" Clock On ${ncon} Off ${ncoff}");
    mdcc_setSpillOff(ncoff);
    mdcc_setSpillRegister(4);
    mdcc_CalibOn(1);
    mdcc_setCalibCount(ntrg);
    mdcc_Status();
    int thrange = (thmax - thmin + 1) ~/ step;
    for (int vth = 0; vth <= thrange; vth++) {
      mdcc_Pause();
      setVthTime(thmax - vth * step);
      sleep(const Duration(milliseconds: 100));
      builder_setHeader(2, thmax - vth * step, 0xFF);

      /// Check Last built event
      var sr = json.decode(await BuilderStatus());
      int firstEvent = sr["event"];

      /// Resume Calibration
      mdcc_ReloadCalibCount();
      mdcc_Resume();
      mdcc_Status();

      /// Wait for ntrg events capture
      int lastEvent = firstEvent;
      int nloop = 0;
      while (lastEvent < (firstEvent + ntrg - 20)) {
        sr = json.decode(await BuilderStatus());
        lastEvent = sr["event"];
        print(" First ${firstEvent} Last ${lastEvent} Step ${vth}");
        sleep(const Duration(milliseconds: 200));
        nloop++;
        if (nloop > 20) break;
      }
      r["${thmax - vth * step}"] = lastEvent - firstEvent + 1;

      /// End of point
      mdcc_CalibOn(0);
      mdcc_Pause();
    }
    return json.encode(r);
  }

  /// SCURVE run
  ///
  /*
      FEBV1: Acquisition loop interface for VTHTIME scan

      :param ch: 255 = according tomode channel/channel, 1023 = all channels, other value= channel(PETIROC) 
      :param spillon: Number of clock active
      :param spiloff: Number of clock off
      :param beg: lowest VTHTIME value
      :param las: highest VTHTIME value
      :param step: Loop step
      :param asic: Asic mask (1,2,or 3)
      :param mode: OLD=WT board,COAX=coaxial PCB, FEBV0=with return line PCB,FEBV1
      */

  Future<String> runScurve(
      int run, int ch, int spillon, int spilloff, int beg, int las,
      {int step = 2,
      int asic = 255,
      String Comment = "PR2 Calibration",
      String Location = "UNKNOWN",
      int nevmax = 50}) async {
    List<int> firmware = [
      3,
      4,
      5,
      6,
      7,
      8,
      9,
      10,
      11,
      12,
      13,
      14,
      15,
      16,
      17,
      18,
      19,
      20,
      21,
      22,
      23,
      24,
      26,
      28,
      30
    ];
    await daq_start(run, location: Location, comment: Comment);
    Map r = new Map();
    r["run"] = run;
    switch (ch) {
      case 255:
        {
          print("Run Scurve on all channel together");
          int mask = 0;
          for (int i in firmware) mask = mask | (1 << i);
          setTdcMask(mask, asic);
          r["${ch}"] = json.decode(
              await febSCurve(nevmax, spillon, spilloff, beg, las, step));
          await daq_stop();
        }
        break;

      case 1023:
        {
          print("Run Scurve on all channel One by One");
          for (int i in firmware) {
            int mask = (1 << i);
            setTdcMask(mask, asic);
            r["${ch}_${i}"] = json.decode(
                await febSCurve(nevmax, spillon, spilloff, beg, las, step));
          }
         await  daq_stop();
        }
        break;

      default:
        {
          print("SCurve on PR2 channel ${ch}");
          int mask = (1 << ch);
          setTdcMask(mask, asic);
          r["${ch}"] = json.decode(
              await febSCurve(nevmax, spillon, spilloff, beg, las, step));

          await daq_stop();
        }
        break;
    }
    return json.encode(r);
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
