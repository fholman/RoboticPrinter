import 'package:flutter/material.dart';
import 'package:motion_tab_bar/MotionTabBar.dart';
import 'package:motion_tab_bar/MotionTabBarController.dart';
import 'package:settings_ui/settings_ui.dart';
import 'package:image_picker/image_picker.dart';
import 'package:flutter/services.dart';
import 'dart:io';
import 'package:image/image.dart' as img;
import 'package:permission_handler/permission_handler.dart';
import 'package:flutter_blue/flutter_blue.dart';
import 'package:path_provider/path_provider.dart';

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
  ScanResult? targetDevice;

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
    if (im == null) {
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(
        content: Text(
          "No Image Selected",
          textAlign: TextAlign.right,
        ),
        duration: Duration(seconds: 3), // Show for 3 seconds
        backgroundColor: Colors.red,
      ));
      return null;
    }

    if (widthtext == null) return null;
    if (heighttext == null) return null;

    if (widthtext! < pagewidth && heighttext! < pageheight) {
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

  Future<void> connectPrinter() async {
    PermissionStatus status = await Permission.location.request();
    List<ScanResult> scanResults = [];
    FlutterBlue flutterBlue = FlutterBlue.instance;

    ScaffoldMessenger.of(context).showSnackBar(SnackBar(
      content: Text(
        "Connecting to Printer",
        textAlign: TextAlign.right,
      ),
      duration: Duration(seconds: 3), // Show for 3 seconds
      backgroundColor: Colors.blue,
    ));

    if (!status.isGranted) {
      return;
    }

    try {
      scanResults = await flutterBlue.startScan(
        scanMode: ScanMode.lowLatency,
        allowDuplicates: false,
        timeout: Duration(seconds: 4)
      ) as List<ScanResult>; // Ensure the type matches
    } catch (e) {
      print("Error while scanning: $e");
    }

    // Now `scanResults` is correctly populated with a `List<ScanResult>`
    print(scanResults);

    ScanResult targetDevice;
    bool fndDevice = false;

    for (ScanResult scanResult in scanResults) {
      if (scanResult.device.name == 'RoboPrinter') {
        targetDevice = scanResult;
        setState(() => this.targetDevice = scanResult);
        await targetDevice.device.connect();
        print('Connected to ${targetDevice.device.name}');
        fndDevice = true;

        try {
          await targetDevice.device.requestMtu(255);
          print('MTU set to 255');
        } catch (e) {
          print('Failed to set MTU: $e');
        }

        break;
      }
    }

    if (!fndDevice) {
      print('Device RoboPrinter not found');
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(
        content: Text(
          "Could not Connect to Printer",
          textAlign: TextAlign.right,
        ),
        duration: Duration(seconds: 3), // Show for 3 seconds
        backgroundColor: Colors.red,
      ));
    }

    flutterBlue.stopScan();
  }

  Future<void> disconnectPrinter() async {
    if (targetDevice != null) {
      await targetDevice!.device.disconnect();
      print('Disconnected from ${targetDevice!.device.name}');
      setState(() => this.targetDevice = null);
    }
    else {
      print("No Device Connected!");
    }
  }

  Future<Uint8List> blackAndWhite() async {
    List<int> imageBytes = await displayedImage!.readAsBytes();

    img.Image? image = img.decodeImage(Uint8List.fromList(imageBytes));

    if (image == null) {
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(
        content: Text(
          "Image Error",
          textAlign: TextAlign.right,
        ),
        duration: Duration(seconds: 3), // Show for 3 seconds
        backgroundColor: Colors.red,
      ));
      throw Exception('Failed to decode image.');
    }

    //print(len(image));

    // img.Pixel pixel = image.getPixel(0, 0);
    // print(pixel[0]);
    // print(pixel[1]);
    // print(pixel[2]);
    // print(pixel);

    List<int> byteArray = [];

    // white = 1
    // black = 0
    int threshold = 128;
    int pixelCounter = 0;
    int numOfPixels = image.height * image.width;

    for (int y = 0; y < image.height; y++) {
      for (int x = 0; x < image.width; x++) {
        img.Pixel pixel = image.getPixel(x, y);
        pixelCounter += pixel[0].toInt();
      }
    }

    threshold = (pixelCounter / numOfPixels).toInt();

    print(threshold);

    int width = (image.width + 7) ~/ 8;
    int lowByte = width & 0xFF;        // Extract the least significant byte
    int highByte = (width >> 8) & 0xFF; // Extract the most significant byte
    byteArray.add(highByte);
    byteArray.add(lowByte);

    print(highByte);
    print(lowByte);

    for (int y = 0; y < image.height; y++) {
      int byte = 0;
      for (int x = 0; x < image.width; x++) {
        img.Pixel pixel = image.getPixel(x, y);
        int color = pixel[0].toInt();
        byte = byte << 1;  // Shift the bits to the left
        if (color < threshold) {
          byte |= 1;  // If the pixel is black, set the last bit to 1
        }

        // Every 8 pixels, push the byte into the byte array
        if ((x + 1) % 8 == 0) {
          byteArray.add(byte);
          //print(byte.toRadixString(2).padLeft(8, '0'));
          byte = 0;  // Reset the byte for the next 8 pixels
        }
        else {
          if (x == image.width - 1) {
            byte = byte << 8 - (image.width % 8);
            byteArray.add(byte);
            //print(byte.toRadixString(2).padLeft(8, '0'));
          }
        }
      }
    }

    int byteArraySize = (byteArray.length-2);

    List<int> sizeBytes = [
      (byteArraySize >> 24) & 0xFF,  // Most significant byte
      (byteArraySize >> 16) & 0xFF,
      (byteArraySize >> 8) & 0xFF,
      byteArraySize & 0xFF           // Least significant byte
    ];

    // Prepend the size bytes to the byteArray
    byteArray.insertAll(0, sizeBytes);

    // print(byteArray.length);
    // print(byteArray);

    // int chunkSize = 200;

    // for (int i = 0; i < byteArray.length; i += chunkSize) {
    //   // Get the next chunk of size 'chunkSize', but ensure we don't go past the end of the list
    //   int end = (i + chunkSize < byteArray.length) ? i + chunkSize : byteArray.length;
    //   List<int> chunk = byteArray.sublist(i, end);
      
    //   // Print the chunk
    //   print('Chunk starting at index $i: $chunk');
    // }

    // Return the byte array
    return Uint8List.fromList(byteArray);
  }

  Future<void> printImage() async {
    if (targetDevice == null) {
      print('No printer connected!');
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(
        content: Text(
          "Not Connected to Printer",
          textAlign: TextAlign.right,
        ),
        duration: Duration(seconds: 3), // Show for 3 seconds
        backgroundColor: Colors.red,
      ));
      return;
    }

    if (image == null) {
      print("No Image Selected!");
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(
        content: Text(
          "No Image Selected",
          textAlign: TextAlign.right,
        ),
        duration: Duration(seconds: 3), // Show for 3 seconds
        backgroundColor: Colors.red,
      ));
      return;
    }

    try {
      BluetoothCharacteristic? targChar;

      final serviceUUID = Guid("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
      final characteristicUUID = Guid("beb5483e-36e1-4688-b7f5-ea07361b26a8");

      List<BluetoothService> services = await targetDevice!.device.discoverServices();

      Uint8List byteArray = await blackAndWhite();

      for (BluetoothService service in services) {
        if (service.uuid == serviceUUID) {
          var characteristics = service.characteristics;
          for(BluetoothCharacteristic c in characteristics) {
            if (c.uuid == characteristicUUID) {
              targChar = c;
              print("Found Target Characteristic");
              break;
            }
          }
        }
        if (targChar != null) break;
      }

      if (targChar == null) {
        print("Target Characteristic Not Found");
        return;
      }

      // print(byteArray[0]);
      // print(byteArray[1]);
      // print(byteArray[2]);
      // print(byteArray[3]);
      // print(byteArray[4]);
      // print(byteArray[5]);
      // print(byteArray.sublist(0, 5));

      await targChar.write(byteArray.sublist(0, 4), withoutResponse: true);
      await Future.delayed(Duration(milliseconds: 50));

      await targChar.write(byteArray.sublist(4, 6), withoutResponse: true);
      await Future.delayed(Duration(milliseconds: 50));

      int chunkSize = 250;

      for (int i = 6; i < byteArray.length; i += chunkSize) {
        // Get the next chunk of size 'chunkSize', but ensure we don't go past the end of the list
        int end = (i + chunkSize < byteArray.length) ? i + chunkSize : byteArray.length;
        List<int> chunk = byteArray.sublist(i, end);
        
        // Print the chunk
        print('Chunk starting at index $i: $chunk');

        await targChar.write(chunk, withoutResponse: true);
        await Future.delayed(Duration(milliseconds: 50));
      }

    }
    catch (e) {
      print("Error");
    }
  }


  @override
  Widget build(BuildContext context) {
    return Center(
      child: Column(
        //mainAxisAlignment: MainAxisAlignment.center,
        children: [
          const SizedBox(height: 20),

          Container(
              padding: EdgeInsets.symmetric(vertical: 10, horizontal: 20),  // Add padding for text inside the box
              decoration: BoxDecoration(
                color: targetDevice != null ? Colors.green : Colors.red,  // Green when connected, red when not
                borderRadius: BorderRadius.circular(10),  // Rounded corners
              ),
              child: Text(
                targetDevice != null ? "Connected to Printer" : "Not Connected to Printer",
                style: TextStyle(
                  fontSize: 20,
                  fontWeight: FontWeight.bold,
                  color: Colors.white,  // Text color will be white for contrast
                ),
              ),
            ),

          //Text(targetDevice != null ? "Connected to Printer" : "Not Connected to Printer", 
          //style: const TextStyle(fontSize: 20, fontWeight: FontWeight.bold)),

          const SizedBox(height: 20),

          ElevatedButton(
            onPressed: () {
              //image != null ? rescaleImage(image) : null;
              if (targetDevice == null) {
                connectPrinter();
              }
              else {
                disconnectPrinter();
              }
            },
            child: Text(targetDevice == null ? "Connect" : "Disconnect"),
          ),

          const SizedBox(height: 20),

          IconButton(
            icon: Icon(Icons.image),
            onPressed: () {
              pickImage();
            }
          ),

          SizedBox(height: 20),

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

              SizedBox(width: 20),

              ElevatedButton(
                onPressed: () {
                  //image != null ? rescaleImage(image) : null;
                  rescaleImage(image, false);
                },
                child: Text("Decrease Size"),
              ),
            ],
          ),

          IconButton(
            icon: Icon(Icons.print),
            //onPressed: null,
            onPressed: () {
              printImage();
            },
            //child: Text("Print"),
          ),

          SizedBox(height: 20),
          
          displayedImage != null ? Image.file(displayedImage!): Text("No Image Selected"),

          widthtext != null ? Text(this.widthtext.toString() + " x " + this.heighttext.toString()): Text(""),


          Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              ElevatedButton(
                onPressed: null,
                child: Text("Start Printing"),
              ),

              SizedBox(width: 20),

              ElevatedButton(
                onPressed: null,
                child: Text("Stop Printing"),
              ),
            ],
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

class DebugPage extends StatefulWidget {
  @override
  _DebugPageState createState() => _DebugPageState();
}

class _DebugPageState extends State<DebugPage> {
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Debug Terminal'),
      ),
      body: Center(
        child: Text('Welcome to the New Page!'),
      ),
    );
  }
}