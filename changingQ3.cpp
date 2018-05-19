#include <stdio.h>
#include <time.h>
#include "E101.h"
 
//WORKED OUT WHILE WORKING ON REPORT
//if left side is completely white pixels,
		//travel forwards
		//until you loose the line or the left side returns to normal
//if loose line after this method
		//turn (left motor negative, right motor positive to turn the robot left)
		//until the line is a similar size, so you are looking at it straight, to before, give or take 20(?) pixels
//if left side returns to normal
		//go back to normal method
//LINE 242 says that if the number of white pixels is smaller than the width/2
//above same with LINE 106 LINE 107
//for turning corner it should be right = -vGo and left = vGo

float Kp = 0.45f;
float Kd = 20.0f;

//Ip address is 10.140.30.203

const int width = 320;
const int MAX_ERROR = 12800;
const bool dev = false;

void findLine(int array[width], int min, int max) {
	
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
			set_motor(1, 50);
			set_motor(2, 50);
		}
	}
}

void openGate(){
	 int i = -1;
	 while(i < 0){
		i = connect_to_server("130.195.6.196", 1024);
		printf("%d", i);
	}
	send_to_server("Please");
	char password[24]; 
	receive_from_server(password);
	//printf("%s", password);
	send_to_server(password);
	
}

int TurnCornerOne(){
	int take_picture();
        int max = 0;
        int min = 225;
    /*    for (int i = 0; i < 320; i++){  
             int pix = get_pixel(120,i,3);
             if (pix > max) {
                  max = pix;
             }
             if (pix < min) {
                  min = pix;
             }
        }*/
        int pixRow[320];
        for (int i = 0; i < width; i++) {
			pixRow[i] = (int)get_pixel(120, i+1, 3);
			if (pixRow[i] > max){
				max = pixRow[i];
			}
			else if (pixRow[i] < min) {
				min = pixRow[i];
			}
		}
        int diff = max-min;
        int nwp = 0;
        for (int i = 0; i < width; i++) {
			if (pixRow[i] > diff/2) {
				pixRow[i] = (i-160); //set this back to width/2 when it works
				nwp++;
			}
			else {
				pixRow[i] = 0;
			}
		}
		
      
	if (nwp < width/2 && max - min < 50 && max < 100) {return 2;}//lostline
	if (diff>50 && nwp < 160){
		return 1; //normal
	}
	return 0;//
}

int turnCornerTwo(){
	int take_picture();
	int max = 0;
        int min = 225;
        for (int i = 0; i < 320; i++){  
             int pix = get_pixel(120,i,3);
             if (pix > max) {
                  max = pix;
             }
             if (pix < min) {
                  min = pix;
             }
        }
        int diff = max-min;
        int whi[320];
		int white;
	
    	for (int i = 0; i<320; i++)
    	{
      		if (whi[i]==200)
        	{
           		white++;
        	}
    	}
	if (diff>50){ return 1;}
	return 0;
}

int main() {
	
	
	init();
	sleep1(0, 0);
	openGate();

	FILE* file;
	file = fopen("log.txt", "w");
	
	int error0 = 0;
	int error1 = 0;
	int threshold;
	int vGo = 40; //set this to the speed that you want the robot to travel at by default
	int dv; //difference in speed between the two motors
	int vL, vR;
	int pixRow[320];
	int nwp = 0;
	//change these to adjust
	 //How aggresively the robot responds to the line not being in the center of the camera
	
	
	int diff_e;
	float diff_t;
	float diff;
	
	int max = 0;
	int min = 255;
	
	clock_t t0 = clock(); // set this right before the loop starts so that there is actually a start time
	
	//note that negative means that the line is on the left and that the robot need to move right
	while(true) {
		//set error1 to 0
		error1 = 0;
		nwp = 0;
		max = 0;
		min = 255;
		//take a picture
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
		
		if (max < 100) {
			findLine(pixRow, min, max);
		}
				
		//set the threshold to equal half the difference between the max and the minumum
		threshold = (max+min)/2;
//		printf("%d/n", threshold);
		if (dev) {
			fprintf(file, "max = %d\nmin = %d\nthreshold = %d\n", max, min, threshold);
		}
		
		//go through the array again and set white pixels
		for (int i = 0; i < width; i++) {
			if (pixRow[i] > threshold) {
				pixRow[i] = (i-160); //set this back to width/2 when it works
				nwp++;
			}
			else {
				pixRow[i] = 0;
			}
		}
		

		if (dev) {
			fprintf(file, "nwp = %d\n", nwp);
		}

		//the end is a solid white line perpendicular to the line that the robot was following, so now check to see
		//if the number of white pixels is larger than a certain value
			//if true, break;
//		if (nwp > width/2 && max - min < 50 && max > 120) {
//			break;
//		}
		
		//set the error value
		int j = width - 1;
		for (int i = 0; i < (width/2); i++) {
			error1 = error1 + (pixRow[i] + pixRow[j]);
			j--;
		}

		if (dev) {
			fprintf(file, "error1(1) = %d\n", error1);
		}
		int whi[320];
		int left = 0;
	    	for (int i = 0; i <160; i++)
    	{
        	left = left + whi[i] * (i-160);
    	}
		
		if (nwp < width/2 && max - min < 50 && max < 100){
			while( TurnCornerOne() ==0){
				set_motor(1, -vGo);
				set_motor(2, -vGo);
		}
		if (TurnCornerOne()==2){
			while(turnCornerTwo()==0){
				set_motor(1, vGo);
				set_motor(2, vGo);
		}
		}
		}

			
		//normalise error (error/nwp) to make it so thiccccer lines dont fuck with my baby
		error1 = error1/nwp;
		if (dev) {
			fprintf(file, "error1(2) = %d\n", error1);
		}

		
		//make error a value between 0 and 255 (tbh it's not but I don't even know)
		error1 = (error1*150)/(MAX_ERROR/nwp);
		
		//get the differential
		diff_e = error1 - error0;
		diff_t = clock() - t0;
		diff = (diff_e/diff_t) * 150;
		t0 = clock();
		error0 = error1;

		if(dev){
			fprintf(file, "diff = %f\n", diff);
			fprintf(file, "final error1 = %d\n", error1);
		}
		// set velocity change
		dv = (int)(error1 * Kp + diff * Kd);

		if(dev) {
			fprintf(file, "dv = %d\n", dv);
		}
		
		//set left and right velocities
		vL = vGo + dv;
		vR = vGo - dv;
		
		set_motor(1, -vL);
		set_motor(2, -vR);

	}
	
	set_motor(1,0);
	set_motor(2,0);
	

	fclose(file);
	return 0;
}
