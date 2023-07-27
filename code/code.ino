#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

//---------------------Define Pin Number
const int flowSensorPin = A2;
const int motionSensorPin = A3;
const int gasSensorPin = 7;
const int gasValvePin = 6;
const int kitchenLightRelay = 5;
const int kitchenFanRelay = 4;
const int alarmBuzzer = 3;
const byte rxPin = 9;
const byte txPin = 10;

//---------------------Define Global Variables
int TIMER_DURATION = 15 * 1000;
int ALERT_DURATION = TIMER_DURATION - 10 * 1000;
unsigned long startTime = 0;  // Variable to store the start time of the timer
bool hasCodeRun = false;
bool hasSentAlert = false;

// Objects
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial mySerial(rxPin, txPin);

void startTimer() {
  startTime = millis();  // Record the current time as the start time
}

bool isTimerExpired() {
  return (millis() - startTime) >= TIMER_DURATION;
}

bool isAlertExpired() {
  return (millis() - startTime) >= ALERT_DURATION;
}

void changeFlowStatus() {
  isFlowing = true;
}

void sendAlarmMessage() {
  sendMessage("Warning! The Gas is Flowing and No ones around!");
}

void sendTurnOffMessage() {
  sendMessage("Alert! The Gas Flow Has Been Cut off!");
}

void sendMessage(const String& message) {
  mySerial.println("AT+CMGF=1");                     //Sets the GSM Module in Text Mode
  delay(1000);                                       // Delay of 1000 milli seconds or 1 second
  mySerial.println("AT+CMGS=\"+8801621555300\"\r");  // Replace x with mobile number
  delay(1000);
  mySerial.println(message);  // The SMS text you want to send
  delay(100);
  mySerial.println((char)26);  // ASCII code of CTRL+Z
  delay(1000);
}

//------Setup
void setup() {

  lcd.init();
  lcd.backlight();

  Serial.begin(9600);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  mySerial.begin(9600);

  pinMode(timerSetBtnPin, INPUT);
  pinMode(timerPotPin, INPUT);
  pinMode(gasSensorPin, INPUT);

  pinMode(gasValvePin, OUTPUT);
  pinMode(kitchenLightRelay, OUTPUT);
  pinMode(kitchenFanRelay, OUTPUT);
  pinMode(alarmBuzzer, OUTPUT);
  startTimer();  // Start the timer when the Arduino boots up


  pinMode(interruptPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), changeFlowStatus, CHANGE);
}



void loop() {
  if (!hasCodeRun) {
    Serial.println("Started!");
    lcd.setCursor(0, 0);
    lcd.print("Started!");
    delay(1000);
    lcd.clear();
    // digitalWrite(gasValvePin1, LOW);
    digitalWrite(gasValvePin, HIGH);
    delay(1000);

    hasCodeRun = true;  // Set the flag to indicate that the code has run
  }
  isFlowing = false;  // to digitize the flow sensor output this measure is taken

  bool hasMovement = digitalRead(motionSensorPin);  // mtion sensor function will be called

  // LCD display showing differnt status
  lcd.setCursor(0, 0);
  lcd.print("f = ");
  lcd.setCursor(5, 0);
  lcd.print(isFlowing);
  lcd.setCursor(8, 0);
  lcd.print("m = ");
  lcd.setCursor(12, 0);
  lcd.print(hasMovement);


  //-------------------Alarm condition
  if (isFlowing && !hasMovement && !isTimerExpired() && isAlertExpired() && !hasSentAlert) {
    // sends alarm and never enters this loop until the code is resetted (!hasSentAlart takes care of that)
    sendAlarmMessage();
    digitalWrite(alarmBuzzer, HIGH);
    Serial.println("msg send");

    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("ALERT! ");
    lcd.setCursor(7, 1);
    lcd.print("G_Flowing.");
    delay(1000);

    hasSentAlert = true;
  }

  //-------------------turn off condition
  if (isFlowing && !hasMovement && isTimerExpired() && hasSentAlert && isAlertExpired()) {
    // Turns off the necessary output and never turns on them until restars
    digitalWrite(gasValvePin, LOW);
    sendTurnOffMessage();
    Serial.println("DANGER! Gas is off.");

    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("DANGER! ");
    lcd.setCursor(8, 1);
    lcd.print("");
    delay(1000);
  }

  //-----------light and fan turn on condition
  if (hasMovement) {
    digitalWrite(kitchenLightRelay, HIGH);
    digitalWrite(kitchenFanRelay, HIGH);
  }

  //-----------light and fan turn on condition
  if (!hasMovement) {
    digitalWrite(kitchenLightRelay, LOexW);
    digitalWrite(kitchenFanRelay, LOW);
  }

  //------------If fire or gas leakage happens
  if (digitalRead(gasSensorPin) > 10) {
    digitalWrite(alarmBuzzer, HIGH);
    sendMessage("Your kitchen is on fire!");
  }
  delay(50);
}
