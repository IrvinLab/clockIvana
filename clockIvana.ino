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
void dragon(){
// warningi:
// 1 - skelet 1yp, 2 - skelet 2yp, 3 - skelet 3yp, 4 - skelet-mag 4yp, 5 - skelet-mag 5yp 6 - lich 6 yp, 7 - knyaz tmi 7yp
// 8 - goblin 1yp, 9 - goblin-mag 2yp, 10 - goblin-koldun 3 yp, 11 - goblin-charodey 4yp., 12 - goblin 4yp, 13 - goblin 5 yp
// 14 - Огр 5 ур, 15 - Огр 6 ур., 16 - Огр 7 ур., 17 - Орк 2 ур., 18 - Орк 4 ур., 19 - Орк 6 ур., 20 - Орк 9 ур.,
// 21 - Некромант 5 ур., 22 - Некромант 6 ур., 23 - Душекрад 9 ур., 24 - Странник 8 ур., 25 - Орк-шаман 9 ур.,
// 26 - Паук 3 ур., 27 - Паук 6 ур., 28 - Суккуб 7 ур., 29 - Суккуб 8 ур., 30 - Вурдалак 7 ур., 31 - Вампир 8 ур.,
// 32 - Некромант 8 ур., 33 - Цербер 9 ур., 34 - Дракон 10 ур
 int test = 1;
 int rnd = random(99);
 int heal = 100;
 int mana = 0;
 int gold = 0;
 int weapon[] = {0,0,0,0,0,0};
 int currentWeapon = 0;
 int armor[] = {0,0,0,0,0,0,0}; 
 int gameCurrent = 0;
 int lvl = 1; // Уровень
 int lvlScore = 4; // Очки уровня, которые дают для прокачки персонажа за новый уровень
 int hp = 0; // Опыт
 int cast[] = {0,0,0,0,0,0,0,0,0,0};
 if (test == 1){
   heal = 100;
   mana = 0;
   gold = 0;
   weapon[0] = 0;
   weapon[1] = 1;
   weapon[2] = 28;
   weapon[3] = 7;
   weapon[4] = 13;
   weapon[5] = 10;
   currentWeapon = 0;
   gameCurrent = 0;
   lvl = 2; // Уровень
   lvlScore = 8; // Очки уровня, которые дают для прокачки персонажа за новый уровень
   hp = 0; // Опыт
   cast[0] = 5;
   cast[1] = 10;
   cast[2] = 2;
   cast[3] = 7;
   cast[4] = 3;
   cast[5] = 8;
   cast[6] = 4;
   cast[7] = 9;
   cast[8] = 11;
   cast[9] = 12;
   
 } 
 else {
   heal = 100;
   mana = 0;
   gold = 0;
   currentWeapon = 0;
   gameCurrent = 0;
   lvl = 1; // Уровень
   lvlScore = 4; // Очки уровня, которые дают для прокачки персонажа за новый уровень
   hp = 0; // Опыт
 }
int currentCast = 0;
int healScore = 1; // Коэфициент здоровья 
int manaScore = 1; // Коэфициент количества маны
int silaScore = 1; // Коэфициент силы удара
int lovkScore = 1; // Коэфициент уклонения/ловкости
 int luckScore = 1; // Удача
 int diplScore = 1; // Дипломатия
 int mozgScore = 1; // Способности к магии
 int menuPosition = 1; // Позиция курсора меню
 int stroki = 0;
 int warning = 0;
 int healVraga = 0;
 int manaVraga = 0;
 int lvlVraga = 0;
 int silaVraga = 0;
 int lovkVraga = 0;
 int luckVraga = 0;
 int mozgVraga = 0;
 int castVraga[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 int damag = 0;
 int dobicha = 0;
 int tmpHP = 0;
 int tmpUdacha = 0;
 int tmpLovkost = 0;
 int n = 0;
 int m = 0;
String save = "";

 tft.fillScreen(TFT_BLACK);
 tft.setCursor(0, 0, 1);
 tft.setTextColor(TFT_GREEN, TFT_BLACK);
 tft.setTextSize(1);
 tft.println("ABYSS DUNGEON");
 tft.println("");
 tft.println("  MENU:");
 tft.println("1. Start Game<=");
 tft.println("2. Load Game");
 tft.println("3. Exit Game");
 
 while(1){
  
  const int keyCount = keyboard.keyCount();

  const BBQ10Keyboard::KeyEvent key = keyboard.keyEvent();
  String state = "pressed";
  if (key.state == BBQ10Keyboard::StateLongPress){
    state = "held down";
    cmd = cmd + '=';
  }
  else if (key.state == BBQ10Keyboard::StateRelease)
    state = "released";

  // pressing 'b' turns off the backlight and pressing Shift+b turns it on
  if (key.state == BBQ10Keyboard::StatePress) {
    if (key.key == '\n') {
      if (gameCurrent == 0){
        if (menuPosition == 1){
          gameCurrent = 1;
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(0, 0, 1);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.setTextSize(1);
          tft.println("Zadai nachalnogo personaja\n");
          tft.print("Level Scores: ");
          tft.println(lvlScore);
          tft.print("Zdorovie: ");
          tft.println(healScore);
          tft.print("Mana: ");
          tft.println(manaScore);
          tft.print("Sila: ");
          tft.println(silaScore);
          tft.print("Lovkost: ");
          tft.println(lovkScore);
          tft.print("Intellekt: ");
          tft.println(mozgScore);
          tft.print("Ydacha: ");
          tft.println(luckScore);
          tft.print("Diplomatia: ");
          tft.println(diplScore);
          tft.println("Press Y for continue");
        }  
        if (menuPosition == 2){
          delay(1);
        }
        if (menuPosition == 3){
          return;
        }
      }
      if (gameCurrent == 4){
        if (stroki >= 25){
          stroki = 0;
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(0, 0, 1);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.setTextSize(1);
        }
        if (m > 3){m++;} // обнулили счётчик стеков заклинаний
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        rnd = random(99);
        if (rnd >= 1 and rnd <= luckScore){ // Сначала Я бью
          tft.setTextColor(TFT_RED, TFT_BLACK);  // От крита увернуться нельзя
          tft.print("!KRIT! ");
          stroki++;
          damag = ((silaScore*4)+weapon[currentWeapon])*2;
          healVraga = healVraga - damag;
          tft.print("Moy Udar -");
          tft.print(damag);
          tft.println("HP");
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
        }
        else {  // НЕ критовый удар
          rnd = random(99);
          if (rnd >= 1 and rnd <= luckVraga){  // Если у Врага Удача, я промазал
            stroki++;
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("YOU MISS");
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
          }
          else {   // Иначе всё же бью
            stroki++;
            damag = (silaScore*4)+weapon[currentWeapon];
            healVraga = healVraga - damag;
            tft.print("Moy Udar -");
            tft.print(damag);
            tft.println("HP");
          }
        }
        tft.print("HP enemy: ");  // Показываем сколько у Врага осталось
        tft.print(healVraga);
        tft.print(" / ");
        tft.print(manaVraga);
        tft.println(" MANA"); 
        stroki++;
        rnd = random(99);
        if (rnd >= 1 and rnd <= luckVraga){  // Противник Бъёт меня. Я тоже не могу увернуться от Крита
          tft.setTextColor(TFT_RED, TFT_BLACK);
          tft.print("!KRIT! ");
          stroki++;
          damag = (silaVraga*2);
          heal = heal - damag;
          tft.print("Udar Sopernika -");
          tft.print(damag);
          tft.print(" HP. My Heal ");
          tft.println(heal);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
        }
        else {                                                                       // Если противник не кританул
            if (castVraga[1] == 5 and healVraga < tmpHP - 370 and manaVraga >= 400){  // Кастуем магию
              manaVraga -= 400;                                                     // Условия и приоритеты заклинаний
              healVraga += 320; 
              damag = 0;
              tft.setTextColor(TFT_PINK, TFT_BLACK);
              tft.print("Lechenie 5yp: "); tft.print("Enemy HP "); tft.print(healVraga); tft.print("/");tft.print(manaVraga); tft.println("MN"); stroki++;
              tft.setTextColor(TFT_GREEN, TFT_BLACK);
              tft.print("Udar Sopernika -");tft.print(damag);tft.print(" HP. My Heal ");tft.println(heal);
            }
            else if (castVraga[1] == 4 and healVraga < tmpHP - 190 and manaVraga >= 200){  
              manaVraga -= 200;                                                    
              healVraga += 160; 
              damag = 0;
              tft.setTextColor(TFT_PINK, TFT_BLACK);
              tft.print("Lechenie 4yp: "); tft.print("Enemy HP "); tft.print(healVraga); tft.print("/");tft.print(manaVraga); tft.println("MN"); stroki++;
              tft.setTextColor(TFT_GREEN, TFT_BLACK);
              tft.print("Udar Sopernika -");tft.print(damag);tft.print(" HP. My Heal ");tft.println(heal);
            }
            else if (castVraga[2] == 11 and manaVraga >= 250 and luckScore != 1) { 
              manaVraga = manaVraga - 250;
              damag = 0;
              luckScore = 1;
              tft.setTextColor(TFT_PINK, TFT_BLACK);
              tft.print("Proklyatie: "); tft.print("Enemy HP "); tft.print(healVraga); tft.print("/");tft.print(manaVraga); tft.println("MN"); stroki++;
              tft.setTextColor(TFT_GREEN, TFT_BLACK);
              tft.print("Udar Sopernika -");tft.print(damag);tft.print(" HP. My Heal ");tft.println(heal);
            }
            else if (castVraga[2] == 12 and manaVraga >= 275 and lovkScore != 1) { 
              manaVraga = manaVraga - 275;
              damag = 0;
              lovkScore = 1;
              tft.setTextColor(TFT_PINK, TFT_BLACK);
              tft.print("Proklyatie: "); tft.print("Enemy HP "); tft.print(healVraga); tft.print("/");tft.print(manaVraga); tft.println("MN"); stroki++;
              tft.setTextColor(TFT_GREEN, TFT_BLACK);
              tft.print("Udar Sopernika -");tft.print(damag);tft.print(" HP. My Heal ");tft.println(heal);
            }
            else if (castVraga[0] == 10 and manaVraga >= 400) { 
              manaVraga = manaVraga - 400;
              damag = 320;
              heal = heal - damag;
              tft.setTextColor(TFT_PINK, TFT_BLACK);
              tft.print("Fire Ball 5yp: "); tft.print("Enemy HP "); tft.print(healVraga); tft.print("/");tft.print(manaVraga); tft.println("MN"); stroki++;
              tft.setTextColor(TFT_GREEN, TFT_BLACK);
              tft.print("Udar Sopernika -");tft.print(damag);tft.print(" HP. My Heal ");tft.println(heal);
            }
            else if (castVraga[0] == 9 and manaVraga >= 200) { 
              manaVraga = manaVraga - 200;
              damag = 160;
              heal = heal - damag;
              tft.setTextColor(TFT_PINK, TFT_BLACK);
              tft.print("Fire Ball 4yp: "); tft.print("Enemy HP "); tft.print(healVraga); tft.print("/");tft.print(manaVraga); tft.println("MN"); stroki++;
              tft.setTextColor(TFT_GREEN, TFT_BLACK);
              tft.print("Udar Sopernika -");tft.print(damag);tft.print(" HP. My Heal ");tft.println(heal);
            }
            else if (castVraga[0] == 8 and manaVraga >= 100) { 
              manaVraga = manaVraga - 100;
              damag = 80;
              heal = heal - damag;
              tft.setTextColor(TFT_PINK, TFT_BLACK);
              tft.print("Fire Ball 3yp: "); tft.print("Enemy HP "); tft.print(healVraga); tft.print("/");tft.print(manaVraga); tft.println("MN"); stroki++;
              tft.setTextColor(TFT_GREEN, TFT_BLACK);
              tft.print("Udar Sopernika -");tft.print(damag);tft.print(" HP. My Heal ");tft.println(heal);
            }
            else if (castVraga[0] == 7 and manaVraga >= 50) { 
              manaVraga = manaVraga - 50;
              damag = 40;
              heal = heal - damag;
              tft.setTextColor(TFT_PINK, TFT_BLACK);
              tft.print("Fire Ball 2yp: "); tft.print("Enemy HP "); tft.print(healVraga); tft.print("/");tft.print(manaVraga); tft.println("MN"); stroki++;
              tft.setTextColor(TFT_GREEN, TFT_BLACK);
              tft.print("Udar Sopernika -");tft.print(damag);tft.print(" HP. My Heal ");tft.println(heal);
            }
            else if (castVraga[0] == 6 and manaVraga >= 30) {  
              manaVraga = manaVraga - 30;
              damag = 20;
              heal = heal - damag;
              tft.setTextColor(TFT_PINK, TFT_BLACK);
              tft.print("Fire Ball 1yp: "); tft.print("Enemy HP "); tft.print(healVraga); tft.print("/");tft.print(manaVraga); tft.println("MN"); stroki++;
              tft.setTextColor(TFT_GREEN, TFT_BLACK);
              tft.print("Udar Sopernika -");tft.print(damag);tft.print(" HP. My Heal ");tft.println(heal);
            }
            else if (castVraga[1] == 3 and healVraga < tmpHP - 100 and manaVraga >= 100){  
              manaVraga -= 100;                                                    
              healVraga += 80; 
              damag = 0;
              tft.setTextColor(TFT_PINK, TFT_BLACK);
              tft.print("Lechenie 3yp: "); tft.print("Enemy HP "); tft.print(healVraga); tft.print("/");tft.print(manaVraga); tft.println("MN"); stroki++;
              tft.setTextColor(TFT_GREEN, TFT_BLACK);
              tft.print("Udar Sopernika -");tft.print(damag);tft.print(" HP. My Heal ");tft.println(heal);
            }
            else if (castVraga[1] == 2 and healVraga < tmpHP - 60 and manaVraga >= 50){  
              manaVraga -= 50;                                                    
              healVraga += 40; 
              damag = 0;
              tft.setTextColor(TFT_PINK, TFT_BLACK);
              tft.print("Lechenie 2yp: "); tft.print("Enemy HP "); tft.print(healVraga); tft.print("/");tft.print(manaVraga); tft.println("MN"); stroki++;
              tft.setTextColor(TFT_GREEN, TFT_BLACK);
              tft.print("Udar Sopernika -");tft.print(damag);tft.print(" HP. My Heal ");tft.println(heal);
            }
            else if (castVraga[1] == 1 and healVraga < tmpHP - 30 and manaVraga >= 30){  
              manaVraga -= 30;                                                    
              healVraga += 20; 
              damag = 0;
              tft.setTextColor(TFT_PINK, TFT_BLACK);
              tft.print("Lechenie 1yp: "); tft.print("Enemy HP "); tft.print(healVraga); tft.print("/");tft.print(manaVraga); tft.println("MN"); stroki++;
              tft.setTextColor(TFT_GREEN, TFT_BLACK);
              tft.print("Udar Sopernika -");tft.print(damag);tft.print(" HP. My Heal ");tft.println(heal);
            }
            else { // Если противник НЕ владеет заклинаниями ИЛИ кончилась МАНА
            // Но и он может промазать если у Меня Удача
            rnd = random(99);
            if (rnd >= 1 and rnd <= luckScore){
              stroki++;
              tft.println("HIM MISS");
            }
            else {  // Ну а если нет я получаю ответочку
              stroki++;
              damag = silaVraga;
              heal = heal - damag;
              tft.print("Udar Sopernika -");
              tft.print(damag);
              tft.print(" HP. My Heal ");
              tft.println(heal);
            }
          }
        }
        m++; // Переключились на другой стек заклинаний  
        if (heal<= 0){  
          tft.println("Game Over");
          return;
        }
        if (healVraga <= 0){
          if (warning != 1 and warning != 2 and warning != 3 and warning != 4 and warning != 5 and warning != 14 and warning != 15 and warning != 16 and warning != 23 and warning != 26 and warning != 27 and warning != 30 and warning != 33){
            dobicha = 10 * lvlVraga + warning;
            gold = gold + dobicha;
            tft.setTextColor(TFT_GOLD, TFT_BLACK);
            tft.print("Nagrada: ");
            tft.print(dobicha);
            tft.println(" GOLD");
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            dobicha = 0;
            stroki++;
          }
          stroki++;
          hp = hp + tmpHP;
          tft.println("Pobeda!");
          gameCurrent = 2;
          luckScore = tmpUdacha;
          lovkScore = tmpLovkost;
          tmpHP=0;
          heal = 100 + (healScore * 25);
          mana = manaScore * 30;
          healVraga = 0;
          manaVraga = 0;
          castVraga[0] = 0;
          castVraga[1] = 0;
          castVraga[2] = 0;
          castVraga[3] = 0;
        }
      }
    }
    if (key.key == 'w') {   //  ИДЁМ ВПЕРЕД
      heal = 100 + (healScore * 25);
      mana = manaScore * 30;
      if (hp >= 1000 and lvl == 1){  // Level UP
        stroki++;tft.setTextColor(TFT_YELLOW, TFT_BLACK);tft.println("LEVEL UP++ 2 LVL PRESS ENTER");
        lvl = 2; lvlScore = 4; hp = 0; gameCurrent = 0; menuPosition = 1;
      }
      else if (hp >= 1500 and lvl == 2){
        stroki++;tft.setTextColor(TFT_YELLOW, TFT_BLACK);tft.println("LEVEL UP++ 3 LVL PRESS ENTER");
        lvl = 3; lvlScore = 4; hp = 0; gameCurrent = 0; menuPosition = 1;
      }
      else if (hp >= 2750 and lvl == 3){
        stroki++;tft.setTextColor(TFT_YELLOW, TFT_BLACK);tft.println("LEVEL UP++ 4 LVL PRESS ENTER");
        lvl = 4; lvlScore = 4; hp = 0; gameCurrent = 0; menuPosition = 1;
      }
      else if (hp >= 5250 and lvl == 4){
        stroki++;tft.setTextColor(TFT_YELLOW, TFT_BLACK);tft.println("LEVEL UP++ 5 LVL PRESS ENTER");
        lvl = 5; lvlScore = 4; hp = 0; gameCurrent = 0; menuPosition = 1;
      }
      else if (hp >= 7500 and lvl == 5){
        stroki++;tft.setTextColor(TFT_YELLOW, TFT_BLACK);tft.println("LEVEL UP++ 6 LVL PRESS ENTER");
        lvl = 6; lvlScore = 4; hp = 0; gameCurrent = 0; menuPosition = 1;
      }
      else if (hp >= 10500 and lvl == 6){
        stroki++;tft.setTextColor(TFT_YELLOW, TFT_BLACK);tft.println("LEVEL UP++ 7 LVL PRESS ENTER");
        lvl = 7; lvlScore = 4; hp = 0; gameCurrent = 0; menuPosition = 1;
      }
      else if (hp >= 14500 and lvl == 7){
        stroki++;tft.setTextColor(TFT_YELLOW, TFT_BLACK);tft.println("LEVEL UP++ 8 LVL PRESS ENTER");
        lvl = 8; lvlScore = 4; hp = 0; gameCurrent = 0; menuPosition = 1;
      }
      else if (hp >= 19500 and lvl == 8){
        stroki++;tft.setTextColor(TFT_YELLOW, TFT_BLACK);tft.println("LEVEL UP++ 9 LVL PRESS ENTER");
        lvl = 9; lvlScore = 4; hp = 0; gameCurrent = 0; menuPosition = 1;
      }
      else if (hp >= 26000 and lvl == 9){
        stroki++;tft.setTextColor(TFT_YELLOW, TFT_BLACK);tft.println("LEVEL UP++ 10 LVL PRESS ENTER");
        lvl = 10; lvlScore = 4; hp = 0; gameCurrent = 0; menuPosition = 1;
      }
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      damag = 0;
      if (stroki >= 24){
          stroki = 0;
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(0, 0, 1);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.setTextSize(1);
        }
      if (gameCurrent == 1 and menuPosition != 1){
        menuPosition--;
      }
      if (gameCurrent == 2){
        int seed = random(999);
        if (seed == 1){
          tft.println("Ti nashel 1000 zolota");
          gold = gold + 1000;
          stroki++;
        }
        else if (seed == 2){
          tft.println("Skali razverzlis nad golovoi geroya");
          tft.println("Ne chto inoe kak bojectvenniy luch");
          tft.println("Ykazal na serdche igroka"); 
          tft.setTextColor(TFT_GOLD, TFT_BLACK);
          tft.println("+1 Luck Score"); 
          luckScore++;
          stroki += 4;
        }
        else if (seed == 3){
          tft.println("Chtobi ne ymeret ot jajdi vi");
          tft.println("Ubili gigntskogo pauka i vipiv");
          tft.println("Ego krov, vi pochustvovali priliv"); 
          tft.setTextColor(TFT_GOLD, TFT_BLACK);
          tft.println("+1 Power Score"); 
          silaScore++;
          stroki += 4;
        }
        else if (seed == 4){
          tft.println("Pod vashimi nogami okazalsya");
          tft.println("Grib Istini. S`ev ego, vi obreli");
          tft.println("poznania tain magicheskih nauk"); 
          tft.setTextColor(TFT_GOLD, TFT_BLACK);
          tft.println("+1 Magic Score"); 
          mozgScore++;
          stroki += 4;
        }
        else if (seed == 5){
          tft.println("Pod vashimi nogami okazalsya");
          tft.println("Grib Sili. S`ev ego, vi stali");
          tft.println("sposobni nakaplivat bolche"); 
          tft.println("magicheskoy energii");
          tft.setTextColor(TFT_GOLD, TFT_BLACK);
          tft.println("+1 Mana Score"); 
          manaScore++;
          stroki += 5;
        }                           // Существа, населяющие подземелье ++++===========++++++++++===========++++++++++============++++++++===========+++++++++==
        else if (seed >= 11 and seed <= 20) {
          healVraga = 80;manaVraga = 0;lvlVraga = 2;silaVraga = 8;lovkVraga = 2;luckVraga = 2;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na puti stoit Skelet 2 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 2;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 21 and seed <= 30) {
          healVraga = 145;manaVraga = 0;lvlVraga = 3;silaVraga = 11;lovkVraga = 3;luckVraga = 4;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na puti stoit Skelet 3 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+2;gameCurrent=3;warning = 3;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 31 and seed <= 40) {
          healVraga = 200; manaVraga = 90;lvlVraga = 4;silaVraga = 11;lovkVraga = 5;luckVraga = 5;mozgVraga = 1;castVraga[0] = 6;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Pregradil put Skelet-Mag 4 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+2;gameCurrent=3;warning = 4;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+2;warning = 0;
          }
        }
        else if (seed >= 41 and seed <= 50) {
          healVraga = 270; manaVraga = 200;lvlVraga = 5;silaVraga = 19;lovkVraga = 5;luckVraga = 5;mozgVraga = 1;castVraga[0] = 7;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Pregradil put Skelet-Mag 5 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+2;gameCurrent=3;warning = 5;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("Aj murashki po koje...");
            stroki=+2;warning = 0;
          }
        }
        else if (seed >= 51 and seed <= 60) {
          healVraga = 375; manaVraga = 450;lvlVraga = 6;silaVraga = 25;lovkVraga = 6;luckVraga = 6;mozgVraga = 1;castVraga[0] = 8;castVraga[2] = 11;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Pregradil put Lich 6 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+2;gameCurrent=3;warning = 6;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("Aj murashki po koje...");
            stroki=+2;warning = 0;
          }
        }
        else if (seed >= 61 and seed <= 70) {
          healVraga = 545; manaVraga = 700;lvlVraga = 7;silaVraga = 25;lovkVraga = 7;luckVraga = 7;mozgVraga = 1;castVraga[0] = 9;castVraga[2] = 11;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Pregradil put Knyaz Tmi 7 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+2;gameCurrent=3;warning = 7;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("Aj murashki po koje...");
            stroki=+2;warning = 0;
          }
        }
        else if (seed >= 71 and seed <= 80) {
          healVraga = 70;manaVraga = 0;lvlVraga = 1;silaVraga = 10;lovkVraga = 1;luckVraga = 2;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na puti stoit Goblin 1 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 8;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 81 and seed <= 90) {
          healVraga = 130;manaVraga = 90;lvlVraga = 2;silaVraga = 15;lovkVraga = 1;luckVraga = 3;mozgVraga = 0;castVraga[0] = 6;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na puti stoit Goblin-Mag 2 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 9;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 91 and seed <= 100) {
          healVraga = 170;manaVraga = 150;lvlVraga = 3;silaVraga = 15;lovkVraga = 3;luckVraga = 4;mozgVraga = 0;castVraga[0] = 6;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na puti stoit Goblin-Mag 3 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 10;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 101 and seed <= 110) {
          healVraga = 225;manaVraga = 200;lvlVraga = 4;silaVraga = 19;lovkVraga = 5;luckVraga = 6;mozgVraga = 0;castVraga[0] = 7;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Stoit Goblin-Charodey 4 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 11;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 121 and seed <= 130) {
          healVraga = 250;manaVraga = 0;lvlVraga = 4;silaVraga = 24;lovkVraga = 5;luckVraga = 6;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na pyti stoit Goblin 4 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 12;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 131 and seed <= 140) {
          healVraga = 310;manaVraga = 0;lvlVraga = 5;silaVraga = 29;lovkVraga = 5;luckVraga = 10;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na pyti stoit Goblin 5 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 13;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 141 and seed <= 150) {
          healVraga = 370;manaVraga = 0;lvlVraga = 5;silaVraga = 29;lovkVraga = 3;luckVraga = 3;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na pyti stoit Ogr 5 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 14;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 151 and seed <= 160) {
          healVraga = 470;manaVraga = 0;lvlVraga = 6;silaVraga = 35;lovkVraga = 3;luckVraga = 4;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na pyti stoit Ogr 6 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 15;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 161 and seed <= 170) {
          healVraga = 575;manaVraga = 0;lvlVraga = 7;silaVraga = 45;lovkVraga = 5;luckVraga = 6;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na pyti stoit Ogr 7 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 16;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 171 and seed <= 180) {
          healVraga = 145;manaVraga = 0;lvlVraga = 2;silaVraga = 17;lovkVraga = 1;luckVraga = 3;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na puti stoit Ork 2 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 17;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 181 and seed <= 190) {
          healVraga = 280;manaVraga = 0;lvlVraga = 4;silaVraga = 28;lovkVraga = 2;luckVraga = 5;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na puti stoit Ork 4 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 18;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 191 and seed <= 200) {
          healVraga = 500;manaVraga = 0;lvlVraga = 6;silaVraga = 34;lovkVraga = 4;luckVraga = 7;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na puti stoit Ork 6 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 19;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 201 and seed <= 210) {
          healVraga = 900;manaVraga = 0;lvlVraga = 9;silaVraga = 64;lovkVraga = 8;luckVraga = 12;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na puti stoit Ork 9 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 20;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 211 and seed <= 220) {
          healVraga = 300;manaVraga = 250;lvlVraga = 5;silaVraga = 29;lovkVraga = 6;luckVraga = 8;mozgVraga = 0;castVraga[0] = 7;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Stoit Nekromant 5 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 21;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 221 and seed <= 230) {
          healVraga = 350;manaVraga = 400;lvlVraga = 6;silaVraga = 36;lovkVraga = 7;luckVraga = 12;mozgVraga = 0;castVraga[0] = 8;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Stoit Nekromant 6 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 22;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 231 and seed <= 240) {
          healVraga = 1250;manaVraga = 0;lvlVraga = 9;silaVraga = 79;lovkVraga = 20;luckVraga = 20;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na puti stoit Dushekrad 9 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 23;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 241 and seed <= 250) {
          healVraga = 765;manaVraga = 250;lvlVraga = 8;silaVraga = 58;lovkVraga = 8;luckVraga = 12;mozgVraga = 0;castVraga[2] = 11;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na puti stoit Strannik 8 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 24;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 251 and seed <= 260) {
          healVraga = 900;manaVraga = 850;lvlVraga = 9;silaVraga = 55;lovkVraga = 10;luckVraga = 20;mozgVraga = 0;castVraga[2] = 11;castVraga[0] = 8;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Stoit Ork-Shaman 9 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 25;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 261 and seed <= 270) {
          healVraga = 145;manaVraga = 0;lvlVraga = 3;silaVraga = 17;lovkVraga = 3;luckVraga = 3;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na puti stoit Pauk 3 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 26;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 271 and seed <= 280) {
          healVraga = 345;manaVraga = 0;lvlVraga = 6;silaVraga = 42;lovkVraga = 6;luckVraga = 6;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na puti stoit Pauk 6 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 27;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 281 and seed <= 290) {
          healVraga = 365;manaVraga = 500;lvlVraga = 7;silaVraga = 60;lovkVraga = 8;luckVraga = 13;mozgVraga = 0;castVraga[0] = 8;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Stoit Sykkub 7 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 28;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 291 and seed <= 300) {
          healVraga = 425;manaVraga = 800;lvlVraga = 8;silaVraga = 70;lovkVraga = 10;luckVraga = 16;mozgVraga = 0;castVraga[0] = 9;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Stoit Sykkub 8 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 29;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 301 and seed <= 310) {
          healVraga = 590;manaVraga = 0;lvlVraga = 7;silaVraga = 55;lovkVraga = 4;luckVraga = 1;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na puti stoit Vurdalak 7 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 30;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 311 and seed <= 320) {
          healVraga = 795;manaVraga = 0;lvlVraga = 8;silaVraga = 75;lovkVraga = 10;luckVraga = 10;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Stoit Vampir 8 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 31;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 321 and seed <= 330) {
          healVraga = 525;manaVraga = 800;lvlVraga = 8;silaVraga = 60;lovkVraga = 10;luckVraga = 16;mozgVraga = 0;castVraga[0] = 9;castVraga[2] = 11;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Stoit Nekromant 8 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 32;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 331 and seed <= 340) {
          healVraga = 1000;manaVraga = 0;lvlVraga = 9;silaVraga = 84;lovkVraga = 15;luckVraga = 12;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na puti stoit Cerber 9 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 33;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 341 and seed <= 350) {
          healVraga = 1500;manaVraga = 0;lvlVraga = 10;silaVraga = 100;lovkVraga = 25;luckVraga = 25;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na puti stoit Dragon 10 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+3;gameCurrent=3;warning = 34;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else if (seed >= 351 and seed <= 360) {  
          healVraga = 40;manaVraga = 0;lvlVraga = 1;silaVraga = 5;lovkVraga = 1;luckVraga = 1;mozgVraga = 0;
          if (lvl >= lvlVraga) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Na puti stoit Skelet 1 lvl, chto delat?");
            tft.println("Y-boy, D-dogovoritsa, G-bejat");
            stroki=stroki+2;gameCurrent=3;warning = 1;tmpHP = healVraga;
          }else{
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Promelknula yjasnaya ten");
            tft.println("vi bejite so vseh nog");
            stroki=+3;warning = 0;
          }
        }
        else {
          tft.print("Nichego ne proishodit ");
          tft.println(seed);
          stroki++;
        }
        
      }
    }
    if (key.key == 'a') {
      delay(1);
    }
    if (key.key == 'd') {
      if (gameCurrent == 3){
        if (warning >=1 and warning <= 7){
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.println("Dogovoritsya ne vishlo, suchestvo yvlaetsa nejitiy");
          tft.println("Y-boy, D-dogovoritsa, G-bejat");
          stroki = stroki + 2;
        }
      }
    }
    if (key.key == 's') {
      if (gameCurrent == 1 and menuPosition != 7){
        menuPosition++;
      }
      if (gameCurrent == 2){
        save = String(heal) +','+String(mana)+','+String(gold)+','+String(weapon[0])+','+String(weapon[1])+','+String(weapon[2])+','
        +String(weapon[3])+','+String(weapon[4])+','+String(weapon[5])+','+String(gameCurrent)+','+String(lvl)+','+String(hp)+','
        +String(cast[0])+','+String(cast[1])+','+String(cast[2])+','+String(cast[3])+','+String(cast[4])+','+String(cast[5])+','
        +String(cast[6])+','+String(cast[7])+','+String(cast[8])+','+String(cast[9])+','+String(healScore)+','+String(manaScore)+','
        +String(silaScore)+','+String(lovkScore)+','+String(luckScore)+','+String(diplScore)+','+String(mozgScore)+';';  // 29 переменных
      unsigned char* buf = new unsigned char[200]; 
      save.getBytes(buf, 200, 0);
      const char *str2 = (const char*)buf;
      writeFile(SD,"/save.drg",str2);
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.println("Save!!!");
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      stroki++;
      }
    }
    if (key.key == 'l'){
      if (gameCurrent == 2){
        int gameStack[] = {0,0,0,0,0,0,0};
        char load[200];
        n = 0;
        m = 0;
        int z = 0; // Счётчик запятых
        int s = 0; // Счётчик стека/степень
        int tempS = 0;
        int tempInt = 0;
        File mySave = SD.open("/save.drg");
        if (mySave) {
          while(mySave.available()){
            load[n] = (char)mySave.read();  // Эврика!!!! Читаем файлы из файла в переменную
            n++; // Очень важно в дальнейшем знать значение переменной n
          }
        }
      else {tft.println("File \"save.drg\" not exist");}    
      mySave.close();
      Serial.println("Simvolov: ");
      Serial.println(n);
      while (m != n){
        if (load[m] == ',' or load[m] == ';'){
          z++;
          Serial.println("Zapyataya!!!");
          tempInt = 0;
          tempS = s;
          s=0;
          while (s != tempS){
            tempInt = tempInt + gameStack[s] * int(pow(float(10),float(tempS-s)));
//            Serial.println("Stack: ");
//            Serial.println(s);
//            Serial.println(gameStack[s]);
//            Serial.println(int(pow(float(10),float(s))));
//            Serial.println(tempInt);
            s++;
          }
          s=0;
          m++;
          tempInt = tempInt/10;
          Serial.println(tempInt);
          gameStack[0] = gameStack[1] = gameStack[2] = gameStack[3] = gameStack[4] = gameStack[5] = gameStack[6] = 0;
          if (z == 1){heal = tempInt;}
          else if (z == 2){mana = tempInt;}
          else if (z == 3){gold = tempInt;}
          else if (z == 4){weapon[0] = tempInt;}
          else if (z == 5){weapon[1] = tempInt;}
          else if (z == 6){weapon[2] = tempInt;}
          else if (z == 7){weapon[3] = tempInt;}
          else if (z == 8){weapon[4] = tempInt;}
          else if (z == 9){weapon[5] = tempInt;}
          else if (z == 10){gameCurrent = tempInt;}
          else if (z == 11){lvl = tempInt;}
          else if (z == 12){hp = tempInt;}
          else if (z == 13){cast[0] = tempInt;}
          else if (z == 14){cast[1] = tempInt;}
          else if (z == 15){cast[2] = tempInt;}
          else if (z == 16){cast[3] = tempInt;}
          else if (z == 17){cast[4] = tempInt;}
          else if (z == 18){cast[5] = tempInt;}
          else if (z == 19){cast[6] = tempInt;}
          else if (z == 20){cast[7] = tempInt;}
          else if (z == 21){cast[8] = tempInt;}
          else if (z == 22){cast[9] = tempInt;}
          else if (z == 23){healScore = tempInt;}
          else if (z == 24){manaScore = tempInt;}
          else if (z == 25){silaScore = tempInt;}
          else if (z == 26){lovkScore = tempInt;}
          else if (z == 27){luckScore = tempInt;}
          else if (z == 28){diplScore = tempInt;}
          else if (z == 29){mozgScore = tempInt;}          
        }
        Serial.println("GameStack");
        Serial.println(load[m]);
        Serial.println(int(load[m]));
        gameStack[s] = int(load[m])-48;
        Serial.println(s);
        Serial.println(gameStack[s]);
        m++;
        s++;
        
      }
      tft.println("Game Loaded");
      }
    } 
    
    if (key.key == 'i') {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextSize(1);
      tft.println("INFORMATION ");
      tft.print("Level: ");
      tft.print(lvl);
      tft.print(" / XP: ");
      tft.println(hp);
      tft.print("Zdorovie: ");
      tft.println(heal);
      tft.print("Mana: ");
      tft.println(mana);
      tft.print("GOLD: ");
      tft.println(gold);
      tft.print("Jivuchest: ");
      tft.println(healScore);
      tft.print("Magicheskaya Sila: ");
      tft.println(manaScore);
      tft.print("Sila: ");
      tft.println(silaScore);
      tft.print("Lovkost: ");
      tft.println(lovkScore);
      tft.print("Ydacha: ");
      tft.println(luckScore);
      tft.print("Diplomatia: ");
      tft.println(diplScore);
      tft.print("Magicheskie Sposobnosti: ");
      tft.println(mozgScore);  
      stroki = 0;
      stroki = stroki + 12;    
    }
    if (key.key == '+') {
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      if (gameCurrent == 1 and menuPosition == 1 and lvlScore != 0){
        healScore++;
        lvlScore--;
      }
      if (gameCurrent == 1 and menuPosition == 2 and lvlScore != 0){
        manaScore++;
        lvlScore--;
      }
      if (gameCurrent == 1 and menuPosition == 3 and lvlScore != 0){
        silaScore++;
        lvlScore--;
      }
      if (gameCurrent == 1 and menuPosition == 4 and lvlScore != 0){
        lovkScore++;
        lvlScore--;
      }
      if (gameCurrent == 1 and menuPosition == 5 and lvlScore != 0){
        mozgScore++;
        lvlScore--;
      }
      if (gameCurrent == 1 and menuPosition == 6 and lvlScore != 0){
        luckScore++;
        lvlScore--;
      }
      if (gameCurrent == 1 and menuPosition == 7 and lvlScore != 0){
        diplScore++;
        lvlScore--;
      }
      if (gameCurrent == 1){
        tft.fillScreen(TFT_BLACK);
          tft.setCursor(0, 0, 1);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.setTextSize(1);
          tft.println("Zadai nachalnogo personaja\n");
          tft.print("Level Scores: ");
          tft.println(lvlScore);
          tft.print("Zdorovie: ");
          tft.println(healScore);
          tft.print("Mana: ");
          tft.println(manaScore);
          tft.print("Sila: ");
          tft.println(silaScore);
          tft.print("Lovkost: ");
          tft.println(lovkScore);
          tft.print("Intellekt: ");
          tft.println(mozgScore);
          tft.print("Ydacha: ");
          tft.println(luckScore);
          tft.print("Diplomatia: ");
          tft.println(diplScore);
          tft.println("Press Y for continue");
      }
    }
    if (key.key == '-') {
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      if (gameCurrent == 1 and menuPosition == 1 and lvlScore != 4 and healScore != 0){
        healScore--;
        lvlScore++;
      }
      if (gameCurrent == 1 and menuPosition == 2 and lvlScore != 4 and manaScore != 0){
        manaScore--;
        lvlScore++;
      }
      if (gameCurrent == 1 and menuPosition == 3 and lvlScore != 4 and silaScore != 0){
        silaScore--;
        lvlScore++;
      }
      if (gameCurrent == 1 and menuPosition == 4 and lvlScore != 4 and lovkScore != 0){
        lovkScore--;
        lvlScore++;
      }
      if (gameCurrent == 1 and menuPosition == 5 and lvlScore != 4 and mozgScore != 0){
        mozgScore--;
        lvlScore++;
      }
      if (gameCurrent == 1 and menuPosition == 6 and lvlScore != 4 and luckScore != 0){
        luckScore--;
        lvlScore++;
      }
      if (gameCurrent == 1 and menuPosition == 7 and lvlScore != 4 and diplScore != 0){
        diplScore--;
        lvlScore++;
      }
      if (gameCurrent == 1){
        tft.fillScreen(TFT_BLACK);
          tft.setCursor(0, 0, 1);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.setTextSize(1);
          tft.println("Zadai nachalnogo personaja\n");
          tft.print("Level Scores: ");
          tft.println(lvlScore);
          tft.print("Zdorovie: ");
          tft.println(healScore);
          tft.print("Mana: ");
          tft.println(manaScore);
          tft.print("Sila: ");
          tft.println(silaScore);
          tft.print("Lovkost: ");
          tft.println(lovkScore);
          tft.print("Intellekt: ");
          tft.println(mozgScore);
          tft.print("Ydacha: ");
          tft.println(luckScore);
          tft.print("Diplomatia: ");
          tft.println(diplScore);
          tft.println("Press Y for continue");
      }
    }
    if (key.key == 'k') {
      delay(1);
    }
    if (key.key == 'g') {
      if (gameCurrent == 3){
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        if (lovkScore > lovkVraga and luckScore > luckVraga){
          gameCurrent = 2;
          stroki++;
          tft.println("Vam udalos sbajat blagodarya lovkosti i yadache");
        }
        else {
          gameCurrent = 4;
          stroki++;
          tft.println("Sbejat NE udalos, pridetsya vstupit v boy");
        }
      }
    }
    if (key.key == 'y') {
      if (stroki >= 25){
          stroki = 0;
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(0, 0, 1);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.setTextSize(1);
        }
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      if (gameCurrent == 1){
        gameCurrent = 2;
        heal = 100 + (healScore * 25);
        mana = manaScore * 30;
      }
      if (gameCurrent == 2){
         tft.fillScreen(TFT_BLACK);
         tft.setCursor(0, 0, 1);
         tft.setTextColor(TFT_GREEN, TFT_BLACK);
         tft.setTextSize(1);
      }
      if (gameCurrent == 3){
        gameCurrent = 4;
        stroki++;
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0, 1);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setTextSize(1);
        tft.println("ENT-Udar, E-sm.Oruj, M-Cast Back-MagUdar");
        if (warning == 1){tft.println("Skelet 1yp.");}
        else if (warning == 2){tft.println("Skelet 2yp.");}
        else if (warning == 3){tft.println("Skelet 3yp.");}
        else if (warning == 4){tft.println("Skelet-Mag 4yp.");}
        else if (warning == 5){tft.println("Skelet-Mag 5yp.");}
        else if (warning == 6){tft.println("Lich 6yp.");}
        else if (warning == 7){tft.println("Knyaz Tmi 7yp.");}
        else if (warning == 8){tft.println("Goblin 1yp.");}
        else if (warning == 9){tft.println("Goblin-Mag 2yp.");}
        else if (warning == 10){tft.println("Goblin-Koldun 3yp.");}
        else if (warning == 11){tft.println("Goblin-Charodey 4yp.");}
        else if (warning == 12){tft.println("Goblin 4yp.");}
        else if (warning == 13){tft.println("Goblin 5yp.");}
        else if (warning == 14){tft.println("Ogr 5yp.");}
        else if (warning == 15){tft.println("Ogr 6yp.");}
        else if (warning == 16){tft.println("Ogr 7yp.");}
        else if (warning == 17){tft.println("Ork 2yp.");}
        else if (warning == 18){tft.println("Ork 4yp.");}
        else if (warning == 19){tft.println("Ork 6yp.");}
        else if (warning == 20){tft.println("Ork 9yp.");}
        else if (warning == 21){tft.println("Nekromant 5yp.");}
        else if (warning == 22){tft.println("Nekromant 6yp.");}
        else if (warning == 23){tft.println("Dushekrad 9yp.");}
        else if (warning == 24){tft.println("Strannik 8yp.");}
        else if (warning == 25){tft.println("Ork-Shaman 9yp.");}
        else if (warning == 26){tft.println("Pauk 3yp.");}
        else if (warning == 27){tft.println("Pauk 6yp.");}
        else if (warning == 28){tft.println("Sykkub 7yp.");}
        else if (warning == 29){tft.println("Sykkub 8yp.");}
        else if (warning == 30){tft.println("Vurdalak 7yp.");}
        else if (warning == 31){tft.println("Vampir 8yp.");}
        else if (warning == 32){tft.println("Nekromant 8yp.");}
        else if (warning == 33){tft.println("Cerber 9yp.");}
        else if (warning == 33){tft.println("DRAGON 10 LVL.");}
        tft.print("Zdorovie: ");
        tft.println(healVraga);
        tft.print("Mana: ");
        tft.println(manaVraga);
        tft.print("Lovkost: ");
        tft.println(lovkVraga);
        tft.print("Udacha: ");
        tft.println(luckVraga);
        tmpUdacha = luckScore;
        tmpLovkost = lovkScore;
        
      }
    }
    if (key.key == 'n') {
      delay(1);
    }
    if (key.key == 'm') {
     if (gameCurrent == 2 or gameCurrent == 4){ 
      if (stroki >= 25){
          stroki = 0;
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(0, 0, 1);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.setTextSize(1);
      }
      currentCast++;
      stroki++;
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      if (currentCast >= 10){currentCast = 0;}
      if (cast[currentCast] == 0){
        tft.println("Empty");
      }
      else if (cast[currentCast] == 1){
        tft.println("Lechenie 1yp (20HP/30MN)");
      }
      else if (cast[currentCast] == 2){
        tft.println("Lechenie 2yp (40HP/50MN)");
      }
      else if (cast[currentCast] == 3){
        tft.println("Lechenie 3yp (80HP/100MN)");
      }
      else if (cast[currentCast] == 4){
        tft.println("Lechenie 4yp (160HP/200MN)");
      }
      else if (cast[currentCast] == 5){
        tft.println("Lechenie 5yp (320HP/400MN)");
      }
      else if (cast[currentCast] == 6){
        tft.println("Fire Ball 1yp (-20HP/30MN)");
      }
      else if (cast[currentCast] == 7){
        tft.println("Fire Ball 2yp (-40HP/50MN)");
      }
      else if (cast[currentCast] == 8){
        tft.println("Fire Ball 3yp (-80HP/100MN)");
      }
      else if (cast[currentCast] == 9){
        tft.println("Fire Ball 4yp (-160HP/200MN)");
      }
      else if (cast[currentCast] == 10){
        tft.println("Fire Ball 5yp (-320HP/400MN)");
      }
      else if (cast[currentCast] == 11){
        tft.println("Proklyatie (1Luck/250MN)");
      }
      else if (cast[currentCast] == 12){
        tft.println("Neuklujest (1Lovk/275MN)");
      }
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
     } 
    }

    if (key.key == '\b'){
     if (gameCurrent == 4){
       if (stroki >= 25){
          stroki = 0;
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(0, 0, 1);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.setTextSize(1);
        }
        damag = 0;
        if (weapon[currentWeapon] == 1){  // Дамаг от магического оружия
           damag = 1;
        }
        else if (weapon[currentWeapon] == 8){
           damag = 8;
        }
        else if (weapon[currentWeapon] == 10){
           damag = 10;
        }
        else if (weapon[currentWeapon] == 16){
           damag = 16;
        }
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        rnd = random(99);
        if (rnd >= 1 and rnd <= luckScore){
          delay(1);
        }
        else {  // Простой удар
          rnd = random(99);
          if (rnd >= 1 and rnd <= luckVraga){ 
            stroki++;
            tft.println("YOU MAGIC FUCKUP");
          }
          else {   // Иначе всё же бью
            stroki++;
            if (cast[currentCast] == 1){
              damag = 0;
              if (mana < 30){
                 tft.print("NET! Y tebia: "); tft.print(heal); tft.print("HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
              else if (heal <= (100+(healScore*25)) - 20 and mana >= 30){
                heal += 20;
                mana -= 30;
                tft.print("Lechenie 1yp: "); tft.print(heal); tft.print("HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
            }
            else if (cast[currentCast] == 2){
              damag = 0;
              if (mana < 50){
                 tft.print("NET! Y tebia: "); tft.print(heal); tft.print("HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
              else if(heal <= (100+(healScore*25)) - 40 and mana >= 50) {
                heal += 40;
                mana -= 50;
                tft.print("Lechenie 2yp: "); tft.print(heal); tft.print("HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
            }
            else if (cast[currentCast] == 3){
              damag = 0;
              if (mana < 100){
                 tft.print("NET! Y tebia: "); tft.print(heal); tft.print("HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
              else if (heal <= (100+(healScore*25)) - 80 and mana >= 100){
                heal += 80;
                mana -= 10;
                tft.print("Lechenie 3yp: "); tft.print(heal); tft.print("HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
            }
            else if (cast[currentCast] == 4){
              damag = 0;
              if (mana < 200){
                 tft.print("NET! Y tebia: "); tft.print(heal); tft.print("HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
              else if (heal <= (100+(healScore*25)) - 120 and mana >= 200){
                heal += 160;
                mana -= 200;
                tft.print("Lechenie 4yp: "); tft.print(heal); tft.print("HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
            }
            else if (cast[currentCast] == 5){
              damag = 0;
              if (mana < 400){
                 tft.print("NET! Y tebia: "); tft.print(heal); tft.print("HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
              else if (heal <= (100+(healScore*25)) - 175 and mana >= 400){
                heal += 320;
                mana -= 400;
                tft.print("Lechenie 5yp: "); tft.print(heal); tft.print("HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
            }
            else if (cast[currentCast] == 6){
              if (mana < 30){
                 tft.print("NET! Y tebia: "); tft.print(heal); tft.print("HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
              else {
                damag = damag + (mozgScore*2) + 20;
                mana -= 30;
                tft.print("Fire Ball 1yp: "); tft.print(heal); tft.print("You HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
            }
            else if (cast[currentCast] == 7){
              if (mana < 50){
                 tft.print("NET! Y tebia: "); tft.print(heal); tft.print("You HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
              else {
                damag = damag + (mozgScore*2) + 40;
                mana -= 50;
                tft.print("Fire Ball 2yp: "); tft.print(heal); tft.print("You HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
            }
            if (cast[currentCast] == 8){
              if (mana < 100){
                 tft.print("NET! Y tebia: "); tft.print(heal); tft.print("You HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
              else {
                damag = damag + (mozgScore*2) + 80;
                mana -= 100;
                tft.print("Fire Ball 3yp: "); tft.print(heal); tft.print("You HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
            }
            if (cast[currentCast] == 9){
              if (mana < 200){
                 tft.print("NET! Y tebia: "); tft.print(heal); tft.print("You HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
              else {
                damag = damag + (mozgScore*2) + 160;
                mana -= 200;
                tft.print("Fire Ball 4yp: "); tft.print(heal); tft.print("You HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
            }
            if (cast[currentCast] == 10){
              if (mana < 400){
                 tft.print("NET! Y tebia: "); tft.print(heal); tft.print("You HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
              else {
                damag = damag + (mozgScore*2) + 320;
                mana -= 400;
                tft.print("Fire Ball 5yp: "); tft.print(heal); tft.print("You HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
            }
            if (cast[currentCast] == 11 and luckVraga != 1){
              if (mana < 250){
                 tft.print("NET! Y tebia: "); tft.print(heal); tft.print("You HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
              else {
                luckVraga = 1;
                mana -= 250;
                tft.print("Proklyatie: "); tft.print(heal); tft.print("You HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
            } else if (cast[currentCast] == 11 and luckVraga > 1){ tft.println("Proklyatie ne vozimeet effecta");}
            if (cast[currentCast] == 12 and lovkVraga != 1){
              if (mana < 275){
                 tft.print("NET! Y tebia: "); tft.print(heal); tft.print("You HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
              else {
                lovkVraga = 1;
                mana -= 275;
                tft.print("Neuklujest: "); tft.print(heal); tft.print("You HP/"); tft.print(mana); tft.println("MN"); stroki++;
              }
            } else if (cast[currentCast] == 12 and lovkVraga > 1) { tft.println("Neuklujest ne vozimeet effecta");}
            healVraga = healVraga - damag;
            tft.print("Udar -");
            tft.print(damag);
            tft.println("HP");
          }
        }
        tft.print("HP enemy: ");  // Показываем сколько у Врага осталось
        tft.println(healVraga);
        stroki++;
        rnd = random(99);
        if (rnd >= 1 and rnd <= luckVraga){  // Я тоже не могу увернуться от Крита
          tft.setTextColor(TFT_RED, TFT_BLACK);
          tft.print("!KRIT! ");
          stroki++;
          damag = (silaVraga*2);
          heal = heal - damag;
          tft.print("Udar Sopernika -");
          tft.print(damag);
          tft.print(" HP. My Heal ");
          tft.println(heal);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
        }
        else {   // Но и он может промазать если у Меня Удача
          rnd = random(99);
          if (rnd >= 1 and rnd <= luckScore){
            stroki++;
            tft.println("HIM MISS");
          }
          else {  // Ну а если нет я получаю ответочку
            stroki++;
            damag = silaVraga;
            heal = heal - damag;
            tft.print("Udar Sopernika -");
            tft.print(damag);
            tft.print(" HP. My Heal ");
            tft.println(heal);
          }
        }
        if (heal<= 0){  
          tft.println("Game Over");
          return;
        }
        if (healVraga <= 0){
          if (warning > 5){
            dobicha = 10 * lvlVraga + warning;
            gold = gold + dobicha;
            tft.setTextColor(TFT_GOLD, TFT_BLACK);
            tft.print("Nagrada: ");
            tft.print(dobicha);
            tft.println(" GOLD");
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            dobicha = 0;
            stroki++;
          }
          stroki++;
          hp = hp + tmpHP;
          tft.setTextColor(TFT_YELLOW, TFT_BLACK);
          tft.println("Pobeda!");
          gameCurrent = 2;
          tmpHP=0;
          heal = 100 + (healScore * 25);
          mana = manaScore * 30;
          healVraga = 0;
          manaVraga = 0;
          castVraga[0] = 0;
          castVraga[1] = 0;
          castVraga[2] = 0;
          castVraga[3] = 0;
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
        }
     }
    }
    if (key.key == 'p') {
      delay(1);
    }
    if (key.key == 'C') {
      tft.fillScreen(TFT_BLACK);
          tft.setCursor(0, 0, 1);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.setTextSize(1);
          tft.println("Exit\n");
      return;
    }
    if (key.key == '1') {
      delay(1);
    }
    if (key.key == '2') {
      delay(1);
    }
    if (key.key == '3') {
      delay(1);
    } 
    if (key.key == '4') {
      delay(1);
    }
    if (key.key == '5') {
      delay(1);
    }
    if (key.key == '6') {
      delay(1);
    }
    if (key.key == '7') {
      delay(1);
    }
    if (key.key == '8') {
      delay(1);
    }
    if (key.key == '9') {
      delay(1);
    }
    if (key.key == '0') {
      delay(1);
    }
    if (key.key == ' ') {
      delay(1);
    }
    if (key.key == 'e'){
      if (stroki >= 28){
          stroki = 0;
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(0, 0, 1);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.setTextSize(1);
        }
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      currentWeapon++;
      if (currentWeapon == 6){
        currentWeapon = 0;
      }
      if (weapon[currentWeapon] == 0){
        tft.println("Kulaki");
      }
      if (weapon[currentWeapon] == 1){
        tft.println("Volshebnaya palochka 1yp.(+1 Mdmg)");
      }
      if (weapon[currentWeapon] == 2){
        tft.println("Korotkii mech 1yp. (+2 dmg)");
      }
      if (weapon[currentWeapon] == 3){
        tft.println("Topor 1yp. (+3 dmg)");
      }
      if (weapon[currentWeapon] == 4){
        tft.println("Korotkii mech 2yp. (+4 dmg)");
      }
      if (weapon[currentWeapon] == 5){
        tft.println("Posoh adepta 2yp. (+5 Mdmg)");
      }
      if (weapon[currentWeapon] == 6){
        tft.println("Topor 2yp. (+6 dmg)");
      }
      if (weapon[currentWeapon] == 7){
        tft.println("Korotkii mech 3yp. (+7 dmg)");
      }
      if (weapon[currentWeapon] == 8){
        tft.println("Volshebnaya palochka 2yp. (+8 Mdmg)");
      }
      if (weapon[currentWeapon] == 9){
        tft.println("Mech 1yp. (+9 dmg)");
      }
      if (weapon[currentWeapon] == 10){
        tft.println("Posoh maga 3yp.(+10 Mdmg)");
      }
      if (weapon[currentWeapon] == 11){
        tft.println("Topor voina 3yp.(+11 dmg)");
      }
      if (weapon[currentWeapon] == 12){
        tft.println("Topor klana Ordi 3yp.(+12 dmg)");
      }
      if (weapon[currentWeapon] == 13){
        tft.println("Rapira 1yp.(+13 dmg)");
      }
      if (weapon[currentWeapon] == 14){
        tft.println("Bulava 1yp.(+14 dmg)");
      }
      if (weapon[currentWeapon] == 15){
        tft.println("Topor Varvara 3yp. (+15 dmg)");
      }
      if (weapon[currentWeapon] == 16){
        tft.println("Posoh kolduna 4yp. (+16 Mdmg)");
      }
      if (weapon[currentWeapon] == 28){
        tft.println("Topor Dush 4yp. (+28 dmg)");
      }
      stroki++;
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

   else if (s[0] == 's' and s[1] == 'd' and s[2] == 'i' and s[3] == 'n' and s[4] == 'i' and s[5] == 't'){
     tft.println("\nMicroSD Card Initialization...");
    if(!SD.begin()){
        tft.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE){
        tft.println("No SD card attached");
        return;
    }
    tft.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        tft.println("MMC");
    } else if(cardType == CARD_SD){
        tft.println("SDSC");
    } else if(cardType == CARD_SDHC){
        tft.println("SDHC");
    } else {
        tft.println("UNKNOWN");
    }
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    tft.printf("SD Card Size: %lluMB\n", cardSize);
    tft.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    tft.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
    tft.println("");
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
   else if (s[0] == 'd' and s[1] == 'r' and s[2] == 'a' and s[3] == 'g' and s[4] == 'o' and s[5] == 'n'){
     dragon();
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
