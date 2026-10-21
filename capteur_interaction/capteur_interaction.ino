// Broches du capteur à ultrasons HC-SR04
const int trigPin = 9; 
const int echoPin = 10;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Initialisation de la communication série
  Serial.begin(9600);
}

void loop() {
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

  // Affichage de la distance dans le moniteur série
  Serial.print("Distance : ");
  Serial.print(distance);
  Serial.println(" cm");

  // Pause d'une demi-seconde pour pouvoir lire confortablement
  delay(500);
}