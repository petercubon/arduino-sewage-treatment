#include <SPI.h>
#include <Ethernet.h>
#include <DS3231_Simple.h>
#include <LiquidCrystal_I2C.h>
#include <HCSR04.h>

DS3231_Simple Clock;
LiquidCrystal_I2C lcd(0x27, 20, 4); // LCD
UltraSonicDistanceSensor distanceSensor(13, 12);  // Initialize sensor that uses digital pins 13 and 12

int hour, minute, second, day, month, year;
char date[8], time[8];

int pump, blower;
int pump_II = 0;
int water = 0;
int temperature = 0;
int water_measurement;

////////////////////////////////////////////VARIABLE DECLARATION////////////////////////////////////////////////
const int blowerPin = 2;
const int pumpPin = 3;
const int pumpingOutPin = 4;

////////////////////////////////////////////ETHERNET DECLARATION ///////////////////////////////////////////////
byte mac[] = { ****, ****, ****, ****, **** };  //MAC Address
char server[] = "********.com";                 //API server
IPAddress ip(***, ***, **, ***);                //Arduino IP address, only used when DHCP is turned off.
EthernetClient client;                          //define 'client' as object
String data, connect;                           //GET query with data
float temperature;
boolean connection = false;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  Clock.begin();
  lcd.init();      
  lcd.backlight();  

  pinMode(blowerPin, OUTPUT);
  digitalWrite(blowerPin, HIGH);

  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, HIGH);

  pinMode(pumpingOutPin, OUTPUT);
  digitalWrite(pumpingOutPin, HIGH);

  for (int i = 0; i < 2; i++)
  {
    lcd.setCursor(0, 0); // set the cursor to column 15, line 0
    lcd.print("STARTING");
    lcd.setCursor(0, 1); // set the cursor to column 15, line 0
    lcd.print("COV");
    lcd.setCursor(0, 2); // set the cursor to column 15, line 0
    lcd.print("version witkout data to server");
    lcd.setCursor(0, 3); // set the cursor to column 15, line 0
    lcd.print("VERSION 5");
    lcd.backlight();
    delay(1000);
    lcd.noBacklight();
    delay(1000);
  }
  lcd.backlight(); // finish with backlight on
  lcd.clear();

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Configure of Ethernet using DHCP Failed");
    Ethernet.begin(mac, ip);
  }
}

void loop()
{
  //////////////////////////////////////////////TIME AND DATE ////////////////////////////////////////////////////
  DateTime MyDateAndTime;
  MyDateAndTime = Clock.read();
  day = MyDateAndTime.Day;
  month = MyDateAndTime.Month;
  year = MyDateAndTime.Year;
  lcd.setCursor(0, 0);
  sprintf(date, "%02d.%02d.%02d", day, month, year);
  lcd.print(date);

  hour = MyDateAndTime.Hour;
  minute = MyDateAndTime.Minute;
  second = MyDateAndTime.Second;
  lcd.setCursor(12, 0); // set the cursor to column 15, line 0
  sprintf(time, "%02d:%02d:%02d", hour, minute, second);
  lcd.print(time);

  /////////////////////////////////////////// PUMP CONTROL /////////////////////////////////////////////////////////
  if (MyDateAndTime.Hour == 3  && MyDateAndTime.Minute <= 30)
  {
    digitalWrite(pumpPin, LOW); Serial.print("pump ON"); Serial.print("\n");
    pump = 1;
    // STATE STAV
    lcd.setCursor(13, 1); lcd.print("ON");
  }
  else {
    digitalWrite(pumpPin, HIGH); Serial.print("pump OFF"); Serial.print("\n");
    pump = 0;
    // STATE
    lcd.setCursor(13, 1); lcd.print("OFF");
  }

  ////////////////////////////////////////////////// BLOWER CONTROL ///////////////////////////////////////////////
  if (
    ((MyDateAndTime.Hour == 3 && MyDateAndTime.Minute > 30) || (MyDateAndTime.Hour == 4 && MyDateAndTime.Minute > 29)
     ||
     (MyDateAndTime.Hour == 5 && MyDateAndTime.Minute <= 30))
    ||
    ((MyDateAndTime.Hour == 7 && MyDateAndTime.Minute >= 29) || (MyDateAndTime.Hour == 8 && MyDateAndTime.Minute < 30))
    ||
    ((MyDateAndTime.Hour == 10 && MyDateAndTime.Minute > 29) || (MyDateAndTime.Hour == 11 && MyDateAndTime.Minute < 30))
    ||
    ((MyDateAndTime.Hour == 15 && MyDateAndTime.Minute > 29) || (MyDateAndTime.Hour == 16 && MyDateAndTime.Minute < 30))
    ||
    ((MyDateAndTime.Hour == 18 && MyDateAndTime.Minute > 29) || (MyDateAndTime.Hour == 19 && MyDateAndTime.Minute < 30))
    ||
    ((MyDateAndTime.Hour == 21 && MyDateAndTime.Minute > 29) || (MyDateAndTime.Hour == 22 && MyDateAndTime.Minute < 30))
    ||
    ((MyDateAndTime.Hour == 23 && MyDateAndTime.Minute < 59)
    ))
  {
    digitalWrite(blowerPin, LOW); Serial.print("blower ON"); Serial.print("\n");
    // STATE
    blower = 1;
    lcd.setCursor(13, 2); lcd.print("ON");
  }
  else {
    if (
      (MyDateAndTime.Hour == 6)
      ||
      (MyDateAndTime.Hour == 9) || (MyDateAndTime.Hour == 12)
      ||
      (MyDateAndTime.Hour == 17) || (MyDateAndTime.Hour == 20)
    )
    {
      digitalWrite(blowerPin, LOW); Serial.print("blower ON"); Serial.print("\n");
      blower = 1;
      // STATE
      lcd.setCursor(13, 2); lcd.print("ON");
    }

    else {
      digitalWrite(blowerPin, HIGH); Serial.print("blower OFF"); Serial.print("\n");
      blower = 0;
      // STATE
      lcd.setCursor(13, 2); lcd.print("OFF");
    }
  }

  //Every 500 miliseconds, do a measurement using the sensor and print the distance in centimeters.
  water_measurement = (distanceSensor.measureDistanceCm());
  if (water_measurement > 0) {
    water = water_measurement;
    Serial.print("\n");
    Serial.print("///////////////////////////////////////////////////////////");
    Serial.print("WATER"); Serial.print(water); Serial.print("\n");
    Serial.print("///////////////////////////////////////////////////////////");
  }
  else {
    water = 999;
    Serial.print("\n");
    Serial.print("///////////////////////////////////////////////////////////");
    Serial.print("WATER"); Serial.print(water); Serial.print("\n");
    Serial.print("///////////////////////////////////////////////////////////");
  }
  delay(1000);

  // 2.line PUMP
  lcd.setCursor(0, 1);
  lcd.print("pump:");

  // 3. line - BLOWER
  lcd.setCursor(0, 2);
  lcd.print("blower:");

  // 3. line - STATE
  lcd.setCursor(13,2);
  lcd.print("OFF");

  // 4. line - WATER LEVEL
  lcd.setCursor(0, 3);
  lcd.print("water:");

  // 4. line - WATER LEVEL VALUE
  lcd.setCursor(6, 3);
  lcd.print(water);
  lcd.print(" cm");

  // 4. line - STATE OF WATER OU PUMPING
  lcd.setCursor(13, 3);
  lcd.print("OFF");

  // 4. line - STATE OF CONNECTION
  lcd.setCursor(18, 3);
  lcd.print("OK");

  ////////////////////////////////////////////// CONNECTION AND SENDING DATA //////////////////////////////////
  dataString(); //GET query with data

  Serial.println("connecting...");

  if (client.connect(server, 80)) {
    sendData();
    connection = true;
    connectStatus = "OK";
    lcd.setCursor(18, 3);
    lcd.print(connection);
  }
  else {
    Serial.println("connection failed");
    connectState = "--";
    lcd.setCursor(14, 0);
    lcd.print(connection);
  }
  while (connection) {
    if (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
    if (!client.connected()) {
      Serial.println();
      Serial.println("disconnecting.");
      Serial.print("Temperature Sent :");
      Serial.println(temperature);
      client.stop();
      connection = false;
      data = ""; //data reset
    }
  }
  delay(60000); // interval
}

////////////////////////////////////////////// SENDING GET REQUEST ////////////////////////////////////////////
void dataString() {
  data += "GET /COV/_inc_cov/save_data_to_cov.php?pump=";
  data += pump;
  data += "&blower=";
  data += blower;
  data += "&pump_II=";
  data += pump_II;
  data += "&water=";
  data += water;
  data += "&temperature=";
  data += temperature;
  data += " HTTP/1.1";
}

void sendData() {
  Serial.println("connected");
  client.println(data);
  client.println("Host: ********.com");
  client.println("Connection: close");
  client.println();
  Serial.print(data);
}
