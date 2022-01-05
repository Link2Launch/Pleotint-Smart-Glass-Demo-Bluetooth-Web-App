#define LEGHEATERRELAY   13
#define THIGHHEATERRELAY 12

void setup() {
  pinMode(LEGHEATERRELAY, OUTPUT); 
  pinMode(THIGHHEATERRELAY, OUTPUT);
  digitalWrite(LEGHEATERRELAY, LOW); // initialize in LOW state
  digitalWrite(THIGHHEATERRELAY, LOW); // initialize in LOW state
}

void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(LEGHEATERRELAY, !digitalRead(LEGHEATERRELAY));
  digitalWrite(THIGHHEATERRELAY, !digitalRead(THIGHHEATERRELAY));

  delay(2000);
}
