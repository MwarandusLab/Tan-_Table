int Green_Led = 3;
int Red_Led = 4;
int Blue_Led = 9;
int Pot_Value = A3;
int Button = 2;

volatile int motorState = 0;  // 0 = stop, 1 = clockwise, 2 = stop, 3 = anticlockwise
bool buttonPressed = false;
int speed = 0;  // Speed from potentiometer

void setup() {
  Serial.begin(115200);  // GRBL usually communicates at 115200 baud rate
  
  pinMode(Green_Led, OUTPUT);
  pinMode(Red_Led, OUTPUT);
  pinMode(Blue_Led, OUTPUT);
  pinMode(Button, INPUT_PULLUP);  // Use internal pull-up for button
  
  // Set initial state (STOP mode)
  motorState = 0;
  digitalWrite(Red_Led, HIGH);   // Red LED ON (Stop mode)
  digitalWrite(Green_Led, LOW);
  digitalWrite(Blue_Led, LOW);

  // GRBL requires a reset on startup
  delay(1000);
  Serial.println("$X");  // Unlock GRBL to enable movement commands

  // Attach interrupt to button press (trigger on falling edge)
  attachInterrupt(digitalPinToInterrupt(Button), buttonPressISR, FALLING);
}

void loop() {
  if (buttonPressed) {
    buttonPressed = false;  // Clear the flag set by the interrupt
    
    // Read the speed from the potentiometer in STOP mode only
    if (motorState == 0 || motorState == 2) {
      int potValue = analogRead(Pot_Value);  // Read potentiometer
      speed = map(potValue, 0, 1023, 0, 100);  // Map 0-1023 to 0-100 (feedrate)
    }

    // Control motor based on motorState
    switch (motorState) {
      case 0:
      case 2:
        // Stop motor (Send Stop command)
        Serial.println("G4 P0");  // Dwell (Stop)
        digitalWrite(Green_Led, LOW);
        digitalWrite(Red_Led, HIGH);  // Red LED indicates stop
        digitalWrite(Blue_Led, LOW);
        break;

      case 1:
        // Clockwise (X+ direction)
        Serial.print("G1 X10 F");  // Move X-axis in positive direction
        Serial.println(speed);     // Feedrate controlled by potentiometer
        digitalWrite(Green_Led, HIGH);  // Green LED indicates clockwise
        digitalWrite(Red_Led, LOW);
        digitalWrite(Blue_Led, LOW);
        break;

      case 3:
        // Anticlockwise (X- direction)
        Serial.print("G1 X-10 F");  // Move X-axis in negative direction
        Serial.println(speed);      // Feedrate controlled by potentiometer
        digitalWrite(Green_Led, LOW);
        digitalWrite(Red_Led, LOW);
        digitalWrite(Blue_Led, HIGH);  // Blue LED indicates anticlockwise
        break;
    }
  }
  
  // Other background tasks can be handled here
}

// Interrupt Service Routine (ISR) for button press
void buttonPressISR() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  
  // Debouncing: ignore if the button is pressed multiple times within 300ms
  if (interruptTime - lastInterruptTime > 300) {
    motorState++;
    if (motorState > 3) {
      motorState = 0;  // Cycle through 0 (stop), 1 (clockwise), 2 (stop), 3 (anticlockwise)
    }
    buttonPressed = true;  // Flag to indicate the button was pressed
  }
  lastInterruptTime = interruptTime;
}
