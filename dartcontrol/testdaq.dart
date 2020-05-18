//import 'dart:html';

import 'exruncontrol.dart';
import 'dart:io';
import 'package:args/args.dart';

//import 'package:http/http.dart' as http;
//import 'package:uri/uri.dart';
import 'dart:convert';
//import 'package:mongo_dart/mongo_dart.dart';
//import 'mgaccess.dart' as mg;
//import 'package:ansicolor/ansicolor.dart';
import 'package:logging/logging.dart';

void main(List<String> arguments) async {
  Logger.root.level = Level.WARNING; // defaults to Level.INFO
  Logger.root.onRecord.listen((record) {
    print('${record.level.name}: ${record.time}: ${record.message}');
  });

  ArgResults argResults;

  final ArgParser argParser = new ArgParser()
    ..addOption('Mongo',
        abbr: 'm', defaultsTo: Platform.environment['MGDBLOGIN'])
    ..addOption('Configuration',
        abbr: 'c', defaultsTo: Platform.environment['DAQMONGO'])
    ..addOption('Command', abbr: 'M', defaultsTo: "NONE", help: "Command name")
    ..addOption('Application',
        abbr: 'a', defaultsTo: "BUILDER", help: "Application name")
    ..addOption('Parameters',
        abbr: 'p', defaultsTo: "{}", help: "Parameters list")
    ..addOption('run', defaultsTo: '0')
    ..addOption('setup', defaultsTo: 'UNKNOWN')
    ..addOption('comment', defaultsTo: 'Test run')
    ..addFlag('infos',
        abbr: 'i',
        negatable: false,
        help: "Displays all processes information.")
    ..addFlag('verbose', abbr: 'v', negatable: false, help: "Verbose mode")
    ..addFlag('jobcontrol',
        abbr: 'j', negatable: false, help: "jobcontrol flag")
    ..addFlag('daqcontrol',
        abbr: 'd', negatable: false, help: "daqcontrol flag")
    ..addFlag('create', abbr: 'R', negatable: false, help: "Create transition ")
    ..addFlag('initialise',
        abbr: 'I', negatable: false, help: "Initialise transition ")
    ..addFlag('configure',
        abbr: 'C', negatable: false, help: "Initialise transition ")
    ..addFlag('start', abbr: 'A', negatable: false, help: "Start transition ")
    ..addFlag('stop', abbr: 'O', negatable: false, help: "Stop transition ")
    ..addFlag('kill',
        abbr: 'K', negatable: false, help: "Kill transition of the job control")
    ..addFlag('destroy',
        abbr: 'D',
        negatable: false,
        help: "Destroy transition of the job control")
    ..addFlag('status', abbr: 's', negatable: false, help: "Status command ")
    ..addFlag('builder', negatable: false, help: "Event Builder flag ")
    ..addFlag('source', negatable: false, help: "Data Source flag ")
    ..addFlag('trigger', negatable: false, help: "trigger flag ")
    ..addFlag('help',
        abbr: 'h', negatable: false, help: "Displays this help information.");
  argResults = argParser.parse(arguments);

  if (argResults['help']) {
    print("""
** HELP **
${argParser.usage}
    """);
    exit(0);
  }

  //print("job ${argResults['jobcontrol']} ${argResults['start']}");

  // print(argResults['server'] +
  //     " " +
  //     argResults['port'] +
  //     " " +
  //     argResults['mongo'] +
  //     " " +
  //     argResults['configuration']);

  exRC d = new exRC(argResults['Mongo'], argResults['Configuration']);
  await d.parseMongo();
  await d.discover();

  // var rep = json.decode(await d.BuilderStatus());
  // print(rep);

  // rep = json.decode(await d.SourceStatus("EXSERVER"));
  // print(rep);

  // rep = json.decode(await d.TriggerStatus("TRIGGER"));
  // print(rep);

  /// Use flag requests
  if (argResults['infos']) {
    print("""
\t \t ******************************    
\t \t ** Applications information **
\t \t ******************************
    """);
    bool verbose = argResults['verbose'];
    try {
      await d.updateInfo(true, verbose);
    } on Exception {
      print("Must exit");
    }
    exit(0);
  }

  if (argResults['status'] && argResults['builder']) {
    print("""
\t \t *************************    
\t \t ** Builder information **
\t \t *************************
    """);
    var rep = json.decode(await d.BuilderStatus());
    for (var k in rep.entries) {
      print("${k.key}");
      for (var x in k.value.entries)
        if (x.key != "builder")
          print("\t ${x.key} ${x.value}");
        else {
          if (x.value != null) for (var y in x.value) print("\t \t ${y}");
        }
    }
  }
  if (argResults['status'] && argResults['source']) {
    print("""
\t \t ******************************    
\t \t ** Data sources information **
\t \t ******************************
    """);
    var rep = json.decode(await d.SourceStatus("EXSERVER"));
    for (var k in rep.entries) {
      print("${k.key}");
      if (k.value != null) for (var x in k.value) print("\t \t ${x}");
    }
  }
  if (argResults['status'] && argResults['trigger']) {
    print("""
\t \t *************************    
\t \t ** Trigger information **
\t \t *************************
    """);
    var rep = json.decode(await d.TriggerStatus("TRIGGER"));
    for (var k in rep.entries) {
      print("${k.key}");
      for (var x in k.value["answer"].entries)
        print("\t \t ${x.key} => ${x.value}");
    }
  }

  if (argResults['jobcontrol'] && argResults['start']) {
    var rep = json.decode(await d.jc_start());
    print(rep);
  }

  if (argResults['jobcontrol'] && argResults['kill']) {
    var rep = json.decode(await d.jc_kill());
    print(rep);
  }

  if (argResults['jobcontrol'] && argResults['initialise']) {
    var rep = json.decode(await d.jc_initialise());
    print(rep);
  }

  if (argResults['jobcontrol'] && argResults['destroy']) {
    var rep = json.decode(await d.jc_destroy());
    print(rep);
  }

  if (argResults['jobcontrol'] && argResults['create']) {
    var rep = json.decode(await d.jc_appcreate());
    print(rep);
  }

  if (argResults['daqcontrol'] && argResults['initialise']) {
    var rep = json.decode(await d.daq_initialise());
    print(rep);
  }

  if (argResults['daqcontrol'] && argResults['configure']) {
    var rep = json.decode(await d.daq_configure());
    print(rep);
  }
  if (argResults['daqcontrol'] && argResults['start']) {
    var rep = json.decode(await d.daq_start(int.parse(argResults['run']),
        location: argResults['setup'], comment: argResults['comment']));
    print(rep);
  }

  if (argResults['daqcontrol'] && argResults['stop']) {
    var rep = json.decode(await d.daq_stop());
    print(rep);
  }

  if (argResults['Command'] != "NONE" && argResults['Application'] != "NONE") {
    var rep = json.decode(await d.processCommand(argResults['Command'],
        argResults['Application'], json.decode(argResults['Parameters'])));
    print(rep);
  }
  /*
  //print(Platform.environment);
  String userinfo = argResults['mongo'].split("@")[0];
  String hostinfo = argResults['mongo'].split("@")[1];
  String dbname = argResults['mongo'].split("@")[2];
  String user = userinfo.split("/")[0];
  String pwd = userinfo.split("/")[1];
  String host = hostinfo.split(":")[0];
  int port = int.parse(hostinfo.split(":")[1]);

  print("${user} ${pwd} ${host} ${port} ${dbname}");
  String config = argResults['configuration'].split(":")[0];
  int version = int.parse(argResults['configuration'].split(":")[1]);
  mg.MongoAccess m =
      new mg.MongoAccess("mongodb://${user}:${pwd}@${host}:${port}/${dbname}");
  await m.open();
  //await m.listConfigurations();
  await m.downloadConfiguration(config, version);
  await m.close();
  Map xconf = json.decode(
      await new File('/dev/shm/mgjob/${config}_${version}.json')
          .readAsString());
  //print("${xconf['HOSTS']}");

  List<FSMaccess> jobcontrols = new List<FSMaccess>();
  Map<String, dynamic> mh = xconf['HOSTS'];
  for (var x in mh.entries) {
    print("Host found ${x.key}");
    var fsm = new FSMaccess("${x.key}", 9999);
    jobcontrols.add(fsm);
  }
  for (var x in jobcontrols) {
    await x.getInfo();
    if (x.state == "FAILED") {
      print("Failed request ${x.url} exiting");
      exit(0);
    }
    var s = await x.sendCommand("STATUS", new Map());
    final m = json.decode(s);
    //print("${x.url} => ${s}");
    for (var pcs in m['answer']['JOBS']) {
      
      print("${pcs['NAME']} on ${pcs['HOST']}:${pcs['PORT']}");
      var bapp= new FSMaccess(pcs['HOST'],int.parse(pcs['PORT']));
      await bapp.getInfo();
      print(bapp.infos);
      print(bapp.params);
    }
  }

  exit(0);

  String server = 'http://lyorpipul02:42000';
  String url = server;

//print(decoded);
  await printFSMStatus(url);

  FSMaccess a = FSMaccess("lyocmsmu04", 45000);

  await a.getInfo();

  if (a.state == "FAILED") {
    print("Failed request exiting");
    exit(0);
  }

  var param = new Map();
  var dir = new Map();
  dir['aa'] = 45;
  dir['bb'] = "unessai";
  param['nc'] = 4;
  param['pluggin'] = 'libbdwr.so';
  param['directory'] = json.encode(dir);

  var s = await a.sendCommand("STATUS", param);
  print(s);
  */
}
