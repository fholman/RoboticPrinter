import 'package:flutter/material.dart';
import 'package:motion_tab_bar/MotionTabBar.dart';
import 'package:motion_tab_bar/MotionTabBarController.dart';
import 'package:settings_ui/settings_ui.dart';
import 'package:image_picker/image_picker.dart';
import 'package:flutter/services.dart';
import 'dart:io';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:image/image.dart' as img;
import 'package:permission_handler/permission_handler.dart';

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
  File? displayedImage;
  int? widthtext;
  int? heighttext;
  int pagewidth = 796;
  int pageheight = 1123;

  Future pickImage() async {
    try {
      final image = await ImagePicker().pickImage(source: ImageSource.gallery);

      if (image == null) return;

      final imageTemp = File(image.path);
      
      final originalImage = img.decodeImage(imageTemp.readAsBytesSync());

      if (originalImage != null) {

        img.Image rescaledImage = img.copyResize(originalImage, width:200);

        rescaledImage = img.grayscale(rescaledImage);

        final directory = await Directory.systemTemp.createTemp(); // Temp directory
        final processedImagePath = '${directory.path}/processed_image.png';
        File(processedImagePath).writeAsBytesSync(img.encodePng(rescaledImage));
        setState(() => this.displayedImage = File(processedImagePath));

        final ogdirectory = await Directory.systemTemp.createTemp(); // Temp directory
        final ogprocessedImagePath = '${ogdirectory.path}/processed_image.png';
        File(ogprocessedImagePath).writeAsBytesSync(img.encodePng(originalImage));
        setState(() => this.image = File(ogprocessedImagePath));

        setState(() => this.widthtext = rescaledImage.width);
        setState(() => this.heighttext = rescaledImage.height);

      }

    } on PlatformException catch(e) {
      print('Failed to pick image: $e');
    }
  }

  Future rescaleImage(File? im, bool up) async {
    if (widthtext == null) return null;
    if (heighttext == null) return null;

    if (widthtext! < pagewidth && heighttext! < pageheight) {

      if (im == null) return null;
      try {
        final bytes = await im.readAsBytes();
        final originalImage = img.decodeImage(bytes);

        if (originalImage == null) return null;

        img.Image rescaledImage = img.copyResize(originalImage, width:widthtext, height:heighttext);

        if (up) {
          rescaledImage = img.copyResize(originalImage, width:rescaledImage.width + 50);
        }
        else {
          rescaledImage = img.copyResize(originalImage, width:rescaledImage.width - 50);
        }

        rescaledImage = img.grayscale(rescaledImage);

        final directory = await Directory.systemTemp.createTemp(); // Temp directory
        final processedImagePath = '${directory.path}/processed_image.png';
        File(processedImagePath).writeAsBytesSync(img.encodePng(rescaledImage));

        setState(() => this.displayedImage = File(processedImagePath));
        setState(() => this.widthtext = rescaledImage.width);
        setState(() => this.heighttext = rescaledImage.height);

      } on PlatformException catch(e) {
        print('Failed to pick image: $e');
      }
    }
  }

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Column(
        //mainAxisAlignment: MainAxisAlignment.center,
        children: [
          // const SizedBox(height: 50),
          // Text(widget.title, style: const TextStyle(fontSize: 20, fontWeight: FontWeight.bold)),
          const SizedBox(height: 50),
          ElevatedButton(
            //  callback function that gets called when the user presses the button

            onPressed: () {
              Navigator.push(
                context,
                MaterialPageRoute(builder: (context) => BluetoothPage()),
              );
            },
            
            // Text to be displayed on the button
            child: Text("Connect to Printer"),
          ),
          const SizedBox(height: 50),
          IconButton(
            icon: Icon(Icons.image),
            onPressed: () {
              pickImage();
            }
          ),
          SizedBox(height: 20,),
          displayedImage != null ? Image.file(displayedImage!): Text("No Image Selected"),
          SizedBox(height: 20,),

          widthtext != null ? Text(this.widthtext.toString() + " x " + this.heighttext.toString()): Text(""),

          Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              ElevatedButton(
                onPressed: () {
                  //image != null ? rescaleImage(image) : null;
                  rescaleImage(image, true);
                },
                child: Text("Increase Size"),
              ),

              ElevatedButton(
                onPressed: () {
                  //image != null ? rescaleImage(image) : null;
                  rescaleImage(image, false);
                },
                child: Text("Decrease Size"),
              ),
            ],
          ),

          const ElevatedButton(
            onPressed: null,
            child: Text("Print"),
          ),
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

class BluetoothPage extends StatefulWidget {
  @override
  _BleScannerState createState() => _BleScannerState();
}

class _BleScannerState extends State<BluetoothPage> {
  FlutterBluePlus flutterBlue = FlutterBluePlus();
  List<BluetoothDevice> devices = [];  // List to store the discovered devices
  bool isScanning = false;

  @override
  void initState() {
    super.initState();
  }

  void startScanning() async {
    PermissionStatus status = await Permission.location.request();
    if (!status.isGranted) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Location permission is required to scan for devices')),
      );
      return;
    }

    if (!isScanning) {
      setState(() {
        isScanning = true;
        devices.clear();  // Clear previous devices
      });

      FlutterBluePlus.startScan(timeout: Duration(seconds: 10));

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


