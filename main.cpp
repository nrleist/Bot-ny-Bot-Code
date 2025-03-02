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
DigitalEncoder leftEncoder(FEHIO::P3_0);
DigitalEncoder rightEncoder(FEHIO::P0_0);
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
const int leftReverse = 1;
const int rightReverse = 1;
const int spinReverse = 1;

// TODO: Get drivetrain width and test counts.

#define PI 3.1415926

// Hardware Config
#define COUNTS_PER_REV 318
#define WHEEL_DIAMETER 2.5
const float WHEEL_CIRCUMFERENCE = PI * WHEEL_DIAMETER; 
#define COUNTS_PER_INCH_RIGHT 17
#define COUNTS_PER_INCH_LEFT 17
#define DRIVETRAIN_WIDTH 8.5

// Function declarations here
void waitForTouch(char prgName[]);
void stopRun();
void moveForward(int percent, int inches);
void moveBackward(int percent, int inches);
void turn(int percent, int direction, int degrees);
void stopMotors();
void encoderTest();

// Drive Prototypes
float rightMotorCorrection(float pwr);
float getLeftMotorPwr(float speed);
float getRightMotorPwr(float speed);
int getEncoderCountsLeft(float dist);
int getEncoderCountsRight(float dist);
bool checkLeftCounts(int targetCounts);
bool checkRightCounts(int targetCounts);
bool isNotTimeout(float startTime, float duration);
bool driveForward(float dist, float speed, float timeout);
bool driveBackward(float dist, float speed, float timeout);
float calculateTurnDist(float degrees);
bool turnLeft(float deg, float speed, float timeout);
bool turnRight(float deg, float speed, float timeout);
bool prgActive();

// Class definitions here
class Telemetry {
    private:
        int row = 4;
        int col = 0;

    public:
        void clear() {
            for(int i = 4; i <= 13; i++) {
                LCD.WriteRC("                          ", i, 0);
            }
            row = 4;
            col = 0;
        }

        void writeLine(char text[]) {
            LCD.WriteRC(text, row, col);
            row++;
            col = 0;
        }

        void writeLine(int num) {
            LCD.WriteRC(num, row, col);
            row++;
            col = 0;
        }

        void writeLine(float num) {
            LCD.WriteRC(num, row, col);
            row++;
            col = 0;
        }

        void writeLine(bool otpn) {
            LCD.WriteRC(otpn, row, col);
            row++;
            col = 0;
        }

        void write(char text[]) {
            LCD.WriteRC(text, row, col);
            col = (col + strlen(text)) % 26;
            row += ((strlen(text)) / 26);
        }

        void write(int num) {
            LCD.WriteRC(num, row, col);
            char text[40];
            itoa(num, text, 10);
            col = (col + strlen(text)) % 26;
            row += ((strlen(text)) / 26);
        }

        void write(float num) {
            LCD.WriteRC(num, row, col);
            char text[400];
            itoa(num, text, 10);
            col = (col + strlen(text)) % 26;
            row += ((strlen(text)) / 26);
        }

        void write(bool optn) {
            LCD.WriteRC(optn, row, col);
            char text[400];
            itoa(optn, text, 10);
            col = (col + strlen(text)) % 26;
            row += ((strlen(text)) / 26);
        }

        void setCursor(int row, int col) {
            this -> row = row;
            this -> col = col;
        }

        void writeAt(char text[], int row, int col) {
            setCursor(row, col);
            write(text);
        }

        void writeAt(int num, int row, int col) {
            setCursor(row, col);
            write(num);
        }

        void writeAt(float num, int row, int col) {
            setCursor(row, col);
            write(num);
        }

        void writeAt(bool optn, int row, int col) {
            setCursor(row, col);
            write(optn);
        }
};

// Class objects here
Telemetry telemetry;


int main(void)
{
    waitForTouch("Drive Test");
    Sleep(2.0);
    
    driveForward(18, 10, 5);
    Sleep(1.0);
    turnLeft(90, 45, 5);
    Sleep(1.0);
    driveForward(18, 10, 5);
    Sleep(1.0);
    turnLeft(90, 45, 5);
    Sleep(1.0);
    driveForward(18, 10, 5);
    Sleep(1.0);
    turnLeft(90, 45, 5);
    Sleep(1.0);
    driveForward(18, 10, 5);
    
    stopRun();
}



// Function definitions here
void waitForTouch(char prgName[]) {
    float x, y;

    LCD.Clear(BLACK);
    LCD.SetFontColor(WHITE);
    LCD.WriteLine("Team F3 -- Bot'ny Bots");
    LCD.WriteLine(prgName);
    LCD.WriteLine("Pre-Run: Touch the screen");
    LCD.WriteLine("/////////////\\\\\\\\\\\\\\\\\\\\\\\\\\");

    while(!LCD.Touch(&x,&y)); //Wait for screen to be pressed
    while(LCD.Touch(&x,&y)); //Wait for screen to be unpressed

    LCD.WriteRC(" Running...               ", 2, 0);
}

void stopRun() {
    stopMotors();
    LCD.WriteRC(" Stopped               ", 2, 0);
}

void moveForward(int percent, int inches) {
    //Reset encoder counts
    leftEncoder.ResetCounts();
    rightEncoder.ResetCounts();

    //Set both motors to desired percent
    leftMotor.SetPercent(percent * leftReverse);
    LCD.WriteLine(percent * leftReverse);
    rightMotor.SetPercent((percent * (rightReverse)) * 1.11);
    LCD.WriteLine(percent * (rightReverse));
    
    //While the average of the left and right encoder is less than counts,
    //keep running motors
    while((leftEncoder.Counts() + rightEncoder.Counts()) / 2.0 < inches * COUNTS_PER_INCH_RIGHT);
    //float x, y;
    //while(true);

    //Turn off motors
    stopMotors();
    LCD.WriteLine("Stopped");
}

void moveBackward(int percent, int inches) {
    //Reset encoder counts
    leftEncoder.ResetCounts();
    rightEncoder.ResetCounts();

    //Set both motors to desired percent
    leftMotor.SetPercent(-1 * (percent * leftReverse));
    LCD.WriteLine(percent * leftReverse);
    rightMotor.SetPercent((-1 *(percent * (rightReverse))) * 1.11);
    LCD.WriteLine(percent * (rightReverse));
    
    //While the average of the left and right encoder is less than counts,
    //keep running motors
    while((leftEncoder.Counts() + rightEncoder.Counts()) / 2.0 < inches * COUNTS_PER_INCH_RIGHT);
    //float x, y;
    //while(true);

    //Turn off motors
    stopMotors();
    LCD.WriteLine("Stopped");
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

void stopMotors() {
    leftMotor.Stop();
    rightMotor.Stop();
}

// Encoder Test code

void setMotorsPercent(float p) {
    leftMotor.SetPercent(p);
    rightMotor.SetPercent(p);
}

void setMotorsPercent(float pl, float pr) {
    leftMotor.SetPercent(pl);
    rightMotor.SetPercent(pr);
}

void resetEncoders() {
    leftEncoder.ResetCounts();
    rightEncoder.ResetCounts();
}

void encoderTest() {

    FEHFile * fptr = SD.FOpen("et.txt", "w");

    for(int i = 5; i < 55; i += 5) {
        telemetry.clear();
        telemetry.write("Running Motors at: ");
        telemetry.writeLine(i);

        resetEncoders();
        setMotorsPercent(i);

        Sleep(4.0);

        stopMotors();

        int leftCounts = leftEncoder.Counts();
        int rightCounts = rightEncoder.Counts();
        telemetry.write("Left Encoder Counts: ");
        telemetry.writeLine(leftCounts);
        telemetry.write("Right Encoder Counts: ");
        telemetry.writeLine(rightCounts);
        SD.FPrintf(fptr, "%d %d %d\n", i, leftCounts, rightCounts);
        Sleep(2.0);
    }

    SD.FClose(fptr);
}

// Drive code

float rightMotorCorrection(float pwr) {
    return pwr * .15; // TODO: Dervive better correction equation
}

float getLeftMotorPwr(float speed) {
    return (speed / WHEEL_CIRCUMFERENCE) * 32.15;
}

float getRightMotorPwr(float speed) {
    float leftPower = getLeftMotorPwr(speed);
    return leftPower + rightMotorCorrection(leftPower);
}

int getEncoderCountsLeft(float dist) {
    return (dist / WHEEL_CIRCUMFERENCE) * COUNTS_PER_REV;
    //return LEFT_COUNTS_PER_INCH * dist;
}

int getEncoderCountsRight(float dist) {
    return (dist / WHEEL_CIRCUMFERENCE) * COUNTS_PER_REV;
    //return LEFT_COUNTS_PER_INCH * dist;
}

bool checkLeftCounts(int targetCounts) {
    return abs(leftEncoder.Counts()) < targetCounts;
}

bool checkRightCounts(int targetCounts) {
    return abs(rightEncoder.Counts()) < targetCounts;
}

bool isNotTimeout(float startTime, float duration) {
    return ((TimeNow() - startTime) < duration);
}

bool driveForward(float dist, float speed, float timeout) {
    resetEncoders();

    int leftTargetCounts = getEncoderCountsLeft(dist);
    int rightTargetCounts = getEncoderCountsRight(dist);

    int leftPwr = getLeftMotorPwr(speed) * leftReverse;
    int rightPwr = getRightMotorPwr(speed) * rightReverse;

    float startTime = TimeNow();
    setMotorsPercent(leftPwr, rightPwr);

    while(checkLeftCounts(leftTargetCounts) && checkRightCounts(rightTargetCounts)
            && isNotTimeout(startTime, timeout) && prgActive());

    stopMotors();
    return isNotTimeout(startTime, timeout + .001);
}

bool driveBackward(float dist, float speed, float timeout) {
    resetEncoders();

    int leftTargetCounts = getEncoderCountsLeft(dist);
    int rightTargetCounts = getEncoderCountsRight(dist);

    int leftPwr = -getLeftMotorPwr(speed) * leftReverse;
    int rightPwr = -getRightMotorPwr(speed) * rightReverse;

    float startTime = TimeNow();
    setMotorsPercent(leftPwr, rightPwr);

    while(checkLeftCounts(leftTargetCounts) && checkRightCounts(rightTargetCounts)
            && isNotTimeout(startTime, timeout) && prgActive());

    stopMotors();
    return isNotTimeout(startTime, timeout + .001);
}

float calculateTurnDist(float degrees) {
    return float((PI * DRIVETRAIN_WIDTH) * (degrees / 360.0));
}

bool turnLeft(float deg, float speed, float timeout) {
    resetEncoders();
    float turnDist = calculateTurnDist(deg);
    float linearSpeed = turnDist / (deg / speed); 

    int leftTargetCounts = getEncoderCountsLeft(turnDist);
    int rightTargetCounts = getEncoderCountsRight(turnDist);

    int leftPwr = -getLeftMotorPwr(linearSpeed) * leftReverse;
    int rightPwr = getRightMotorPwr(linearSpeed) * rightReverse;

    float startTime = TimeNow();
    setMotorsPercent(leftPwr, rightPwr);

    while(checkLeftCounts(leftTargetCounts) && checkRightCounts(rightTargetCounts)
            && isNotTimeout(startTime, timeout) && prgActive());

    stopMotors();
    return isNotTimeout(startTime, timeout + .001);
}


bool turnRight(float deg, float speed, float timeout) {
    resetEncoders();
    float turnDist = calculateTurnDist(deg);
    float linearSpeed = turnDist / (deg / speed); 

    int leftTargetCounts = getEncoderCountsLeft(turnDist);
    int rightTargetCounts = getEncoderCountsRight(turnDist);

    int leftPwr = getLeftMotorPwr(linearSpeed) * leftReverse;
    int rightPwr = -getRightMotorPwr(linearSpeed) * rightReverse;

    float startTime = TimeNow();
    setMotorsPercent(leftPwr, rightPwr);

    while(checkLeftCounts(leftTargetCounts) && checkRightCounts(rightTargetCounts)
            && isNotTimeout(startTime, timeout) && prgActive());

    stopMotors();
    return isNotTimeout(startTime, timeout + .001);
}

bool prgActive() {
    return true;
}

