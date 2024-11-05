import 'package:flutter/material.dart';
import 'package:motion_tab_bar/MotionBadgeWidget.dart';
import 'package:motion_tab_bar/MotionTabBar.dart';
import 'package:motion_tab_bar/MotionTabBarController.dart';
import 'package:settings_ui/settings_ui.dart';
import 'package:image_picker/image_picker.dart';
import 'package:flutter/services.dart';
import 'dart:io';

void main() => runApp(const MyApp());

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'PRINTBOT LABS',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: const MyHomePage(title: 'PRINTBOT LABS'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({Key? key, this.title}) : super(key: key);

  final String? title;

  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> with TickerProviderStateMixin {
  // TabController? _tabController;
  MotionTabBarController? _motionTabBarController;

  @override
  void initState() {
    super.initState();
    //// Use normal tab controller
    // _tabController = TabController(
    //   initialIndex: 1,
    //   length: 4,
    //   vsync: this,
    // );

    //// use "MotionTabBarController" to replace with "TabController", if you need to programmatically change the tab
    _motionTabBarController = MotionTabBarController(
      initialIndex: 0,
      length: 2,
      vsync: this,
    );
  }

  @override
  void dispose() {
    super.dispose();
    _motionTabBarController!.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(widget.title!),
        backgroundColor: Color.fromARGB(255, 33, 150, 243),
      ),
      bottomNavigationBar: MotionTabBar(
        controller: _motionTabBarController, // Add this controller if you need to change your tab programmatically
        initialSelectedTab: "Print",
        useSafeArea: true, // default: true, apply safe area wrapper
        labels: const ["Print", "Settings"],
        icons: const [Icons.print, Icons.settings],

        // optional badges, length must be same with labels
        badges: [
          // Default Motion Badge Widget
          // const MotionBadgeWidget(
          //   text: '10+',
          //   textColor: Colors.white, // optional, default to Colors.white
          //   color: Colors.red, // optional, default to Colors.red
          //   size: 18, // optional, default to 18
          // ),
          // Custom badge for Home tab
          // Container(
          //   color: Colors.black,
          //   padding: const EdgeInsets.all(2),
          //   child: const Text(
          //     '50',
          //     style: TextStyle(
          //       fontSize: 14,
          //       color: Colors.white,
          //     ),
          //   ),
          // ),
          null,

          // allow null
          null,

          // // Default Motion Badge Widget with indicator only
          // const MotionBadgeWidget(
          //   isIndicator: true,
          //   color: Colors.red, // optional, default to Colors.red
          //   size: 5, // optional, default to 5,
          //   show: true, // true / false
          // ),
        ],
        tabSize: 50,
        tabBarHeight: 55,
        textStyle: const TextStyle(
          fontSize: 12,
          color: Colors.black,
          fontWeight: FontWeight.w500,
        ),
        tabIconColor: Colors.blue[600],
        tabIconSize: 28.0,
        tabIconSelectedSize: 26.0,
        tabSelectedColor: Colors.blue[900],
        tabIconSelectedColor: Colors.white,
        tabBarColor: Colors.white,
        onTabItemSelected: (int value) {
          setState(() {
            _motionTabBarController!.index = value;
          });
        },
      ),
      body: TabBarView(
        physics: const NeverScrollableScrollPhysics(), // swipe navigation handling is not supported
        controller: _motionTabBarController,
        children: <Widget>[
          MainPageContentComponent(title: "Print Page", controller: _motionTabBarController!),
          SettingsPageContentComponent(title: "Settings Page", controller: _motionTabBarController!),
        ],
      ),
    );
  }
}

class MainPageContentComponent extends StatefulWidget {
  const MainPageContentComponent({
    required this.title,
    required this.controller,
    Key? key,
  }) : super(key: key);

  final String title;
  final MotionTabBarController controller;

  @override
  _MainPageContentComponentState createState() => _MainPageContentComponentState();
}

class _MainPageContentComponentState extends State<MainPageContentComponent> {

  File? image;

  Future pickImage() async {
    try {
      final image = await ImagePicker().pickImage(source: ImageSource.gallery);

      if (image == null) return;

      final imageTemp = File(image.path);

      setState(() => this.image = imageTemp);
    } on PlatformException catch(e) {
      print('Failed to pick image: $e');
    }
  }

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Column(
        //mainAxisAlignment: MainAxisAlignment.center,
        children: [
          const SizedBox(height: 50),
          Text(widget.title, style: const TextStyle(fontSize: 20, fontWeight: FontWeight.bold)),
          const SizedBox(height: 50),
          //const Text('Go to "X" page programmatically'),
          const SizedBox(height: 10),
          IconButton(
            icon: Icon(Icons.image),
            //onPressed: () => controller.index = 0,
            //child: const Text('Upload Image'),
            onPressed: () {
              pickImage();
              // Navigator.push(
              //   context,
              //   MaterialPageRoute(builder: (context) => const UploadImagePage()),
              // );
            }
          ),
          IconButton(
            icon: Icon(Icons.file_download),
            onPressed: () {
              Navigator.push(
                context,
                MaterialPageRoute(builder: (context) => const UploadImagePage()),
              );
            }
          ),
          // ElevatedButton(
          //   onPressed: () => controller.index = 1,
          //   child: const Text('Upload File'),
          // ),
          SizedBox(height: 20,),
          image != null ? Image.file(image!): Text("No Image Selecetd")
        ],
      ),
    );
  }
}

class SettingsPageContentComponent extends StatelessWidget {
  const SettingsPageContentComponent({
    required this.title,
    required this.controller,
    Key? key,
  }) : super(key: key);

  final String title;
  final MotionTabBarController controller;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
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
          ],)
        ],
      ),
    );
  }
}

class UploadImagePage extends StatelessWidget {
  const UploadImagePage({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        //title: const Text('Second Route'),
      ),
      // body: Center(
      //   child: ElevatedButton(
      //     onPressed: () {
      //       Navigator.pop(context);
      //     },
      //     child: const Text('Go back!'),
      //   ),
      //),
    );
  }
}
