/*
 Chat  Server

 A simple server that distributes any incoming messages to all
 connected clients.  To use telnet to  your device's IP address and type.
 You can see the client's input in the serial monitor as well.
 Using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe

 */

#include <SPI.h>
#include <Ethernet2.h>
#include <Keypad.h>
#include <ArduinoJson.h>

const int buzzer = 12; 
const int led = 47;
//Keypad
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
      {'1', '2', '3', 'A'},
      {'4', '5', '6', 'B'},
      {'7', '8', '9', 'C'},
      {'*', '0', '#', 'D'},
};
byte rowPins[ROWS] = {5,4,3,2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9,8,7,6}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

char previousKey = ' ';
bool key1 = false;
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
int parts[4] = {192,168,0,177};
String ipstring = String(parts[0]) + "." + String(parts[1]) + "." + (parts[2]) + "." + String(parts[3]);
IPAddress ip(parts[0], parts[1], parts[2], parts[3]);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ipServer(192, 168, 0, 100);


// telnet defaults to port 23
EthernetServer server(23);
EthernetClient client;
boolean alreadyConnected = false; // whether or not the client was connected previously
int incomingByte = 0;   // dùng để lưu giá trị được gửi
String str;

StaticJsonDocument<200> json;
StaticJsonDocument<200> doc;


void setup() {
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer,LOW);
  delay(100);
  
//  digitalWrite(buzzer,LOW);
//  delay(100);
//  tone(buzzer, 1000, 500);
//  delay(1000);
//  noTone(buzzer);     // Stop sound...
//  delay(1000);
  // initialize the ethernet device
  Ethernet.begin(mac, ip, gateway, subnet);
  // start listening for clients
  server.begin();
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial1.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.print("Chat server address:");
  Serial.println(Ethernet.localIP());
  
  delay(1000);
  client.connect(ipServer, 23);
}

void loop() {

//  readserial();
  getkey();
//  client.flush();
//  client.println("check!");
//  delay(1000);
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
      char thisChar = client.read();
      // echo the bytes back to the client:
      server.write(thisChar);
      // echo the bytes to the server as well:
//      Serial.write(thisChar);
    }
    
  }
  Ethernet.maintain();
}


void parser(char doc){
  DeserializationError error = deserializeJson(json, doc);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
}

void getkey(){
  char key = keypad.getKey();
  
  if (key != NO_KEY){
    switch (key) {
      case 'A':
        previousKey = key;
        key1 = false;
        key2 = false;
        key3 = false;
        key4 = false;
        break;
      case 'D': // Enter
        
        doc["action"] = "enter";
        doc["ip"] = ipstring;
        serializeJson(doc, client);     
        break;

      case 'C': //Call
//        char data4 = "{\"timestamp\":1564658642,\"action\":\"callnumber\",\"ip\": \"192.168.0.177\"}";
        doc["action"] = "callnumber";  
        doc["ip"] = ipstring;
        serializeJson(doc, client);
        
        break;
    }
    if (key >= '0' && key <='9' && previousKey == 'A'){
      Serial.print(key);
//      Serial.print('\n');

      if (key1 == false){
        key1 = true;
        varkey1 = key;
      } else if (key2 == false){
        key2 = true;
        varkey2 = key;
      } else if (key3 == false){
        key3 = true;
        varkey3 = key;
      } else if (key4 == false){
        key4 = true;
        varkey4 = key;
      }

      delay(100);
    }
    
    Serial.print(key);
    beep(1);
    
  }
}
void readserial(){
    // nếu còn có thể đọc được 
  if (Serial1.available() > 0) {
    delay(10);
    // đọc chữ liệu
    incomingByte = Serial1.read();
  
    if (incomingByte == -1) {
      Serial.println("Toi khong nhan duoc gi ca");
    } else {
      str += char(incomingByte);
  //                  Serial.p(char(incomingByte));
    }

  } else {

    Serial.println(str);
    str = "";
    delay(1000);

  }
}

void beep(int number){
  for (int i = 0; i <= number; i++) {
    digitalWrite(buzzer, HIGH);
    delay(20);
    digitalWrite(buzzer, LOW);
  }
}

