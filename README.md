# ESP32 Robot Control Lab 2: IR Remote Speed & Motion Control
 
**Course:** ICT 361 / Robotics - Embedded Systems
**Submission Type:** Group
 
## Group Members
- LINH KIMSOURMANA
- LY SOKPISEY
## Objective
This project implements an IR remote-controlled 4-wheel robot system where students decode IR signals and map remote buttons to robot motion and speed control. The robot responds to arrow buttons for directional movement, dedicated buttons for speed adjustment, and numeric input for direct speed setting. This lab demonstrates teleoperation principles and real-time embedded control.
 
## Hardware Setup
 
### IR Receiver Connection
| Component      | Connection  | Function                |
|----------------|-------------|-------------------------|
| IR Receiver    | GPIO 36     | Signal input            |
| IR Receiver    | 3.3V        | Power supply            |
| IR Receiver    | GND         | Ground                  |
 
### Motor Driver Outputs
| Motor       | Direction A | Direction B | PWM (LEDC)   |
|-------------|-------------|-------------|--------------|
| Motor 1     | GPIO 25     | GPIO 26     | GPIO 33 (Ch0)|
| Motor 2     | GPIO 27     | GPIO 32     | GPIO 14 (Ch1)|
| Motor 3     | GPIO 18     | GPIO 21     | GPIO 5  (Ch2)|
| Motor 4     | GPIO 22     | GPIO 23     | GPIO 19 (Ch3)|
 
### ESP32 and IR Remote System
- Microcontroller: ESP32
- IR Protocol: NEC (standard IR remote protocol)
- Motor Driver: Supports 4 independent DC motor control
- PWM Frequency: 20 kHz
- PWM Resolution: 8-bit (0-255)
## IR Remote Button Mapping
 
| Button    | IR Code   | Function                      |
|-----------|-----------|-------------------------------|
| UP        | 0xFF18E7  | Move forward                  |
| DOWN      | 0xFF4AB5  | Move backward                 |
| LEFT      | 0xFF10EF  | Turn left                     |
| RIGHT     | 0xFF5AA5  | Turn right                    |
| OK        | 0xFF38C7  | Stop robot                    |
| *         | 0xFF6897  | Decrease speed by 5           |
| #         | 0xFFB04F  | Increase speed by 5           |
| 0         | 0xFF9867  | Confirm numeric speed entry   |
| 1-9       | Various   | Build numeric speed input     |
 
## Control Logic Summary
 
### Motion Control
- Arrow buttons control robot direction: forward, backward, turn left, turn right
- OK button immediately stops the robot and cancels any pending speed input
- Pressing a direction button interrupts any active digit entry, resetting the buffer
- Motor configuration creates differential drive for turning (left and right motors reversed)
### Speed Control Mechanisms
The project implements two independent speed adjustment methods:
 
#### Method 1: Incremental Button Adjustment

- button decreases current speed by 5 units
- button increases current speed by 5 units
- Speed is always constrained to valid range: 0-100
- Used for quick, real-time speed tweaking during operation
- Default starting speed: 50

#### Method 2: Numeric Direct Input

- Buttons 1-9 accumulate digits into a three-digit buffer
- Maximum 3 digits can be entered (allows values 0-999)
- Button 0 confirms the entered value and applies it as new speed
- If entered value exceeds 100, it is automatically clamped to 100
- Buffer resets after confirmation or when any direction button is pressed
- Serial monitor shows each digit as it is entered for debugging
### Speed Mapping

- Speed percentage (0-100) is converted to PWM duty cycle (0-255) using Arduino map() function
- Formula: PWM = (speedPercent / 100) * 255
- This linear mapping ensures proportional motor speed relative to user input
- All four motors receive identical PWM signal for synchronized forward/backward motion
### Motor Direction Logic
 
**Forward Motion:**

- Motors 1 & 2: Dir A HIGH, Dir B LOW
- Motors 3 & 4: Dir A LOW, Dir B HIGH

**Backward Motion:**

- Motors 1 & 2: Dir A LOW, Dir B HIGH
- Motors 3 & 4: Dir A HIGH, Dir B LOW

**Turn Left:**

- Motors 1 & 2: Dir A LOW, Dir B HIGH (backward)
- Motors 3 & 4: Dir A LOW, Dir B HIGH (forward)

**Turn Right:**
  
- Motors 1 & 2: Dir A HIGH, Dir B LOW (forward)
- Motors 3 & 4: Dir A HIGH, Dir B LOW (backward)

**Stop:**

- All PWM signals set to 0, preventing motor rotation

## Code Structure Overview
 
### Main Components
 
**Global Variables:**
- currentDirection: Enum tracking current motion state (STOPPED, FORWARD, BACKWARD, LEFT, RIGHT)
- speedPercent: Current speed value (0-100)
- numBuffer: Accumulates numeric input digits
- digitCount: Tracks number of entered digits
**Setup Function:**
- Initializes serial communication (115200 baud)
- Enables IR receiver on GPIO 36
- Configures motor direction GPIO pins as outputs
- Sets up PWM channels with frequency 20 kHz and 8-bit resolution
- Implements version-checking for ESP32 Arduino core compatibility (v2.x and v3.x)
- Stops all motors and displays startup message
**Main Loop:**
- Continuously checks for incoming IR signals
- Decodes IR signals and calls handleButton()
- Drives motors based on current direction and speed
- Repeats every 100 ms
**handleButton() Function:**
Processes IR codes and implements the control logic:
- Direction buttons (UP, DOWN, LEFT, RIGHT, OK): Update currentDirection, reset numeric buffer
- Speed buttons (*, #): Adjust speedPercent, reset numeric buffer
- Numeric buttons (1-9): Append digit to numBuffer, increment digitCount
- Confirm button (0): Validates and applies numeric speed input
**driveMotors() Function:**
- Converts speedPercent to PWM value
- Calls appropriate motor control function based on currentDirection
- Executes synchronously with loop timing
**Motor Control Functions:**
- moveForward(): Sets motor directions for forward motion
- moveBackward(): Sets motor directions for backward motion
- turnLeft(): Creates differential drive by reversing left motors
- turnRight(): Creates differential drive by reversing right motors
- stopMotors(): Sets all PWM outputs to 0
### Backward Compatibility
Code uses preprocessor directives to support both ESP32 Arduino v2.x and v3.x:
- v3.x uses ledcAttach() and ledcWrite()
- v2.x uses ledcSetup(), ledcAttachPin(), and ledcWrite()
## Flowchart

 <img width="815" height="832" alt="lab2roboticflowchart" src="https://github.com/user-attachments/assets/710d37fd-ab25-4cfa-8f7e-1acd8cdc77cb" />
 
## Demo Video
Watch the demonstration here: ([Demo Video](https://drive.google.com/file/d/1bEz_p9UjF9pH4UdrQJwyLPCokV44GMbI/view?usp=sharing))

## Explanation
The video demonstrate:
- Robot responding to all direction buttons (UP, DOWN, LEFT, RIGHT)
- Speed changes using both incremental (* and #) and numeric input methods
- Proper motor synchronization during forward/backward motion
- Smooth turning behavior
- Stop command halting all motion immediately
- Serial monitor output showing IR codes and speed values
- Edge case handling (rapid inputs, exceeding 100, buffer reset)
### 1. Purpose of IR Remote Control
Infrared remote control provides wireless teleoperation of the robot without physical connections. Each button press generates a unique 32-bit NEC protocol code that the IR receiver demodulates and sends to the ESP32. This approach is advantageous because it allows operators to control the robot from a distance without modifying the robot's physical structure, making it ideal for testing, demonstration, and real-world applications like surveillance robots or industrial equipment.
 
### 2. Dual Speed Control Methods
The implementation provides two distinct mechanisms for speed adjustment, each serving different use cases. The incremental method (* and # buttons) allows quick real-time adjustments during operation without stopping to enter digits. The numeric method (1-9 and 0 buttons) provides absolute control for situations where an exact speed value is required. This dual approach gives operators both convenience and precision.
 
### 3. Button Code Decoding
The IR receiver captures modulated infrared pulses and decodes them into 32-bit codes following the NEC protocol. The IRremoteESP8266 library handles the hardware timing and protocol parsing. Each unique button code is matched against predefined constants, allowing the microcontroller to identify which button was pressed. The CODE_REPEAT value (0xFFFFFFFF) indicates a button held down for an extended period, which is intentionally ignored to prevent continuous repeated actions.
 
### 4. Numeric Input Buffer Management
The numBuffer mechanism accumulates up to three digits, allowing speeds from 0 to 999. Each digit press multiplies the existing buffer by 10 and adds the new digit (e.g., pressing 7 then 5 creates 75). Pressing 0 confirms the value and applies it to speedPercent. The confirmSpeed() function implements clamping to ensure out-of-range values are brought within 0-100. The buffer resets when any direction button is pressed, preventing accidental application of partially entered values if the operator changes their mind.
 
### 5. Motor Direction Control Through GPIO
Differential drive requires independent control of motor rotation direction. Each motor pair (left and right) has two GPIO pins that form an H-bridge with the motor driver:
- Setting Dir A HIGH and Dir B LOW causes forward rotation
- Setting Dir A LOW and Dir B HIGH causes backward rotation
- Both pins LOW or both pins HIGH (depending on driver polarity) causes the motor to stop
For forward motion, all motors rotate in the same direction. For turning, one pair reverses while the other continues forward, creating the differential effect. This simple but effective control allows precise point-in-place rotations.
 
### 6. PWM Speed Modulation
Pulse-Width Modulation (PWM) controls motor speed by varying the percentage of time power is applied. An 8-bit PWM value ranges from 0 (0% duty = no power = stopped) to 255 (100% duty = full power = maximum speed). The speed percentage (0-100) is linearly mapped to this range so that speedPercent=50 produces PWM=127 (approximately 50% power). This proportional control ensures smooth speed transitions and allows operators to apply precisely tuned speeds.
 
### 7. IR Protocol Robustness
The NEC protocol includes built-in error checking and repetition codes, making it robust against electrical noise and electromagnetic interference common in robotics environments. The CODE_REPEAT handling ensures that a held button doesn't produce hundreds of unintended speed adjustments or direction changes during the time the button is physically pressed, which would otherwise occur with interrupt-driven decoding.
 
### 8. Control Loop Timing
The main loop includes a 100 ms delay, establishing a 10 Hz control update frequency. This timing is chosen to balance responsiveness (fast enough to feel immediate to an operator) with stability (slow enough to filter brief IR noise). The loop frequency is much faster than human reaction time, so operators perceive the robot as continuously responsive even though commands are processed at intervals.
 
## Serial Monitor Output Format
 
When running the program, the serial monitor displays information as follows:
 
- IR codes appear in hexadecimal: e.g., `FF18E7` when UP button is pressed
- Direction changes: `Direction: FORWARD`, `Direction: BACKWARD`, etc.
- Speed adjustments: `Speed decreased -> 45`, `Speed increased -> 55`
- Numeric entry: `Digit entered. Buffer = 7` (on pressing button 7)
- Confirmation: `Speed CONFIRMED -> 75`
- Startup message: `Robot ready. Default speed = 50.`
## Technical Specifications
 
- IR Protocol: NEC (38 kHz carrier frequency)
- IR Reception Range: 3-5 meters (typical)
- Motor PWM Frequency: 20 kHz (inaudible to human hearing)
- Control Loop Frequency: 10 Hz (100 ms period)
- Speed Range: 0-100 (integer values)
- Maximum Speed Adjustment Step: 5 units
- Default Speed: 50
- Maximum Numeric Input Digits: 3
- Serial Baud Rate: 115200
