import 'package:flutter/material.dart';
import 'bluetooth_service.dart';

class CustomBluetoothIndicator extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 50,  // Ensure space for the circle
      height: 50, // Give it some height
      child: Stack(
        children: [
          Positioned(
            right: 30,  // Adjust position from the right
            top: 30,     // Adjust position from the top
            child: Container(
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
          ),
        ],
      ),
    );
  }
}
