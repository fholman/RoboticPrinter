import 'package:flutter_blue/flutter_blue.dart';
import 'package:permission_handler/permission_handler.dart';
import 'dart:async';
import 'package:flutter/material.dart';

class TheBluetoothService {
  static final TheBluetoothService _singleton = TheBluetoothService._internal();
  ScanResult? connectedDevice;
  final ValueNotifier<bool> isConnected = ValueNotifier<bool>(false);
  ValueNotifier<List<String>> entriesNotifier = ValueNotifier<List<String>>([]);
  StreamSubscription? debugSubscription;
  StreamSubscription? statusSubscription;
  DateTime? previous;

  factory TheBluetoothService() {
    return _singleton;
  }

  TheBluetoothService._internal();

  // Set the connected device
  void setDevice(ScanResult? device) {
    connectedDevice = device;
  }

  // Get the connected device
  ScanResult? getDevice() {
    return connectedDevice;
  }

  void checkPermissions() async {
    if (
      await Permission.location.request().isGranted &&
      await Permission.bluetoothScan.request().isGranted &&
      await Permission.bluetoothConnect.request().isGranted &&
      await Permission.bluetoothAdvertise.request().isGranted
    ) {
      if (getDevice() == null) {
        startScanningAndConnect();
      }
    }
  }

  void startScanningAndConnect() {
    FlutterBlue flutterBlue = FlutterBlue.instance;

    Future<void> scanAndConnect() async {
      print('Starting scan...');
      await flutterBlue.startScan(
        scanMode: ScanMode.lowPower,
        allowDuplicates: false,
        timeout: Duration(seconds: 4),
      );

      // Listen for scan results
      await for (List<ScanResult> results in flutterBlue.scanResults) {
        for (ScanResult r in results) {
          if (r.device.name == 'RoboPrinter') {
            print('Found RoboPrinter, attempting to connect...');
            await flutterBlue.stopScan(); // Stop scanning when device is found

            try {
              await r.device.connect();
              print('Connected to ${r.device.name}');

              try {
                await r.device.requestMtu(255);
                print('MTU set to 255');
              } catch (mtuError) {
                print('Failed to set MTU: $mtuError');
              }

              connectedDevice = r; // Store the connected device
              isConnected.value = true;
              debugListener();
              statusListener();
              return; // Exit the function upon successful connection
            } catch (connectionError) {
              print('Connection failed: $connectionError');
              // Retry scanning if connection fails
            }
          }
        }

        // If the device wasn't found in the current batch, restart scanning
        print('Target device not found, restarting scan...');
        isConnected.value = false;
        await flutterBlue.stopScan(); // Stop the current scan
        await Future.delayed(Duration(seconds: 1)); // Optional delay before restarting
        await scanAndConnect(); // Recursively call the function to scan again
        return; // Ensure proper exit after the recursive call
      }
    }

    scanAndConnect(); // Start the scanning process
  }

  Future<void> statusListener() async {
    try {
      // Listen for the connection state changes
      statusSubscription = connectedDevice!.device.state.listen((state) {
        if (state == BluetoothDeviceState.connected) {
          print("Device is connected.");
          isConnected.value = true;
          if (debugSubscription == null) {
            debugListener();
          }
        } else if (state == BluetoothDeviceState.disconnected) {
          print("Device is disconnected.");
          isConnected.value = false;
          //statusSubscription!.cancel();
          if (debugSubscription != null) {
            debugSubscription!.cancel();
            debugSubscription = null;
          }
        }
      });
    } catch (e) {
      print('Failed to connect: $e');
    }
  }

  Future<void> debugListener() async {
    BluetoothCharacteristic? c = await charFinder(Guid("64f90866-d4bb-493d-bd01-e532e4e34021"));

    if (c != null) {

      await c.setNotifyValue(true);

      try {
        // Listen for changes to the characteristic value
        debugSubscription = c.value.listen(
          (value) {
            DateTime now = DateTime.now();
            String message = "${now.hour.toString().padLeft(2, '0')}:${now.minute.toString().padLeft(2, '0')}:${now.second.toString().padLeft(2, '0')} - ${String.fromCharCodes(value)}";
            if (String.fromCharCodes(value) != "") {
              entriesNotifier.value = [message] + List.from(entriesNotifier.value);
            }
            print(message);  // Convert byte data to string
          },
          onError: (error) {
            print("Error: $error");  // Handle error
          },
        );
      } catch (e) {
        print("Error setting notify value: $e");
      }

    }
  }

  Future<BluetoothCharacteristic?> charFinder(final char) async {
    BluetoothCharacteristic? targChar;

    final serviceUUID = Guid("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
    final characteristicUUID2 = char;

    try {
      List<BluetoothService> services = await connectedDevice!.device.discoverServices();

      for (BluetoothService service in services) {
        if (service.uuid == serviceUUID) {
          var characteristics = service.characteristics;
          for(BluetoothCharacteristic c in characteristics) {
            print(c.uuid);
            if (c.uuid == characteristicUUID2) {
              targChar = c;
              print("Found Target Characteristic");
              return targChar;
            }
          }
        }
        if (targChar != null) break;
      }

    } catch (e) {
      print("Error during service discovery: $e");
    }

    print("Target Characteristic Not Found");
    return null;
    
  }
}