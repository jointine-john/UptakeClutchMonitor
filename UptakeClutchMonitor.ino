volatile unsigned int counter, relayLedState {};
volatile bool eStopState = false;

constexpr auto printInterval { 250lu };
auto printNow { 0lu };
auto lastLeftHallStateChange{ 0lu };
auto lastRightHallStateChange{ 0lu };

int idx{ 0 };
int relays[]{ D0, D1, D2, D3 };
int leds[]{ LED_D0, LED_D1, LED_D2, LED_D3 };
bool statuses[]{ true, true, true, true };
int sensorValue = 0; 
int leftCounter = 0;
int rightCounter = 0;

const unsigned long tripTimeout = 500; // Timeout in milliseconds (e.g., 1 second)

volatile unsigned long lastLeftRotationTime = 0;
volatile unsigned long lastRightRotationTime = 0;
bool leftShaftStopped = true;
bool rightShaftStopped = true;

void setup(){
  Serial.begin(115200);
  for (const auto timeout = millis() + 2500; !Serial && millis() < timeout; delay(250))
      ;

  // Output Configuration
  for (int i = 0; i < 4; i++) {
    pinMode(relays[i], OUTPUT);
    pinMode(leds[i], OUTPUT);
  }

  // Input Configuration 
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);

  pinMode(BTN_USER, INPUT);

  lastLeftRotationTime = millis();
  lastRightRotationTime = millis();

  // Interrupt configuration
  attachInterrupt(digitalPinToInterrupt(BTN_USER), resetController, RISING);
  attachInterrupt(digitalPinToInterrupt(A0), handleLeftRotation, RISING );
  attachInterrupt(digitalPinToInterrupt(A1), handleRightRotation, RISING );
  attachInterrupt(digitalPinToInterrupt(A2), resetController, RISING);
}

void loop(){
    checkRotation();
    if(abs(leftCounter - rightCounter) > 5){
      eStopState = true;
      Serial.println("E-Stop !! - Press Reset");
      digitalWrite(leds[0], HIGH);
      digitalWrite(relays[0], HIGH);

      while(eStopState){
      };
    }
    
    if (leftShaftStopped) {
      digitalWrite(leds[0], LOW);
      leftShaftStopped = false; // Reset the flag
    }

    if (rightShaftStopped) {
      digitalWrite(leds[1], LOW);
      rightShaftStopped = false; // Reset the flag
    } 


  if (millis() > printNow) {
    Serial.print(leftCounter);
    Serial.print(" - ");
    Serial.print(rightCounter);
    Serial.print(" = ");
    Serial.println(abs(leftCounter - rightCounter));
    
      printNow = millis() + printInterval;
  }
}

/**
  ISR functions. Below are related inputs with respective ISR function.
  - BTN_USER: resetController()
  - A0: handleLeftRotation()
  - A1: handleRightRotation()
*/

void resetController(){
  eStopState = false;
  leftCounter = 0;
  rightCounter = 0;
  digitalWrite(relays[0], LOW);
  digitalWrite(leds[0], LOW);
}

void handleLeftRotation() {
  lastLeftRotationTime = millis();
  leftCounter++;
}

void handleRightRotation() {
  lastRightRotationTime = millis();
  rightCounter++;
}


void checkRotation() {
  if (millis() - lastLeftRotationTime > tripTimeout && lastLeftRotationTime != 0) { // Check for timeout AND ensure it's not the initial 0 value
    leftShaftStopped = true;
  } else {
    rightCounter=0;
  }

  if (millis() - lastRightRotationTime > tripTimeout && lastRightRotationTime != 0) { // Check for timeout AND ensure it's not the initial 0 value
    rightShaftStopped = true;
  }else {
    leftCounter=0;
  }
}
