import 'daqcontrol.dart';
import 'rcbase.dart';
import 'dart:convert';

class exRC extends daqControl {
  FSMaccess _fdaq;
  static const String _rcname = "RUNCONTROL";

  /// Constructor
  exRC(String account, String config) : super(account, config) {}

  /// Connection
  Future<void> Connect() async {
    await this.parseMongo();
    await this.discover();
    if (appMap.containsKey(_rcname))
      _fdaq = this.appMap[_rcname][0];
    else
      throw Exception("No ${_rcname} available");
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

  /// ExServer Status
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

    /// Configure Builder
    for (var x in appMap['BUILDER']) {
      var s = json.decode(await x.sendTransition("CONFIGURE", new Map()));
      r["BUILDER_${x.appInstance}"] = s;
    }

    /// Initialise EXSERVER
    for (var x in appMap['EXSERVER']) {
      var s = json.decode(await x.sendTransition("INITIALISE", new Map()));
      r["EXSERVER_${x.appInstance}"] = s;
    }

    return json.encode(r);
  }

  ///
  /// Configure
  ///
  Future<String> daq_configure() async {
    Map r = new Map();

    /// Configure trigger
    var s = json.decode(
        await appMap['TRIGGER'][0].sendTransition("CONFIGURE", new Map()));
    r["TRIGGER"] = s;

    /// Configure EXSERVER
    for (var x in appMap['EXSERVER']) {
      var s = json.decode(await x.sendTransition("CONFIGURE", new Map()));
      r["EXSERVER_${x.appInstance}"] = s;
    }

    return json.encode(r);
  }

  ///
  /// Stop
  ///
  Future<String> daq_stop() async {
    Map r = new Map();

    /// Stop trigger
    var s = json
        .decode(await appMap['TRIGGER'][0].sendTransition("STOP", new Map()));
    r["TRIGGER"] = s;

    /// Stop EXSERVER
    for (var x in appMap['EXSERVER']) {
      s = json.decode(await x.sendTransition("STOP", new Map()));
      r["EXSERVER_${x.appInstance}"] = s;
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

    /// Start EXSERVER
    for (var x in appMap['EXSERVER']) {
      var s = json.decode(await x.sendTransition("START", new Map()));
      r["EXSERVER_${x.appInstance}"] = s;
      print(s);
    }

    /// Start trigger
    var s = json
        .decode(await appMap['TRIGGER'][0].sendTransition("START", new Map()));
    r["TRIGGER"] = s;
    print(s);

    return json.encode(r);
  }
}
