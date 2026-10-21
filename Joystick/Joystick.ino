const int pinX = A0;
const int pinY = A1;
const int pinSW = 2;

bool paired = false;
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 20;

bool lastButtonState = false;

void setup() {
  pinMode(pinSW, INPUT_PULLUP);
  Serial.begin(115200);
}

void loop() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "DISCOVER") {
      Serial.println("GDSERIAL_ACK:arduino_joystick");
      paired = true;
    }
  }

  if (!paired) {
    return;
  }

  unsigned long now = millis();

  if (now - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = now;

    int x = analogRead(pinX);
    int y = analogRead(pinY);
    bool pressed = digitalRead(pinSW) == LOW;

    Serial.println("joystick_x:" + String(x));
    Serial.println("joystick_y:" + String(y));

    if (pressed != lastButtonState) {
      lastButtonState = pressed;
      Serial.println("button:" + String(pressed ? "1" : "0"));
    }
  }
}   