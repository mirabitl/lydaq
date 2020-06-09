import 'rcbase.dart';
import 'mgaccess.dart' as mg;
import 'dart:io';
import 'dart:convert';
import 'package:logging/logging.dart';

class daqControl {
  String _account;
  String _config;

  List<FSMaccess> _jobcontrols;
  Map _mgConfig;
  Map _appMap;
  final _log = Logger('daqControl');

  mg.MongoAccess _mongoAccess;

  /// getters
  ///
  ///
  mg.MongoAccess get mongoAccess {
    return _mongoAccess;
  }

  Map get appMap {
    return _appMap;
  }

  List<FSMaccess> get jobcontrols {
    return _jobcontrols;
  }

  String get account {
    return _account;
  }

  String get config_name {
    return _config;
  }

  Map get config_map {
    return _mgConfig;
  }

  /// Constructor
  daqControl(String account, String config) {
    _account = account;
    _config = config;
    _jobcontrols = new List<FSMaccess>();
    _appMap = new Map();
    //this.parseMongo();
  }

  /// Mongo access
  Future<void> parseMongo() async {
    String userinfo = _account.split("@")[0];
    String hostinfo = _account.split("@")[1];
    String dbname = _account.split("@")[2];
    String user = userinfo.split("/")[0];
    String pwd = userinfo.split("/")[1];
    String host = hostinfo.split(":")[0];
    int port = int.parse(hostinfo.split(":")[1]);

    _log.info(" Account is ${user} ${pwd} ${host} ${port} ${dbname}");
    String config = _config.split(":")[0];
    int version = int.parse(_config.split(":")[1]);
    _mongoAccess = new mg.MongoAccess(
        "mongodb://${user}:${pwd}@${host}:${port}/${dbname}");
    await _mongoAccess.open();
    //await m.listConfigurations();
    await _mongoAccess.downloadConfiguration(config, version);
    await _mongoAccess.close();
    _mgConfig = json.decode(
        await new File('/dev/shm/mgjob/${config}_${version}.json')
            .readAsString());
    //print(_mgConfig);
  }

  Future<void> discover() async {
    if (!_mgConfig.containsKey('HOSTS')) return;
    _appMap.clear();
    Map<String, dynamic> mh = _mgConfig['HOSTS'];
    for (var x in mh.entries) {
      //print("Host found ${x.key}");
      var fsm = new FSMaccess("${x.key}", 9999);
      _jobcontrols.add(fsm);
    }
    for (var x in _jobcontrols) {
      await x.getInfo();
      if (x.state == "FAILED") {
        _log.shout("Failed request ${x.url} exiting");
        exit(0);
      }
      var s = await x.sendCommand("STATUS", new Map());
      final m = json.decode(s);

      if (m['answer']['JOBS'] == null)
        print("${x.url} has NO Jobs : ${s}");
      else
        for (var pcs in m['answer']['JOBS']) {
          if (pcs['STATUS'].split(' ')[0] == 'X') {
            print(pcs);
            continue;
          }
          var bapp = new FSMaccess(pcs['HOST'], int.parse(pcs['PORT']));
          await bapp.getInfo();
          //print(bapp.infos);
          //print(bapp.params);
          if (!_appMap.containsKey(bapp.infos['name'])) {
            var l = new List<FSMaccess>();
            l.add(bapp);
            _appMap[bapp.infos['name']] = l;
          } else
            _appMap[bapp.infos['name']].add(bapp);
        }
    }
    /*
    for (var x in _appMap['BUILDER']) {
      print("${x.infos}");
      var rep = json.decode(await x.sendCommand("STATUS", new Map()));
      print("\t ${rep['answer']['answer']}");
    }
    */
  }

  /// Update infos and params
  ///
  ///
  Future<void> updateInfo(bool printout, bool vv) async {
    if (this._appMap.length == 0) {
      print("No Application Map found. Please Connect first or create process");
    }
    for (var k in _appMap.entries)
      for (var x in k.value) {
        await x.getInfo();
        if (printout) {
          x.printInfos(vv);
        }
      }
  }

  /// Job control
  /// Default transition
  ///
  Future<String> jc_transition(String Transition, Map par) async {
    Map rep = new Map();
    if (this.jobcontrols.length == 0)
      throw Exception("No jobcontrols found. Please Connect first");
    for (var x in jobcontrols) {
      String lrep = await x.sendTransition(Transition, par);
      rep[x.host_name] = json.decode(lrep);
    }
    return json.encode(rep);
  }

  /// Default command
  ///
  Future<String> jc_command(String command, Map par) async {
    Map rep = new Map();
    if (this.jobcontrols.length == 0)
      throw Exception("No jobcontrols found. Please Connect first");
    for (var x in jobcontrols) {
      String lrep = await x.sendCommand(command, par);
      rep[x.host_name] = json.decode(lrep);
    }
    return json.encode(rep);
  }

  /// CREATE transition
  ///
  Future<String> jc_initialise() async {
    Map par = new Map();
    par['mongo'] = this.config_name;
    return await this.jc_transition("INITIALISE", par);
  }

  /// START transition
  ///
  Future<String> jc_start() async {
    return await this.jc_transition("START", new Map());
  }

  /// KILL transition
  ///
  Future<String> jc_kill() async {
    return await this.jc_transition("KILL", new Map());
  }

  /// DESTROY transition
  ///
  Future<String> jc_destroy() async {
    return await this.jc_transition("DESTROY", new Map());
  }

  /// STATUS command
  ///
  Future<String> jc_status() async {
    return await this.jc_command("STATUS", new Map());
  }

  /// Broadcast APPCREATE command
  ///
  Future<String> jc_appcreate() async {
    return await this.jc_command("APPCREATE", new Map());
  }

  /// Standard command
  Future<String> processCommand(String cmd, String appname, Map param) async {
    Map r = new Map();

    if (!appMap.containsKey(appname))
      return '{"answer":"invalidname","status":"FAILED"}';

    /// Loop on all applications
    ///
    for (var x in appMap[appname]) {
      var s = json.decode(await x.sendCommand(cmd, param));
      r["${appname}_${x.appInstance}"] = s;
    }

    return json.encode(r);
  }

  ///  MDCC specific
  ///
  /// Status
  ///
  Future<String> mdcc_Status() async {
    Map param = new Map();
    return await processCommand("STATUS", "MDCCSERVER", param);
  }

  /// Pause
  ///
  Future<String> mdcc_Pause() async {
    Map param = new Map();
    return await processCommand("PAUSE", "MDCCSERVER", param);
  }

  /// Resume
  ///
  Future<String> mdcc_Resume() async {
    Map param = new Map();
    return await processCommand("RESUME", "MDCCSERVER", param);
  }

  /// ECAL Pause
  ///
  Future<String> mdcc_EcalPause() async {
    Map param = new Map();
    return await processCommand("ECALPAUSE", "MDCCSERVER", param);
  }

  /// ECAL Resume
  ///
  Future<String> mdcc_EcalResume() async {
    Map param = new Map();
    return await processCommand("ECALRESUME", "MDCCSERVER", param);
  }

  /// Calib On
  ///
  Future<String> mdcc_CalibOn(int value) async {
    Map param = new Map();
    param["value"] = value;
    return await processCommand("CALIBON", "MDCCSERVER", param);
  }

  /// Calib Off
  ///
  Future<String> mdcc_CalibOff() async {
    Map param = new Map();
    return await processCommand("CALIBOFF", "MDCCSERVER", param);
  }

  /// Reload Calibration count
  ///
  Future<String> mdcc_ReloadCalibCount() async {
    Map param = new Map();
    return await processCommand("RELOADCALIB", "MDCCSERVER", param);
  }

  /// Set Calibration count
  ///
  Future<String> mdcc_setCalibCount(int nclock) async {
    Map param = new Map();
    param["nclock"] = nclock;
    return await processCommand("SETCALIBCOUNT", "MDCCSERVER", param);
  }

  /// Reset
  ///
  Future<String> mdcc_Reset() async {
    Map param = new Map();
    return await processCommand("RESET", "MDCCSERVER", param);
  }

  /// read register
  ///
  Future<String> mdcc_ReadRegister(int address) async {
    Map param = new Map();
    param["address"] = address;
    return await processCommand("READREG", "MDCCSERVER", param);
  }

  /// Write register
  ///
  Future<String> mdcc_WriteRegister(int address, int value) async {
    Map param = new Map();
    param["address"] = address;
    param["value"] = value;
    return await processCommand("WRITEREG", "MDCCSERVER", param);
  }

  /// set Spill on length
  ///
  Future<String> mdcc_setSpillOn(int nclock) async {
    Map param = new Map();
    param["nclock"] = nclock;
    return await processCommand("SPILLON", "MDCCSERVER", param);
  }

  /// set Spill off length
  ///
  Future<String> mdcc_setSpillOff(int nclock) async {
    Map param = new Map();
    param["nclock"] = nclock;
    return await processCommand("SPILLOFF", "MDCCSERVER", param);
  }

  /// set Reset TDC bit
  ///
  Future<String> mdcc_setResetTdcBit(int value) async {
    Map param = new Map();
    param["value"] = value;
    return await processCommand("RESETTDC", "MDCCSERVER", param);
  }

  /// reset TDC
  ///
  Future<String> mdcc_resetTdc(int nclock) async {
    await mdcc_setResetTdcBit(0);
    return await mdcc_setResetTdcBit(1);
  }

  /// set Beam On length
  ///
  Future<String> mdcc_setBeamOn(int nclock) async {
    Map param = new Map();
    param["nclock"] = nclock;
    return await processCommand("BEAMON", "MDCCSERVER", param);
  }

  /// set HardReset
  ///
  Future<String> mdcc_setHardReset(int value) async {
    Map param = new Map();
    param["value"] = value;
    return await processCommand("SETHARDRESET", "MDCCSERVER", param);
  }

  /// set Spill Register
  ///
  Future<String> mdcc_setSpillRegister(int value) async {
    Map param = new Map();
    param["value"] = value;
    return await processCommand("SETSPILLREGISTER", "MDCCSERVER", param);
  }

  /// set External Trigger delay
  ///
  Future<String> mdcc_setExternal(int value) async {
    Map param = new Map();
    param["value"] = value;
    return await processCommand("SETEXTERNAL", "MDCCSERVER", param);
  }

  /// set Calibration Register
  ///
  Future<String> mdcc_setCalibRegister(int value) async {
    Map param = new Map();
    param["value"] = value;
    return await processCommand("SETCALIBREGISTER", "MDCCSERVER", param);
  }

  /// set trigger external delays
  ///
  Future<String> mdcc_setTriggerDelays(int delay, int busy) async {
    Map param = new Map();
    param["delay"] = delay;
    param["busy"] = busy;
    return await processCommand("SETTRIGEXT", "MDCCSERVER", param);
  }

  /// Builder specific
  ///
  /// RUn Header
  Future<String> builder_setHeader(int type, int value, int mask) async {
    Map param = new Map();
    List<int> l = new List(3);
    l[0] = type;
    l[1] = value;
    l[2] = mask;
    param["header"] = l;
    return await processCommand("SETHEADER", "BUILDER", param);
  }
}
