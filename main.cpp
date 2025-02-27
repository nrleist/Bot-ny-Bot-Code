#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHUtility.h>
#include <FEHSD.h>
#include <FEHBattery.h>
#include <FEHBuzzer.h>
#include <FEHMotor.h>
#include <FEHServo.h>

#include <cmath>
#include <string.h>
#include <iostream>
using namespace std;

//Declarations for encoders & motors 
DigitalEncoder leftEncoder(FEHIO::P0_0);
DigitalEncoder rightEncoder(FEHIO::P3_7);
FEHMotor leftMotor(FEHMotor::Motor0, 9.0);
FEHMotor rightMotor(FEHMotor::Motor1, 9.0);
FEHMotor spinner(FEHMotor::Motor2, 5.0);  // TODO: Check max volts for CRS

// Declarations for servos
FEHServo armServo(FEHServo::Servo0);
FEHServo hookServo(FEHServo::Servo1);

// Declarations for sensors
AnalogInputPin leftOpto(FEHIO::P1_0);
AnalogInputPin centerOpto(FEHIO::P1_0);
AnalogInputPin rightOpto(FEHIO::P1_0);
AnalogInputPin lightSenor(FEHIO::P1_4);

// Values to reverse motors
const int leftReverse = 0;
const int rightReverse = 0;
const int spinReverse = 0;

// TODO: Get drivetrain width and test counts.
#define COUNTS_PER_INCH_RIGHT 43
#define COUNTS_PER_INCH_LEFT 43
#define DRIVETRAIN_WIDTH 7

#define PI 3.1415926

// Function declarations here
void waitForTouch(char prgName[100]);
void moveForward(int percent, int inches);
void turn(int percent, int direction, int degrees);

int main(void)
{
    waitForTouch("Milestone 01");

    // TODO: Write Steps for Milestone 1

    leftMotor.SetPercent(20);
    rightMotor.SetPercent(20);

    Sleep(5.0);

    leftMotor.Stop();
    rightMotor.Stop();
}

// Function definitions here
void waitForTouch(char prgName[100]) {
    float x, y;

    LCD.Clear(BLACK);
    LCD.SetFontColor(WHITE);
    LCD.WriteLine(prgName);
    LCD.WriteLine("Touch the screen");

    while(!LCD.Touch(&x,&y)); //Wait for screen to be pressed
    while(LCD.Touch(&x,&y)); //Wait for screen to be unpressed

    LCD.Clear();
    LCD.WriteLine(prgName);
    LCD.WriteLine("Running...");
}


void moveForward(int percent, int inches) {
    //Reset encoder counts
    leftEncoder.ResetCounts();
    rightEncoder.ResetCounts();

    //Set both motors to desired percent
    leftMotor.SetPercent(percent * leftReverse);
    rightMotor.SetPercent(percent * rightReverse);
    
    //While the average of the left and right encoder is less than counts,
    //keep running motors
    while((leftEncoder.Counts() + rightEncoder.Counts()) / 2. < inches * COUNTS_PER_INCH_RIGHT);

    //Turn off motors
    rightMotor.Stop();
    leftMotor.Stop();
}

void turn(int percent, int direction, int degrees) {
    int turnInches = float((PI * DRIVETRAIN_WIDTH) * (degrees / 360.0));
    //LCD.WriteLine("Turning");

    leftEncoder.ResetCounts();
    rightEncoder.ResetCounts();

    leftMotor.SetPercent(percent * direction * leftReverse);
    rightMotor.SetPercent(percent * -direction * rightReverse);

    //While the average of the left and right encoder is less than counts,
    //keep running motors
    while(((abs(leftEncoder.Counts()) + abs(rightEncoder.Counts())) / 2.0) < turnInches * COUNTS_PER_INCH_LEFT);

    //Turn off motors
    rightMotor.Stop();
    leftMotor.Stop();
}