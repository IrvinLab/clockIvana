#include <WiFi.h>
#include "FS.h"
#include <SPI.h>
#include <SD.h>
#include <string.h>
#include <TFT_eSPI.h>
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
#define I2C_INT                      35
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

#define PCF8563address 0x51  // I2C адрес RTC
#define minimum(a,b)     (((a) < (b)) ? (a) : (b))
#define USE_SPI_BUFFER // Comment out to use slower 16 bit pushColor()


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

String charName[512] = {};
String boolName[512] = {};
String intName[512] = {};
String floatName[512] = {};
String doubleName[512] = {};
String longName[512] = {};
int varNumber[6] = {0,0,0,0,0,0};
  
char *tmp1, *tmp2, *stemp[65536]; // 64 KBytes for the current user program
//char code[] = {"int abc=1234;"};
const char *code[] = {"","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","",""};
int numStr = 0;
char toCompile = 'start.ttg';
String cmd = "";
String pwd = "/";
const char *lastCmd = "";
const char *tempFile = "";

byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
String days[] = {"VOS", "PON", "VTO", "SRE", "CHE", "PTN", "SUB" };
int globalClock = 0;

File myFile;  

// ESP.restart();
TFT_eSPI tft = TFT_eSPI();

void KeyIsr(void)
{
  dataReady = true;
}

byte bcdToDec(byte value)
{
 return ((value / 16) * 10 + value % 16);
}
// И обратно
byte decToBcd(byte value){
 return (value / 10 * 16 + value % 10);
}

// функция установки времени и даты в PCF8563
void setPCF8563()
{
 Wire.beginTransmission(PCF8563address);
 Wire.write(0x02);
 Wire.write(decToBcd(second)); 
 Wire.write(decToBcd(minute));
 Wire.write(decToBcd(hour));   
 Wire.write(decToBcd(dayOfMonth));
 Wire.write(decToBcd(dayOfWeek)); 
 Wire.write(decToBcd(month));
 Wire.write(decToBcd(year));
 Wire.endTransmission();
}

// функция считывания времени и даты из PCF8563
void readPCF8563()
{
 Wire.beginTransmission(PCF8563address);
 Wire.write(0x02);
 Wire.endTransmission();
 Wire.requestFrom(PCF8563address, 7);
 second   = bcdToDec(Wire.read() & B01111111); // удаление ненужных бит из данных 
 minute   = bcdToDec(Wire.read() & B01111111); 
 hour    = bcdToDec(Wire.read() & B00111111); 
 dayOfMonth = bcdToDec(Wire.read() & B00111111);
 dayOfWeek = bcdToDec(Wire.read() & B00000111); 
 month   = bcdToDec(Wire.read() & B00011111); 
 year    = bcdToDec(Wire.read());
}

void setup()
{
   pinMode(12, OUTPUT);
   digitalWrite(12, HIGH); // Подсветка
   pinMode(25, OUTPUT); //объявляем пин как выход. Этим пином издаём звуки
   pinMode(36, INPUT); // Пользовательская кнопка
   Serial.begin(115200);
   Wire.begin();
   keyboard.begin();
   keyboard.attachInterrupt(interruptPin, KeyIsr);
   keyboard.setBacklight(0.5f);

//   second = 0;  установка даты и времени
//   minute = 10;
//   hour = 13;
//   dayOfWeek = 0;
//   dayOfMonth = 6;
//   month = 12;
//   year = 20;
//   setPCF8563();

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

tft.println("");

}

void loop()
{
  if (digitalRead(36) == LOW){
    if (globalClock == 0) {
      globalClock == 1;
    }
    else if (globalClock == 1){
      globalClock = 0;
    }
  }
  const int keyCount = keyboard.keyCount();
  int n = 0;
  if (keyCount == 0)
    return;

  const BBQ10Keyboard::KeyEvent key = keyboard.keyEvent();
  String state = "pressed";
  if (key.state == BBQ10Keyboard::StateLongPress){
    state = "held down";
    tft.print('=');
    cmd = cmd + '=';
  }
  else if (key.state == BBQ10Keyboard::StateRelease)
    state = "released";

  // pressing 'b' turns off the backlight and pressing Shift+b turns it on
  if (key.state == BBQ10Keyboard::StatePress) {
    if (key.key == '\n') {
    unsigned char* buf = new unsigned char[100]; 
    cmd.getBytes(buf, 100, 0);
    const char *str2 = (const char*)buf;
    Serial.println(str2);
    exe(str2);
    lastCmd = str2;
    str2 = "";
//      keyboard.setBacklight(0);
//    } else if (key.key == 'B') {
//      keyboard.setBacklight(1.0);
    }
    tft.print(key.key);
    if (key.key != '\n'){
      cmd = cmd + key.key;
    }
    else if (key.key == '~') {
      cmd = cmd + '\n';
    }
    
    n++;
  }
}

String keyInput(){
delay(1);
}

void pingPong(){
  int score = 0;
  int life = 3;
  int weapon = 0;
  int x1 = 0;
  int y1 = 0;
  int n = 0;
  int xg = 0;
  int yg = 0;
  int xk = 0;
  int yk = 10;
  // 0 - void
  // 1 - wall
  // 2 - teleport
  // 3 - standart enemy
  // 4 - +1 life
  // 5 - +100 score
  // 6 - hard enemy
  // 7 - very hard enemy, vashe pizdec
  // 8 - SAS weapon
  // 9 - you
  int gameMatrix[22][22] = {
    {9,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
    {0,1,0,0,0,0,0,0,1,0,1,1,1,1,1,0,1,0,0,0,0},
    {0,1,0,1,1,1,1,1,1,0,1,0,0,0,1,0,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,2,0,0,1,0,0,0,0,0,0},
    {1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,0,1,0,0,0,0},
    {0,0,0,1,0,1,0,0,1,0,1,0,0,0,0,0,1,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,1,0,1,0,1,0,1,1,1,1,1},
    {0,0,0,0,0,1,1,1,1,1,1,0,1,0,1,0,0,0,0,0,0},
    {0,0,0,1,0,1,0,0,0,0,0,0,1,0,1,0,1,1,0,1,1},
    {0,0,0,1,0,1,0,0,0,0,0,0,1,0,1,0,1,0,0,1,0},
    {1,1,1,1,0,1,1,1,0,1,1,0,1,0,0,0,1,0,1,1,0},
    {0,1,0,0,0,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,0},
    {0,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,1,0,1,0,1,1,1,1,0,1,1},
    {1,1,0,1,0,1,1,1,1,0,0,0,0,0,1,0,0,1,0,0,0},
    {0,1,0,0,0,1,0,0,1,0,1,1,1,1,1,0,0,1,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1},
    {1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,0,0,0},
    {0,0,0,1,0,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0},
    {0,1,0,1,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0},
    };

  
  tft.fillCircle(x1, y1, 5, TFT_BLUE);
  tft.fillRect(0, 10, 240, 5,  TFT_WHITE);
  tft.fillRect(0, 10, 5, 240,  TFT_WHITE);
  tft.fillRect(235, 10, 235, 240,  TFT_WHITE);
  tft.fillRect(0, 235, 240, 240,  TFT_WHITE);
  while (1){
    tft.setCursor(0,0);
    tft.print("SCORE: ");
    tft.print(score);
    if (weapon == 1) {
      tft.setCursor(90,0);
      tft.print("IMBA");
    }
    else {
      tft.setCursor(90,0);
      tft.print("    ");
    }  
    tft.setCursor(190,0);
    tft.print("LIFE: ");
    tft.print(life);
    while (n!=500){
      if(gameMatrix[yg][xg] == 1){
        tft.fillRect(((xg+1)*5)+xk,((yg+1)*5)+yk,5,5,TFT_WHITE); // Wall
      }
      if(gameMatrix[yg][xg] == 2){
        tft.fillRect(((xg+1)*5)+xk,((yg+1)*5)+yk,5,5,TFT_YELLOW); // Teleport
      }
      if(gameMatrix[yg][xg] == 3){
        tft.fillRect(((xg+1)*5)+xk,((yg+1)*5)+yk,5,5,TFT_RED);  // Standart enemy
      }
      if(gameMatrix[yg][xg] == 4){
        tft.fillRect(((xg+1)*5)+xk,((yg+1)*5)+yk,5,5,TFT_GREEN); // Life
      }
      if(gameMatrix[yg][xg] == 5){
        tft.fillRect(((xg+1)*5)+xk,((yg+1)*5)+yk,5,5,TFT_GOLD); // +100 Score
      }
      if(gameMatrix[yg][xg] == 6){
        tft.fillRect(((xg+1)*5)+xk,((yg+1)*5)+yk,5,5,TFT_MAGENTA); // Hard Enemy
      }
      if(gameMatrix[yg][xg] == 7){
        tft.fillRect(((xg+1)*5)+xk,((yg+1)*5)+yk,5,5,TFT_BROWN);  // Very Hard Enemy
      }
      if(gameMatrix[yg][xg] == 8){
        tft.fillRect(((xg+1)*5)+xk,((yg+1)*5)+yk,5,5,TFT_ORANGE); // Weapon
      }
      if(gameMatrix[yg][xg] == 9){
        tft.fillRect(((xg+1)*5)+xk,((yg+1)*5)+yk,5,5,TFT_BLUE); // YOU
      }
      if(gameMatrix[yg][xg] == 0){
        tft.fillRect(((xg+1)*5)+xk,((yg+1)*5)+yk,5,5,TFT_BLACK);  // Void
      }

      n++;
      xg++;
      if (xg == 22){
        xg = 0;
        yg++;
      }
      if (yg == 23){
        return;
      }        
    }
    xg = 0;
    yg = 0;
    
    
  const int keyCount = keyboard.keyCount();
//  if (keyCount == 0)
//    return;

  const BBQ10Keyboard::KeyEvent key = keyboard.keyEvent();
  String state = "pressed";
  

  // pressing 'b' turns off the backlight and pressing Shift+b turns it on
  if (key.state == BBQ10Keyboard::StatePress) {
    if (key.key == '\n') {
      delay(1);
    }
    if (key.key == 'w') {
      if (y1 > 0){
        gameMatrix[y1][x1] = 0;
        y1--;
        gameMatrix[y1][x1] = 9;
        
      }
    }
    if (key.key == 's') {
      if (y1 < 21){
        gameMatrix[y1][x1] = 0;
        y1++;
        gameMatrix[y1][x1] = 9;
      }
    }
    if (key.key == 'a') {
      if (x1 > 0){
        gameMatrix[y1][x1] = 0;
        x1--;
        gameMatrix[y1][x1] = 9;
      }
    }
    if (key.key == 'd') {
      if (x1 < 21) {
        gameMatrix[y1][x1] = 0;
        x1++;
        gameMatrix[y1][x1] = 9;
      }
    }
    if (key.key == ' ') {
     tft.fillScreen(TFT_BLACK);
     tft.setCursor(0, 0, 1);
     tft.setTextColor(TFT_GREEN, TFT_BLACK);
     tft.setTextSize(1);
     tft.println("Quit");
      return;
    }
    
    if (key.key == '\n'){
      Serial.println("OK");
    }
    else if (key.key == '~') {
      delay(1);
    }
    
  }  
  }
}

void lexer(int n){
  String cStr = "";
  int m = 0;
  int o = 0;
  String tempName = "";
  myChar[varNumber[0]] = 0;
  myBool[varNumber[1]] = 0;
  myInt[varNumber[2]] = 0;
  myFloat[varNumber[3]] = 0;
  myDouble[varNumber[4]] = 0;
  myLong[varNumber[5]] = 0;
  while (n != numStr){
    cStr = String(code[n]);
    cStr.trim();
    if (cStr[0] == 'i' and cStr[1] == 'n' and cStr[2] == 't' and cStr[3] == ' '){
      m = 4;
      while (cStr[m] != '='){     // Определили длинну имени переменной
        if (isAlpha(cStr[m])){
          m++;
        }
      }
      o = 4;
      tempName = "";
      while(o != m){              // Определили имя переменной
        tempName = tempName + cStr[o];
        o++;
      }
      intName[varNumber[2]] = tempName;  
      Serial.println("Say Var Name - " + String(intName[varNumber[2]]));
      m++;
      while (cStr[m] != ';'){     // Определили значение переменной
        if (isDigit(cStr[m])){
          m++;
        }
      }
      o++;
      tempName = "";
      while(o != m){         
        tempName = tempName + cStr[o];
        o++;
      }
      unsigned char* buf = new unsigned char[100];  // О чудо!!!! Преобразование String в const char*
      tempName.getBytes(buf, 100, 0);
      const char *str2 = (const char*)buf;
      myInt[varNumber[2]] = atoi(str2);  
      Serial.println("Varible value - " + String(myInt[varNumber[2]]));
      varNumber[2]++;
    }
    else {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("\nSyntax Error in line: " + String(n+1));
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      return;
    } 
    n++;
  }
  
}


void exe(const char * s) {
   if (s[0] == 'c' and s[1] == 'l' and s[2] == 's'){
     tft.fillScreen(TFT_BLACK);
     tft.setCursor(0, 0, 1);
     tft.setTextColor(TFT_GREEN, TFT_BLACK);
     tft.setTextSize(1);
   }
   else if (s[0] == 'd' and s[1] == 'o' and s[2] == 's' and s[3] == 'c' and s[4] == 'o' and s[5] == 'l' and s[6] == 'o' and s[7] == 'r'){
     tft.fillScreen(TFT_BLUE);
     tft.setCursor(0, 0, 1);
     tft.setTextColor(TFT_WHITE, TFT_BLUE);
     tft.setTextSize(1); 
   }
   else if (s[0] == 't' and s[1] == 'e' and s[2] == 'r' and s[3] == 'm' and s[4] == 'i' and s[5] == 'n' and s[6] == 'a' and s[7] == 't' and s[8] == 'o' and s[9] == 'r'){
     tft.fillScreen(TFT_RED);
     tft.setCursor(0, 0, 1);
     tft.setTextColor(TFT_WHITE, TFT_RED);
     tft.setTextSize(1); 
   }
   else if (s[0] == 'l' and s[1] == 's'){  // Просмотр текущей директории
     int tmp = 0;
     int n = 0;
     if (pwd != "/"){
       n = pwd.lastIndexOf('/');
       Serial.println(n);
       if (n > 1){
         pwd.remove(n);
         Serial.println(pwd);
         tmp = 1;
       }
     }
     unsigned char* buf = new unsigned char[255]; 
     Serial.println(pwd);
     pwd.getBytes(buf, 100, 0);
     const char *str2 = (const char*)buf;
     listDir(SD, str2, 2); 
     if(pwd.endsWith("/") and tmp == 1){
       pwd.setCharAt(n, '/');
       Serial.println("CharAt");
       Serial.println(pwd);
     }
   }
   else if (s[0] == 'e' and s[1] == 'x' and s[2] == 'e' and s[3] == 'c' and s[4] == ' '){
     if (s[4] == ' ' and s[5] == '\"'){
       int n = 6;
       int m = 6;
       unsigned char* buf = new unsigned char[100]; 
       String tempName = "";
       while (s[n] != '\"'){
         if (s[n] != '\"'){
           n++;   
         }
       while(m != n){         
         tempName = tempName + s[m];
         m++;
       } 
       }
       tempName = pwd + tempName;
       tempName.getBytes(buf, 100, 0);
       const char *str2 = (const char*)buf;  
       //Serial.println(str2);
       n=0;
       int nStr = 0;
       char ch;
       String stroka;

       if(SD.exists(str2)){
         myFile = SD.open(str2);
         while (myFile.available()) {
           ch = char(myFile.read());
           if (ch == '\n'){
             //Serial.println(nStr);
             unsigned char* buf = new unsigned char[100]; 
             stroka.getBytes(buf, 100, 0);
             const char *str2 = (const char*)buf;
             code[nStr] = str2;
             stroka = "";
             //Serial.println(code[nStr]);
             nStr++;
             numStr++;
           }
           else {
             stroka += ch;
           }
         }
         lexer(0);
         myFile.close();
       }
       else {tft.println("\nFile does not exist");}
     }
     else{tft.println("\nSyntax Error");}
   }
   else if (s[0] == 'r' and s[1] == 'e' and s[2] == 'a' and s[3] == 'd' and s[4] == ' '){
     if (s[4] == ' ' and s[5] == '\"'){
       int n = 6;
       int m = 6;
       unsigned char* buf = new unsigned char[100]; 
       String tempName = "";
       while (s[n] != '\"'){
         if (s[n] != '\"'){
           n++;   
         }
       while(m != n){         
         tempName = tempName + s[m];
         m++;
       } 
       }
       tempName = pwd + tempName;
       tempName.getBytes(buf, 100, 0);
       const char *str2 = (const char*)buf;  
       Serial.println(str2);
       readFile(SD, str2);
     }
     else{tft.println("\nSyntax Error");}
   }
   else if (s[0] == 't' and s[1] == 'o' and s[2] == 'u' and s[3] == 'c' and s[4] == 'h' and s[5] == ' '){
    if (s[5] == ' ' and s[6] == '\"'){
       int n = 7;
       int m = 7;
       unsigned char* buf = new unsigned char[100]; 
       String tempName = "";
       while (s[n] != '\"'){
         if (s[n] != '\"'){
           n++;   
         }
       while(m != n){         
         tempName = tempName + s[m];
         m++;
       } 
       }
       tempName = pwd + tempName;
       tempName.getBytes(buf, 100, 0);
       const char *str2 = (const char*)buf;  
       Serial.println(str2);
       tft.println("");
       myFile = SD.open(str2, FILE_WRITE);
       myFile.close(); 
       tft.println("File Created");
     }
     else{tft.println("\nSyntax Error");}
       
   }
   else if (s[0] == 'r' and s[1] == 'm' and s[2] == ' '){
    if (s[2] == ' ' and s[3] == '\"'){
       int n = 4;
       int m = 4;
       unsigned char* buf = new unsigned char[100]; 
       String tempName = "";
       while (s[n] != '\"'){
         if (s[n] != '\"'){
           n++;   
         }
       while(m != n){         
         tempName = tempName + s[m];
         m++;
       } 
       }
       tempName = pwd + tempName;
       tempName.getBytes(buf, 100, 0);
       const char *str2 = (const char*)buf;  
       Serial.println(str2);
       tft.println("");
       deleteFile(SD, str2);
     }
     else{tft.println("\nSyntax Error");}
       
   }
   else if (s[0] == 'm' and s[1] == 'k' and s[2] == 'd' and s[3] == 'i' and s[4] == 'r' and s[5] == ' '){
    if (s[5] == ' ' and s[6] == '\"'){
       int n = 7;
       int m = 7;
       unsigned char* buf = new unsigned char[100]; 
       String tempName = "";
       while (s[n] != '\"'){
         if (s[n] != '\"'){
           n++;   
         }
       while(m != n){         
         tempName = tempName + s[m];
         m++;
       } 
       }
       tempName = pwd + tempName;
       tempName.getBytes(buf, 100, 0);
       const char *str2 = (const char*)buf;  
       Serial.println(str2);
       tft.println("");
       createDir(SD, str2);
     }
     else{tft.println("\nSyntax Error");}
       
   }
   else if (s[0] == 'r' and s[1] == 'm' and s[2] == 'd' and s[3] == 'i' and s[4] == 'r' and s[5] == ' '){
    if (s[5] == ' ' and s[6] == '\"'){
       int n = 7;
       int m = 7;
       unsigned char* buf = new unsigned char[100]; 
       String tempName = "";
       while (s[n] != '\"'){
         if (s[n] != '\"'){
           n++;   
         }
       while(m != n){         
         tempName = tempName + s[m];
         m++;
       } 
       }
       tempName = pwd + tempName;
       tempName.getBytes(buf, 100, 0);
       const char *str2 = (const char*)buf;  
       Serial.println(str2);
       tft.println("");
       removeDir(SD, str2);
     }
     else{tft.println("\nSyntax Error");}
       
   }
   else if (s[0] == 'c' and s[1] == 'd' and s[2] == ' '){
    if (s[2] == ' ' and s[3] == '\"'){
       int n = 4;
       int m = 4;
       unsigned char* buf = new unsigned char[100]; 
       String tempName = "";
       while (s[n] != '\"'){
         if (s[n] != '\"'){
           n++;   
         }
       while(m != n){         
         tempName = tempName + s[m];
         m++;
       } 
       }
       tempName = pwd + tempName;
       if(SD.exists(tempName)){
         pwd = tempName;
         tempName.getBytes(buf, 100, 0);
         const char *str2 = (const char*)buf;
         tft.println("");
         tft.println(str2);
         pwd = tempName + '/';
       }  
       else {tft.println("Directory not found");}
       
     }
     else if (s[2] == ' ' and s[3] == '/'){
       pwd = "/";
       tft.println("");
       tft.println("/");      
     }
     else{tft.println("\nSyntax Error");}
   }
   else if (s[0] == 'p' and s[1] == 'w' and s[2] == 'd'){
    tft.println("");
    tft.println(pwd);
   }
   else if (s[0] == 'd' and s[1] == 'a' and s[2] == 't' and s[3] == 'e'){
     readPCF8563();
     tft.println("");
     tft.print(days[dayOfWeek]); 
     tft.print(" "); 
     tft.print(dayOfMonth, DEC);
     tft.print("/");
     tft.print(month, DEC);
     tft.print("/20");
     tft.print(year, DEC);
     tft.print(" - ");
     tft.print(hour, DEC);
     tft.print(":");
     if (minute < 10)
     {
      tft.print("0");
     }
     tft.print(minute, DEC);
     tft.print(":"); 
     if (second < 10)
     {
      tft.print("0");
     } 
     tft.println(second, DEC); 
    
   }
   else if (s[0] == 'r' and s[1] == 'e' and s[2] == 'b' and s[3] == 'o' and s[4] == 'o' and s[5] == 't'){
     tft.println("\nRestarting system...");
     ESP.restart();
   }
   else if (s[0] == 'p' and s[1] == 'r' and s[2] == 'i' and s[3] == 'n' and s[4] == 't'){
    delay(1);
   }
   else if (s[0] == 'i' and s[1] == 'f' and s[2] == 'c' and s[3] == 'o' and s[4] == 'n' and s[5] == 'f' and s[6] == 'i' and s[7] == 'g'){
      tft.print("\nIP Address:           ");
      tft.println(WiFi.localIP());
      tft.print("MAC Address:          ");
      tft.println(WiFi.macAddress());
      tft.print("Subnet:               ");
      tft.println(WiFi.subnetMask());
      tft.print("Gateway:              ");
      tft.println(WiFi.gatewayIP());
   }
   else if (s[0] == 'e' and s[1] == 'c' and s[2] == 'h' and s[3] == 'o' and s[4] == ' '){
      if (s[4] == ' ' and s[5] == '\"'){
       int n = 6;
       int m = 6;
       unsigned char* buf = new unsigned char[100]; 
       String tempName = "";
       while (s[n] != '\"'){
         if (s[n] != '\"'){
           n++;   
         }
       while(m != n){         
         tempName = tempName + s[m];
         m++;
       } 
       }
       tempName = pwd + tempName;
       tempName.getBytes(buf, 100, 0);
       const char *str2 = (const char*)buf;  
       Serial.println(str2);
       Serial.println(n);
       Serial.println(s[n]);
       Serial.println(m);
       // тут мы определили имя файла
       n = n + 3;
       m = n;
       Serial.println(s[n]);
       tempName = "";
       while (s[n] != '\"'){
         Serial.println(tempName);
         if (s[n] != '\"'){
           n++;   
         }
       while(m != n){         
         if (s[m] != '~'){
         tempName = tempName + s[m];
         m++;
         } else {
          tempName = tempName + '~';
          m++;
         }
       } 
       }
       buf = new unsigned char[100];
       tempName.getBytes(buf, 100, 0);
       const char *str3 = (const char*)buf; 
       // Тут мы определили, что будем записывать в конец файла
       Serial.println(str3);
       Serial.println(n);
       Serial.println(m);
       
       
       writeFile(SD, str2,str3);
   }
   }
   else if (s[0] == 'c' and s[1] == 'o' and s[2] == 'n' and s[3] == 'n' and s[4] == 'e' and s[5] == 'c' and s[6] == 't'){
      WiFi.begin(ssid, password); 
      while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      }
      tft.println("\nWiFi connection established");
      tft.print("IP address: ");
      tft.println(WiFi.localIP());
   }
   else if (s[0] == 's' and s[1] == 'c' and s[2] == 'a' and s[3] == 'n'){
     WiFi.mode(WIFI_STA);
     WiFi.disconnect();
     delay(100);
     int n = WiFi.scanNetworks();
    tft.println("");
    if (n == 0) {
        tft.println("no networks found");
    } else {
      for (int i = 0; i < n; ++i) {
        tft.print(i + 1);
        tft.print(": ");
        tft.print(WiFi.SSID(i));
        tft.print(" (");
        tft.print(WiFi.RSSI(i));
        tft.print(")");
        tft.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
        delay(10);
        if (i >= 9) { break; }
    }
   }  

   }
   else if (s[0] == 'i' and s[1] == 'm' and s[2] == 'a' and s[3] == 'g' and s[4] == 'e' and s[5] == ' '){
     if (s[5] == ' ' and s[6] == '\"'){
       int n = 7;
       int m = 7;
       unsigned char* buf = new unsigned char[100]; 
       String tempName = "";
       while (s[n] != '\"'){
         if (s[n] != '\"'){
           n++;   
         }
       while(m != n){         
         tempName = tempName + s[m];
         m++;
       } 
       }
       tempName = pwd + tempName;
       tempName.getBytes(buf, 100, 0);
       const char *str2 = (const char*)buf;
       if (SD.exists(str2)){
         File jpgFile = SD.open( str2, FILE_READ);  
         
         }
         else { tft.println("\nFile not exist");}
       }
   }
   else if (s[0] == 'W' and s[1] == 'a' and s[2] == 'k' and s[3] == 'e' and s[4] == ' ' and s[5] == 'u' and s[6] == 'p' and s[7] == ',' and s[8] == ' ' and s[9] == 'N' and s[10] == 'e' and s[11] == 'o' and s[12] == '.' and s[13] == '.' and s[14] == '.'){
      delay(2500);
      tft.println("\nThe Matrix has you...");
      delay(2500);
      tft.println("Follow the white rabbit.");
      delay(2500);
      tft.println("");
      tft.println("");
      tft.println("Knock, knock, Neo.");
   }
   else if (s[0] == 'd' and s[1] == 'e' and s[2] == 'm' and s[3] == 'o'){
     tft.fillScreen(TFT_BLACK);
     tft.setCursor(0, 0, 1);
     tft.setTextColor(TFT_GREEN, TFT_BLACK);
     tft.setTextSize(1);
     pingPong();  

   }
   else if (s[0] == 'h' and s[1] == 'e' and s[2] == 'l' and s[3] == 'p'){
    tft.println("\nCD - change directory");
    tft.println("IFCONFIG - net configuration");
    tft.println("SCAN - scan wifi range");
    tft.println("CONNECT - connect to WiFi AP");
    tft.println("ECHO - write text to the end of the file");
    tft.println("DATE - print current date and time ");
    tft.println("PWD - print current directory");
    tft.println("READ - print file. Ex. read \"myFile.txt\"");
    tft.println("LS - list directory");
    tft.println("CLS - clear screen");
    tft.println("TOUCH - create empty file");
    tft.println("RM - remove file");
    tft.println("MKDIR - create directory");
    tft.println("RMDIR - remove directory");
    tft.println("REBOOT - restart computer");
   }
   else {
     tft.println("\nUnknown command " + String(s));
   }
   cmd = "";
}

//void thisIntVar(){
//  int n = 4;
//  while (code[n] != '='){     // Определили длинну имени переменной
//    if (isAlpha(code[n])){
//      n++;
//      Serial.println(n);
//    }
//  }
//  int m = 4;
//  String tempName = "";
//  while(m != n){              // Определили имя переменной
//    tempName = tempName + code[m];
//    m++;
//    Serial.println(tempName);
//  }
//  varName[varNumber[0]] = tempName;  
//  Serial.println(varName[varNumber[0]]);
//  n++;
//  while (code[n] != ';'){     // Определили значение переменной
//    if (isDigit(code[n])){
//      n++;
//      Serial.println(n);
//    }
//  }
//  m++;
//  tempName = "";
//  while(m != n){         
//    tempName = tempName + code[m];
//    m++;
//    Serial.println(tempName);
//  }
//  Serial.println(tempName);
//  myInt[varNumber[0]] = 0;
//  unsigned char* buf = new unsigned char[100];  // О чудо!!!! Преобразование String в const char*
//  tempName.getBytes(buf, 100, 0);
//  const char *str2 = (const char*)buf;
//  myInt[varNumber[0]] = atoi(str2);  
//  Serial.println(myInt[varNumber[0]]);
//  varNumber[0]++;
//  Serial.println(code);

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    tft.println("");
    Serial.printf("Listing directory: %s\n", dirname);
    tft.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        tft.printf("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        tft.printf("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            tft.printf("DIR : ");
            tft.println(file.name());
//            if(levels){
//                listDir(fs, file.name(), levels -1);
//            }
        } else {
            Serial.print("FILE: ");
            Serial.print(file.name());
            Serial.print("SIZE: ");
            Serial.println(file.size());
            tft.print("FILE: ");
            tft.print(file.name());
            tft.print("  SIZE: ");
            tft.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        tft.println("Dir created");
    } else {
        tft.println("Dir creating failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        tft.println("Dir removed");
    } else {
        tft.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    char tempChr;
    tempFile = "";
    Serial.printf("Reading file: %s\n", path);
    File file = fs.open(path);
    if(!file){
        tft.println("\nFailed to open file for reading");
        return;
    }

    tft.println("");
    while(file.available()){
        tempChr = char(file.read());
        tft.print(tempChr);
        //tempFile = tempFile + &tempChr;
    }
    file.close();
    tft.println("");
    
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        tft.println("\nFailed to open file for writing");
        return;
    }
    if(file.println(message)){
        tft.println("\nFile written");
    } else {
        tft.println("\nWrite failed");
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
        tft.println("File deleted");
    } else {
        tft.println("File does not exist");
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
