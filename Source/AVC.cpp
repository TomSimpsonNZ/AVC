#include <stdio.h>
#include <time.h>
#include "E101.h"

int diff_e;
int error0 = 0;
int error1 = 0;
int threshold;
int vGo = 40; //set this to the speed that you want the robot to travel at by default
int dv; //difference in speed between the two motors
int vL, vR;
int pixRow[320];
int leftRow[100];
int rightRow[100];
int nwp = 1;
int	max = 0;
int	min = 255;
int quadrant = 1;

int red;
int green;
int blue;

float diff_t;
float rateOfChange;

clock_t t0 = clock();
FILE* file;

float Kp = 0.5f;
float Kd = 0.5f;

//Ip address is 10.140.30.203

const int width = 320;
const int lineHeight = 100;
const int MAX_ERROR = 12800;
const bool dev = true;

void openGate();
void lineFollower();
void lineMaze();
bool turnLeft();
bool turnRight();
void turnMaze(bool left);
void turnMaze180();
void wallMaze();
void findLine(int array[width], int min, int max);
void calculateThreshold();
void calculateProportionalError();
void calculateDerivative();
void calculateDv();
int calculateLeftError();

int main() {

	file = fopen("log.txt", "w");

	init();

	while(true){

		nwp = 1;

		switch(quadrant){

			case 1:
				openGate();
				t0 = clock();
				break;

			case 2:
				lineFollower();
				break;

			case 3:
				lineMaze();
				break;

			case 4:
				wallMaze();
				break;
		}
	}

	fclose(file);
}

/*
 * Opens the network gate
 */
void openGate(){

	int i = -1;

	while(i < 0){

		i = connect_to_server("130.195.6.196", 1024);

		fprintf(file, "%d", i);

	}

	send_to_server("Please");

	char password[24];
	receive_from_server(password);

	if(dev){
		fprintf(file, "password: %s\n", password);
	}

	send_to_server(password);
	quadrant++;
}

/*
 * Follows the line in Q2
 */
void lineFollower(){

	calculateThreshold();

	if (max < 100) { // if max < 100 then white line is not being sensed by camera

		findLine(pixRow, min, max);

	}

	calculateProportionalError();

	calculateDerivative();

	calculateDv();

	// if it reaches white line break and continue to quadrant three code
	bool left = turnLeft();
	bool right = turnRight();
	if (left && right) {
		sleep1(1, 0);
		Kp = 0.45;
		Kd = 20.0f;
		set_motor(1, 0);
		set_motor(2, 0);
		sleep1(0, 500000);
		quadrant++;
	}

	//set left and right velocities
	vL = vGo + dv;
	vR = vGo - dv;

	set_motor(1, -vL);
	set_motor(2, -vR);
}

/*
 * Runs throught the line maze
 */
void lineMaze(){

	calculateThreshold();
	calculateProportionalError();
	calculateDerivative();
	calculateDv();


	bool left = turnLeft();
	bool right = turnRight();

	if (left) {
		turnMaze(true);
	}
	else if (right) {
		turnMaze(false);
	}
	else if (left && right) {
		turnMaze(true);
	}
	else if (!left && !right && max < 100) { //if it's at the end of a line
		turnMaze180();
	}

	//check if it's at the end of the quadrant
	red = (int) get_pixel(120, 160, 0);
	green = (int) get_pixel(120, 160, 1);
	blue = (int) get_pixel(120, 160, 2);

	if (red > 200 && green > 100 && blue < 100) {
		quadrant++;
		set_motor(1, 0);
		set_motor(2, 0);
		return;
	}

	//set left and right velocities
	vL = vGo + dv;
	vR = vGo - dv;

	set_motor(1, -vL);
	set_motor(2, -vR);
}
/*
 * Checks if the error is to the right
 */
bool turnLeft() {

	int total = 0;

	for (int i = 0; i < lineHeight; i++) {
		total = total + leftRow[i];
	}

	if (total > 0){
		return true;
	}
	else {
		return false;
	}
}

/*
 * Checks if the error is to the right
 */
bool turnRight() {

	int total = 0;

	for (int i = 0; i < lineHeight; i++) {
		total = total + rightRow[i];
	}

	if (total > 0){
		return true;
	}
	else {
		return false;
	}
}

/*
 * Turns the robot until the line is found
 */
 void turnMaze (bool left) {
 	set_motor(1, -vGo);
 	set_motor(2, -vGo);
 	sleep1(0, 500000);
 	set_motor(1, 0);
 	set_motor(2, 0);

 	bool turn = true;

 	while (turn) {
 		calculateThreshold();
 		calculateProportionalError();

 		//if there isnt a line or the line isnt in the center, turn
 		if (error1 != 0 || max < 100) {
 			//turn
 			if (left) {
 				set_motor(1, -vGo);
 				set_motor(2, vGo);
 			}
 			else {
 				set_motor(1, vGo);
 				set_motor(2, -vGo);
 			}
 		}
 		else {
 			//stop turning
 			set_motor(1, 0);
 			set_motor(2, 0);
 			turn = false;
 		}
 	}
 }


 /*
  * Navigates the wall maze
  */
void wallMaze(){

}

void findLine(int array[width], int min, int max) {

	//Aiden If you delete this I will hunt you down :)
	/* if (fucked) {
	 * 	  unfuck();
	 * }*/

	bool foundLine = false;

	while (!foundLine){

		take_picture();

		for (int i = 0; i < width; i++) {

			array[i] = (int)get_pixel(120, i + 1, 3);

			if (array[i] > max){

				max = array[i];

			}
			else if (array[i] < min) {

				min = array[i];

			}
		}

		if (max > 150) {

			foundLine = true;

			set_motor(1, 0);
			set_motor(2, 0);

		}
		else {

			set_motor(1, vGo);
			set_motor(2, vGo);

		}
	}
}

/*
 * Creates the image arrays, inputs the pixel values and calculates the threshold
 */
void calculateThreshold(){

	take_picture();

	//make an array of all the pixels in a row of the image and set max and min
	for (int i = 0; i < width; i++) {

		pixRow[i] = (int)get_pixel(120, i+1, 3);

		if (pixRow[i] > max){
			max = pixRow[i];
		}
		else if (pixRow[i] < min) {
			min = pixRow[i];
		}
	}

	//gets vertical lines for the line maze so we can easily tell when to turn left or right
	for (int i = 0; i < lineHeight; i++) {
		leftRow[i] = (int)get_pixel(61+i, 50, 3);
		rightRow[i] = (int)get_pixel(61+i, width-50, 3);
	}

	threshold = (max+min)/2; 	//set the threshold to equal half the difference between the max and the minumum
}

/*
 * Calculates the distance away from the center and sets the left anf right arrays to the appropriate values
 */
void calculateProportionalError(){

	error1 = 0;

	if (dev) {
		fprintf(file, "max = %d\nmin = %d\nthreshold = %d\n", max, min, threshold);
	}

	//change all values below threshold to 0, and all values above to index
	for (int i = 0; i < width; i++) {	//go through the array again and set white pixels

		if (pixRow[i] > threshold) {
			pixRow[i] = (i-160); 		//set this back to width/2 when it works
			nwp++;
		}
		else {
			pixRow[i] = 0;
		}
	}

	//Sets the values of the left and right array to 1 or 0 depending if it contains white pixels
	for (int i = 0; i < lineHeight; i++) {
		//set the left side
		if (leftRow[i] > threshold) {
			leftRow[i] = 1;
		}
		else {
			leftRow[i] = 0;
		}

		//set the right side
		if (rightRow[i] > threshold) {
			rightRow[i] = 1;
		}
		else {
			rightRow[i] = 0;
		}
	}


	if (dev) {
		fprintf(file, "nwp = %d\n", nwp);
	}

	//calculate error
	int j = width - 1;
	for (int i = 0; i < (width/2); i++) {
		error1 = error1 + (pixRow[i] + pixRow[j]);
		j--;
	}

	if (dev) {
		fprintf(file, "error1(1) = %d\n", error1);
	}

	/*if (nwp = 0) {
		nwp = 1;
	}*/

	//normalise error
	error1 = error1/nwp;

	if (dev) {
		fprintf(file, "error1(2) = %d\n", error1);
	}
}

/*
 * Does some calculous bullshit
 */
void calculateDerivative(){

	//calculate rate of change
	diff_e = error1 - error0;
	diff_t = clock() - t0;
	rateOfChange = (diff_e/diff_t) * 150;

	t0 = clock();
	error0 = error1;
}

/*
 * Calculates the needed change in velocity
 */
void calculateDv(){
	//calculate difference in motor speed
	dv = (error1* Kp + rateOfChange * Kd);
}
