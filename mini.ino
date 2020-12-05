/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read new NUID from a PICC to //Serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a //mfrc522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to the read data from a PICC (that is: a RFID Tag or Card) using a //mfrc522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the //mfrc522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the //mfrc522 Reader/PCD, the serial output
 * will show the type, and the NUID if a new card has been detected. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 * 
 * @license Released into the public domain.
 * 
 * Typical pin layout used
 * -----------------------------------------------------------------------------------------
 *             //mfrc522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */

#include <SPI.h>
#include <avr/wdt.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Keypad.h>
#include <Adafruit_NeoPixel.h>
#define LEDCount 210

Adafruit_NeoPixel strip = Adafruit_NeoPixel( LEDCount, 13, NEO_RGB + NEO_KHZ800 );


#define SERVO_PIN 9
#define SS_PIN 10
#define RST_PIN 9
#define BUZZER_PIN 2
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo Servo1;

char WebReq[16] = "Web Request";
char InsideButton[16] = "Inside Req";
char OutsidePassword[16] = "Password Pass";
char* OpenFor = OutsidePassword;


LiquidCrystal_I2C lcd(0x27, 20, 4);

int userKeyPattern[9] = {3, 2, 9, 2, 3, 3, 6, 6, 6};
int UserKeyPatternNext[9];

int userKeyInput[128];
int userKeyInputIndex = 0;
int userKeyLastInput = -1;
int userKeyLastPressed = -1;



const byte rows = 4; //four rows
const byte cols = 3; //three columns
char keys[rows][cols] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'#', '0', '*'}};
byte rowPins[rows] = {4, 5, 6, 7}; //connect to the row pinouts of the keypad
byte colPins[cols] = {8, 12, 10};   //connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

unsigned long CardAvailbleIDs[10];
int CardNum = 10;

float Lux = 1;
//mode 0 not mode 1 all
void setLED(int where, int r, int g, int b, int mode){
  if(mode == 0){
    if(where % 2 == 0 && where < LEDCount)strip.setPixelColor(where, b*Lux, r*Lux, g*Lux);
    else strip.setPixelColor(where, 0,0,0);
  }
  else 
    if(where < LEDCount)strip.setPixelColor(where, b*Lux, r*Lux, g*Lux);
}

//total 60*5=300 led

long led_clock1 = 0;
long led_clock2 = 0;
int ledState = 0;
//0 off 1 facedetect 2locking 3unlocking
//2s - 300 : 6ms
unsigned int tail = 0;
unsigned int head = 80;
int lockingUp = 130;
int lockingDown = 130;
int locked = 0;
unsigned int lockedMod = 0;


int unlocked = 0;
int unlockingUp = 0;
int unlockingDown = 300;

int unlockedBlue = 0;

void maintainLED(){
  Serial.print("State: ");
  Serial.println(ledState);

  if(lockedMod > 32760) lockedMod = 0;

  if(userKeyInputIndex > 10){
      clearKey();
      for(int i = 0 ; i < 200; i++){
        setLED(i, 120, 0, 0, 1);
      }
      strip.show();
      setBuzzer(1);
      for(int i = 0 ; i < 100 ; i++){
        for(int i = 0 ; i < 200; i++){
          setLED(i, 250, 0, 0, 1);
        }
        strip.show(); 
        delay(22);
         for(int i = 0 ; i < 200; i++){
          setLED(i, 0, 250, 0, 1);
        }
        strip.show(); 
        delay(22);
        
      }
      setBuzzer(0);
  }

  //6ms
  if(millis() - led_clock1 >= 3){
    led_clock1 = millis();
    if(ledState == 1){
      tail += 1;
      head += 1;
      for(int i = 0 ; i < LEDCount ; i++) setLED(i,0,0,0, 1);
      int j = 0;
      for(int i = (tail%LEDCount); i <= (tail%LEDCount) + 60 ; i ++){
        setLED(i%LEDCount, 255 - abs(j-30) * 8.5, 255 - abs(j-30) * 8.5, 255 - abs(j-30) * 8.5, 1);
        j++;
      }
      strip.show();
    }
    else if(ledState == 2){
      if(locked == 0){
        lockingUp += 1;
        lockingDown -= 1;
        if(lockingUp >= 300) locked = 1;
        for(int i = lockingDown ; i <= lockingUp ; i++){
          setLED(i, 255, 0, 0, 0);
        }
        strip.show();
      } 
    }
    else if(ledState == 3){
      if(unlocked == 0){
        unlockingUp ++;
        unlockingDown --;
        if(unlockingDown - unlockingUp <= 5){
          unlocked = 1;
        }
        for(int i = 0 ; i <= 300 ; i++){
          if(i >= unlockingUp && i <= unlockingDown){
            setLED(i, 0, 255, 0, 0);
          }
          else setLED(i, 0, 0, 0, 0);
        }
        strip.show();
      }
    }
  }
  if(millis() - led_clock2 >= 20){
    led_clock2 = millis();
    if(ledState == 0){
      for(int i = 0; i <= LEDCount; i ++){
        setLED(i, 0, 0, 0, 1);
      }
      strip.show();
    }
    else if(ledState == 2 && locked){
      lockedMod ++;
      /*
      if(lockedMod % 12 <= 4){
        for(int i = 0 ; i <= LEDCount ; i++){
          setLED(i, 50, 0, 0);
        }
        strip.show();
      }
      else{
        for(int i = 0 ; i <= LEDCount ; i++){
          setLED(i, 0, 0, 0);
        }
        strip.show();
      }
      */
      for(int i = 0; i <= LEDCount; i ++){
        setLED(i, 0, 0, 0, 1);
      }
      int j = 0;
      for(int i = (lockedMod % LEDCount); i <= (lockedMod % LEDCount) + 25 ; i++ ) 
      {
        if(j <= 6 || j>= 19){
          setLED(i%LEDCount, 10, 255, 0, 1);
        }
        else
        {
          setLED(i%LEDCount, 255, 10, 0, 1);
        }
        
        j++;
      }
      Serial.println((lockedMod % LEDCount));
      strip.show();
      Serial.println(66666);

    }
    else if(ledState == 3 && unlocked){
      lockedMod ++;
      if(unlockedBlue == 0){
          for(int i = 0 ; i <= 300 ; i++){
            setLED(i, 0, 0, 50, 0);
        }
        strip.show();
      }
      /*
      if(lockedMod % 80 <= 40){
        for(int i = 0 ; i <= 300 ; i++){
          setLED(i, 0, unlockedBlue, 50, 0);
        } 
        strip.show();
      }
      else{
        for(int i = 0 ; i <= 300 ; i++){
          setLED(i, 0, 0, 0, 0);
          if(unlockedBlue == 0)
            setLED(i, 0, 0, 50, 0);
        }
        strip.show();
      }*/
      else{
        for(int i = 0; i <= LEDCount; i ++){
          setLED(i, 0, 0, 0, 1);
        }
        for(int i = (lockedMod % LEDCount); i <= (lockedMod % LEDCount) + 10 ; i++ )setLED(i, 0, 200, 200, 1);
        Serial.println((lockedMod % LEDCount));
        strip.show();
        Serial.println(66666); 
      }
    }

  }
}


void setAll(int r, int g, int b){
  for(int i = 0 ; i < LEDCount ; i++)setLED(i, r, g, b, 0);
}

/*
void maintainLED(){
  Serial.println(ledState);
  if(ledState == 0) setAll(0,0,0);
  if(ledState == 1) setAll(255,255,255);
  if(ledState == 2) setAll(1,0,0);
  if(ledState == 3) setAll(0,0,1);
  strip.show();
}*/


int KMPTest(int *TestVal, int TestLen)
{
  int k = 0;
  int j = 0;
  while (j < TestLen && k < 9)
  {
    if (k == -1 || TestVal[j] == userKeyPattern[k])
    {
      k++;
      j++;
    }
    else
    {
      k = UserKeyPatternNext[k];
    }
  }
  //Serial.println(k);
  for(int i = 0 ; i < 128 ; i ++) //Serial.print(TestVal[i]);
  ////Serial.print("Testing: "); //Serial.println(TestVal);
  if (k == 9)
    return 1;
  else
    return -1;
}

/*
void ksm()
{
  int j = 0;
  for (int i = 2; i <= n; ++i)
  {
    for (; a[i] != a[j + 1]; j = nx[j])
      ;
    if (a[i] == a[nx[j] + 1])
      ++j;
    nx[i] = j;
      ;
    if (a[i] == a[nx[j] + 1])
      ++j;
    nx[i] = j;
  }
}*/

void generatKMPNextTable()
{
  UserKeyPatternNext[0] = -1;
  int i = -1;
  int j = 0;
  while (j < 9)
  {
    if (i == -1 || userKeyPattern[i] == userKeyPattern[j])
    {
      i++;
      j++;
      UserKeyPatternNext[j] = i;
    }
    else
    {
      i = UserKeyPatternNext[i];
    }
  }
  //for (int i = 0; i < 9; i++)
    //Serial.print(UserKeyPatternNext[i]), //Serial.print(" ");
}
int lt = 255;
void setBuzzer(int state)
{
  if (state == 0)
  {
    digitalWrite(BUZZER_PIN, 1);
  }
  else
    digitalWrite(BUZZER_PIN, 0);
}

unsigned long getID()
{
  if (!mfrc522.PICC_IsNewCardPresent())
    return -1;
  if (!mfrc522.PICC_ReadCardSerial())
  { //Since a PICC placed get Serial and continue
    return -1;
  }
  unsigned long hex_num;
  hex_num = mfrc522.uid.uidByte[0] << 24;
  hex_num += mfrc522.uid.uidByte[1] << 16;
  hex_num += mfrc522.uid.uidByte[2] << 8;
  hex_num += mfrc522.uid.uidByte[3];
  return hex_num;
}



void clearKey()
{
  for (int i = 0; i < 128; i++)
  {
    userKeyInput[i] = 0;
  }
  userKeyInputIndex = 0;
}

long starMOD = 0;

void handleInput()
{
  /*if (userKeyLastPressed == 1)
  {
    if (checkPressedKey() == -1)
    {
      userKeyLastPressed = 0;
      setBuzzer(0);
    }
    if (checkPressedKey() == userKeyLastInput)
      ;
    if (checkPressedKey() != userKeyLastInput)
    {
      userKeyInputIndex++;
      if (userKeyInputIndex > 120)
        clearKey();
      userKeyInput[userKeyInputIndex] = checkPressedKey();
      setBuzzer(1);
    }
  }
  else
  {
    if (checkPressedKey() == -1)
    {
      setBuzzer(0);
    }
    if (checkPressedKey() >= 0)
    {
      userKeyInputIndex++;
      if (userKeyInputIndex > 120)
        clearKey();
      setBuzzer(1);
      userKeyInput[userKeyInputIndex] = checkPressedKey();
      userKeyLastInput = checkPressedKey();
      userKeyLastPressed = 1;
    }
  }*/
  char newInput = keypad.getKey();
  //Serial.println(newInput);
  if(newInput == '*'){
    lt = 255;
    ledState = 1;
    starMOD = millis();
    setBuzzer(1);
    delay(80);
    setBuzzer(0);
  }
  else if( millis() - starMOD  >= 5000 &&  millis() - starMOD <= 5500)
  {
    ledState = 2;
  }
  
  if (newInput >= '0' && newInput <= '9')
  {
    if(userKeyInputIndex >= 120) userKeyInputIndex=0,clearKey();
    userKeyInput[userKeyInputIndex] = newInput - '0';
    //Serial.print("Set Index: "),//Serial.print(userKeyInputIndex),//Serial.print(" TO : "),//Serial.print(userKeyInput[userKeyInputIndex] );
    userKeyInputIndex++;
    setBuzzer(1);
    delay(100);
    setBuzzer(0);
  }
  //Serial.println("UniPrint:");

  //for(int i = 0 ; i < 128 ; i++) //Serial.print(userKeyInput[i]);
}

int checkVaildInputInInputArray()
{
  int res = KMPTest(userKeyInput, 128);
  //Serial.print("KMPtest:"), //Serial.println(res);
  return res;
}

int checkInputVaild()
{
  //Serial.print("Avail");
  handleInput();
  return checkVaildInputInInputArray();
}

int checkButtonPressed()
{
  return -1;
}


void buzzNotice()
{
/*
    setBuzzer(1);
    delay(300);
    setBuzzer(0);
    delay(300);
  */
}


unsigned long OpenDoorUntil = 0;

void displayMessage(char *Line1, char *Line2)
{
  int len1 = 15, len2 = 15;
  for (int i = 0; i < 16; i++)
  {
    if (Line1[i] == 0)
    {
      len1 = i;
    }
    if (Line2[i] == 0)
    {
      len2 = i;
    }
  }
  //Serial.print(len1), //Serial.print(" / "), //Serial.println(len2);
  lcd.clear();
  lcd.setCursor(7 - (len1) / 2, 0);
  lcd.print(Line1);
  lcd.setCursor(7 - (len2) / 2, 1);
  lcd.print(Line2);
}

void setup()
{
  
    Serial.begin(115200);
    Serial.println("ver 3");
    wdt_reset();
  OpenDoorUntil = 0;
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(11, INPUT_PULLUP);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  strip.begin();
  strip.setBrightness(255);
  strip.setBrightness(255);
  setBuzzer(0);
  //delay(10000);

  //Serial.write("Hello I am on.");
  //SPI.begin();        // Init SPI bus
  ////mfrc522.PCD_Init(); // Init //mfrc522 card
  //Servo1.attach(SERVO_PIN);
  //Servo1.write(90);
  pinMode(9,OUTPUT);
  analogWrite(9,148);
  //for(int i = 0 ; i < 180 ; i++)  Servo1.write(i),delay(10);
  lcd.init(); // initialize the lcd
  lcd.backlight();
  displayMessage("Powered By:", "Eric!");
  generatKMPNextTable();
  buzzNotice();
  buzzNotice();
  //while(1) Serial.println(digitalRead(11));
}

int doorState = 0;//0 close 1 open
int doorHasOpened = 0;

void  setOpenDoor(){
  doorState = 1;
  doorHasOpened = 0;
  //OpenDoorUntil = millis() + 13000;
  //setBuzzer(1);
}

long lastUpdate = 0;

int nowDeg = 148;


int flag = 0;
int readDoorState(){
  if(digitalRead(A1) == 0) return 0;
  else return 1;
}


long motorTime = 0;

void setMotor(int stateM){
  if(stateM == 0){
   if(lastUpdate <= millis() - 800) displayMessage("Use Card/Passwd", "or Scan Face"),lastUpdate = millis();//
    if(nowDeg <= 148){
      Serial.println("2");
      analogWrite(9,nowDeg);
      if(millis() - motorTime > 3){
        nowDeg++;
        motorTime == millis();
      }
      if(nowDeg == 147) {
        buzzNotice();
        maintainLED();
        setBuzzer(1);
        delay(100);
        setBuzzer(0);
      }
    } 
  }
  else{
    if(nowDeg >= 44){
      analogWrite(9,nowDeg);
      if(millis() - motorTime > 3){
        nowDeg --;
        motorTime = millis();
      }
      if(nowDeg == 46){
        buzzNotice();
        maintainLED();
        setBuzzer(1);
        delay(100);
        setBuzzer(0);
      }
    }
    if(lastUpdate <= millis() - 800) {displayMessage("Door Opened:", OpenFor ),lastUpdate = millis();}
     
  }
}

int justClose = 0;

void loop()
{

wdt_reset();
//strip.show();
  maintainLED();
  if(doorState == 1){
    justClose = 0;
    if(doorHasOpened == 0){
      unlockedBlue = 0;
      Serial.println("door is set open but not yet");
      setMotor(1);
      if(nowDeg <= 44)
      {if(readDoorState() == 1){
        delay(80);
      buzzNotice();
      if(readDoorState() == 1)
        doorHasOpened = 1;
        starMOD = 0;
      }}
    }
    else if(doorHasOpened == 1){
      Serial.println("Waitting for door to close");
      unlockedBlue = 55;
    if(lastUpdate <= millis() - 800) displayMessage("CLOSE DOOR", "CLOSE DOOR"),lastUpdate = millis();//
      if(readDoorState() == 0){
        delay(80);
      if(readDoorState() == 0)
        doorState = 0;
        starMOD = 0;
        
      }
    }
  }
  else{
    if(justClose == 0){
      ledState = 2;
      lockingUp = 130;
      lockingDown = 130;
      locked = 0;
      justClose = 1;
    }
    setMotor(0);
    Serial.println("Door has closed idle");
    if(lastUpdate <= millis() - 800) 
    {
      displayMessage("Use Card/Passwd", "or Scan Face"),lastUpdate = millis();
      if(lt<0) lt = 0;
      }//
  }
  /*
  if (millis() <= OpenDoorUntil)
  {
    Serial.println("open");
    // now should open door;
    while(nowDeg >= 44){
      analogWrite(9,nowDeg);
      delay(18);
      nowDeg --;
    }
    if(lastUpdate <= millis() - 800) {displayMessage("Door Opened:", OpenFor ),lastUpdate = millis();}
    
    clearKey();
    if(flag == 0) buzzNotice(),flag = 1;
  }
  else
  {
    if(flag == 1) buzzNotice(),flag = 0;
    Serial.println("1");
    OpenDoorUntil = 0;
    if(lastUpdate <= millis() - 800) displayMessage("Use Card/Passwd", "or Scan Face"),lastUpdate = millis();//
    while(nowDeg <= 148){
      Serial.println("2");
      analogWrite(9,nowDeg);
      delay(16);
      nowDeg++;
    }
    //delay(100);
  }
  */
  if (checkInputVaild() == 1)
  {
    setOpenDoor();
    Serial.println("Door Opened.");
    OpenFor = OutsidePassword;clearKey();
    ledState = 3;
    unlockingDown = 300;
    unlockingUp = 0;
    unlocked = 0;
  }
  if (digitalRead(11) == 0)
  {
    ledState = 3;
    setOpenDoor();
    Serial.println("Button Pressed.");
    OpenFor = InsideButton;
    ledState = 3;
    unlockingDown = 300;
    unlockingUp = 0;
    unlocked = 0;
  }
  if (analogRead(A0) >= 500)
  {
    delay(150);
    if(analogRead(A0) <= 500) return;
    setOpenDoor();
    Serial.println("Button Pressed.");
    OpenFor = WebReq;
    ledState = 3;
    unlockingDown = 300;
    unlocked = 0;
    unlockingUp = 0;
  }

  /*unsigned long cardID = getID();
  for (int i = 0; i < CardNum; i++)
  {
    if (cardID == CardAvailbleIDs[i])
    {
      setOpenDoor();
    }
  }*/
}

