/*
  Chat  Server

  A simple server that distributes any incoming messages to all
  connected clients.  To use telnet to  your device's IP address and type.
  You can see the client's input in the serial monitor as well.
  Using an Arduino Wiznet Ethernet shield.

  Circuit:
   Ethernet shield attached to pins 10, 11, 12, 13
   Analog inputs attached to pins A0 through A5 (optional)

  created 18 Dec 2009
  by David A. Mellis
  modified 9 Apr 2012
  by Tom Igoe

*/

#include <SPI.h>
#include <Ethernet2.h>
#include <Keypad.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <ArduinoJson.h>
#include <string.h>


// For the LCD.
#define TFT_DC 49
#define TFT_CS 48
#define TFT_MOSI 51
#define TFT_CLK 10
#define TFT_RST 9
#define TFT_MISO 50

#define buzzer 12
#define led 47

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);


//Keypad
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'},
};
byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 8, 7, 6}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

char previousKey = ' ';
bool enterKey = false;
bool key2 = false;
bool key3 = false;
bool key4 = false;

char varkey1;
char varkey2;
char varkey3;
char varkey4;


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

const char ipstring[15] = "192.168.1.179";
IPAddress ip(192, 168, 1, 179);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ipServer(192, 168, 1, 100);


// telnet defaults to port 23
EthernetServer server(23);
EthernetClient client;
boolean alreadyConnected = false; // whether or not the client was connected previously
int incomingByte = 0;   // dùng để lưu giá trị được gửi
String str;

StaticJsonDocument<200> docEnter;
StaticJsonDocument<200> docCall;
StaticJsonDocument<200> docRecall;
StaticJsonDocument<200> docStore;
bool flagNumber = false;
bool flagMoveTo = false;
bool flagLogin = false;
int number1 = -1;
int number2 = -1;
int number3 = -1;
int number4 = -1;
int service;
String userID = "";
String counterID = "";
int x = 0;
int y = 0;

void setup() {
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);

  Serial1.begin(115200);
  pinMode(22, OUTPUT);
  //
  Serial2.begin(115200);
  pinMode(23, OUTPUT);

  // LCD start
  tft.begin();
  flag(ILI9341_CYAN);

  //  testFastLines(ILI9341_RED, ILI9341_BLUE);
  tft.setRotation(1);
  tft.fillScreen(ILI9341_WHITE);
  login();
  menubar();

  // initialize the ethernet device
  Ethernet.begin(mac, ip, gateway, subnet);
  // start listening for clients
  server.begin();
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.print("Client address:");
  Serial.println(Ethernet.localIP());

  delay(1000);
  client.connect(ipServer, 23);
}

void loop() {

  //  readserial();
  getkey();
  if (client) {
    if (!alreadyConnected) {
      // clear out the input buffer:
      client.flush();
      Serial.println("We have a new client");
      //      client.println("Hello, client!");
      //      client.println("DEVICE_NAME,counter 1");
      //      client.println("DEVICE_MAC,0xDE:0xAD:0xBE:0xEF:0xFE:0xED");

      alreadyConnected = true;
    }

    if (client.available() > 0) {
      // read the bytes incoming from the client:
      //      char thisChar = client.read();
      // echo the bytes back to the client:
      //      server.write(thisChar);
      // echo the bytes to the server as well:
      //      Serial.write(thisChar);
      parser();
    }

  }
  Ethernet.maintain();

}


void parser() {
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonDocument docParser(capacity);

  // Parse JSON object
  DeserializationError error = deserializeJson(docParser, client);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
  } else {
    Serial.print(docParser["number"][0].as<int>());
    Serial.print(docParser["number"][1].as<int>());
    Serial.print(docParser["number"][2].as<int>());
    Serial.print(docParser["number"][3].as<int>());

    number1 = docParser["number"][0].as<int>();
    number2 = docParser["number"][1].as<int>();
    number3 = docParser["number"][2].as<int>();
    number4 = docParser["number"][3].as<int>();
    String number = String(number1) + String(number2) + String(number3) + String(number4);
    sendRS485(number);
    service = docParser["service"].as<int>();
    flagNumber = true;
  }
}

void getkey() {
  char key = keypad.getKey();

  if (key != NO_KEY) {
    beep(1);
    switch (key) {
      case 'A':
        clearDisplay();
        if (previousKey == 'A') {

        }
        menu();
        break;
      case 'B':
        clearChar();
        break;
      case 'D': // Enter
        if (flagLogin == true) {
        //        data0 = "{\"timestamp\":1564658642,\"action\":\"login\",\"subject_type\":\"user\",\"user_id\":1,\"counter_ip\": \"192.168.0.177\"}"; //login
          docEnter["action"] = "login";
          docEnter["user_id"] = userID.toInt();
          docEnter["counter_ip"] = ipstring;
          serializeJson(docEnter, client);
        } else {
          docEnter["action"] = "enter";
          docEnter["ip"] = ipstring;
          serializeJson(docEnter, client);
  
          enterKey = true;
          flagNumber = false;
          number1 = -1;
          number2 = -1;
          number3 = -1;
          number4 = -1;
        }
        break;

      case 'C': //Call
        //        char data4 = "{\"timestamp\":1564658642,\"action\":\"callnumber\",\"ip\": \"192.168.0.177\"}";
        if (enterKey == true) {
          docCall["action"] = "callnumber";
          docCall["ip"] = ipstring;
          serializeJson(docCall, client);
        } else {
          if (flagNumber == true) {
            docRecall["action"] = "recall";
            docRecall["service"] = service;
            JsonArray data = docRecall.createNestedArray("number");
            data.add(number1);
            data.add(number2);
            data.add(number3);
            data.add(number4);
            serializeJson(docRecall, client);
          } else {
            docCall["action"] = "callnumber";
            docCall["ip"] = ipstring;
            serializeJson(docCall, client);
          }
        }
        enterKey = false;
        break;

      case '*': //Store
        //      data1 = "{\"timestamp\":1564658642,\"action\":\"addnumber\",\"service\":1,\"number\":\[0,0,0,2]\}"; //add number
        if (flagNumber) {
          docStore["action"] = "addnumber";
          docStore["service"] = service;
          JsonArray data = docStore.createNestedArray("number");
          data.add(number1);
          data.add(number2);
          data.add(number3);
          data.add(number4);
          serializeJson(docStore, client);

          number1 = -1;
          number2 = -1;
          number3 = -1;
          number4 = -1;
        }
        enterKey = true;
        flagNumber = false;
        break;
      case '0':
        getChar(key);
        break;
      case '1':
        getChar(key);
        if (previousKey == 'A') {
          flagLogin = true;
          flagMoveTo = false;
          clearDisplay();
          tft.setTextColor(ILI9341_BLUE);
          tft.setTextSize(1);
          tft.setCursor(5, 50);
          tft.printlnUTF8("Mã số đăng nhập: ");
        }
        break;
      case '2':
        getChar(key);
        if (previousKey == 'A') {
          flagMoveTo = true;
          flagLogin = false;
          clearDisplay();
          tft.setTextColor(ILI9341_BLUE);
          tft.setTextSize(1);
          tft.setCursor(5, 50);
          tft.printlnUTF8("Chuyển Đến Dịch Vụ: ");
        }
        break;
      case '3':
        getChar(key);
        break;
      case '4':
        getChar(key);
        break;
      case '5':
        getChar(key);
        break;
      case '6':
        getChar(key);
        break;
      case '7':
        getChar(key);
        break;
      case '8':
        getChar(key);
        break;
      case '9':
        getChar(key);
        break;

    }
    if (key >= '0' && key <= '9' && previousKey == 'A') {

    }

    Serial.println(key);
    previousKey = key;

  }
}

void clearLine(int y) {
  Serial.println("clear line");
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  tft.setCursor(0, y);
  tft.printlnUTF8("                                       ");
}

void clearChar() {
  Serial.print("clear char: ");
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  tft.setCursor(x , y);
  if (x - 10 > 0) {
    x = x - 10;
  }
  Serial.println(x);
  tft.printlnUTF8("  ");
  if (flagLogin == true) {
    subString(userID);
    Serial.println("user:" + userID);
  } else if (flagMoveTo == true) {
    subString(counterID);
    Serial.println("counter:" + counterID);
  }
}

void clearDisplay() {
  tft.fillScreen(ILI9341_WHITE);
  menubar();
  getStateConnection();
}

unsigned long flag(uint16_t color) {
  unsigned long start, t;
  int           x1, y1, x2, y2,
                w = tft.width(),
                h = tft.height();

  tft.fillScreen(ILI9341_BLACK);
  yield();

  x1 = y1 = 0;
  y2    = h - 1;
  start = micros();
  for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = w - 1;
  for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);

}
unsigned long login() {
  int w = tft.width(),
      h = tft.height();
  tft.setCursor(5, 0);
  tft.setTextColor(ILI9341_DARKCYAN);
  tft.setTextSize(2);
  //  tft.setFont(&FreeSerif9pt7b);
  tft.printlnUTF8("QSystem");

}
unsigned long menubar() { //
  int          x1, y1, x2, y2,
               w = tft.width(),
               h = tft.height();
  tft.setCursor(5, 0);
  tft.setTextColor(ILI9341_DARKCYAN);
  tft.setTextSize(2);
  tft.printlnUTF8("QSystem");

  tft.setTextSize(3);
  tft.drawLine(0, 35, w, 35, ILI9341_DARKCYAN);
  tft.drawLine(0, 36, w, 36, ILI9341_DARKCYAN);
  tft.drawLine(0, 222, w, 222, ILI9341_DARKCYAN);
  tft.drawLine(0, 223, w, 223, ILI9341_DARKCYAN);

}

unsigned long menu() { //
  int          x1, y1, x2, y2,
               w = tft.width(),
               h = tft.height();

  tft.setTextColor(ILI9341_BLUE);
  tft.setTextSize(1);//  tft.setFont(&Cousine_Regular_16);
  tft.setCursor(5, 50);
  tft.printlnUTF8("1.Đăng Nhập");
  tft.setCursor(5, 70);
  tft.printlnUTF8("2.Chuyển Dịch Vụ");
  tft.setCursor(5, 90);
  tft.printlnUTF8("3.Số Lượng Khách");
  tft.setCursor(5, 110);
  tft.printlnUTF8("4.Đăng Xuất");
}

void getStateConnection() {
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_DARKCYAN);
  if (alreadyConnected) {
    clearLine(223);
    tft.setCursor(225, 223);
    tft.printUTF8("Đã Kết Nối!");
    //    Serial.println("Đã Kết Nối!");
  } else {
    clearLine(223);
    tft.setCursor(220, 223);
    tft.printUTF8("Ngắt Kết Nối!");
    //    Serial.println("Ngắt Kết Nối!");
  }
}

void beep(int number) {
  for (int i = 0; i <= number; i++) {
    digitalWrite(buzzer, HIGH);
    delay(20);
    digitalWrite(buzzer, LOW);
  }
}

void sendRS485(String number) {
  digitalWrite(22, HIGH);
  digitalWrite(23, HIGH);
  Serial1.println(number);
  Serial2.println(number);
  delay(1000);
  digitalWrite(22, LOW);
  digitalWrite(23, LOW);
}

void addString(String &string, char key) {
  string = string + key;
}
void subString(String &string) {
  string.remove(string.length() - 1);
}

void getChar(char key) {
  if (flagLogin == true) {
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK);
    x = x + 10;
    y = 70;
    tft.setCursor(x, y);
    printChar(key);
    addString(userID, key);
    Serial.println("user:" + userID);
  } else if (flagMoveTo == true) {
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK);
    x = x + 10;
    y = 70;
    tft.setCursor(x, y);
    printChar(key);
    addString(counterID, key);
    Serial.println("counter:" + counterID);
  }
}
void printChar(char key) {
  switch (key) {
    case '0':
      tft.printlnUTF8("0");
      break;
    case '1':
      tft.printlnUTF8("1");
      break;
    case '2':
      tft.printlnUTF8("2");
      break;
    case '3':
      tft.printlnUTF8("3");
      break;
    case '4':
      tft.printlnUTF8("4");
      break;
    case '5':
      tft.printlnUTF8("5");
      break;
    case '6':
      tft.printlnUTF8("6");
      break;
    case '7':
      tft.printlnUTF8("7");
      break;
    case '8':
      tft.printlnUTF8("8");
      break;
    case '9':
      tft.printlnUTF8("9");
      break;
  }

}

