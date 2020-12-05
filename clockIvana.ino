#include <WiFi.h>
#include "FS.h"
#include <SPI.h>
#include <SD.h>
#include <string.h>
#include <TFT_eSPI.h>
#include <JPEGDecoder.h>
#include <Wire.h>
#include <BBQ10Keyboard.h>

//#define TWATCH_TFT_MISO             0
//#define TWATCH_TFT_MOSI             19
//#define TWATCH_TFT_SCLK             18
//#define TWATCH_TFT_CS               5
//#define TWATCH_TFT_DC               19 //27
//#define TWATCH_TFT_RST              32 // -1 
//#define TWATCH_TFT_BL               12
//#define TFT_HEIGHT 240 // ST7789 240 x 240
//#define TFT_WIDTH  240
//#define TFT_CS   5  // Chip select control pin D8
//#define TFT_DC   27  // Data Command control pin
//#define TFT_RST  -1  // Reset pin (could connect to NodeMCU RST, see next line)

//#define TOUCH_SDA                   23
//#define TOUCH_SCL                   32
//#define TOUCH_INT                   38
//// #define TOUCH_RST                Use AXP202 EXTEN Pin control
//
#define I2C_SDA                     21  // Клавиатура
#define I2C_SCL                     22
#define I2C_INT                      4
//#define RTC_INT_PIN                 37
//#define AXP202_INT                  35
//#define BMA423_INT1                 39
//
//#define TWATCH_2020_IR_PIN          4
//
////GPS power domain is AXP202 LDO4
//#define GPS_1PPS                    34
//#define GPS_RX                      25
//#define GPS_TX                      26
//#define GPS_WAKE                    33
//#define GPS_BAUD_RATE               9600

BBQ10Keyboard keyboard;
volatile bool dataReady = false;

const int interruptPin = 4;


const char* ssid     = "Nadejda";
const char* password = "adadad11";
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
char code[] = {"int abc=1234;"};
char toCompile = 'start.ttg';
// ESP.restart();
TFT_eSPI tft = TFT_eSPI();

void KeyIsr(void)
{
  dataReady = true;
}

void setup()
{
   pinMode(12, OUTPUT);
   digitalWrite(12, HIGH); // Подсветка
   pinMode(25, OUTPUT); //объявляем пин как выход. Этим пином издаём звуки

   Wire.begin();
   keyboard.begin();
   keyboard.attachInterrupt(interruptPin, KeyIsr);
   keyboard.setBacklight(0.5f);
   
   Serial.begin(115200);
   Serial.printf("Starting OS\n\t");
   tft.init();
   tft.setRotation(1);
   tft.fillScreen(TFT_BLACK);
   tft.setCursor(0, 0, 1);
   tft.setTextColor(TFT_BLUE, TFT_BLACK);
   tft.setTextSize(2);
   tft.println("Irvin Lab");
   tft.setTextColor(TFT_GREEN, TFT_BLACK);
   tft.setTextSize(1);
   tft.println("");
   tft.println("");
   tft.println("MicroSD Card Initialization...");
   
    if(!SD.begin()){
        Serial.println("Card Mount Failed");
        tft.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        tft.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    tft.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
        tft.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
        tft.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
        tft.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
        tft.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    tft.printf("SD Card Size: %lluMB\n", cardSize);
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
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
    tft.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    tft.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));




  

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

//lexer(code);


}

void loop()
{
  const int keyCount = keyboard.keyCount();
  if (keyCount == 0)
    return;

  const BBQ10Keyboard::KeyEvent key = keyboard.keyEvent();
  String state = "pressed";
  if (key.state == BBQ10Keyboard::StateLongPress)
    state = "held down";
  else if (key.state == BBQ10Keyboard::StateRelease)
    state = "released";

  // pressing 'b' turns off the backlight and pressing Shift+b turns it on
  if (key.state == BBQ10Keyboard::StatePress) {
    tft.print(key.key);
//    if (key.key == 'b') {
//      keyboard.setBacklight(0);
//    } else if (key.key == 'B') {
//      keyboard.setBacklight(1.0);
//    }
  }
}


void Error(char *s) { // Report an Error 
  delay(1);
  Serial.printf("Error: ", s,'.');
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

void Expected(char *s){ // Report What Was Expected 
  delay(1);
  Abort(s + ' Expected');
}

void Abort(char *s){ // Report Error and Halt
  delay(1);
  Error(s);
  return;
}


//isDigit
//isAlpha

void thisIntVar(){
  int n = 4;
  while (code[n] != '='){     // Определили длинну имени переменной
    if (isAlpha(code[n])){
      n++;
      Serial.println(n);
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

void lexer(char * x) {
  delay(1);
  
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
