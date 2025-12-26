const int MotorPin1 = 27;
const int MotorPin2 = 26;
const int PWMPin = 12;
const int pinLED  = 2;    
const int PWMChannel = 0;
const int PWMResolution = 8;
const int Baud = 115200;
const int Frequency = 30000;

char Buffer[64];
char NewLine = '\n';
char space = ' ';
String Command;

int motorSpeed = 185;

void ConvertSerial()
{
  int Length = Serial.readBytesUntil(NewLine, Buffer, sizeof(Buffer) - 1);
  Buffer[Length] = '\0';
  Command = String(Buffer);
  memset(Buffer,0,sizeof(Buffer));

  Command.trim();
  Command.toUpperCase();
}


void ReadCommand()
{
  if (Command == "ON") {
    Serial.println("MOTOR ON");

    digitalWrite(MotorPin1, HIGH);
    digitalWrite(MotorPin2, LOW);
    ledcWrite(PWMChannel, motorSpeed);
    digitalWrite(pinLED, HIGH);
  }
  else if (Command == "OFF") {
    Serial.println("MOTOR OFF");

    digitalWrite(MotorPin1, LOW); 
    digitalWrite(MotorPin2, LOW);
    ledcWrite(PWMChannel, 0);
    digitalWrite(pinLED, LOW);
  }
  else {
    Serial.println("Wrong Input! (Insert ON / OFF)");
  }
}


void setup() {

  memset(Buffer,0,sizeof(Buffer));
  Serial.begin(Baud);

  pinMode(MotorPin1, OUTPUT);
  pinMode(MotorPin2, OUTPUT);
  pinMode(pinLED, OUTPUT);

  digitalWrite(MotorPin1, LOW);
  digitalWrite(MotorPin2, LOW);

  ledcSetup(PWMChannel, Frequency, PWMResolution);
  ledcAttachPin(PWMPin, PWMChannel);

}

void loop() {
  if (Serial.available() > 0) {
    ConvertSerial();
    ReadCommand();
  }
}
