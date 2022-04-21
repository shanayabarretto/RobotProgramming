//Final source code: Agent Morse's Secret Mission
#include "PC_FILEIO.c"
#define COL 6 // columns and rows required for the morse code array
#define ROW 26
#define TOUCH_SENSOR_PORT S1 // defining the ports
#define GYRO_SENSOR_PORT S3
#define COLOUR_SENSOR_PORT S2
#define SOUND_SENSOR_PORT S4

const int MOTOR_POWER = 50; // constants maintain uniform robot speed
const int TURN_SPEED = 50;
const int REVERSE_SPEED = -25;
const int ARM_SPEED = 30;

const int SHORT_WAIT = 1;
const int LONG_WAIT = 3;

char morseArray[ROW][COL];

bool readInMorse(TFileHandle & fin); //written by shanaya
void waitTimeUnit(int timeUnit); //written by brianna
void driveToWall(tSensors TOUCH_SENSOR_PORT, tSensors GYRO_SENSOR_PORT); //written by brianna
void longTap(); //written by ella
void shortTap(); //written by ella
void calibrateGyro(tSensors GYRO_SENSOR_PORT); //taken from course notes
void rotateRobot(tSensors GYRO_SENSOR_PORT, int angle, int speed); //taken from course notes
bool findWallSpot(tSensors COLOUR_SENSOR_PORT, tSensors TOUCH_SENSOR_PORT); //written by cassandra
void tapChar(char letter); //written by all group members together


task main() //written by all group members together
{
	const int SOUND_THRESHOLD = 60;
	const int WAIT_BTWN_WORDS = 7;
	//calibration
	SensorType[COLOUR_SENSOR_PORT] = sensorEV3_Color;
	wait1Msec(50);
	SensorMode[COLOUR_SENSOR_PORT] = modeEV3Color_Color;
	wait1Msec(50);
	SensorType[TOUCH_SENSOR_PORT] = sensorEV3_Touch;
	wait1Msec(50);
	calibrateGyro(GYRO_SENSOR_PORT);
	SensorType[SOUND_SENSOR_PORT] = sensorSoundDBA;
	wait1Msec(50);

	// waits for the button to be pressed
	while (!getButtonPress(buttonEnter))
	{}
	while(getButtonPress(buttonEnter))
	{}

	//from RobotC File I/O video- opens both files
	TFileHandle fin;
	TFileHandle fin2;

	bool fileOkay = openReadPC(fin, "morse_code.txt");
	bool fileOkay2 = openReadPC(fin2, "escaperoutes.txt");
	if (!fileOkay || !fileOkay2)
	{
		displayString(5, "MISSION FAILED!");
		wait1Msec(5000);
	}
	else //shanayas
	{
		bool read = readInMorse(fin);
		if (!read)
		{
			playSound(soundBlip); // blip noise indicates that it has not been read in properly
			wait1Msec(1000); //spies will communicate verbally to neglect the message if this occurs
		}
		//briannas
		driveToWall(S1, S3);

		//cassandras
		motor[motorA] = motor[motorD] = MOTOR_POWER;
		// find indicates whether the red line was found
		bool find = findWallSpot(COLOUR_SENSOR_PORT, TOUCH_SENSOR_PORT);
		motor[motorA] = motor[motorD] = 0;
		// occurs if the red line could not be found
		if (!find)
		{
			playSound(soundBeepBeep);
			wait1Msec(500);
			displayString(5, "MISSION FAILED!");
			wait1Msec(5000);
		}
		else
		{
			//drives up to the wall to type the message
			motor[motorA] = motor[motorD] = MOTOR_POWER;
			while(SensorValue[TOUCH_SENSOR_PORT] == 0)
			{}
			motor[motorA] = motor[motorD] = 0;

			//ellas
			// gives the first letter a starting value
			char currLetter = 'x';
			while (readCharPC(fin2, currLetter)) // reads in letter by letter
			{
				// period indicates a space- a longer wait must occur
				if (currLetter == '.')
					waitTimeUnit(WAIT_BTWN_WORDS);
				else // if it is a letter it will be sent to the tapChar function
					tapChar(currLetter);
			}

			// waits for the other spy to singal that they have heard the message
			while (SensorValue(SOUND_SENSOR_PORT) < SOUND_THRESHOLD)
			{}

			// indicates that Agent Morse has completed her mission
			displayString(5, "MISSION ACCOMPLISHED!");
			playSound(soundBeepBeep);
			wait1Msec(5000);
		}
	}

	// closes files
	closeFilePC(fin);
	closeFilePC(fin2);
}

bool readInMorse(TFileHandle & fin) // shanayas
{
	// occurs for each letter
	for (int counter = 0; counter < ROW; counter++)
	{
		// occurs for all 6 coloumns filled with periods, 'S', or 'L'
		for (int index = 0; index < COL; index++)
		{
			// reads in the character to the specific spot in the array
			bool readChar = readCharPC(fin, morseArray[counter][index]);
			// checks if it has read in properly
			if (!readChar)
				return false;
		}
	}
	// indicates that the file has been read in properly
	return true;
}


void driveToWall(tSensors TOUCH_SENSOR_PORT, tSensors GYRO_SENSOR_PORT) // briannas
{
	resetGyro(GYRO_SENSOR_PORT);
	const int ANGLE = 70;
	//drives forward until touch sensor pressed
	motor[motorA] = motor[motorD] = MOTOR_POWER;
	while(SensorValue[TOUCH_SENSOR_PORT] == 0)
	{}
	motor[motorA] = motor[motorD] = -MOTOR_POWER;
	wait1Msec(500);
	//turns until facing perpendicularangle
	rotateRobot(GYRO_SENSOR_PORT, ANGLE, TURN_SPEED);
}

void calibrateGyro(tSensors GYRO_SENSOR_PORT) // from class notes
{
	SensorType[GYRO_SENSOR_PORT] = sensorEV3_Gyro;
	wait1Msec(50);
	SensorMode[GYRO_SENSOR_PORT] = modeEV3Gyro_Calibration;
	wait1Msec(50);
	SensorMode[GYRO_SENSOR_PORT] = modeEV3Gyro_RateAndAngle;
	wait1Msec(50);
}

void rotateRobot(tSensors GYRO_SENSOR_PORT, int angle, int speed) //from class notes
{
	resetGyro(GYRO_SENSOR_PORT);
	if (angle > 0) // turns right or left based on the sign of the angle inputted
	{
		motor[motorA] = -speed;
		motor[motorD] = speed;
	}
	else
	{
		motor[motorA] = speed;
		motor[motorD] = -speed;
	}

	angle = abs(angle);

	while (abs(getGyroDegrees(GYRO_SENSOR_PORT)) < angle) //turns in that direction till the angle has been reached
	{}

	motor[motorA] = motor[motorD] = 0;
}

bool findWallSpot(tSensors COLOUR_SENSOR_PORT, tSensors TOUCH_SENSOR_PORT) //cassandras
{
	const int FULL_TURN = 160; // angle for complete turn around
	const int BEFORE_TURN = -75; // angle for turn if it didn't hit the wall yet
	const int AFTER_TURN = 66; // angle for turn if it already hit the wall and turned once
	while ((SensorValue[COLOUR_SENSOR_PORT] != (int)colorRed) && (SensorValue[TOUCH_SENSOR_PORT] == 0)) // while not red and not touching, do nothing
	{}
	if (SensorValue[TOUCH_SENSOR_PORT] != 0) // if it hits the wall
	{
		motor[motorA] = motor[motorD] = 0;
		wait1Msec(50);
		motor[motorA] = motor[motorD] = REVERSE_SPEED;
		wait1Msec(500); // reverse for 0.5 second
		motor[motorA] = motor[motorD] = 0;

		rotateRobot(GYRO_SENSOR_PORT, FULL_TURN, TURN_SPEED);// call the turn function
	}
	else
	{
		rotateRobot(GYRO_SENSOR_PORT, BEFORE_TURN, TURN_SPEED); // turns to wall
		return true;
	}
	motor[motorA] = motor[motorD] = MOTOR_POWER;
	while ((SensorValue[COLOUR_SENSOR_PORT] != (int)colorRed) && (SensorValue[TOUCH_SENSOR_PORT] == 0)) // while not red and not touching, do nothing
	{}

	if (SensorValue[COLOUR_SENSOR_PORT] == (int)colorRed)
	{
		rotateRobot(GYRO_SENSOR_PORT, AFTER_TURN, TURN_SPEED); // turns to wall
		return true;
	}
	else
		return false; // did not find the line
}

void waitTimeUnit(int timeUnit) // briannas
{
	//waits the corresponding amount of time
	const int timeConv = 250; // our time 'unit'
	int timeUnitMsec = timeUnit * timeConv;
	time1[T1] = 0;
	while (time1[T1] < timeUnitMsec)
	{}
}

void longTap() //ellas
{
	motor[motorB]= ARM_SPEED; //turns motor on
	waitTimeUnit(LONG_WAIT); // waits at wall for 3 time units
	motor[motorB]= -ARM_SPEED; // moves motor backwards
	wait1Msec(200); // time it takes to move motor back to starting position
	motor[motorB]=0; // turns off motor
	waitTimeUnit(SHORT_WAIT); // standard time between taps
}

void shortTap() //ellas
{
	motor[motorB]= ARM_SPEED; // turns motor on
	waitTimeUnit(SHORT_WAIT); // waits at wall for 1 time unit
	motor[motorB]= -ARM_SPEED; // moves motor backwards
	wait1Msec(200); // time it takes to move motor back to starting position
	motor[motorB]=0; // turns off motor
	waitTimeUnit(SHORT_WAIT); // time between taps
}

void tapChar(char letter) // everyone collaborated
{
	for (int counter = 0; counter < ROW; counter++) // iterates through each row of morse array
	{
		if (morseArray[counter][0] == letter) // we have found the right row
		{
			for (int index = 1; index < COL; index++) // goes through the whole column
			{
				if (morseArray[counter][index] == 'L') // does a long tap if the letter is L
					longTap();
				else if (morseArray[counter][index] == 'S') // short tap if the letter is S
					shortTap();
			}
		}
	}
	waitTimeUnit(LONG_WAIT); // standard time between letters
}
