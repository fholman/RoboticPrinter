import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'dart:io';
import 'package:image/image.dart' as img;
import 'package:flutter_blue/flutter_blue.dart';
import 'package:robotic_printer_gui/custom_drawer.dart';
import 'dart:async';
import 'bluetooth_service.dart'; 
import 'custom_bluetooth_indicator.dart';
import 'status_page.dart';

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

  img.Image? image; // original image
  File? displayedImage; // file to be displayed and printed
  int? widthtext;
  int? heighttext;
  // int pagewidth = 796;
  // int pageheight = 1123;

  double? portraitHeight;
  double? sheetHeight;
  double? sheetWidth;

  bool isPortrait = true;

  String _selectedText = "A5";

  @override
  void initState() {
    super.initState();

    setupImage();
  }

  @override
  void didChangeDependencies() {
    super.didChangeDependencies();
    
    sheetHeight = MediaQuery.of(context).size.height - 300;
    portraitHeight = sheetHeight;
    sheetWidth = sheetHeight! * 0.707;

    resizeImage();
  }

  Future setupImage() async {
    final originalImage = img.decodeImage(widget.printImage.readAsBytesSync());

    if (originalImage != null) {

      img.Image rescaledImage = img.copyResize(originalImage, width:300);

      final directory = await Directory.systemTemp.createTemp(); // Temp directory
      final processedImagePath = '${directory.path}/processed_image.png';
      File(processedImagePath).writeAsBytesSync(img.encodePng(rescaledImage));
      setState(() => this.displayedImage = File(processedImagePath));

      setState(() => this.widthtext = rescaledImage.width);
      setState(() => this.heighttext = rescaledImage.height);

      image = img.decodeImage(widget.printImage.readAsBytesSync());
    }
  }

  Future resizeImage() async {
    if (widthtext == null) return null;
    if (heighttext == null) return null;

    try {

      if (image == null) return null;

      img.Image rescaledImage = img.copyResize(image!, width:widthtext, height:heighttext);

      bool update = false;

      if (widthtext! > sheetWidth!) {
        rescaledImage = img.copyResize(image!, width:sheetWidth!.toInt());
        widthtext = rescaledImage.width;
        heighttext = rescaledImage.height;
        update = true;
      }
      if (heighttext! > sheetHeight!) {
        rescaledImage = img.copyResize(image!, height:sheetHeight!.toInt());
        widthtext = rescaledImage.width;
        heighttext = rescaledImage.height;
        update = true;
      }

      if (update) {
        final directory = await Directory.systemTemp.createTemp(); // Temp directory
        final processedImagePath = '${directory.path}/processed_image.png';
        File(processedImagePath).writeAsBytesSync(img.encodePng(rescaledImage));

        setState(() => this.displayedImage = File(processedImagePath));
        setState(() => this.widthtext = rescaledImage.width);
        setState(() => this.heighttext = rescaledImage.height);
      }

    } on PlatformException catch(e) {
      print('Failed to pick image: $e');
    }
  }

  Future rescaleImage(bool up) async {
    if (widthtext == null) return null;
    if (heighttext == null) return null;

    try {

      if (image == null) return null;

      img.Image rescaledImage = img.copyResize(image!, width:widthtext, height:heighttext);

      bool update = false;

      if (up) {
        if (widthtext != sheetWidth || heighttext != sheetHeight) {
          if (widthtext! > sheetWidth! - 15) {
            rescaledImage = img.copyResize(image!, width:sheetWidth!.toInt());
            update = true;
          }
          else if (heighttext! > sheetHeight! - 15) {
            rescaledImage = img.copyResize(image!, height:sheetHeight!.toInt());
            update = true;
          }
          else {
            rescaledImage = img.copyResize(image!, width:rescaledImage.width + 15);
            update = true;
          }
        }
      }
      else if (widthtext! > 50 && heighttext! > 50) {
        rescaledImage = img.copyResize(image!, width:rescaledImage.width - 15);
        update = true;
      }

      if (update) {
        final directory = await Directory.systemTemp.createTemp(); // Temp directory
        final processedImagePath = '${directory.path}/processed_image.png';
        File(processedImagePath).writeAsBytesSync(img.encodePng(rescaledImage));

        setState(() => this.displayedImage = File(processedImagePath));
        setState(() => this.widthtext = rescaledImage.width);
        setState(() => this.heighttext = rescaledImage.height);
      }

    } on PlatformException catch(e) {
      print('Failed to pick image: $e');
    }
    
  }

  Future<Uint8List> blackAndWhite(File imageToPrint) async {
    List<int> imageBytes = await imageToPrint.readAsBytes();

    img.Image? image = img.decodeImage(Uint8List.fromList(imageBytes));

    image = img.grayscale(image!);

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

    print(image.width);

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

    print(sizeBytes);

    // Prepend the size bytes to the byteArray
    byteArray.insertAll(0, sizeBytes);

    return Uint8List.fromList(byteArray);
  }

  Future<void> printImage() async {
    if (TheBluetoothService().isConnected.value) {
      List<String> values = TheBluetoothService().statusNotifier.value.split(',');
      String progress;
      progress = values.length == 3 ? values[1] : "-1";
      if (double.tryParse(progress) != null) {
        if (double.parse(progress) <= 0) {
          int w;

          // sheetHeight = MediaQuery.of(context).size.height - 300;
          // sheetWidth = sheetHeight! * 0.707;

          double percent = widthtext!.toDouble() / sheetWidth!;

          if (isPortrait) {
            if (_selectedText == "A3") {
              int A3w = 1123;
              w = (percent * A3w).toInt();
            }
            else if (_selectedText == "A4") {
              int A4w = 797;
              w = (percent * A4w).toInt();
            }
            else {
              int A5w = 560;
              w = (percent * A5w).toInt();
            }
          }
          else {
            if (_selectedText == "A3") {
              int A3w = 1584;
              w = (percent * A3w).toInt();
            }
            else if (_selectedText == "A4") {
              int A4w = 1123;
              w = (percent * A4w).toInt();
            }
            else {
              int A5w = 797;
              w = (percent * A5w).toInt();
            }
          }

          img.Image rescaledImage = img.copyResize(image!, width:w);

          final directory = await Directory.systemTemp.createTemp(); // Temp directory
          final processedImagePath = '${directory.path}/processed_image.png';
          File(processedImagePath).writeAsBytesSync(img.encodePng(rescaledImage));

          File imageToPrint = File(processedImagePath);

          try {
            final characteristicUUID = Guid("beb5483e-36e1-4688-b7f5-ea07361b26a8");

            BluetoothCharacteristic? targChar = await TheBluetoothService().charFinder(characteristicUUID);

            Uint8List byteArray = await blackAndWhite(imageToPrint);

            if (targChar == null) {
              print("Target Characteristic Not Found");
              return;
            }

            final characteristicUUID2 = Guid("64f90866-d4bb-493d-bd01-e532e4e34021");

            BluetoothCharacteristic? targChar2 = await TheBluetoothService().charFinder(characteristicUUID2);

            if (targChar2 == null) {
              print("Target Characteristic Not Found");
              return;
            }

            print(byteArray.sublist(0, 6));
            await targChar2.write(byteArray.sublist(0, 6), withoutResponse: true); // this is how many bytes will be received
            await Future.delayed(Duration(milliseconds: 50));

            // print(byteArray.sublist(4, 6));
            // await targChar.write(byteArray.sublist(4, 6), withoutResponse: true); // width in number of bytes (not pixels!)
            // await Future.delayed(Duration(milliseconds: 50));

            int chunkSize = 490;

            for (int i = 6; i < byteArray.length; i += chunkSize) {
              // Get the next chunk of size 'chunkSize', but ensure we don't go past the end of the list
              int end = (i + chunkSize < byteArray.length) ? i + chunkSize : byteArray.length;
              List<int> chunk = byteArray.sublist(i, end);
              
              // Print the chunk
              print('Chunk starting at index $i: $chunk');

              await targChar.write(chunk, withoutResponse: true);
              await Future.delayed(Duration(milliseconds: 100));
            }

            print("DONE");
            await targChar2.write([1], withoutResponse: true);
            await Future.delayed(Duration(milliseconds: 50));
            print("ABOVE MESSAGE SENT");

            Navigator.push(
              context,
              MaterialPageRoute(builder: (context) => StatusPageContentComponent()),
            );

          }
          catch (e) {
            print("Error");
          }
        }
        else {
          ScaffoldMessenger.of(context).showSnackBar(SnackBar(
          content: Text(
            "Cannot Upload Image while Device is Printing",
            textAlign: TextAlign.left,
          ),
          duration: Duration(seconds: 3), // Show for 3 seconds
          backgroundColor: Colors.red,
        ));
        }
      }
    }
    else {
      print('No printer connected!');
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(
        content: Text(
          "Not Connected to Printer",
          textAlign: TextAlign.left,
        ),
        duration: Duration(seconds: 3), // Show for 3 seconds
        backgroundColor: Colors.red,
      ));
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        //title: Text("Print Page"),
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
          //mainAxisAlignment: MainAxisAlignment.center,
          crossAxisAlignment: CrossAxisAlignment.center, // Centers content horizontally
          children: [
            SizedBox(height: 80),

            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.center,
              children: [
                Row(
                  mainAxisAlignment: MainAxisAlignment.center,
                  crossAxisAlignment: CrossAxisAlignment.center,
                  children: [
                    TextButton(
                      onPressed: () async {
                        final RenderBox button = context.findRenderObject() as RenderBox;
                        final Offset position = button.localToGlobal(Offset.zero);
                        final newValue = await showMenu<String>(
                          context: context,
                          position: RelativeRect.fromLTRB(
                            position.dx, // X position (horizontal)
                            position.dy, // Below the button
                            position.dx + button.size.width, // Width of the button
                            position.dy + button.size.height, // Arbitrary bottom margin
                          ),
                          items: [
                            PopupMenuItem<String>(
                              value: "A3",
                              child: Text("A3"),
                            ),
                            PopupMenuItem<String>(
                              value: "A4",
                              child: Text("A4"),
                            ),
                            PopupMenuItem<String>(
                              value: "A5",
                              child: Text("A5"),
                            ),
                          ],
                        );

                        if (newValue != null) {
                          setState(() {
                            _selectedText = newValue;
                          });
                        }
                      },
                      child: Text(_selectedText),
                    ),

                    SizedBox(width: 10),

                    IconButton(
                      icon: Image.asset(
                        'assets/icons/portraitIcon.png', // Replace with your actual image path
                        width: 20, 
                        height: 20, 
                      ),
                      onPressed: () {
                        if (isPortrait) {
                          setState(() => this.sheetHeight = sheetWidth! * 0.707);
                        }
                        else {
                          setState(() => this.sheetHeight = portraitHeight);
                        }
                        isPortrait = !isPortrait;
                        resizeImage();
                      },
                    ),

                    SizedBox(width: 40),

                    IconButton(
                      icon: Image.asset(
                        'assets/icons/plusButton.png', // Replace with your actual image path
                        width: 20, 
                        height: 20, 
                      ),
                      onPressed: () {
                        rescaleImage(true);
                      }
                    ),

                    SizedBox(width: 10),

                    IconButton(
                      //iconSize: 60,
                      icon: Image.asset(
                        'assets/icons/minusButton.png', // Replace with your actual image path
                        width: 20, 
                        height: 20, 
                      ),
                      onPressed: () {
                        rescaleImage(false);
                      }
                    ),

                    SizedBox(width: 40),

                    IconButton(
                      icon: Icon(Icons.print),
                      onPressed: () {
                        printImage();
                      },
                    ),
                  ],
                ),

                Center(
                  child: Container(
                    // this should represent the rough ratios of a sheet of A4 paper
                    // this allows the user to get a good representation of what their printed image should look like
                    width: sheetWidth, // Larger than image width
                    height: sheetHeight, // Larger than image height
                    decoration: BoxDecoration(
                      border: Border.all(
                        color: Colors.black,  // Black border
                        width: 5,             // Adjust thickness of the border
                      ),
                    ),
                    child: SizedBox(
                      width: 200, // Fixed width for inner box
                      height: 200, // Fixed height for inner box
                      child: Stack(
                        children: [
                          Positioned(
                            left: 0,
                            top: 0,
                            child: Image.file(
                              displayedImage!,
                              width: widthtext!.toDouble(),
                              height: heighttext!.toDouble(),
                            ),
                          ),
                        ],
                      ),
                    ),
                  ),
                ),
              ],
            )
          ],
        ),
      )
    );
  }
}