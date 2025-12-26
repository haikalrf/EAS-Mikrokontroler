#include <WiFi.h>
#include <PubSubClient.h>

int MotorPin1 = 27;
int MotorPin2 = 26;
int PWMPin = 12;
int SensorPin = 13;
int LEDPin = 2;
int PWMChannel = 0;
int LEDChannel = 1;

int Baud = 115200;
int Frequency = 30000;
int PWMResolution = 8;
int LEDResolution = 8;

float SetPoint = 0;
float Output = 0;
float Error = 0;
float LastError = 0;
float ProcessVariable = 0;
float DeltaTime = 0;
float LastI = 0;
float KProportional = 0;
float KIntegral = 0;
float KDerivative = 0;

volatile unsigned long PulseCount = 0;
unsigned long TotalPulse = 0;
unsigned long CurrentMillis = 0;
unsigned long MillisLastTIme = 0;
float RPM = 0;
float Holes = 20;

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

  if ((String)Topic == "SetPointKanov") {
    TuningValue = Message.toInt();
    SetPoint = TuningValue;
    Serial.println("SetPoint Set To: " + (String)SetPoint);
  }
  else if((String)Topic == "ProportionalKanov") {
    TuningValue = Message.toFloat();
    KProportional = TuningValue;
    Serial.println("Kp Set To: " + (String)KProportional);
  }
  else if ((String)Topic == "IntegralKanov") {
    TuningValue = Message.toFloat();
    KIntegral = TuningValue;
    Serial.println("Ki Set To: " + (String)KIntegral);
  }
  else if ((String)Topic == "DerivativeKanov") {
    TuningValue = Message.toFloat();
    KDerivative = TuningValue;
    Serial.println("Kd Set To: " + (String)KDerivative);
  }
  else if ((String)Topic == "LEDKanov") {
    if (Message == "1") {
      digitalWrite(LEDPin, HIGH);
      Serial.println("LED Status: ON");
    }
    else {
      digitalWrite(LEDPin, LOW);
      Serial.println("LED Status: OFF");
    }
  }
}

void ReconnectServer()
{
  while(!client.connected()) {
    String ClientID = "ESP32ClientKanov765";
    Serial.println("Client Connecting");

    if (client.connect(ClientID.c_str())) {
      Serial.println(">>> BERHASIL TERHUBUNG KE MOSQUITTO! <<<");
      client.subscribe("SetPointKanov");
      client.subscribe("ProportionalKanov");
      client.subscribe("IntegralKanov");
      client.subscribe("DerivativeKanov");
      client.subscribe("LEDKanov");
      Serial.println("Subscribe Success");
    }
    else {
      Serial.print("GAGAL KONEK. State Error = ");
      Serial.println(client.state());
      delay(1000); 
    }
  }
}

void PID() 
{
  float P = 0;
  float I = 0;
  float D = 0;

  if (DeltaTime <= 0) return;

  ProcessVariable = RPM;

  Error = SetPoint - ProcessVariable;

  P = KProportional * Error;
  I = LastI + (KIntegral * Error * DeltaTime);
  D = KDerivative * ((Error - LastError) / DeltaTime);

  I = constrain(I, -255, 255);
  Output = P + I + D;
  Output = constrain(Output, 0, 255);

  LastI = I;
  LastError = Error;

  if (Output > 0) {
    Serial.println("Output PID: "+ String(Output));
  }
}

void IRAM_ATTR SensorRead()
{
  PulseCount++;
}

void CalculateRPM()
{
  noInterrupts();
  TotalPulse = PulseCount;
  PulseCount = 0;
  interrupts();

  RPM = ((float)TotalPulse / (float)Holes) / DeltaTime * 60.0;


  if (RPM > 0) {
    Serial.println("Output RPM: " + (String)RPM);
  }
}

void setup() {
  Serial.begin(Baud);

  delay(3000);

  ConnectWifi();

  client.setServer(MQTTServer, MQTTPort);
  client.setCallback(CallBack);

  pinMode(MotorPin1, OUTPUT);
  pinMode(MotorPin2, OUTPUT);
  pinMode(SensorPin, INPUT_PULLUP);
  pinMode(LEDPin, OUTPUT);

  digitalWrite(MotorPin1, HIGH);
  digitalWrite(MotorPin2, LOW);

  ledcSetup(PWMChannel, Frequency, PWMResolution);
  ledcSetup(LEDChannel, Frequency, LEDResolution);

  ledcAttachPin(PWMPin, PWMChannel);

  attachInterrupt(digitalPinToInterrupt(SensorPin), SensorRead, RISING);

}
                                                                
void loop() {
  if (!client.connected()) {
    ReconnectServer();
  }

  CurrentMillis = millis();

  if (CurrentMillis - MillisLastTIme >= 100) {
    DeltaTime = (float)(CurrentMillis - MillisLastTIme) / 1000.0;

    MillisLastTIme = CurrentMillis;

    CalculateRPM();
    PID();

    ledcWrite(PWMChannel, Output);
  }

  client.loop();
}

