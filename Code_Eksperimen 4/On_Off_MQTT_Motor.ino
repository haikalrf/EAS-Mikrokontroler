#include <WiFi.h>
#include <PubSubClient.h>

const int MotorPin1 = 27;
const int MotorPin2 = 26;
const int PWMPin = 12;
const int pinLED  = 2;    
const int PWMChannel = 0;
const int PWMResolution = 8;
const int Baud = 115200;
const int Frequency = 30000;

int motorSpeed = 180;

char* SSID = "765";
char* Password = "12345678";
char* MQTTServer = "10.143.106.234";
int MQTTPort = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void ConnectWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, Password);

  Serial.print("Connecting WIFI");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
}

void CallBack(char* Topic, byte* payload, unsigned int length)
{
  Serial.println("Message arrived [" + (String)Topic + "]");

  String Message = "";
  float TuningValue = 0;
  for (int index = 0; index < length; index++) {
    Message += (char)payload[index];
  }

  Message.trim();

  if (Message == "1")  {
    Serial.println("MOTOR ON");

    digitalWrite(MotorPin1, HIGH);
    digitalWrite(MotorPin2, LOW);
    ledcWrite(PWMChannel, motorSpeed);
    digitalWrite(pinLED, HIGH);
  }

  if (Message == "0") {
    Serial.println("MOTOR OFF");

    digitalWrite(MotorPin1, LOW); 
    digitalWrite(MotorPin2, LOW);
    ledcWrite(PWMChannel, 0);
    digitalWrite(pinLED, LOW);
  }
}

void ReconnectServer()
{
  while(!client.connected()) {
    String ClientID = "ESP32ClientKanov765";
    Serial.println("Client Connecting");

    if (client.connect(ClientID.c_str())) {
      Serial.println(">>> BERHASIL TERHUBUNG KE MOSQUITTO! <<<");
      client.subscribe("MotorSwitch");
    }
    else {
      Serial.print("GAGAL KONEK. State Error = ");
      Serial.println(client.state());
      delay(1000); 
    }
  }
}

void setup() {
  Serial.begin(Baud);

  ConnectWifi();

  client.setServer(MQTTServer, MQTTPort);
  client.setCallback(CallBack);

  pinMode(MotorPin1, OUTPUT);
  pinMode(MotorPin2, OUTPUT);
  pinMode(pinLED, OUTPUT);

  digitalWrite(MotorPin1, LOW);
  digitalWrite(MotorPin2, LOW);

  ledcSetup(PWMChannel, Frequency, PWMResolution);
  ledcAttachPin(PWMPin, PWMChannel);

}

void loop() {
  if (!client.connected()) {
    ReconnectServer();
  }

  client.loop();
}