//import 'dart:html';

import 'gricrc.dart';
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
    ..addOption('comment', defaultsTo: 'NOTSET')
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

  gricRC d = new gricRC(argResults['Mongo'], argResults['Configuration']);
  await d.parseMongo();
  await d.discover();

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
          if (x.value != null)
            for (var y in x.value) {
              print(
                  "\t \t ID ${int.parse(y['id'].split('-')[2]).toRadixString(16)} => ${y['received']}");
            }
        }
    }
  }
  if (argResults['status'] && argResults['source']) {
    print("""
\t \t ******************************    
\t \t ** Data sources information **
\t \t ******************************
    """);
    var rep = json.decode(await d.SourceStatus("GRICSERVER"));
    for (var k in rep.entries) {
      print("${k.key}");
      if (k.value != null) for (var x in k.value) print("\t \t ${x}");
    }
  }
  if (argResults['status'] && argResults['trigger']) {
    print("""
\t \t ****************************    
\t \t ** MDCCSERVER information **
\t \t ****************************
    """);
    var rep = json.decode(await d.TriggerStatus("MDCCSERVER"));
    for (var k in rep.entries) {
      print("${k.key}");
      for (var x in k.value["COUNTERS"].entries)
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
  if (argResults['daqcontrol'] && argResults['destroy']) {
    var rep = json.decode(await d.daq_destroy());
    print(rep);
  }

  if (argResults['daqcontrol'] && argResults['configure']) {
    var rep = json.decode(await d.daq_configure());
    print(rep);
  }
  if (argResults['daqcontrol'] && argResults['start']) {
    String sloc = argResults['setup'];
    if (sloc == "UNKNOWN" && Platform.environment.containsKey('DAQSETUP'))
      sloc = Platform.environment['DAQSETUP'];
    String scomment = argResults['comment'];
    if (argResults['comment'] == 'NOTSET') {
      print("Please enter a comment for this run");
      var line = stdin.readLineSync(encoding: Encoding.getByName('utf-8'));
      scomment = line;
    }
    var rep = json.decode(await d.daq_start(int.parse(argResults['run']),
        location: sloc, comment: scomment));
    print(rep);
  }

  if (argResults['daqcontrol'] && argResults['stop']) {
    var rep = json.decode(await d.daq_stop());
    print(rep);
  }

  if (argResults['Command'] != "NONE" && argResults['Application'] != "NONE") {
    print(argResults['Parameters']);
    var rep = json.decode(await d.processCommand(argResults['Command'],
        argResults['Application'], json.decode(argResults['Parameters'])));
    print(rep);
  }
}
