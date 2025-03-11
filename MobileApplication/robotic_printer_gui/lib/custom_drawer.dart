import 'package:flutter/material.dart';
import 'main_page.dart';
import 'settings.dart';
import 'debug_page.dart';

class CustomDrawer extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Drawer(
        width: 90,
        backgroundColor: Colors.white,
        child: ListView(
          padding: EdgeInsets.zero,
          children: [
            SizedBox(height: 50), // Space at the top
            IconButton(
              icon: Icon(Icons.print), 
              onPressed: () {
                Navigator.push(
                  context,
                  MaterialPageRoute(builder: (context) => MainPageContentComponent()),
                );
              }),
            IconButton(
              icon: Icon(Icons.bug_report), 
              onPressed: () {
                Navigator.push(
                    context,
                    MaterialPageRoute(builder: (context) => DebugPage()),
                  );
              }),
          ],
        ),
      );
  }
}
