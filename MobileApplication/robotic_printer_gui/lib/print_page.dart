import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'dart:io';
import 'package:image/image.dart' as img;
import 'package:flutter_blue/flutter_blue.dart';
import 'package:robotic_printer_gui/custom_drawer.dart';
import 'dart:async';
import 'bluetooth_service.dart'; 

class PrintPageContentComponent extends StatefulWidget {
  const PrintPageContentComponent({
    required this.printImage,

    Key? key,
  }) : super(key: key);

  final File printImage;

  @override
  _PrintPageContentComponentState createState() => _PrintPageContentComponentState();
}

class _PrintPageContentComponentState extends State<PrintPageContentComponent> {

  File? image;
  File? displayedImage;
  int? widthtext;
  int? heighttext;
  int pagewidth = 796;
  int pageheight = 1123;

  void initState() {
    super.initState();

    image = widget.printImage;
    displayedImage = widget.printImage;
    final tst = img.decodeImage(image!.readAsBytesSync());
    widthtext = tst!.width;
    heighttext = tst.height;
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

    return Uint8List.fromList(byteArray);
  }

  Future<void> printImage() async {
    if (!TheBluetoothService().isConnected.value) {
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
      final characteristicUUID = Guid("beb5483e-36e1-4688-b7f5-ea07361b26a8");

      BluetoothCharacteristic? targChar = await TheBluetoothService().charFinder(characteristicUUID);

      Uint8List byteArray = await blackAndWhite();

      if (targChar == null) {
        print("Target Characteristic Not Found");
        return;
      }

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
    return Scaffold(
      appBar: AppBar(
        title: Text("Print Page"),
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
      body: Center(
        child: Column(
          //mainAxisAlignment: MainAxisAlignment.center,
          children: [
            const SizedBox(height: 20),

            Container(
              padding: EdgeInsets.symmetric(vertical: 10, horizontal: 25), // Add padding for text inside the box
              child: ValueListenableBuilder<bool>(
                valueListenable: TheBluetoothService().isConnected,
                builder: (context, isConnected, child) {
                  return Container(
                    decoration: BoxDecoration(
                      color: isConnected ? Colors.green : Colors.red, // Green when connected, red when not
                      borderRadius: BorderRadius.circular(5), // Rounded corners
                    ),
                    // child: Text(
                    //   isConnected ? "Connected to Printer" : "Not Connected to Printer",
                    //   style: TextStyle(
                    //     fontSize: 20,
                    //     fontWeight: FontWeight.bold,
                    //     color: Colors.white, // Text color will be white for contrast
                    //   ),
                    // ),
                  );
                },
              ),
            ),

            //Text(targetDevice != null ? "Connected to Printer" : "Not Connected to Printer", 
            //style: const TextStyle(fontSize: 20, fontWeight: FontWeight.bold)),

            const SizedBox(height: 20),

            const SizedBox(height: 20),

            IconButton(
              icon: Icon(Icons.image),
              onPressed: () {
                // pickImage();
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
      ),
    );
  }
}