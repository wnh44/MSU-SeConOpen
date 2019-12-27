#define MOF_PIN 6
#define POT_PIN A1

int pot = 0;

void setup() {
  pinMode(MOF_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT);
  Serial.begin(9600);
}

void loop() {
  pot = analogRead(POT_PIN);
  Serial.println(pot);
  
  analogWrite(MOF_PIN, pot/4);
}
