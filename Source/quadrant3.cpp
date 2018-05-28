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


float diff_t;
float rateOfChange;

clock_t t0;
FILE* file;

//WORKED OUT WHILE WORKING ON REPORT
//if left side is completely white pixels,
		//travel forwards
		//until you loose the line or the left side returns to normal
//if loose line after this method
		//turn (left motor negative, right motor positive to turn the robot left)
		//until the line is a similar size, so you are looking at it straight, to before, give or take 20(?) pixels
//if left side returns to normal
		//go back to normal method
//PROBLEMS WITH THE CODE AT THE MOMENT!
//LINE 242 says that if the number of white pixels is smaller than the width/2
//above same with LINE 106 LINE 107
//for turning corner it should be right = -vGo and left = vGo

float Kp = 0.40f;
float Kd = 10.0f;

//Ip address is 10.140.30.203

const int width = 320;
const int height = 240;
const int MAX_ERROR = 12800;
const bool dev = false;

void findLine(int array[width], int min, int max) {
	bool foundLine = false;
	while (!foundLine){
		take_picture();
		for (int i = 0; i < width; i++) {
			array[i] = (int)get_pixel(180, i + 1, 3);
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
		}else {
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

void calculateThreshold(){
//	take_picture();	
	//make an array of all the pixels in a row of the image and set max and min
	for (int i = 0; i < width; i++) { 
		pixRow[i] = (int)get_pixel(120, i+1, 3);
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

	// if it reaches white line break and continue to quadrant three code
//	if (nwp > width/2 && max - min < 50 && max > 120) {
	//	quadrant++;
	//}
	if(nwp > 240 || min > 200){
		quadrant++;
		set_motor(1, 0);
		set_motor(2, 0);
	}else{
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
	//if (max < 100) { // if max < 100 then white line is not being sensed by camera
	//	findLine(pixRow, min, max);
	//}
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
		}else if(rightNWP > 20){
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
	}else{
	vL = vGo + dv;
	vR = vGo - dv;
	set_motor(1, -vL);
	set_motor(2, -vR);
	}
}


void wallMaze(){
	
}
int main() {
	file = fopen("log.txt", "w");
	init();
	//int total = 0;
	//for(int i = 0; i < 100; i++){
	//calculateThreshold();
	//total += threshold;
	//}
	//initialThreshold = total/100;
	fprintf(file, "initial threshold: %d", initialThreshold);
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
}


