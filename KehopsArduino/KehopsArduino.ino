// Sketch Arduino unifié pour Kehops.
//
// Matériel :
// - Joystick : X sur A0, Y sur A1, bouton sur D2
// - HC-SR04 : TRIG sur D9, ECHO sur D10
//
// Trames envoyées à Godot :
// - joystick_x:<0..1023>
// - joystick_y:<0..1023>
// - button:0|1
// - ultrason:<distance_en_cm>
//
// Les sketches d'origine restent dans Joystick/ et capteur_interaction/
// comme sauvegardes.

const int PIN_JOYSTICK_X = A0;
const int PIN_JOYSTICK_Y = A1;
const int PIN_JOYSTICK_BUTTON = 2;
const int PIN_ULTRASON_TRIG = 9;
const int PIN_ULTRASON_ECHO = 10;

const unsigned long JOYSTICK_SEND_INTERVAL_MS = 20;
const unsigned long ULTRASON_SEND_INTERVAL_MS = 500;
const unsigned long ULTRASON_TIMEOUT_US = 30000;

bool paired = false;
bool last_button_state = false;
unsigned long last_joystick_send_ms = 0;
unsigned long last_ultrason_send_ms = 0;

String serial_command;


void setup() {
  pinMode(PIN_JOYSTICK_BUTTON, INPUT_PULLUP);
  pinMode(PIN_ULTRASON_TRIG, OUTPUT);
  pinMode(PIN_ULTRASON_ECHO, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(PIN_ULTRASON_TRIG, LOW);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(115200);
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
}


void read_serial_commands() {
  while (Serial.available() > 0) {
    const char character = static_cast<char>(Serial.read());

    if (character == '\n') {
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
    Serial.println("GDSERIAL_ACK:kehops_unified");
    paired = true;
    return;
  }

  if (command == "led:1") {
    digitalWrite(LED_BUILTIN, HIGH);
  } else if (command == "led:0") {
    digitalWrite(LED_BUILTIN, LOW);
  }
}


void send_joystick_state() {
  const int joystick_x = analogRead(PIN_JOYSTICK_X);
  const int joystick_y = analogRead(PIN_JOYSTICK_Y);
  const bool button_pressed = digitalRead(PIN_JOYSTICK_BUTTON) == LOW;

  Serial.println("joystick_x:" + String(joystick_x));
  Serial.println("joystick_y:" + String(joystick_y));

  if (button_pressed != last_button_state) {
    last_button_state = button_pressed;
    Serial.println("button:" + String(button_pressed ? "1" : "0"));
  }
}


void send_ultrason_distance() {
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
    return;
  }

  const unsigned int distance_cm = duration * 0.034 / 2;
  Serial.println("ultrason:" + String(distance_cm));
}
