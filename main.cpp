#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHUtility.h>
#include <FEHMotor.h>
#include <cmath>

//Declarations for encoders & motors // TODO: Get correct ports
DigitalEncoder right_encoder(FEHIO::P0_0);
DigitalEncoder left_encoder(FEHIO::P3_0);
FEHMotor right_motor(FEHMotor::Motor1,9.0);
FEHMotor left_motor(FEHMotor::Motor0,9.0);

// TODO: Add sensor declarations

// TODO: Get drivetrain width and test counts.
#define COUNTS_PER_INCH_RIGHT 43
#define COUNTS_PER_INCH_LEFT 43
#define DRIVETRAIN_WIDTH 7

#define PI 3.1459

// Function declarations here
void wait_for_touch();
void move_forward(int percent, int inches);
void turn(int percent, int direction, int degrees);

int main(void)
{
    float x, y; //for touch screen

    //Initialize the screen
    LCD.Clear(BLACK);
    LCD.SetFontColor(WHITE);

    LCD.WriteLine("Encoder Nav");
    LCD.WriteLine("Touch the screen");
    while(!LCD.Touch(&x,&y)); //Wait for screen to be pressed
    while(LCD.Touch(&x,&y)); //Wait for screen to be unpressed

    move_forward(30, 14);
    Sleep(50);
    turn(20, -1, 90);
    Sleep(50);
    move_forward(30, 10);
    Sleep(50);
    turn(20, 1, 90);
    Sleep(50);
    move_forward(20, 4);
    Sleep(50);
}

// Function definitions here
void waitForTouch() {
    float x, y;
    while(!LCD.Touch(&x,&y)); //Wait for screen to be pressed
    while(LCD.Touch(&x,&y)); //Wait for screen to be unpressed
}


void move_forward(int percent, int inches) {
    //Reset encoder counts
    right_encoder.ResetCounts();
    left_encoder.ResetCounts();

    //Set both motors to desired percent
    right_motor.SetPercent(percent);
    left_motor.SetPercent(percent);

    //While the average of the left and right encoder is less than counts,
    //keep running motors
    while((left_encoder.Counts() + right_encoder.Counts()) / 2. < inches * COUNTS_PER_INCH_RIGHT);

    //Turn off motors
    right_motor.Stop();
    left_motor.Stop();
}

void turn(int percent, int direction, int degrees) {
    int turnInches = float((PI * DRIVETRAIN_WIDTH) * (degrees / 360.0));
    LCD.WriteLine("Turning");

    right_encoder.ResetCounts();
    left_encoder.ResetCounts();

    right_motor.SetPercent(percent * -direction);
    left_motor.SetPercent(percent * direction);

    //While the average of the left and right encoder is less than counts,
    //keep running motors
    while(((abs(left_encoder.Counts()) + abs(right_encoder.Counts())) / 2.0) < turnInches * COUNTS_PER_INCH_LEFT);

    //Turn off motors
    right_motor.Stop();
    left_motor.Stop();
}