import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'ESP32 LED Control',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: MyHomePage(),
    );
  }
}

class MyHomePage extends StatefulWidget {
  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  final String esp32Ip = 'http://192.168.4.1';  // ESP32 IP address (change if necessary)

  // Function to send a request to turn the LED on (1) or off (0)
  Future<void> sendValue(int value) async {
    try {
      // Construct the URL to send the request to ESP32
      final response = await http.get(Uri.parse('$esp32Ip/$value'));

      if (response.statusCode == 200) {
        // Print the response (LED status)
        print('Response: ${response.body}');
      } else {
        print('Failed to send value');
      }
    } catch (e) {
      print('Error: $e');
    }
  }

  // Build the UI with two buttons to control the LED
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('ESP32 LED Control'),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            ElevatedButton(
              onPressed: () => sendValue(1),  // Send "1" to turn LED on
              child: Text('Turn LED ON'),
            ),
            ElevatedButton(
              onPressed: () => sendValue(0),  // Send "0" to turn LED off
              child: Text('Turn LED OFF'),
            ),
          ],
        ),
      ),
    );
  }
}