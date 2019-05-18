#include "main.h"

enum CONTROL_STYLE
{
	ARCADE_CONTROL,
	TANK_CONTROL
};

/*
    Maps from (-127) -> 127 to itself using the function 
    (((128 * pow(4, ((abs(x)-50)/12.5)))/(pow(4, ((abs(x)-50)/12.5))+1))) - 1)* ((x > 0) - (x < 0))

    Used to ease in/out joystick movement for more precise control.
    
    If the horizontal input is at 25%, it only has 5% of the power, reducing the amount that
    the robot will slowly veer off course fron an imperfect control stick.
    
    Lookup table to reduce computation
    */
int sigmoid_map[255] = {-127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -127, -126, -126, -126, -126, -126, -126, -126, -126, -126, -126, -125, -125, -125, -125, -124, -124, -124, -123, -123, -123, -122, -122, -121, -120, -119, -119, -118, -117, -116, -114, -113, -112, -110, -108, -107, -105, -103, -100, -98, -95, -93, -90, -87, -84, -80, -77, -74, -70, -67, -63, -59, -56, -52, -49, -46, -42, -39, -36, -33, -31, -28, -26, -23, -21, -19, -18, -16, -14, -13, -12, -10, -9, -8, -7, -7, -6, -5, -4, -4, -3, -3, -3, -2, -2, -2, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 5, 6, 7, 7, 8, 9, 10, 12, 13, 14, 16, 18, 19, 21, 23, 26, 28, 31, 33, 36, 39, 42, 46, 49, 52, 56, 59, 63, 67, 70, 74, 77, 80, 84, 87, 90, 93, 95, 98, 100, 103, 105, 107, 108, 110, 112, 113, 114, 116, 117, 118, 119, 119, 120, 121, 122, 122, 123, 123, 123, 124, 124, 124, 125, 125, 125, 125, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127};

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol()
{
	pros::Controller master(pros::E_CONTROLLER_MASTER);
	pros::Controller partner(pros::E_CONTROLLER_PARTNER);
	pros::Motor left_mtr(1);
	pros::Motor right_mtr(2);
	CONTROL_STYLE mode = TANK_CONTROL;
	short switch_cooldown = 0;
	while (true)
	{
		/*
		  Tank control - Default control style.
		  
		  Left joystick controls left wheels, right joystick controls right
		  wheels. 
		 */
		if (mode = TANK_CONTROL)
		{
			int left = sigmoid_map[master.get_analog(ANALOG_LEFT_Y) + 127];
			int right = sigmoid_map[master.get_analog(ANALOG_RIGHT_Y) + 127];

			left_mtr = left;
			right_mtr = right;
		}
		/*
		  Arcade control

		  Left joystick controls the entire robot. Up/down for forward/backwards,
		  left/right for turning.
		*/
		else if (mode = ARCADE_CONTROL)
		{
			int forward = sigmoid_map[master.get_analog(ANALOG_LEFT_Y) + 127];
			int steer = sigmoid_map[master.get_analog(ANALOG_LEFT_X) + 127];

			left_mtr = forward + steer;
			right_mtr = forward - steer;
		}

		/*
		  Control style switching logic.

		  Switches from tank to arcade and vice versa when all four shoulder
		  buttons are held down.
		  
		  1 second cooldown between switches.
		*/
		switch_cooldown--;
		if (master.get_digital(DIGITAL_L1) && master.get_digital(DIGITAL_L2) && master.get_digital(DIGITAL_R1) && master.get_digital(DIGITAL_R2) && switch_cooldown <= 0)
		{
			if (mode = TANK_CONTROL)
			{
				mode = ARCADE_CONTROL;
			}
			else if (mode = ARCADE_CONTROL)
			{
				mode = TANK_CONTROL;
			}
			switch_cooldown = 25;
		}
		pros::delay(20);
	}
}