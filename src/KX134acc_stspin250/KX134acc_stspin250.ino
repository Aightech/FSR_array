#include <Wire.h>
#include <SparkFun_KX13X.h>

SparkFun_KX134 kxAccel;
outputData myData;

int PWM_pin = 23;
int DIR_pin = 22;
int EN_pin = 21;

int ADD_pin = 20;

void setup() {
  pinMode(ADD_pin, OUTPUT);
  digitalWrite(ADD_pin, LOW);
  Wire.begin();

  kxAccel.begin(0x1e);
  kxAccel.softwareReset();
  delay(5);
  kxAccel.enableAccel(false);
  kxAccel.setRange(SFE_KX134_RANGE8G);
  kxAccel.setOutputDataRate(15);

  kxAccel.enableDataEngine();
  kxAccel.enableAccel();


  // put your setup code here, to run once:
  pinMode(EN_pin, OUTPUT);
  pinMode(DIR_pin, OUTPUT);
  pinMode(PWM_pin, OUTPUT);

  digitalWrite(EN_pin, HIGH);
  digitalWrite(DIR_pin, LOW);
  digitalWrite(PWM_pin, LOW);

  analogWriteFrequency(PWM_pin, 36621.09);
  analogWriteResolution(12);

  Serial.begin(9600);
}

void loop() {


  if (kxAccel.dataReady()) {
    kxAccel.getAccelData(&myData);
    float x = myData.xData;
    float y = myData.yData;
    float z = myData.zData;
    double a = sqrt(x * x + y * y + z * z) - 1;
    int val = a * 20;
    int dir = (val > 0) ? 1 : -1;
    int amp = dir * val;
    amp = (amp < 5) ? 0 : amp * 50;

    // // put your main code here, to run repeatedly:
    // digitalWrite(DIR_pin, LOW);
    // delay(dt);
    // digitalWrite(DIR_pin, HIGH);
    // delay(dt);
    // int val = analogRead(15)*4;
    digitalWrite(DIR_pin, dir);
    analogWrite(PWM_pin, amp);
    // Serial.print(dir);
    // Serial.print(x);
    // Serial.print(" ");
    // Serial.print(y);
    // Serial.print(" ");
    Serial.println(amp);
    //delay(50);
  }
}
