import 'package:flutter/material.dart';
import 'package:motion_tab_bar/MotionTabBarController.dart';
import 'package:settings_ui/settings_ui.dart';
import 'debug_page.dart'; 
import 'custom_drawer.dart';

class SettingsPageContentComponent extends StatelessWidget {
  const SettingsPageContentComponent({
    Key? key,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text("Settings"),
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
      ),
      drawer: CustomDrawer(),
      body: SettingsList(
        sections: [
          SettingsSection(
            // title: Text('Settings'),
            tiles: [
              SettingsTile.navigation(
                leading: Icon(Icons.language),
                title: Text('Language'),
                value: Text('English'),
                onPressed: (context) {
                  // Navigate to a new screen or show a dialog
                },
              ),
              SettingsTile.navigation(
                leading: Icon(Icons.brightness_6),
                title: Text('Theme'),
                // value: Text(theme),
                // onPressed: (context) {
                //   _showThemeDialog();
                // },
              ),
              SettingsTile.switchTile(
                onToggle: (value) {},
                initialValue: true,
                leading: Icon(Icons.format_paint),
                title: Text('Enable custom theme'),
              ),
              SettingsTile.navigation(
                leading: Icon(Icons.bug_report),
                title: Text('Debug Terminal'),
                onPressed: (context) {
                  Navigator.push(
                    context,
                    MaterialPageRoute(builder: (context) => DebugPage()),
                  );
                },
              ),
          ],)
        ],
      ),
    );
  }
}