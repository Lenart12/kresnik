int optopins[8] = {A0, A1, A2, A3, A4, A5, A6, A7};

void setup() {
  for(int i = 0; i < 8; i++){
    pinMode(optopins[i], OUTPUT);
    digitalWrite(optopins[i], LOW);
  }
}

void loop() {
  for(int i = 0; i < 8; i++){
   digitalWrite(optopins[i], HIGH);
   delay(1000); 
   digitalWrite(optopins[i], LOW);
  }
}
