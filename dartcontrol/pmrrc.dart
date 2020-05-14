import 'daqcontrol.dart';
//import 'rcbase.dart';
import 'dart:convert';

class pmrRC extends daqControl {

  /// Constructor
  pmrRC(String account, String config) : super(account, config) {}

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

  /// PMRMANAGER Status
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
              mr["answer"]["zmSenders"];
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

    // Scan PMRManager
    for (var x in appMap['PMRMANAGER']) {
      var s = json.decode(await x.sendTransition("SCAN", new Map()));
      r["PMRMANAGER_${x.appInstance}"] = s;
    }

    /// Initialise PMRMANAGER
    for (var x in appMap['PMRMANAGER']) {
      var s = json.decode(await x.sendTransition("INITIALISE", new Map()));
      r["PMRMANAGER_${x.appInstance}"] = s;
    }

    return json.encode(r);
  }

  ///
  /// Configure
  ///
  Future<String> daq_configure() async {
    Map r = new Map();

    /// Configure PMRMANAGER
    for (var x in appMap['PMRMANAGER']) {
      var s = json.decode(await x.sendTransition("CONFIGURE", new Map()));
      r["PMRMANAGER_${x.appInstance}"] = s;
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

    /// Stop PMRMANAGER
    for (var x in appMap['PMRMANAGER']) {
      s = json.decode(await x.sendTransition("STOP", new Map()));
      r["PMRMANAGER_${x.appInstance}"] = s;
    }

    /// Stop Builder
    for (var x in appMap['BUILDER']) {
      s = json.decode(await x.sendTransition("STOP", new Map()));
      r["BUILDER_${x.appInstance}"] = s;
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

    /// Start PMRMANAGER
    for (var x in appMap['PMRMANAGER']) {
      var s = json.decode(await x.sendTransition("START", new Map()));
      r["PMRMANAGER_${x.appInstance}"] = s;
      print(s);
    }

    /// Start trigger
    var s = json
        .decode(await appMap['TRIGGER'][0].sendTransition("RESET", new Map()));
    s = json.decode(
        await appMap['TRIGGER'][0].sendTransition("ECALRESUME", new Map()));
    r["TRIGGER"] = s;
    print(s);

    return json.encode(r);
  }
}
