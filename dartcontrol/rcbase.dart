// Import the math library to get access to the sqrt function.
//import 'dart:math' as math;
//import 'dart:html';
//import 'package:http/http.dart' as http;
//import 'dart:io';
//import 'package:args/args.dart';

import 'package:http/http.dart' as http;
//import 'package:uri/uri.dart';
import 'dart:convert';
//import 'package:mongo_dart/mongo_dart.dart';
//import 'mgaccess.dart' as mg;
import 'package:ansicolor/ansicolor.dart';

class FSMException implements Exception {
  String errMsg() => 'FSMAccess problem';
  @override
  String toString() {
    // TODO: implement toString
    return "FSMAccess issue";
  }
}

class FSMaccess {
  /// Monngoose url of the Finite State Machine
  String url;
  String host;
  int port;
  String prefix;
  int pid;
  String fUrl;
  String state;
  Map procInfos;
  Map infos;
  String appType;
  int appInstance;
  Map params;

  /// Getters
  ///
  String get host_name {
    return host;
  }

  String get state_name {
    return state;
  }

  /*
  * Constructor
  * @param host the hostname
  * @param port the webserver port
  */
  FSMaccess(String vhost, int vport) {
    host = vhost;
    port = vport;
    url = "http://${host}:${port}";
    getInfo();
    appType = 'UNKNOWN';
    appInstance = 0;
    infos={};
    procInfos={};
  }

  /// Proc and FSM info
  ///
  Future<void> getProcInfo() async {
    var rep;
    try {
      rep = await http.read(url).timeout(
        Duration(seconds: 4),
        onTimeout: () {
          // time has run out, do what you wanted to do
          print('getInfo TIMEOUT');
          return;
        },
      );
    } catch (e) {
      print("Exception catched: ${e}");
      prefix = 'NONE';
      fUrl = "http://${host}:${port}/${prefix}";
      state = "FAILED";
      pid=-1;
      procInfos['STATE']="FAILED";
      return;
    }

    procInfos = json.decode(rep);
    pid = procInfos['PID'];
    prefix = procInfos['PREFIX'];
    fUrl = "http://${host}:${port}/${prefix}";
    state = procInfos['STATE'];
  }

  /// base application info
  ///
  Future<void> getInfo() async {
    await getProcInfo();
    if (pid<0) return;
    if (isBaseApplication(procInfos)) {
      var sinf = await this.sendCommand('INFO', new Map());
      infos = json.decode(sinf)['answer']['INFO'];
      appType = infos['name'];
      appInstance = infos['instance'];
      var spar = await this.sendCommand('GETPARAM', new Map());
      params = json.decode(spar)['answer']['PARAMETER'];
    }
  }

  /// Send transition
  ///
  Future<String> sendTransition(String name, Map content) async {
    await this.getProcInfo();
    bool isValid = false;
    for (dynamic k in procInfos.entries)
      if (k.key == 'ALLOWED')
        for (dynamic x in k.value) if (x['name'] == name) isValid = true;
    if (!isValid) return '{"answer":"invalid","status":"FAILED"}';

    Map param=new Map();
    param['command'] = name;
    param['content'] = content;
    var luri = "${fUrl}/FSM?command=${name}&content=${jsonEncode(content)}";
    var rep = await http.read(luri).timeout(
      Duration(seconds: 4),
      onTimeout: () {
        // time has run out, do what you wanted to do
        return '{"answer":"none","status":"TIMEOUT"}';
      },
    );
    return rep;
  }

  /// Send Command
  ///
  Future<String> sendCommand(String name, dynamic content) async {
    await this.getProcInfo();
    bool isValid = false;
    for (dynamic k in procInfos.entries)
      if (k.key == 'CMD')
        for (dynamic x in k.value) if (x['name'] == name) isValid = true;
    if (!isValid) return '{"answer":"invalid","status":"FAILED"}';

    //print("The content is ${content}   and ${name} ${fUrl}");
    var luri = "${fUrl}/CMD?name=${name}";

    if (content != null)
      for (dynamic x in content.entries) luri = luri + "&${x.key}=${x.value}";
    //print("on av imprimer les parametres");

    String rep = "TOTO";
    //print("the value of ${rep}");
    //return rep;

    //print(luri);
    rep = await http.read(luri).timeout(
      Duration(seconds: 4),
      onTimeout: () {
        // time has run out, do what you wanted to do
        return '{"answer":"none","status":"TIMEOUT"}';
      },
    );
    return rep;
  }

  /// Dump full process info
  ///
  void printInfos(bool vv) {
    AnsiPen BAPen = AnsiPen()
      ..yellow(bg: true)
      ..blue();
    AnsiPen BBPen = AnsiPen()
      ..green(bg: true)
      ..red();
    AnsiPen BCPen = AnsiPen()
      ..white(bg: true)
      ..blue();
    if (vv) {
      print(
          "\n FSM is ${BCPen(procInfos['STATE'])} on ${BCPen(url)}, PID ${procInfos['PID']}  Service ${procInfos['PREFIX']}:\n");
      for (dynamic k in procInfos.entries) {
        if (k.key == 'ALLOWED' || k.key == 'CMD' || k.key == 'FSM') {
          List<dynamic> v = k.value;
          String s = "${BAPen(k.key)} \t";
          for (dynamic x in v) s = s + x['name'] + " ";
          print(s);
        }
      }

      String sinfo = "BaseApplication ${infos['name']}  " +
          " instance ${infos['instance']}";
      print("\n ${BBPen(sinfo)}");
      print("Parameters ");

      for (dynamic k in params.entries)
        print("\t ${BBPen(k.key)} : ${k.value}");
    } else {
      print(
          "${BBPen(infos['name'])}:${infos['instance']}  ${BCPen(procInfos['STATE'])} on ${BCPen(url)}, PID ${procInfos['PID']}  Service ${procInfos['PREFIX']}");
    }
  }
}

///Utilities
///
Future<void> printFSMStatus(String url) async {
  var rep = await http.read(url);

  dynamic m = json.decode(rep);
  AnsiPen BBPen = AnsiPen()
    ..yellow(bg: true)
    ..blue();

  print(
      "FSM is ${m['STATE']} on ${url}, PID ${m['PID']}  Service ${m['PREFIX']}:");
  for (dynamic k in m.entries) {
    if (k.key == 'ALLOWED' || k.key == 'CMD' || k.key == 'FSM') {
      List<dynamic> v = k.value;
      String s = "${BBPen(k.key)} \t";
      for (dynamic x in v) s = s + x['name'] + " ";
      print(s);
    }
  }
  //decoded.forEach((k,v) => print('${k}: ${v}'));
  if (isBaseApplication(m)) {
    await dumpParameters(url, m['PREFIX']);
  }
  //int pid = m['PID'].toInt();

  //print('the pid of the job control is ${pid}');
}

bool isBaseApplication(Map m) {
  bool base = false;
  for (dynamic k in m.entries) {
    if (k.key == 'CMD') {
      List<dynamic> v = k.value;

      for (dynamic x in v) {
        if (x['name'] == 'GETPARAM') base = true;
      }
    }
  }
  return base;
}

bool hasCommand(Map m, String name) {
  bool base = false;
  for (dynamic k in m.entries) {
    if (k.key == 'CMD') {
      List<dynamic> v = k.value;

      for (dynamic x in v) {
        if (x['name'] == name) base = true;
      }
    }
  }
  return base;
}

Future<void> dumpParameters(String url, String prefix) async {
  AnsiPen BBPen = AnsiPen()
    ..green(bg: true)
    ..red();
  String purl = url + "/" + prefix + "/CMD?name=INFO";
  var rep = await http.read(purl);
  var dp1 = json.decode(rep);
  dynamic info = dp1['answer']['INFO'];
  String sinfo =
      "BaseApplication ${info['name']}  " + " instance ${info['instance']}";
  print(sinfo);
  print("\n Parameters ");
  purl = url + "/" + prefix + "/CMD?name=GETPARAM";
  rep = await http.read(purl);
  final decodep = json.decode(rep);
  for (dynamic k in decodep['answer']['PARAMETER'].entries)
    print("\t ${BBPen(k.key)} : ${k.value}");
}
