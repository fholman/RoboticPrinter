import 'package:flutter/material.dart';
import 'package:image_picker/image_picker.dart';
import 'package:flutter/services.dart';
import 'dart:io';
import 'package:image/image.dart' as img;
import 'package:robotic_printer_gui/custom_drawer.dart';
import 'dart:async';
import 'bluetooth_service.dart'; 
import 'custom_bluetooth_indicator.dart';
import 'print_page.dart';

class MainPageContentComponent extends StatefulWidget {
  const MainPageContentComponent({
    //required this.title,
    //required this.controller,
    Key? key,
  }) : super(key: key);

  //final String title;
  //final MotionTabBarController controller;

  @override
  _MainPageContentComponentState createState() => _MainPageContentComponentState();
}

class _MainPageContentComponentState extends State<MainPageContentComponent> {

  File? image;
  File? displayedImage;
  int? widthtext;
  int? heighttext;
  // int pagewidth = 796;
  // int pageheight = 1123;
  //ScanResult? targetDevice= TheBluetoothService().getDevice();

  void initState() {
    super.initState();

    TheBluetoothService().checkPermissions();
  }

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

        File imageFile = File(image.path);

        Navigator.push(
          context,
          MaterialPageRoute(builder: (context) => PrintPageContentComponent(printImage: imageFile)),
        );

      }

    } on PlatformException catch(e) {
      print('Failed to pick image: $e');
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(""),
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
          mainAxisAlignment: MainAxisAlignment.center, // Centers content vertically
          crossAxisAlignment: CrossAxisAlignment.center, // Centers content horizontally
          children: [
            IconButton(
              //iconSize: 60,
              icon: Image.asset(
                'assets/icons/imageButton.png', // Replace with your actual image path
                width: 200, 
                height: 200, 
              ),
              onPressed: () {
                pickImage();
              }
            ),
          ]
        ),
      ),
    );
  }
}