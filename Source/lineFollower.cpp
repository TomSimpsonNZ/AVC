#include <stdio.h>
#include <time.h>

int main() {
	init();
	
	int error0 = 0;
	int error1 = 0;
	int threshold;
	unsigned char vGo; //set this to the speed that you want the robot to travel at by default
	unsigned char dv; //difference in speed between the two motors
	int pixRow[320];
	int nwp = 0;
	float Kp = 0.005; //How aggresively the robot responds to the line not being in the center of the camera
	float Kd = 0.005;
	
	
	unsigned char max = 0;
	unsigned char min = 255;
	
	clock_t t0 = clock(); // set this right before the loop starts so that there is actually a start time
	
	//note that negative means that the line is on the left and that the robot need to move right
	while(true) {
		//set error1 to 0
		//take a picture
		//make an array of all the pixels in a row of the image
		//go through all the values in the array
			//check if the current value is greater than the maximum
				// if true set the maximum to this value
			//check if the current value is smaller than the minimum
				//if true set minumum to the current value
				
		//set the threshold to equal half the difference between the max and the minumum
		//go through the array again
			//check if the current value is greater than the threshhold
				//if true set the value to 1 and add 1 to nwp
				//else set it to 0
				
			//multiply the value by the amount of values it is away from the center
			//(could do this by multiplying the value by (i-numPixelsInARow/2) where i=0 will give -160 and i=320 will give 160
		
		//the end is a solid white line perpendicular to the line that the robot was following, so now check to see
		//if the number of white pixels is larger than a certain value
			//if true, break;
		
		//set the error value
		//decare int j = 319;
		//go through the array again (accidentally halved the time for this loop by doing this lol (thought about this a bit, maybe not. still stops overflow issues))
			//add the furthest left pixel to the furthest right pixel
			//add that value to error1
			//decrease j by 1
			
		//normalise error (error/nwp) to make it so thiccccer lines dont fuck with my baby
		//make error a value between 0 and 255
			//divide the error by the maximum possible error (figure this shit out later)
			//multiply this by 255 and type cast it as an unsigned char
		
		//get the differential
			//set diff_e to error1-error0
			//set diff_t to ((float)(clock() - t0) / CLOCKS_PER_SEC );
			//set diff to diff_e/diff_t
		//set t0 to current time (t0 = clock() 	
		
		//set dv to error * Kp + diff * Kd
		//set motor speeds
			//vL = v_go + dv
			//vR = v_go - dv
			// set_motor(1, vL)
			// set_motor(2, vR)
	}
	
}
