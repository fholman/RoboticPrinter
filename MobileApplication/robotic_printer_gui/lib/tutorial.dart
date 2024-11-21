import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: BleScanner(),
    );
  }
}

class BleScanner extends StatefulWidget {
  @override
  _BleScannerState createState() => _BleScannerState();
}

class _BleScannerState extends State<BleScanner> {
  FlutterBluePlus flutterBlue = FlutterBluePlus();
  List<BluetoothDevice> devices = [];  // List to store the discovered devices
  bool isScanning = false;

  @override
  void initState() {
    super.initState();
  }

  void startScanning() async {
    if (!isScanning) {
      setState(() {
        isScanning = true;
        devices.clear();  // Clear previous devices
      });

      FlutterBluePlus.startScan(timeout: Duration(seconds: 4));

      FlutterBluePlus.scanResults.listen((results) {
        for (ScanResult result in results) {
          if (!devices.contains(result.device)) {
            setState(() {
              devices.add(result.device);  // Add new device to list
            });
          }
        }
      });

      // Stop scanning after the timeout
      FlutterBluePlus.stopScan();
      setState(() {
        isScanning = false;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('BLE Device Scanner'),
        actions: [
          IconButton(
            icon: Icon(Icons.search),
            onPressed: startScanning,
          ),
        ],
      ),
      body: isScanning
          ? Center(child: CircularProgressIndicator())  // Show progress while scanning
          : ListView.builder(
              itemCount: devices.length,
              itemBuilder: (context, index) {
                return ListTile(
                  title: Text(devices[index].name.isEmpty
                      ? "Unnamed Device"
                      : devices[index].name),
                  subtitle: Text(devices[index].id.toString()),
                  onTap: () {
                    // You can handle tap to connect or show more info about the device
                  },
                );
              },
            ),
    );
  }

  @override
  void dispose() {
    super.dispose();
    FlutterBluePlus.stopScan();  // Stop scanning when the widget is disposed
  }
}
