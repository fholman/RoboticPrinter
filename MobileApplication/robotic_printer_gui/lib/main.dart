import 'package:flutter/material.dart';
import 'package:motion_tab_bar/MotionTabBarController.dart';
import 'main_page.dart';
import 'settings.dart';

ValueNotifier<List<String>> entriesNotifier = ValueNotifier<List<String>>([]);
ValueNotifier<bool> isDevice = ValueNotifier<bool>(false);

// Color col1 = Color(0xFFF7C59F);
// Color col2 = Color(0xFF2A324B);
// Color col3 = Color(0xFF767B91);
// Color col4 = Color(0xFFC7CCDB);
// Color col5 = Color(0xFFE1E5EE);

void main() => runApp(const MyApp());

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'PRINTBOT LABS',
      theme: ThemeData(
        //primarySwatch: Colors.blue,
        primaryColor: Colors.white,
      ),
      home: const MainPageContentComponent(),
      //home: const MyHomePage(title: 'PRINTBOT LABS'),
    );
  }
}