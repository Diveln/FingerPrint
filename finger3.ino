
#include <FastIO.h>
#include <I2CIO.h>
#include <LCD.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal_SR.h>
#include <LiquidCrystal_SR_LCD3.h>

#include <LiquidCrystal_I2C.h>

/*************************************************** 
  This is an example sketch for our optical Fingerprint sensor

  Designed specifically to work with the Adafruit BMP085 Breakout 
  ----> http://www.adafruit.com/products/751

  These displays use TTL Serial to communicate, 2 pins are required to 
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#define IN1  8
#define IN2  9
#define IN3  10
#define IN4  11

//#define dht_dpin 11 //定義訊號要從Pin A0 進來 
#define KEY_ROWS 4  // 薄膜按鍵的列數
#define KEY_COLS 4  // 薄膜按鍵的行數
#define LCD_ROWS 2  // LCD顯示器的列數
#define LCD_COLS 16 // LCD顯示器的行數

const byte numRows= 4; //number of rows on the keypad
const byte numCols= 4; //number of columns on the keypad

// 設置按鍵模組
char keymap[KEY_ROWS][KEY_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[numRows] = {12,11,10,9}; //Rows 0 to 3
byte colPins[numCols]= {8,7,6,5}; //Columns 0 to 3

Keypad keypad = Keypad(makeKeymap(keymap), rowPins, colPins, KEY_ROWS, KEY_COLS);

String passcode = "4321";   // 預設密碼
String inputCode = "";      // 暫存用戶的按鍵字串
bool acceptKey = true;      // 代表是否接受用戶按鍵輸入的變數，預設為「接受」


LiquidCrystal_I2C lcd(0x27,16,2);
int getFingerprintIDez();
//const int buttonPin = 7;
const int ledPin = 13; // 第13隻接腳請連接到 LED，以便控制 LED 明滅。
uint8_t id;
int sum;
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
//int buttonState = 0; 

void setup()  
{
  lcd.begin();
  lcd.backlight();
  resetLocker();
  // set the data rate for the sensor serial port
  //pinMode(buttonPin, OUTPUT);
  pinMode(ledPin, OUTPUT);             // 控制鎖 設置成 OUTPUT    
  digitalWrite(ledPin, LOW); 
  
  finger.begin(57600);
  Serial.begin(9600);
  
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1);
  }
  Serial.println("Waiting for valid finger...");
  
  
}


void loop()                     // run over and over again
{
  
  
   char key = keypad.getKey();
    
  // 若目前接受用戶輸入，而且有新的字元輸入…
  if (acceptKey && key != NO_KEY) {
if (key == '*') {   // 清除畫面
      clearRow(4);  // 從第4個字元開始清除
      inputCode = "";
    } else if (key == '#') {  // 比對輸入密碼
      checkPinCode();
    } else {
      inputCode += key;  // 儲存用戶的按鍵字元
      lcd.print('*');
    }
  }
  
  getFingerprintIDez();
}

/*
void getButton(){

  buttonState = digitalRead(buttonPin);  
  
  Serial.println("Ready to enroll a fingerprint! Please Type in the ID # you want to save this finger as...");
  id = random(66,158);
  Serial.print("Enrolling ID #");
  Serial.println(id);
  
  while (!  getFingerprintEnroll() );
  
  
}

*/




void clearRow(byte n) {
  byte last = LCD_COLS - n;
  lcd.setCursor(n, 1); // 移動到第2行，"PIN:"之後

  for (byte i = 0; i < last; i++) {
    lcd.print(" ");
  }
  lcd.setCursor(n, 1);
}

void resetLocker() {
  lcd.clear();
  lcd.print("Knock, knock...");
  lcd.setCursor(0, 1);  // 切換到第2行
  lcd.print("PIN:");
  lcd.cursor();

  acceptKey = true;
  inputCode = "";
}

void checkPinCode() {
  acceptKey = false;  // 暫時不接受用戶按鍵輸入
  clearRow(0);        // 從第0個字元開始清除LCD畫面
  lcd.noCursor();
  lcd.setCursor(0, 1);  // 切換到第2行
  // 比對密碼
  if (inputCode == passcode) {
    lcd.print("OPEN");    //開門指令
    openDoor();
  } else {
    lcd.print("***ERROR!!***");
  }
  delay(500);
  resetLocker();     // 重設LCD顯示文字和輸入狀態
}

void openDoor(){
  digitalWrite(ledPin, HIGH); // 設定PIN13腳位為高電位= 0V ，LED 處於發亮狀態!!
  delay(10000); // 等待100 毫秒 (也就是發亮 0.1 秒)
  digitalWrite(ledPin, LOW); // 設定PIN13腳位為高電位= 0V ，LED 處於發亮狀態!!
}

uint8_t readnumber(void) {
  uint8_t num = 0;
  boolean validnum = false; 
  while (1) {
    while (! Serial.available());
    char c = Serial.read();
    if (isdigit(c)) {
       num *= 10;
       num += c - '0';
       validnum = true;
    } else if (validnum) {
      return num;
    }
  }
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  //openDoor(); 
  lcd.print("OK!!!!!!!!");
  
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  {
    //lcd.print("Put Finger");
    //delay(200);
  return -1;
}

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  {
  lcd.print("ERROR!");
  delay(3000);
  lcd.clear();
  resetLocker();
  return -1;}

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  {
  lcd.print("ERROR!");
  delay(3000);
  lcd.clear();
  resetLocker();
  return -1;}
  
  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); 
  Serial.println(finger.confidence);
  lcd.clear();
  lcd.print("OPEN");
  
  //lcd.print(finger.fingerID);
  //delay(3000);
  openDoor();
  resetLocker();
  return finger.fingerID; 
}


uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
}


