// Sketch Arduino unifié pour Kehops.
//
// Matériel :
// - Joystick : X sur A0, Y sur A1, bouton sur D2
// - HC-SR04 : TRIG sur D9, ECHO sur D10
// - Capteur LDR : sur A2
//
// Trames envoyées à Godot :
// - joystick_x:<0..1023>
// - joystick_y:<0..1023>
// - button:0|1
// - ultrason:<distance_en_cm>
// - luminosite:<0..1023>

const int PIN_JOYSTICK_X = A0;
const int PIN_JOYSTICK_Y = A1;
const int PIN_JOYSTICK_BUTTON = 2;
const int PIN_ULTRASON_TRIG = 9;
const int PIN_ULTRASON_ECHO = 10;
const int PIN_LDR = A2; // Ajout du capteur de lumière sur A2
const int PIN_TREASURE_LEDS[] = {A3, A4, A5};
const int TREASURE_LED_COUNT = sizeof(PIN_TREASURE_LEDS) / sizeof(PIN_TREASURE_LEDS[0]);

const unsigned long JOYSTICK_SEND_INTERVAL_MS = 20;
const unsigned long ULTRASON_SEND_INTERVAL_MS = 500;
const unsigned long LDR_SEND_INTERVAL_MS = 200; // Rafraîchissement 5 fois par seconde (comme le test initial)
const unsigned long ULTRASON_TIMEOUT_US = 30000;

// Les mesures ne sont envoyées qu'après l'association avec Godot.
bool paired = false;
// Sert à n'envoyer un événement de bouton que lors d'un changement d'état.
bool last_button_state = false;
unsigned long last_joystick_send_ms = 0;
unsigned long last_ultrason_send_ms = 0;
unsigned long last_ldr_send_ms = 0; // Chrono pour la LDR

// Accumule une commande reçue jusqu'au saut de ligne.
String serial_command;
// Indique la prochaine LED de trésor à allumer progressivement.
int next_treasure_led = 0;


void setup() {
  pinMode(PIN_JOYSTICK_BUTTON, INPUT_PULLUP);
  pinMode(PIN_ULTRASON_TRIG, OUTPUT);
  pinMode(PIN_ULTRASON_ECHO, INPUT);
  for (int i = 0; i < TREASURE_LED_COUNT; i++) {
    pinMode(PIN_TREASURE_LEDS[i], OUTPUT);
    digitalWrite(PIN_TREASURE_LEDS[i], LOW);
  }
  // Pas besoin de pinMode pour A2 (les broches Analogiques sont en entrée par défaut)

  digitalWrite(PIN_ULTRASON_TRIG, LOW);
  Serial.begin(115200); // Gardé à 115200 comme dans le code Kehops (attention, vérifiez que Godot est configuré à cette même vitesse)
  Serial.setTimeout(10);
  serial_command.reserve(32);
}


void loop() {
  read_serial_commands();

  if (!paired) {
    return;
  }

  const unsigned long now = millis();

  if (now - last_joystick_send_ms >= JOYSTICK_SEND_INTERVAL_MS) {
    last_joystick_send_ms = now;
    send_joystick_state();
  }

  if (now - last_ultrason_send_ms >= ULTRASON_SEND_INTERVAL_MS) {
    last_ultrason_send_ms = now;
    send_ultrason_distance();
  }

  if (now - last_ldr_send_ms >= LDR_SEND_INTERVAL_MS) {
    last_ldr_send_ms = now;
    send_ldr_state();
  }
}


void read_serial_commands() {
  while (Serial.available() > 0) {
    const char character = static_cast<char>(Serial.read());

    if (character == '\n') {
      // Une ligne complète est nettoyée puis interprétée comme une commande.
      serial_command.trim();
      handle_command(serial_command);
      serial_command = "";
    } else if (character != '\r') {
      serial_command += character;
    }
  }
}


void handle_command(const String& command) {
  if (command == "DISCOVER") {
    // L'accusé de réception autorise ensuite l'envoi des capteurs.
    Serial.println("GDSERIAL_ACK:kehops_unified");
    paired = true;
    return;
  }

  if (command == "led:1") {
    // Chaque validation allume la LED suivante jusqu'à épuisement du tableau.
    if (next_treasure_led < TREASURE_LED_COUNT) {
      digitalWrite(PIN_TREASURE_LEDS[next_treasure_led], HIGH);
      Serial.println("ack:led" + String(next_treasure_led + 1) + ":1");
      next_treasure_led++;
    } else {
      Serial.println("ack:led:ignored");
    }
  } else if (command == "led:0") {
    // Cette commande remet toutes les LED et la progression à zéro.
    for (int i = 0; i < TREASURE_LED_COUNT; i++) {
      digitalWrite(PIN_TREASURE_LEDS[i], LOW);
    }
    next_treasure_led = 0;
    Serial.println("ack:led:0");
  }
}


void send_joystick_state() {
  const int joystick_x = analogRead(PIN_JOYSTICK_X);
  const int joystick_y = analogRead(PIN_JOYSTICK_Y);
  const bool button_pressed = digitalRead(PIN_JOYSTICK_BUTTON) == LOW;

  Serial.println("joystick_x:" + String(joystick_x));
  Serial.println("joystick_y:" + String(joystick_y));

  // Le bouton est envoyé uniquement quand sa valeur change afin de limiter le trafic série.
  if (button_pressed != last_button_state) {
    last_button_state = button_pressed;
    Serial.println("button:" + String(button_pressed ? "1" : "0"));
  }
}


void send_ultrason_distance() {
  // Une impulsion courte déclenche la mesure; la durée de retour donne la distance.
  digitalWrite(PIN_ULTRASON_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_ULTRASON_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRASON_TRIG, LOW);

  const unsigned long duration = pulseIn(
    PIN_ULTRASON_ECHO,
    HIGH,
    ULTRASON_TIMEOUT_US
  );

  if (duration == 0) {
    // Aucun écho dans le délai prévu: on ignore cette mesure invalide.
    return;
  }

  const unsigned int distance_cm = duration * 0.034 / 2;
  Serial.println("ultrason:" + String(distance_cm));
}


void send_ldr_state() {
  // La valeur brute de la LDR est convertie en trame comprise par Godot.
  const int valeur_ldr = analogRead(PIN_LDR);
  Serial.println("luminosite:" + String(valeur_ldr));
}