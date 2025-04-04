import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'dart:io';
import 'package:image/image.dart' as img;
import 'package:flutter_blue/flutter_blue.dart';
import 'package:robotic_printer_gui/custom_drawer.dart';
import 'dart:async';
import 'bluetooth_service.dart'; 
import 'custom_bluetooth_indicator.dart';

class StatusPageContentComponent extends StatefulWidget {
  const StatusPageContentComponent({
    Key? key,
  }) : super(key: key);

  @override
  _StatusPageContentComponentState createState() => _StatusPageContentComponentState();
}

class _StatusPageContentComponentState extends State<StatusPageContentComponent> {

  // status = 0 means paused
  // status = 1 means playing
  // status = 2 means stopped
  int status = 0;

  Future<void> sendMessage(int msg) async {
    if (TheBluetoothService().isConnected.value) {
      //setState(() => this.status = msg);

      final characteristicUUID = Guid("48524cb9-9db9-4ce3-b263-85169799a6f3");
      BluetoothCharacteristic? targChar = await TheBluetoothService().charFinder(characteristicUUID);

      if (targChar == null) {
        print("Target Characteristic Not Found");
        return;
      }

      Uint8List bytes = Uint8List.fromList([msg]);

      await targChar.write(bytes, withoutResponse: true);
    }
    else {
      print('No printer connected!');
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(
        content: Text(
          "Not Connected to Printer",
          textAlign: TextAlign.left,
        ),
        duration: Duration(seconds: 3), // Show for 3 seconds
        backgroundColor: Colors.red,
      ));
    }
  }
  
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        //title: Text("Print Page"),
        leading: Builder(
          builder: (context) {
            return IconButton(
              icon: const Icon(Icons.menu),
              onPressed: () {
                Scaffold.of(context).openDrawer();
              },
            );
          },
        ),
        actions: [
          CustomBluetoothIndicator(),
        ],
      ),
      drawer: CustomDrawer(),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          crossAxisAlignment: CrossAxisAlignment.center, // Centers content horizontally
          children: [
            ValueListenableBuilder<String>(
              valueListenable: TheBluetoothService().statusNotifier,
              builder: (context, statusNotifier, child) {
                List<String> values = statusNotifier.split(',');
                String progress;
                progress = values.length == 3 ? values[1] : "-1";
                status = values.length == 3 ? int.parse(values.last) : 0;
                //status = int.parse(values.last);
                return Column (
                  children: [
                    SizedBox(
                      width: (MediaQuery.of(context).size.width) * 0.85,
                      height: 10,
                      child: LinearProgressIndicator(
                        minHeight: 5,
                        backgroundColor: Colors.black,
                        value: double.tryParse(progress) != null && double.parse(progress) >= 0 && double.parse(progress) <= 100 ? double.parse(progress) / 100 : 0,
                        color: Colors.green,
                      ),
                    ),

                    SizedBox(height: 5),

                    Row (
                      mainAxisAlignment: MainAxisAlignment.center,
                      crossAxisAlignment: CrossAxisAlignment.center,
                      children: [
                        IconButton(
                          iconSize: 30,
                          color: status != 1 && double.tryParse(progress) != null && double.parse(progress) != -1 ? Colors.black : Colors.grey,
                          onPressed: status != 1 && double.tryParse(progress) != null && double.parse(progress) != -1 ? () {sendMessage(1);} : null,
                          icon: Icon(Icons.play_arrow)
                        ),
                        //SizedBox(width: 5),
                        IconButton(
                          iconSize: 30,
                          color: status == 1 && double.tryParse(progress) != null && double.parse(progress) != -1 ? Colors.black : Colors.grey,
                          onPressed: status == 1 && double.tryParse(progress) != null && double.parse(progress) != -1 ? () {sendMessage(0);} : null,
                          icon: Icon(Icons.pause)
                        ),
                        SizedBox(width: 140),
                        IconButton(
                          iconSize: 30,
                          color: status != 2 && double.tryParse(progress) != null && double.parse(progress) > 0 ? Colors.red : Colors.grey,
                          onPressed: status != 2 && double.tryParse(progress) != null && double.parse(progress) != -1 ? () {sendMessage(2);} : null, 
                          icon: Icon(Icons.stop)
                        ),
                      ],
                    ),
                  ],
                );
              },
            ),
          ]
        ),
      )
    );
  }
}