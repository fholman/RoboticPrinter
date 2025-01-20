import 'package:flutter_blue/flutter_blue.dart';
import 'package:permission_handler/permission_handler.dart';
import 'dart:async';
import 'package:flutter/material.dart';

class TheBluetoothService {
  static final TheBluetoothService _singleton = TheBluetoothService._internal();
  ScanResult? connectedDevice;
  final ValueNotifier<bool> isConnected = ValueNotifier<bool>(false);

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
      //await Permission.bluetooth.request().isGranted &&
      await Permission.bluetoothAdvertise.request().isGranted
    ) {
      startScanningAndConnect();
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
}