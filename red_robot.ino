#include <AFMotor.h>
#include <PS2X_lib.h>
#include <Servo.h>

/******************************************************************
 * set pins connected to PS2 controller:
 *   - 1e column: original 
 *   - 2e colmun: Stef?
 * replace pin numbers by the ones you use
 ******************************************************************/
#define PS2_DAT        A0  //14    
#define PS2_CMD        A1  //15
#define PS2_SEL        A2  //16
#define PS2_CLK        A3  //17

/******************************************************************
 * select modes of PS2 controller:
 *   - pressures = analog reading of push-butttons 
 *   - rumble    = motor rumbling
 * uncomment 1 of the lines for each mode selection
 ******************************************************************/
//#define pressures   true
#define pressures   false
//#define rumble      true
#define rumble      false

PS2X ps2x; // create PS2 Controller Class

//right now, the library does NOT support hot pluggable controllers, meaning 
//you must always either restart your Arduino after you connect the controller, 
//or call config_gamepad(pins) again after connecting the controller.

int error = 0;
byte type = 0;
byte vibrate = 0;

//declare base motors
AF_DCMotor bl_motor(1, MOTOR12_8KHZ);
AF_DCMotor br_motor(2, MOTOR12_8KHZ);
AF_DCMotor fr_motor(3, MOTOR34_8KHZ);
AF_DCMotor fl_motor(4, MOTOR34_8KHZ);

// servos
Servo hook;
int hookmin = 0;
int hookmax = 90;
int hookpos = 0;
Servo presser;
int pressmin = 20;
int pressmax = 150;
int presspos = 20;

void setup(){
  Serial.begin(57600);
  
  delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
  
  
  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  
  if(error == 0){
    Serial.print("Found Controller, configured successful ");
  
    Serial.print("pressures = ");
    if (pressures)
      Serial.println("true ");
    else
      Serial.println("false");
      
    Serial.print("rumble = ");
    if (rumble)
      Serial.println("true)");
    else
      Serial.println("false");
      Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
      Serial.println("holding L1 or R1 will print out the analog stick values.");
      Serial.println("Note: Go to www.billporter.info for updates and to report bugs.");
  } else if(error == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips"); 
  else if(error == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");
  else if(error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
  
//  Serial.print(ps2x.Analog(1), HEX);
  
  type = ps2x.readType(); 
  if (type != 1)
    Serial.print("Beware, DualShock Controller not found ");

  // setup motors
  bl_motor.setSpeed(200);
  br_motor.setSpeed(200);
  fr_motor.setSpeed(200);
  fl_motor.setSpeed(200);
  bl_motor.run(RELEASE);
  br_motor.run(RELEASE);
  fr_motor.run(RELEASE);
  fl_motor.run(RELEASE);

  hook.attach(10);
  hook.write(hookpos);
  presser.attach(9);
  presser.write(presspos);
  delay(500);
}

void loop() {
  /* You must Read Gamepad to get new values and set vibration values
     ps2x.read_gamepad(small motor on/off, larger motor strenght from 0-255)
     if you don't enable the rumble, use ps2x.read_gamepad(); with no values
     You should call this at least once a second
   */  
  if(error == 1) //skip loop if no controller found
    return; 
  
  ps2x.read_gamepad(false, vibrate);
  
  if (ps2x.Button(PSB_L1) && presspos < pressmax) {
    presspos += 2;
    presser.write(presspos);
  }
  else if (ps2x.Button(PSB_L2) && presspos > pressmin) {
    presspos -= 2;
    presser.write(presspos);
  }

  if (ps2x.Button(PSB_R1) && hookpos < hookmax) {
    hookpos += 2;
    hook.write(hookpos);
  }
  else if (ps2x.Button(PSB_R2) && hookpos > hookmin) {
    hookpos -= 2;
    hook.write(hookpos);
  }

//  Serial.println((int)ps2x.Analog(PSS_LY));
//  Serial.println((int)ps2x.Analog(PSS_RY));
  setMotorSpeeds(ps2x.Analog(PSS_LY), ps2x.Analog(PSS_RY));

  // Arcade Drive
//  setMotorSpeeds(ps2x.Analog(PSS_RY) + ps2x.Analog(PSS_RX), ps2x.Analog(PSS_RY) - ps2x.Analog(PSS_RX));

  
  delay(10);
}

void setMotorSpeeds(byte left_speed, byte right_speed) {
  int speedlim = 255;
  int right_speed_adj = map((int)right_speed, 255, 0, -speedlim, speedlim);
  int left_speed_adj = map((int)left_speed, 255, 0, -speedlim, speedlim);

//  Serial.println(left_speed_adj);
//  Serial.println(right_speed_adj);
  
  if(abs(right_speed_adj) > 25) {
    br_motor.setSpeed(abs(right_speed_adj));
    fr_motor.setSpeed(abs(right_speed_adj));
    if(right_speed_adj > 0) {
      br_motor.run(FORWARD);
      fr_motor.run(FORWARD);
    } else {
      br_motor.run(BACKWARD);
      fr_motor.run(BACKWARD);
    }
  } else {
    br_motor.run(RELEASE);
    fr_motor.run(RELEASE);
  }

  if(abs(left_speed_adj) > 25) {
    bl_motor.setSpeed(abs(left_speed_adj));
    fl_motor.setSpeed(abs(left_speed_adj));
    if(left_speed_adj > 0) {
      bl_motor.run(FORWARD);
      fl_motor.run(FORWARD);
    } else {
      bl_motor.run(BACKWARD);
      fl_motor.run(BACKWARD);
    }
  } else {
    bl_motor.run(RELEASE);
    fl_motor.run(RELEASE);
  }
}
