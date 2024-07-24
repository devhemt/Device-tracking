#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <AltSoftSerial.h>

#define rxPin 2
#define txPin 3
SoftwareSerial sim800L(rxPin, txPin);

// GPS Module RX pin to Arduino 9
// GPS Module TX pin to Arduino 8
AltSoftSerial neogps;

TinyGPSPlus gps;

unsigned long previousMillis = 0;
long interval = 60000;

void setup()
{
  // Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(115200);
  
  // Begin serial communication with Arduino and SIM800L
  sim800L.begin(9600);

  // Begin serial communication with Arduino and GPS
  neogps.begin(9600);

  Serial.println("Initializing...");
  
  // Once the handshake test is successful, it will return OK
  sendATcommand("AT", "OK", 2000);
  sendATcommand("AT+CMGF=1", "OK", 2000);
}

void loop()
{
  while (sim800L.available()) {
    Serial.println(sim800L.readString());
  }
  while (Serial.available()) {
    sim800L.println(Serial.readString());
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    sendGpsToServer();
  }
}

int sendGpsToServer()
{
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 2000;) {
    while (neogps.available()) {
      if (gps.encode(neogps.read())) {
        newData = true;
        break;
      }
    }
  }

  if (newData) {
    String latitude = String(gps.location.lat(), 6);
    String longitude = String(gps.location.lng(), 6);
    float altitude = gps.altitude.meters();
    unsigned long date = gps.date.value();
    unsigned long time = gps.time.value();
    float speed = gps.speed.kmph();
    
    Serial.print("Latitude= "); 
    Serial.print(latitude);
    Serial.print(" Longitude= "); 
    Serial.println(longitude);

    String payload = "{\"latitude\":" + latitude + ",\"longitude\":" + longitude + ",\"altitude\":" + String(altitude) + ",\"speed\":" + String(speed) + "}";
    
    sendATcommand("AT+CFUN=1", "OK", 2000);
    sendATcommand("AT+CGATT=1", "OK", 2000);
    sendATcommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"", "OK", 2000);
    sendATcommand("AT+SAPBR=3,1,\"APN\",\"internet\"", "OK", 2000);
    sendATcommand("AT+SAPBR=1,1", "OK", 2000);
    sendATcommand("AT+HTTPINIT", "OK", 2000);
    sendATcommand("AT+HTTPPARA=\"CID\",1", "OK", 1000);
    sendATcommand("AT+HTTPPARA=\"URL\",\"http://localhost:8080/api/v1/rpX1dv3tqbPZ8gM0zs2Z/telemetry\"", "OK", 1000);
    sendATcommand("AT+HTTPPARA=\"CONTENT\",\"application/json\"", "OK", 1000);
    
    sim800L.print("AT+HTTPDATA="); 
    sim800L.print(payload.length());
    sim800L.print(",10000\r");
    delay(100);
    sim800L.print(payload);
    delay(100);
    
    sendATcommand("AT+HTTPACTION=1", "0,200", 1000);
    sendATcommand("AT+HTTPTERM", "OK", 1000);
    sendATcommand("AT+CIPSHUT", "SHUT OK", 1000);
  }
  return 1;    
}

int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout)
{
  uint8_t x = 0, answer = 0;
  char response[100];
  unsigned long previous;

  // Initialize the string
  memset(response, '\0', 100);
  delay(100);
  
  // Clean the input buffer
  while (sim800L.available() > 0) sim800L.read();
  
  if (ATcommand[0] != '\0') {
    // Send the AT command 
    sim800L.println(ATcommand);
  }

  x = 0;
  previous = millis();

  // This loop waits for the answer with time out
  do {
    if (sim800L.available() != 0) {
      response[x] = sim800L.read();
      x++;
      // Check if the desired answer is in the response of the module
      if (strstr(response, expected_answer) != NULL) {
        answer = 1;
      }
    }
  } while ((answer == 0) && ((millis() - previous) < timeout));

  Serial.println(response);
  return answer;
}
