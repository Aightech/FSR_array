int16_t vals[8 * 8];

int add = 0;
// the setup routine runs once when you press reset:
void setup() {

  Serial.begin(115200);
  for (int i = 50; i < 55; i++)
    pinMode(i, OUTPUT);

  for (int i = 0; i < 4; i++)
    digitalWrite(i + 50, LOW);

  pinMode(49, OUTPUT);
  digitalWrite(49, HIGH);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  delay(10);
  Serial.write(0x40);
  char c;
  while (c != 0xbb)
  {
    c = Serial.read();
  }
  digitalWrite(13, HIGH);

}

// the loop routine runs over and over again forever:
void loop() {             // wait for a second
  if (Serial.available() > 0)
  {
    char c = Serial.read();
    for (int a = 0; a < 8; a++)
    {
      for (int i = 0; i < 4; i++)
        digitalWrite(i + 50, ((a >> i) & 0b1) );
      delay(10);
      for (int i = 0; i < 8 ; i++)
        vals[i + a * 8] = analogRead(i);
    }
    Serial.write((uint8_t*)vals, 128);
    delay(100);
  }
}