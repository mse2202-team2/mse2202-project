// MSE 2202
// Team 2 Project Robot Code
// Modified from E J Porter's robot base code

/*
  esp32                                           MSE-DuinoV2
  pins         description                        Brd Jumpers /Labels                                                                  User (Fill in chart with user PIN usage)
  1             3v3                               PWR 3V3                                                                              3V3
  2             gnd                               GND                                                                                  GND
  3             GPIO15/AD2_3/T3/SD_CMD/           D15 (has connections in both 5V and 3V areas)
  4             GPIO2/AD2_2/T2/SD_D0              D2(has connections in both 5V and 3V areas)  /INDICATORLED ( On ESP32 board )        Heartbeat LED
  5             GPIO4/AD2_0/T0/SD_D1              D4(has connections in both 5V and 3V areas)                                          Left Motor, Channel A
  6             GPIO16/RX2                        Slide Switch S1b                                                                     IR Receiver
  7             GPIO17/TX2                        Slide Switch S2b                                                                     Left Encoder, Channel A
  8             GPIO5                             D5 (has connections in both 5V and 3V areas)                                         Left Encoder, Channel B
  9             GPIO18                            D18 (has connections in both 5V and 3V areas)                                        Left Motor, Channel B
  10            GPIO19/CTS0                       D19 (has connections in both 5V and 3V areas)                                        Right Motor, Channel A
  11            GPIO21                            D21/I2C_DA
  12            GPIO3/RX0                         RX0
  13            GPIO1//TX0                        TX0
  14            GPIO22/RTS1                       D22/I2C_CLK
  15            GPIO23                            D23 (has connections in both 5V and 3V areas)
  16            EN                                JP4 (Labeled - RST) for reseting ESP32
  17            GPI36/VP/AD1_0                    AD0
  18            GPI39/VN/AD1_3/                   AD3
  19            GPI34/AD1_6/                      AD6
  20            GPI35/AD1_7                       Potentiometer R2 / AD7
  21            GPIO32/AD1_4/T9                   Potentiometer R1 / AD4                                                               Pot 1 (R1)
  22            GPIO33/AD1_5/T8                   IMon/D33  monitor board current
  23            GPIO25/AD2_8/DAC1                 SK6812 Smart LEDs / D25                                                              Smart LEDs
  24            GPIO26/A2_9/DAC2                  Push Button PB2                                                                      Limit switch
  25            GPIO27/AD2_7/T7                   Push Button PB1                                                                      PB1
  26            GPOP14/AD2_6/T6/SD_CLK            Slide Switch S2a                                                                     Right Encoder, Channel A
  27            GPIO12/AD2_5/T5/SD_D2/            D12(has connections in both 5V and 3V areas)                                         Right Motor, Channel B
  28            GPIO13/AD2_4/T4/SD_D3/            Slide Switch S1a                                                                     Right Encoder, Channel B
  29            GND                               GND                                                                                  GND
  30            VIN                               PWR 5V t 7V                                                                          PWR 5V to 7V
*/

// Sensor Pin assignments
const int ciPB1 = 27;
const int ciPot1 = A4;              //GPIO 32  - when JP2 has jumper installed Analog pin AD4 is connected to Poteniometer R1
const int ciLimitSwitch = 26;
const int ciCurrentSensor = A5;

const int ciEncoderLeftA = 5;
const int ciEncoderLeftB = 17;
const int ciEncoderRightA = 14;
const int ciEncoderRightB = 13;


// Motor Pin Assignments
const int ciMotorLeftA = 4;
const int ciMotorLeftB = 18;
const int ciMotorRightA = 19;
const int ciMotorRightB = 12;
const int ciMotorClimbA = 23;
const int ciMotorClimbB = 25;

#include "0_Core_Zero.h"

#include "drive.h"
#include "climb.h"
#include "MyWEBserver.h"
#include "BreakPoint.h"
#include "WDT.h";

boolean btToggle = true;
int curButtonState;
int prevButtonState = HIGH;

void setup() {
  Serial.begin(115200);

  Core_ZEROInit();
  Core_ONEInit();

  // Setup for drive and climb pin modes, LEDC channels
  setupDrive();
  setupClimb();
  
  pinMode(ciPB1, INPUT_PULLUP);
}

void loop() {
  curButtonState = digitalRead(ciPB1);
  
  // Average the encoder tick times
  ENC_Averaging();

  if (curButtonState == LOW && prevButtonState == HIGH) {   // Rising edge of PB1 press (as soon as it's pressed)
    toggleDrive();  // Stop the drive if its on, start if its off
    stopClimb();    // Stop the climb if it's running
  }

  handleDrive();          // Handle drive state machine (non-blocking)
  if (readyToClimb()) {   // Determine whether the robot is ready to start climbing (on last drive maneuver)
    startClimb();         // Switch the climb state to go up
  }
  handleClimb();          // Handle climb state machine (non-blocking)

  prevButtonState = curButtonState;
  delay(1);
}
