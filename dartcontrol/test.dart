// Import the math library to get access to the sqrt function.
import 'dart:math' as math;
import 'package:http/http.dart' as http;
import 'dart:convert';
//import 'package:mongo_dart/mongo_dart.dart';
import 'mgaccess.dart' as mg;

// Create a class for Point.
class Point {
  // Final variables cannot be changed once they are assigned.
  // Create two instance variables.
  num x, y;

  // A constructor, with syntactic sugar for setting instance variables.
  // The constructor has two mandatory parameters
  Point(this.x, this.y);

  // A named constructor with an initializer list.
  Point.origin()
      : x = 0,
        y = 0;

  // A method.
  num distanceTo(Point other) {
    var dx = x - other.x;
    var dy = y - other.y;
    return math.sqrt(dx * dx + dy * dy);
  }

  // Example of Operator Overloading
  Point operator +(Point other) => Point(x + other.x, y + other.y);
  // When you instantiate a class such as Point in Dart 2+, new is
  // an optional word
}

// All Dart programs start with main().
void main() async {
  // Instantiate point objects.
  var p1 = Point(10, 10);
  var p2 = Point.origin();
  for (var i = 1; i <= 10000000; i++) {
    p1.x = p1.x + 1.0 * math.sqrt(p1.y);
    p1.y = p1.y + 1.0 * math.sqrt(p1.x);

    var distance = p1.distanceTo(p2);
    print(distance);
  }

  var rep = await http.read('http://lyocmsmu04:9999');

  final decoded = json.decode(rep);
//print(decoded);
  decoded.forEach((k, v) => print('${k}: ${v}'));
  int pid = decoded['PID'].toInt();

  print('the pid of the job control is ${pid}');

/*
var url = 'https://example.com/whatsit/create';
var response =  await http.post(url, body: {'name': 'doodle', 'color': 'blue'});
print('Response status: ${response.statusCode}');
print('Response body: ${response.body}');

Db db =
      new Db("mongodb://acqilc:RPC_2008@localhost:27017/LYONROC");
  await db.open();
  print('''
    Data acquistion DB with
    Configuration and ROC asics table
        ''');
  var coll = db.collection('configurations');
  var conf=await coll.find().toList();
  conf.forEach((v) => print("${v['name']} ${v['version']} ${v['comment']}"));

  conf=await coll.find(where.eq("name","daq_4b_4p").eq("version",6)).toList();
  conf.forEach((v) => print(json.encode(v['content'])));
  await db.close();
*/
  mg.MongoAccess m =
      new mg.MongoAccess("mongodb://acqilc:RPC_2008@lyocmsmu04:27017/LYONROC");
  await m.open();
  await m.listConfigurations();
  await m.downloadConfiguration("daq_*", 6);
  await m.close();
}
