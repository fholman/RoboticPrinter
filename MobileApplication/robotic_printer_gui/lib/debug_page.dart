import 'package:flutter/material.dart';
import 'bluetooth_service.dart'; 

class DebugPage extends StatefulWidget {
  @override
  _DebugPageState createState() => _DebugPageState();
}

class _DebugPageState extends State<DebugPage> {

   @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Debug Terminal'),
      ),
      body: ValueListenableBuilder<List<String>>(
        valueListenable: TheBluetoothService().entriesNotifier,
        builder: (context, entries, child) {
          return ListView.separated(
            padding: const EdgeInsets.all(8),
            itemCount: entries.length,
            itemBuilder: (BuildContext context, int index) {
              return Container(
                height: 50,
                child: Text(entries[index]),
              );
            },
            separatorBuilder: (BuildContext context, int index) => const Divider(),
          );
        },
      ),
    );
  }
}