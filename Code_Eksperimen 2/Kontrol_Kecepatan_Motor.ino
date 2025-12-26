int MotorPin1 = 27;
int MotorPin2 = 26;
int PWMPin = 12;
int LEDPin = 2;
int PWMChannel = 0;
int LEDChannel = 1;

int Baud = 115200;
int Frequency = 30000;
int PWMResolution = 8;
int LEDResolution = 8;

char Buffer[64];
char NewLine = '\n';
char space = ' ';
String Command;
int InputNumber;
int SerialInput;
int idx;


void ConvertSerial()
{
  Serial.readBytesUntil(NewLine, Buffer, sizeof(Buffer));
  Command = String(Buffer);
  memset(Buffer,0,sizeof(Buffer));

  idx = Command.indexOf(space);

  if (idx != -1) {
    String NumberString = Command.substring(idx+1);
    NumberString.trim();
    InputNumber = NumberString.toInt();
    Command = Command.substring(0, idx);
  }

  Command.trim();
  Command.toUpperCase();
}

void ReadCommand()
{
  if (Command == "OP") {
    InputNumber = constrain(InputNumber, 0, 100);
    SerialInput = map(InputNumber, 0, 100, 0, 255);
    
    if (SerialInput  > 0){
      digitalWrite(MotorPin1, HIGH);
      digitalWrite(MotorPin2, LOW);
    }
    else {
      digitalWrite(MotorPin1, LOW);
      digitalWrite(MotorPin2, LOW);
    }

    ledcWrite(PWMChannel, SerialInput);
    Serial.println("Speed: " + String(InputNumber) + "%");
  }
  else if (Command == "LED") {
    InputNumber = constrain(InputNumber, 0, 100);
    SerialInput = map(InputNumber, 0, 100, 0, 255);
    ledcWrite(LEDChannel, SerialInput);
  }
  else if (Command == "X") {
    digitalWrite(MotorPin1, LOW);
    digitalWrite(MotorPin2, LOW);

    ledcWrite(PWMChannel, 0);
    Serial.println("Motor Stop");
  }
  else if (Command.length() > 0){
    Serial.println("Invalid Command!");
  }
}

void setup() {
  Serial.begin(Baud);

  pinMode(MotorPin1, OUTPUT);
  pinMode(MotorPin2, OUTPUT);

  digitalWrite(MotorPin1, LOW);
  digitalWrite(MotorPin2, LOW);

  ledcSetup(PWMChannel, Frequency, PWMResolution);
  ledcSetup(LEDChannel, Frequency, LEDResolution);

  ledcAttachPin(PWMPin, PWMChannel);
  ledcAttachPin(LEDPin, LEDChannel);

}

void loop() {
  if (Serial.available() > 0) {
    ConvertSerial();
    ReadCommand();
  }
}