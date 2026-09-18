#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>

const uint16_t RECV_PIN = 36;

IRrecv irrecv(RECV_PIN);
decode_results results;

// MATCHED NEC IR REMOTE CODES
#define CODE_UP     0xFF18E7
#define CODE_DOWN   0xFF4AB5
#define CODE_LEFT   0xFF10EF
#define CODE_RIGHT  0xFF5AA5
#define CODE_OK     0xFF38C7

#define CODE_STAR   0xFF6897   // '*' -> speed - 5
#define CODE_HASH   0xFFB04F   // '#' -> speed + 5

#define CODE_0      0xFF9867   // Button 0 strictly confirms speed
#define CODE_1      0xFFA25D
#define CODE_2      0xFF629D
#define CODE_3      0xFFE21D
#define CODE_4      0xFF22DD
#define CODE_5      0xFF02FD
#define CODE_6      0xFFC23D
#define CODE_7      0xFFE01F
#define CODE_8      0xFFA857
#define CODE_9      0xFF906F

#define CODE_REPEAT 0xFFFFFFFF

enum Direction { STOPPED, FORWARD, BACKWARD, LEFT, RIGHT };
Direction currentDirection = STOPPED;

int  speedPercent = 50;
long numBuffer     = 0;
int  digitCount    = 0;

void setup() {
  Serial.begin(115200);
  irrecv.enableIRIn();

  // Motor direction pin configurations
  pinMode(25, OUTPUT); pinMode(26, OUTPUT);
  pinMode(27, OUTPUT); pinMode(32, OUTPUT);
  pinMode(18, OUTPUT); pinMode(21, OUTPUT);
  pinMode(22, OUTPUT); pinMode(23, OUTPUT);

  // Backward-compatible PWM setup (Works on both Core v2.x and v3.x)
  #if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcAttach(33, 20000, 8);
    ledcAttach(14, 20000, 8);
    ledcAttach(5, 20000, 8);
    ledcAttach(19, 20000, 8);
  #else
    ledcSetup(0, 20000, 8); ledcAttachPin(33, 0);
    ledcSetup(1, 20000, 8); ledcAttachPin(14, 1);
    ledcSetup(2, 20000, 8); ledcAttachPin(5, 2);
    ledcSetup(3, 20000, 8); ledcAttachPin(19, 3);
  #endif

  stopMotors();
  Serial.println("Robot ready. Default speed = 50.");
}

void loop() {
  if (irrecv.decode(&results)) {
    uint32_t irCode = results.value;
    Serial.println(irCode, HEX);
    if (irCode != CODE_REPEAT) handleButton(irCode);
    irrecv.resume();
  }
  driveMotors();
  delay(100);
}

void resetBuffer() {
  numBuffer = 0;
  digitCount = 0;
}

void handleButton(uint32_t code) {
  // Movement Logic (resets pending digits if interrupted)
  if (code == CODE_UP) {
    currentDirection = FORWARD;
    resetBuffer();
    Serial.println("Direction: FORWARD");
  }
  else if (code == CODE_DOWN) {
    currentDirection = BACKWARD;
    resetBuffer();
    Serial.println("Direction: BACKWARD");
  }
  else if (code == CODE_LEFT) {
    currentDirection = LEFT;
    resetBuffer();
    Serial.println("Direction: TURN LEFT");
  }
  else if (code == CODE_RIGHT) {
    currentDirection = RIGHT;
    resetBuffer();
    Serial.println("Direction: TURN RIGHT");
  }
  else if (code == CODE_OK) {
    currentDirection = STOPPED;
    resetBuffer();
    Serial.println("Direction: STOP");
  }
  // Incremental Speed Adjustment Logic (* and #)
  else if (code == CODE_STAR) {
    speedPercent = constrain(speedPercent - 5, 0, 100);
    resetBuffer();
    Serial.print("Speed decreased -> "); Serial.println(speedPercent);
  }
  else if (code == CODE_HASH) {
    speedPercent = constrain(speedPercent + 5, 0, 100);
    resetBuffer();
    Serial.print("Speed increased -> "); Serial.println(speedPercent);
  }
  // Numeric Speed Input Logic (Buttons 1–9 store digits, Button 0 confirms)
  else if (isDigit1to9(code)) {
    if (digitCount < 3) {
      numBuffer = numBuffer * 10 + digitValue(code);
      digitCount++;
      Serial.print("Digit entered. Buffer = "); Serial.println(numBuffer);
    }
  }
  else if (code == CODE_0) {
    if (digitCount > 0) {
      confirmSpeed();
    }
  }
}

void confirmSpeed() {
  speedPercent = constrain((int)numBuffer, 0, 100);
  Serial.print("Speed CONFIRMED -> "); Serial.println(speedPercent);
  resetBuffer();
}

bool isDigit1to9(uint32_t code) {
  return code == CODE_1 || code == CODE_2 || code == CODE_3 ||
         code == CODE_4 || code == CODE_5 || code == CODE_6 ||
         code == CODE_7 || code == CODE_8 || code == CODE_9;
}

int digitValue(uint32_t code) {
  if (code == CODE_1) return 1;
  if (code == CODE_2) return 2;
  if (code == CODE_3) return 3;
  if (code == CODE_4) return 4;
  if (code == CODE_5) return 5;
  if (code == CODE_6) return 6;
  if (code == CODE_7) return 7;
  if (code == CODE_8) return 8;
  if (code == CODE_9) return 9;
  return 0;
}

void driveMotors() {
  uint8_t pwm = map(speedPercent, 0, 100, 0, 255);
  switch (currentDirection) {
    case FORWARD:  moveForward(pwm);  break;
    case BACKWARD: moveBackward(pwm); break;
    case LEFT:     turnLeft(pwm);     break;
    case RIGHT:    turnRight(pwm);    break;
    case STOPPED:
    default:       stopMotors();      break;
  }
}

void writeMotorPWM(uint8_t speed) {
  #if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(33, speed); ledcWrite(14, speed); ledcWrite(5, speed); ledcWrite(19, speed);
  #else
    ledcWrite(0, speed); ledcWrite(1, speed); ledcWrite(2, speed); ledcWrite(3, speed);
  #endif
}

void stopMotors() {
  writeMotorPWM(0);
}

void moveBackward(uint8_t speed) {
  digitalWrite(26, HIGH); digitalWrite(25, LOW);
  digitalWrite(32, HIGH); digitalWrite(27, LOW);
  digitalWrite(18, HIGH); digitalWrite(21, LOW);
  digitalWrite(22, HIGH); digitalWrite(23, LOW);
  writeMotorPWM(speed);
}

void moveForward(uint8_t speed) {
  digitalWrite(26, LOW); digitalWrite(25, HIGH);
  digitalWrite(32, LOW); digitalWrite(27, HIGH);
  digitalWrite(18, LOW); digitalWrite(21, HIGH);
  digitalWrite(22, LOW); digitalWrite(23, HIGH);
  writeMotorPWM(speed);
}

void turnLeft(uint8_t speed) {
  digitalWrite(26, HIGH); digitalWrite(25, LOW);
  digitalWrite(32, HIGH); digitalWrite(27, LOW);
  digitalWrite(18, LOW);  digitalWrite(21, HIGH);
  digitalWrite(22, LOW);  digitalWrite(23, HIGH);
  writeMotorPWM(speed);
}

void turnRight(uint8_t speed) {
  digitalWrite(26, LOW); digitalWrite(25, HIGH);
  digitalWrite(32, LOW); digitalWrite(27, HIGH);
  digitalWrite(18, HIGH); digitalWrite(21, LOW);
  digitalWrite(22, HIGH); digitalWrite(23, LOW);
  writeMotorPWM(speed);
}