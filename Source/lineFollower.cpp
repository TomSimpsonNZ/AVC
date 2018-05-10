#include <stdio.h>
#include <time.h>
#include "E101.h"

//Ip address is 10.140.30.203

const int width = 320;
const int MAX_ERROR = 12800;

int main() {
	
	init();
	
	int error0 = 0;
	int error1 = 0;
	unsigned char threshold;
	unsigned char vGo = 125; //set this to the speed that you want the robot to travel at by default
	unsigned char dv; //difference in speed between the two motors
	int vL, vR;
	int pixRow[320];
	int nwp = 0;
	float Kp = 0.005f; //How aggresively the robot responds to the line not being in the center of the camera
	float Kd = 0.005f;
	
	int diff_e, diff_t;
	float diff;
	
	unsigned char max = 0;
	unsigned char min = 255;
	
	clock_t t0 = clock(); // set this right before the loop starts so that there is actually a start time
	
	//note that negative means that the line is on the left and that the robot need to move right
	while(true) {
		//set error1 to 0
		error1 = 0;
		//take a picture
		take_picture();		
		
		//make an array of all the pixels in a row of the image
		//go through all the values in the array
			//check if the current value is greater than the maximum
				// if true set the maximum to this value
			//check if the current value is smaller than the minimum
				//if true set minumum to the current value
		for (int i = 0; i < width; i++) {
			pixRow[i] = get_pixel(120, i + 1, 3);
			if (pixRow[i] > max){
				max = pixRow[i];
			}
			else if (pixRow[i] < min) {
				min = pixRow[i];
			}
		}
				
		//set the threshold to equal half the difference between the max and the minumum
		threshold = (unsigned char)((max + min)/2.0F);			//check if this is the same
		
		//go through the array again
			//check if the current value is greater than the threshhold
				//if true set the value to 1 and add 1 to nwp
				//else set it to 0
				
			//multiply the value by the amount of values it is away from the center
			//(could do this by multiplying the value by (i-numPixelsInARow/2) where i=0 will give -160 and i=320 will give 160
		
		for (int i = 0; i < width; i++) {
			if (pixRow[i] > threshold) {
				pixRow[i] = (i-(width/2)); //1 x A is A, so you can skip the multiplication step
				nwp++;
			}
			else {
				pixRow[i] = 0;
			}
		}
		
		//the end is a solid white line perpendicular to the line that the robot was following, so now check to see
		//if the number of white pixels is larger than a certain value
			//if true, break;
		/*if (nwp > width/2) {
			break;						//comment this out
		}*/
		
		//set the error value
		//decare int j = 319;
		//go through the array again (accidentally halved the time for this loop by doing this lol (thought about this a bit, maybe not. still stops overflow issues))
			//add the furthest left pixel to the furthest right pixel
			//add that value to error1
			//decrease j by 1
		int j = width - 1;
		for (int i = 0; i < width; i++) {
			error1 += pixRow[i] + pixRow[j];
			j--;
		}
			
		//normalise error (error/nwp) to make it so thiccccer lines dont fuck with my baby
		error1 = error1/nwp;
		
		//make error a value between 0 and 255
			//divide the error by the maximum possible error (figure this shit out later)
			//multiply this by 255
		error1 = (error1/MAx_ERROR) * 255;
		
		//get the differential
			//set diff_e to error1-error0
			//set diff_t to ((float)(clock() - t0) / CLOCKS_PER_SEC );
			//set diff to diff_e/diff_t
		//set t0 to current time (t0 = clock() 	
		//set error0 = error1
		diff_e = error1 - error0;
		diff_t = ((float)(clock() - t0)/ CLOCKS_PER_SEC);
		printf("float conversion complete\n"); //insert this
		diff = (float)diff_e/(float)diff_t;
		t0 = clock();
		error0 = error1;
		
		//set dv to error * Kp + diff * Kd
		//set motor speeds
			//vL = v_go + dv
			//vR = v_go - dv
			// set_motor(1, vL)
			// set_motor(2, vR)
		
		dv = (int)(error1 * Kp + diff * Kd); //change this
		printf("int conversion complete\n"); //insert this
		vL = vGo + dv;
		vR = vGo - dv;
		
		set_motor(1, vL);
		set_motor(2, vR);
	}
	
	set_motor(1,0);
	set_motor(2,0);
	
	return 0;
}
