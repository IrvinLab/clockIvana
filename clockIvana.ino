#include <WiFi.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include "FS.h"
#include <SPI.h>
#include <SD.h>
#include <string.h>

#if defined(ARDUINO_FEATHER_ESP32) // Feather Huzzah32
  #define TFT_CS         14
  #define TFT_RST        15
  #define TFT_DC         32
#else
  // For the breakout board, you can use any 2 or 3 pins.
  // These pins will also work for the 1.8" TFT shield.
  #define TFT_CS        10
  #define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC         8
#endif

// OPTION 1 (recommended) is to use the HARDWARE SPI pins, which are unique
// to each board and not reassignable. For Arduino Uno: MOSI = pin 11 and
// SCLK = pin 13. This is the fastest mode of operation and is required if
// using the breakout board's microSD card.

// For 1.14", 1.3", 1.54", and 2.0" TFT with ST7789:
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

const char* ssid     = "npoMGI";
const char* password = "173841173841";
float p = 3.1415926;

// for custom variables
char myChar[512]; // 1 Bytes     - 0
bool myBool[256]; // 1 Byte      - 1
int myInt[512]; // 2 Bytes       - 2
float myFloat[512]; // 4 Bytes   - 3
double myDouble[512]; // 4 Bytes - 4
long myLong[256]; // 4 Bytes     - 5

String varName[4096] = {};
int varNumber[6] = {0,0,0,0,0,0};
  
char *tmp1, *tmp2, *stemp[65536]; // 64 KBytes for the current user program
const char *list[] = {
  "int a=256;",
  "out(\"SAS\");"
  };
char code[] = {};
char toCompile = 'start.ttg';


void setup()
{
   Serial.begin(115200);
   Serial.printf("Starting OS\n\t");
// Раскомментировать для теста на реальном железе
//    if(!SD.begin()){
//        Serial.println("Card Mount Failed");
//        return;
//    }
//    uint8_t cardType = SD.cardType();
//
//    if(cardType == CARD_NONE){
//        Serial.println("No SD card attached");
//        return;
//    }
//
//    Serial.print("SD Card Type: ");
//    if(cardType == CARD_MMC){
//        Serial.println("MMC");
//    } else if(cardType == CARD_SD){
//        Serial.println("SDSC");
//    } else if(cardType == CARD_SDHC){
//        Serial.println("SDHC");
//    } else {
//        Serial.println("UNKNOWN");
//    }
//
//    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
//    Serial.printf("SD Card Size: %lluMB\n", cardSize);
//
//    listDir(SD, "/", 0);
//    createDir(SD, "/mydir");
//    listDir(SD, "/", 0);
//    removeDir(SD, "/mydir");
//    listDir(SD, "/", 2);
//    writeFile(SD, "/hello.txt", "Hello ");
//    appendFile(SD, "/hello.txt", "World!\n");
//    readFile(SD, "/hello.txt");
//    deleteFile(SD, "/foo.txt");
//    renameFile(SD, "/hello.txt", "/foo.txt");
//    readFile(SD, "/foo.txt");
//    testFileIO(SD, "/test.txt");
//    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
//    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
//
//
//    tft.init(240, 240); // Display initialization
//    uint16_t time = millis();
//    tft.fillScreen(ST77XX_BLACK);
//    time = millis() - time;
//    Serial.println(time, DEC);
//    delay(500);
//    // large block of text
//    tft.fillScreen(ST77XX_BLACK);


  

  // Подключаемся к WiFi
//  WiFi.begin(ssid, password); 
//  while (WiFi.status() != WL_CONNECTED) {
//      delay(500);
//  }
//  Serial.printf("WiFi connection established");


myInt[0] = 1;
myInt[1] = 2;
myFloat[0] = 0.11;
myFloat[1] = 0.22;
myDouble[0] = 0.1111;
myDouble[1] = 0.2222;
myLong[0] = 1010101;
myLong[1] = 2121212;
myChar[0] = 'a';
myChar[1] = 'b';
myBool[0] = 1;
myBool[1] = 0;

lexer(code);

pinMode(2, OUTPUT);
}

void loop()
{
  delay(1000);
  digitalWrite(2, HIGH);
  delay(50);
  digitalWrite(2, LOW);
}

//void Match(char *x){ // Match a Specific Input Character
//  delay(1);
//  if (look = *x){
//    GetChar();
//  }
//  else {
//    Expected(x);
//  }
//}


void thisIntVar(){
  int n = 4;
  while (code[n] != '='){     // Определили длинну имени переменной
    if (isAlpha(code[n])){
      Serial.println(n);
      n++;
    }
  }
  int m = 4;
  String tempName = "";
  while(m != n){              // Определили имя переменной
    tempName = tempName + code[m];
    m++;
    Serial.println(tempName);
  }
  varName[varNumber[0]] = tempName;  
  Serial.println(varName[varNumber[0]]);
  n++;
  while (code[n] != ';'){     // Определили значение переменной
    if (isDigit(code[n])){
      n++;
      Serial.println(n);
    }
  }
  m++;
  tempName = "";
  while(m != n){         
    tempName = tempName + code[m];
    m++;
    Serial.println(tempName);
  }
  Serial.println(tempName);
  myInt[varNumber[0]] = 0;
  unsigned char* buf = new unsigned char[100];  // О чудо!!!! Преобразование String в const char*
  tempName.getBytes(buf, 100, 0);
  const char *str2 = (const char*)buf;
  myInt[varNumber[0]] = atoi(str2);  
  Serial.println(myInt[varNumber[0]]);
  varNumber[0]++;
  Serial.println(code);
}

void thisCharVar(){
  delay(1);
}

void thisFloatVar(){
  delay(1);
}

void thisDoubleVar(){
  delay(1);
}

void thisBoolVar(){
  delay(1);
}

void thisLongVar(){
  delay(1);
}

void printLed(){   // Печать строки или переменной
  int n = 4;
  int m = 5;
  String tempName = "";
  Serial.println(code[n]);
  if (code[n] = '\"'){
    n++;
    while (code[n] != '\"'){     // Определили длинну строки
      Serial.println(n);
      n++; 
    }
    while(m != n){              // Определили саму строку
      tempName = tempName + code[m];
      m++;
    }
    Serial.println(tempName);
    while (code[n] != ';'){
      if (isDigit(code[n])){
        Serial.println("Error: incorrect syntax.");
        return;
      }
      if (isAlpha(code[n])){
        while (code[n] != '\"'){     // Определили длинну имени переменной
          Serial.println(n);
          n++; 
        }
        while (m != n) {             // Определили имя переменной
          tempName = tempName + code[m];
          m++;
        }
      }
      n++;
      if (n >= strlen(code)){
        Serial.println("Error: ; expected.");
        return;
      }
    }
  }
  
}

void lexer(char * x) {
  delay(1);
  size_t numLines = sizeof(list)/sizeof(list[0]);
  int n = 0;
  Serial.println("Num lines = " + numLines);
  while (n != numLines){
    Serial.println(n); 
    code = list[n];
    if (code[0] == 'i' and code[1] == 'n' and code[2] == 't' and code[3] == ' '){ // int = Integer
      thisIntVar();
    }
    else if (code[0] == 'c' and code[1] == 'h' and code[2] == 'r' and code[3] == ' '){ // chr = Char
      delay(1);
    }
    else if (code[0] == 'f' and code[1] == 'l' and code[2] == 'o' and code[3] == ' '){ // flo = Float
      delay(1);
    }
    else if (code[0] == 'd' and code[1] == 'o' and code[2] == 'b' and code[3] == ' '){ // dob = Double
      delay(1);
    }
    else if (code[0] == 'l' and code[1] == 'o' and code[2] == 'n' and code[3] == 'g' and code[4] == ' '){ // long = Long
      delay(1);
    }
    else if (code[0] == 'b' and code[1] == 'y' and code[2] == 't' and code[3] == 'e' and code[4] == ' '){ // byte = Byte
      delay(1);
    }
    else if (code[0] == 'o' and code[1] == 'u' and code[2] == 't' and code[3] == '('){ // byte = Byte
      printLed();
    }
    n++;
  }
  Serial.println("OK");
}













void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);
    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
        code[(sizeof(code)+1)] = file.read(); // Прочитали фаил в массив code
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}
//
//void testDisplay7789(uint16_t color) {
//  tft.fillScreen(ST77XX_BLACK);
//  tft.drawLine(0, 0, x, tft.height()-1, color);
//  tft.setCursor(0, 0);
//  tft.setTextColor(color);
//  tft.setTextWrap(true);
//  tft.print(text);
//  tft.drawFastHLine(0, y, tft.width(), color1);
//  tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color);
//  tft.fillRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color1);
//  tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color2);
//  tft.fillCircle(x, y, radius, color);
//  tft.drawTriangle(w, y, y, x, z, x, color);
//  tft.drawRoundRect(x, y, w, h, 5, color);
//
//  tft.setTextWrap(false);
//  tft.fillScreen(ST77XX_BLACK);
//  tft.setCursor(0, 30);
//  tft.setTextColor(ST77XX_RED);
//  tft.setTextSize(1);
//  tft.println("Hello World!");
//  tft.setTextColor(ST77XX_YELLOW);
//  tft.setTextSize(2);
//  tft.println("Hello World!");
//  tft.setTextColor(ST77XX_GREEN);
//  tft.setTextSize(3);
//  tft.println("Hello World!");
//  tft.setTextColor(ST77XX_BLUE);
//  tft.setTextSize(4);
//  tft.print(1234.567);
//  delay(1500);
//  tft.setCursor(0, 0);
//  tft.fillScreen(ST77XX_BLACK);
//  tft.setTextColor(ST77XX_WHITE);
//  tft.setTextSize(0);
//  tft.println("Hello World!");
//  tft.setTextSize(1);
//  tft.setTextColor(ST77XX_GREEN);
//  tft.print(p, 6);
//  tft.println(" Want pi?");
//  tft.println(" ");
//  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
//  tft.println(" Print HEX!");
//  tft.println(" ");
//  tft.setTextColor(ST77XX_WHITE);
//  tft.println("Sketch has been");
//  tft.println("running for: ");
//  tft.setTextColor(ST77XX_MAGENTA);
//  tft.print(millis() / 1000);
//  tft.setTextColor(ST77XX_WHITE);
//  tft.print(" seconds.");
//}
