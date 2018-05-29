#include <stdio.h>
#include <time.h>
#include "E101.h"

int diff_e;
int error0 = 0;
int error1 = 0;
int threshold = 0;
int vGo = 45; //set this to the speed that you want the robot to travel at by default
int dv; //difference in speed between the two motors
int vL, vR;
int pixRow[320];
int pixColLeft[240];
int pixColRight[240];
int nwp = 0;
int max = 0;
int min = 255;

int quadrant = 1;

int leftError = 0;
int rightError = 0;
int initialThreshold = 0;
int leftNWP = 0;
int rightNWP = 0;
int wallDist = 600;
int adcReadingFront;
int adcReadingRight;
int count = 0;


float diff_t;
float rateOfChange;

clock_t t0;
FILE* file;
FILE* wallFile;

float Kp = 0.40f;
float Kd = 8.0f;

//Ip address is 10.140.30.203

const int width = 320;
const int height = 240;
const int MAX_ERROR = 12800;
const bool dev = false;

void calculateProportionalWall();
void wallTurn(bool left);


void findLine(int array[width], int min, int max) {
	bool foundLine = false;
	while (!foundLine){
		take_picture();
		for (int i = 0; i < width; i++) {
			array[i] = (int)get_pixel(160, i + 1, 3);
			if (array[i] > max){
				max = array[i];
			}else if (array[i] < min) {
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


bool detectRedLine(){
	int redTotal = 0;
	int greenTotal = 0;
	int blueTotal = 0;

	redTotal = get_pixel(160, 120, 0);
	greenTotal = get_pixel(160, 120, 1);
	blueTotal = get_pixel(160, 120, 2);

	if(redTotal > 200 && blueTotal < 140){
		return true;
	}
	else{
		return false;
	}
}


void calculateThreshold(){
//	take_picture();
	//make an array of all the pixels in a row of the image and set max and min
	for (int i = 0; i < width; i++) {
		pixRow[i] = (int)get_pixel(160, i+1, 3);
		if (pixRow[i] > max){
			max = pixRow[i];
		}else if (pixRow[i] < min) {
			min = pixRow[i];
		}
	}
	threshold = (max+min)/2; 	//set the threshold to equal half the difference between the max and the minumum
}


void calculateVerticalNWP(){
	take_picture();
	leftNWP = 0;
	rightNWP = 0;
	for(int i = 0; i < height;  i++){
		pixColLeft[i] = (int)get_pixel(i + 1, 5, 3);
		pixColRight[i] = (int)get_pixel(i + 1, 315, 3);
	}
	for (int i = 0; i < height; i++) {	//go through the array again and set white pixels
		if (pixColLeft[i] > threshold) {
			leftNWP++;
		}
		if (pixColRight[i] > threshold) {
			 rightNWP++;
		}
	}
}


void calculateProportionalError(){
	error1 = 0;
	nwp = 0;
	if (dev) {
		fprintf(file, "max = %d\nmin = %d\nthreshold = %d\n", max, min, threshold);
	}
	//change all values below threshold to 0, and all values above to index
	for (int i = 0; i < width; i++) {	//go through the array again and set white pixels
		if (pixRow[i] > threshold) {
			pixRow[i] = (i-160); 		//set this back to width/2 when it works
			nwp++;
		}else {
			pixRow[i] = 0;
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
	//normalise error
	//max error =
	if(nwp > 0){
	error1 = (error1)/nwp;
	}
	if (dev) {
		fprintf(file, "error1(2) = %d\n", error1);
	}
}


void calculateDerivative(){
	//calculate rate of change
	diff_e = error1 - error0;
	diff_t = clock() - t0;
	rateOfChange = (diff_e/diff_t) * 150;
	t0 = clock();
	error0 = error1;
}


void calculateDv(){
	//calculate difference in motor speed
	dv = (error1* Kp + rateOfChange * Kd);
}


// make sure calculateThreshold() and calculatorProportionalError() are called before these two methods
int calculateLeftError(){
	for(int i = 0; i < width/2; i++){
		if(pixRow[i] > threshold){
			leftError += i;
		}
	}
	return leftError;
}


void lineFollower(){
	t0 = clock();
	take_picture();

	if (max < 100) { // if max < 100 then white line is not being sensed by camera
		findLine(pixRow, min, max);
	}

	calculateThreshold();
	calculateProportionalError();
	calculateDerivative();
	calculateDv();

	if(nwp > 240 || min > 200){
		quadrant++;
		set_motor(1, 0);
		set_motor(2, 0);
	}
	else{
		vL = vGo + dv;
		vR = vGo - dv;
		set_motor(1, -vL);
		set_motor(2, -vR);
	}
}


void lineMaze(){
	t0 = clock();
	take_picture();
	calculateThreshold();
	calculateProportionalError();
	calculateDerivative();
	calculateDv();

	if (detectRedLine()) {
		quadrant++;
		set_motor(1, 0);
		set_motor(2, 0);
	}

	if(nwp < 40 || max < 100){

		nwp = 0;
		error1 = 0;
		set_motor(1, 0);
		set_motor(2, 0);
		calculateVerticalNWP();
		fprintf(file, "All Black: %d\n Left: %d Right: %d", nwp, leftNWP, rightNWP);

		if(leftNWP > 10){

			while(nwp < 50 /*&& error1 < -2000*/){
				set_motor(1, vGo - 10);
				set_motor(2, -vGo);
				t0 = clock();
				take_picture();
				calculateThreshold();
				calculateProportionalError();
				calculateDerivative();
				calculateDv();
				fprintf(file, "Left NWP: %d\n", nwp);

			}
		}
		else if(rightNWP > 20){

			while(nwp < 50 /*&& error1 > 2000*/){
				t0 = clock();
				take_picture();
				calculateThreshold();
				calculateProportionalError();
				calculateDerivative();
				calculateDv();
				fprintf(file, "Right NWP: %d\n", nwp);
				set_motor(1, -vGo);
				set_motor(2, vGo - 10);
			}
		}
	}
	else{
	vL = vGo + dv;
	vR = vGo - dv;
	set_motor(1, -vL);
	set_motor(2, -vR);
	}
}


void calculateProportionalWall() {

	if (count == 0) {
		set_motor(1, -vGo);
		set_motor(2, -vGo);
		sleep1(1, 0);
		count++;
	}

	adcReadingRight = read_analog(5); //change this for which one the right sensor is plugged in to
	adcReadingFront = read_analog(7);

	if(adcReadingRight < 200) {
		wallTurn(false);
	}
	else if (adcReadingFront > 400) {
		set_motor(1, 0);
		set_motor(2, 0);
		wallTurn(true);
	}

	error1 = wallDist - adcReadingRight;
	fprintf(wallFile, "error1:%d\n", error1);
}


void wallTurn(bool left) {
	if (!left) {

		while (adcReadingRight < 200) {
			set_motor(1, -vGo);
			set_motor(2, -vGo * 0.25);
			adcReadingRight = read_analog(5);
		}
	}
	else {
		set_motor(1, vGo);
		set_motor(2, -vGo);
		sleep1(0, 750000);
	}
}


void stopAndWait() {
	set_motor(1, 0);
	set_motor(2, 0);

	adcReadingFront = read_analog(7);

	while (adcReadingFront > 100) {
		adcReadingFront = read_analog(7);
		sleep1(0, 250000);
	}
}



void wallMaze(){
	t0 = clock();
	error1 = 0;

	take_picture();
	
	calculateProportionalWall();
	calculateDerivative();
	calculateDv();

	if (detectRedLine()) {
		stopAndWait();
	}

	vL = vGo + dv;
	vR = vGo - dv;
	set_motor(1, -vL);
	set_motor(2, -vR);
}


int main() {
	file = fopen("log.txt", "w");
	wallFile = fopen("wall-log.txt", "w");

	init();

	while(true){

	fprintf(file, "quadrant: %d\n", quadrant);
		switch(quadrant){
			case 1:
				openGate();
				break;
			case 2:
				lineFollower();
				break;
			case 3:
			//	threshold = initialThreshold;
				lineMaze();
				break;
			case 4:
				wallMaze();
				break;
		}
	}
	fclose(file);
	fclose(wallFile);
}
