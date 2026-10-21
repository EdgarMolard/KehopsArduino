// Broches du capteur à ultrasons HC-SR04
const int trigPin = 9; 
const int echoPin = 10;

String commande = "";
unsigned long derniere_mesure = 0;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(115200);
  Serial.setTimeout(10);
}

void loop() {
  while (Serial.available() > 0) {
    char caractere = Serial.read();
    if (caractere == '\n') {
      commande.trim();
      if (commande == "DISCOVER") {
        Serial.println("GDSERIAL_ACK");
      } else if (commande == "led:1") {
        digitalWrite(LED_BUILTIN, HIGH);
      } else if (commande == "led:0") {
        digitalWrite(LED_BUILTIN, LOW);
      }
      commande = "";
    } else {
      commande += caractere;
    }
  }

  if (millis() - derniere_mesure < 500) {
    return;
  }
  derniere_mesure = millis();

  // Envoi d'une impulsion sur la broche Trig
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Lecture du temps de retour de l'onde sur la broche Echo
  long duration = pulseIn(echoPin, HIGH);
  
  // Conversion du temps en centimètres
  int distance = duration * 0.034 / 2;

  Serial.print("ultrason:");
  Serial.println(distance);
}