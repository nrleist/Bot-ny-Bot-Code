#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHUtility.h>
#include <FEHSD.h>
#include <FEHBattery.h>
#include <FEHBuzzer.h>
#include <FEHMotor.h>
#include <FEHServo.h>
#include <FEHRCS.h>

#include <cmath>
#include <string.h>
#include <iostream>
using namespace std;

#define TEAM_ID_STRING "0150F3VKF"

enum Levers {
    LEFT,
    MIDDLE,
    RIGHT
};

//Declarations for encoders & motors 
DigitalEncoder leftEncoder(FEHIO::P3_0);
DigitalEncoder rightEncoder(FEHIO::P0_0);
FEHMotor leftMotor(FEHMotor::Motor0, 9.0);
FEHMotor rightMotor(FEHMotor::Motor1, 9.0);

FEHMotor spinner(FEHMotor::Motor2, 5.0);  // TODO: Check max volts for CRS

// Declarations for servos
FEHServo armServo(FEHServo::Servo6);

// Declarations for sensors
AnalogInputPin leftOpto(FEHIO::P1_0);
AnalogInputPin centerOpto(FEHIO::P1_0);
AnalogInputPin rightOpto(FEHIO::P1_0);
AnalogInputPin lightSenor(FEHIO::P0_1);

// Values to reverse motors
const int leftReverse = 1;
const int rightReverse = -1;
const int spinReverse = 1;

// TODO: Get test counts.

#define PI 3.1415926

// Hardware Config
#define COUNTS_PER_REV 318
#define WHEEL_DIAMETER 2.5
const float WHEEL_CIRCUMFERENCE = PI * WHEEL_DIAMETER; 
#define COUNTS_PER_INCH_RIGHT 17
#define COUNTS_PER_INCH_LEFT 17
#define DRIVETRAIN_WIDTH 8.5

// Battery Max Voltage
#define MAX_BATT_VOLTS 11.5

// Light Sensor Threshold Here
#define LIGHT_ON_THRESHOLD 0.941
#define BLUE_THRESHOLD 1.131
#define RED_THRESHOLD 0.445

int rampAdjust = .15;

// Function declarations here
void waitForTouch(char prgName[]);
int preRun(char prgName[]);
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
float powerAdjust(float desiredPwr);
bool driveForward(float dist, float speed, float timeout);
bool driveBackward(float dist, float speed, float timeout);
float calculateTurnDist(float degrees);
bool turnLeft(float deg, float speed, float timeout);
bool turnRight(float deg, float speed, float timeout);
bool prgActive();

void connectRCS();
void waitForStartLight(char prgName[]);
int getLightColor();
void lightSensorReadout();
void cdsTest();

// Class definitions here
class Telemetry {
    private:
        const int startRow = 5;

        int row = startRow;
        int col = 0;

    public:
        void clear() {
            for(int i = 4; i <= 13; i++) {
                LCD.WriteRC("                          ", i, 0);
            }
            row = startRow;
            col = 0;
        }

        void correctRow() {
            if(row > 13) {row = startRow;}
        }

        void writeLine(char text[]) {
            LCD.WriteRC(text, row, col);
            row++;
            correctRow();
            col = 0;
        }

        void writeLine(int num) {
            LCD.WriteRC(num, row, col);
            row++;
            correctRow();
            col = 0;
        }

        void writeLine(float num) {
            LCD.WriteRC(num, row, col);
            row++;
            correctRow();
            col = 0;
        }

        void writeLine(bool otpn) {
            LCD.WriteRC(otpn, row, col);
            row++;
            correctRow();
            col = 0;
        }

        void write(char text[]) {
            LCD.WriteRC(text, row, col);
            col = (col + strlen(text)) % 26;
            row += ((strlen(text)) / 26);
            correctRow();
        }

        void write(int num) {
            LCD.WriteRC(num, row, col);
            char text[40];
            itoa(num, text, 10);
            col = (col + strlen(text)) % 26;
            row += ((strlen(text)) / 26);
            correctRow();
        }

        void write(float num) {
            LCD.WriteRC(num, row, col);
            char text[400];
            itoa(num, text, 10);
            col = (col + strlen(text)) % 26;
            row += ((strlen(text)) / 26);
            correctRow();
        }

        void write(bool optn) {
            LCD.WriteRC(optn, row, col);
            char text[400];
            itoa(optn, text, 10);
            col = (col + strlen(text)) % 26;
            row += ((strlen(text)) / 26);
            correctRow();
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

        void setBackgroundRed() {
            LCD.SetBackgroundColor(RED);
            this->clear();
        }

        void setBackgroundBlue() {
            LCD.SetBackgroundColor(BLUE);
            this->clear();
        }

        void setBackgroundBlack() {
            LCD.SetBackgroundColor(BLACK);
            this->clear();
        }
};

// Class objects here
Telemetry telemetry;

int lever;

int main(void)
{
    rampAdjust = .17;
    preRun("Milestone 5");
    telemetry.write("Lever is ");
    telemetry.writeLine(lever);

    spinner.SetPercent(50);
    Sleep(3000);
    spinner.SetPercent(0);

    Sleep(1000);

    spinner.SetPercent(-50);
    Sleep(3000);
    spinner.SetPercent(0);


    stopRun();
}



// Function definitions here

int preRun(char prgName[]) {
    armServo.SetMin(970);
    armServo.SetMax(2100);
    armServo.SetDegree(180);
    connectRCS();
    waitForStartLight(prgName);
    //waitForTouch(prgName);
    lever = RCS.GetLever();
    //lever = 0;
    return RCS.GetLever();
    //return 0;
}

void waitForTouch(char prgName[]) {
    float x, y;

    LCD.Clear(BLACK);
    LCD.SetFontColor(WHITE);
    LCD.WriteLine("Team F3 -- Bot'ny Bots");
    LCD.WriteLine(prgName);
    LCD.WriteLine("Pre-Run: Touch the screen");
    LCD.Write("Batt: ");
    LCD.Write(Battery.Voltage());
    LCD.WriteLine(" V");
    LCD.WriteLine("/////////////\\\\\\\\\\\\\\\\\\\\\\\\\\");

    while(!LCD.Touch(&x,&y)); //Wait for screen to be pressed
    while(LCD.Touch(&x,&y)); //Wait for screen to be unpressed

    LCD.WriteRC(" Running...               ", 2, 0);
}

void stopRun() {
    stopMotors();
    armServo.Off();
    LCD.WriteRC(" Stopped               ", 2, 0);
    LCD.WriteRC("                    ", 3, 0);
    LCD.WriteRC("Batt: ", 3, 0);
    LCD.WriteRC(Battery.Voltage(), 3, 6);
    LCD.WriteRC(" V", 3, 12);
    Buzzer.Buzz(500);
    while(true);
}

void moveForward(int percent, int inches) { // DEPRECIATED - Do not use unless for testing
    telemetry.writeLine("WARNING: moveForward() method should not be used.");
    
    //Reset encoder counts
    leftEncoder.ResetCounts();
    rightEncoder.ResetCounts();

    //Set both motors to desired percent
    leftMotor.SetPercent(percent * leftReverse);
    //LCD.WriteLine(percent * leftReverse);
    rightMotor.SetPercent((percent * (rightReverse)) * 1.11);
    //LCD.WriteLine(percent * (rightReverse));
    
    //While the average of the left and right encoder is less than counts,
    //keep running motors
    while((leftEncoder.Counts() + rightEncoder.Counts()) / 2.0 < inches * COUNTS_PER_INCH_RIGHT);
    //float x, y;
    //while(true);

    //Turn off motors
    stopMotors();
    //LCD.WriteLine("Stopped");
}

void moveBackward(int percent, int inches) { // DEPRECIATED - Do not use unless for testing
    telemetry.writeLine("WARNING: moveBackward() method should not be used.");

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

void turn(int percent, int direction, int degrees) { // DEPRECIATED - Do not use unless for testing
    telemetry.writeLine("WARNING: turn() method should not be used.");

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
    leftMotor.SetPercent(powerAdjust(p));
    rightMotor.SetPercent(powerAdjust(p));
}

void setMotorsPercent(float pl, float pr) {
    leftMotor.SetPercent(powerAdjust(pl));
    rightMotor.SetPercent(powerAdjust(pr));
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
    return pwr * rampAdjust; // TODO: Dervive better correction equation
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

float powerAdjust(float desiredPwr) {
    return (MAX_BATT_VOLTS / Battery.Voltage()) * desiredPwr;
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

// Write RCS Code

void connectRCS() {
    RCS.InitializeTouchMenu(TEAM_ID_STRING);
    Sleep(250);
}

// Write CdS Cell code 

void waitForStartLight(char prgName[]) {
    LCD.Clear(BLACK);
    LCD.SetFontColor(WHITE);
    LCD.WriteLine("Team F3 -- Bot'ny Bots");
    LCD.WriteLine(prgName);
    LCD.WriteLine("Pre-Run: Await Start Light");
    LCD.Write("Batt: ");
    LCD.Write(Battery.Voltage());
    LCD.WriteLine(" V");
    LCD.WriteLine("/////////////\\\\\\\\\\\\\\\\\\\\\\\\\\");

    while(lightSenor.Value() > LIGHT_ON_THRESHOLD) {Sleep(5);}

    Sleep(500);
    LCD.WriteRC(" Running...               ", 2, 0);
}

int getLightColor() {
    float lightVal = lightSenor.Value();

    if(lightVal > BLUE_THRESHOLD) {
        return 1;
    } else if(lightVal > RED_THRESHOLD) {
        return 0;
    } else {
        return 0;
    }
}

void lightSensorReadout() {
    telemetry.clear();
    telemetry.write("CdS Value: ");
    telemetry.writeLine(lightSenor.Value());
}

void cdsTest() {
    //waitForTouch("Light Sensor Test");
    while(true)
    {
        lightSensorReadout();
        Sleep(100);
    }
}
