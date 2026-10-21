const int pinX = A0;  // VRx relié à A0
const int pinY = A1;  // VRy relié à A1
const int pinSW = 2;  // SW relié à la pin numérique 2

void setup() {
  pinMode(pinSW, INPUT_PULLUP); // Active la résistance de pull-up interne pour le bouton
  Serial.begin(9600);         // Initialisation de la communication série à 9600 bauds
}

void loop() {
  int x = analogRead(pinX);    // Lecture de l'axe X (0 à 1023)
  int y = analogRead(pinY);    // Lecture de l'axe Y (0 à 1023)
  int sw = digitalRead(pinSW); // Lecture du bouton (1 = relâché, 0 = pressé)

  // Envoi des données séparées par des virgules
  Serial.print(x);
  Serial.print(",");
  Serial.print(y);
  Serial.print(",");
  Serial.println(sw);

  delay(20); // Petite pause pour stabiliser le flux (~50 envois par seconde)
}