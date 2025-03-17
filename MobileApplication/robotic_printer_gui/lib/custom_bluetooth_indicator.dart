import 'package:flutter/material.dart';
import 'bluetooth_service.dart';

class CustomBluetoothIndicator extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 100,  // Ensure space for the circle
      height: 55, // Give it some height
      child: Stack(
        children: [
          Positioned(
            right: 30,  // Adjust position from the right
            top: 30,     // Adjust position from the top
            child: Row(
              mainAxisAlignment: MainAxisAlignment.end,
              children: [
                Container(
                  child: ValueListenableBuilder<String>(
                    valueListenable: TheBluetoothService().statusNotifier,
                    builder: (context, statusNotifier, child) {

                      String percent = statusNotifier.split(',').first;

                      return Container(
                        child: percent == "" 
                          ? Text("") 
                          : Text(
                              '$percent%', 
                              style: TextStyle(
                                fontWeight: FontWeight.bold,  // Makes the text bold
                                fontSize: 14,  // Adjust font size as needed
                                fontFamily: 'Arial',  // You can replace 'Arial' with any font you prefer
                              ),
                            ),
                      );
                    },
                  ),
                ),
                SizedBox(width: 8),
                Container(
                  child: ValueListenableBuilder<bool>(
                    valueListenable: TheBluetoothService().isConnected,
                    builder: (context, isConnected, child) {
                      return Container(
                        width: 20,  // Circle size
                        height: 20,
                        decoration: BoxDecoration(
                          color: isConnected ? Colors.green : Colors.red,
                          shape: BoxShape.circle,
                        ),
                      );
                    },
                  ),
                ),
              ]
            ),
          ),
        ],
      ),
    );
  }
}
