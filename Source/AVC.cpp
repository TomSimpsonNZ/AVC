#include <stdio.h>
#include "E101.h"
#include "networkGate.h"
#include "lineFollower.h"
#include "lineMaze.h"
#include "wallMaze.h"

//if you want to test your specific function, just set the value of quadrant to the quadrant that your function applies to.

int main() {
	
	init();
	
	int quadrant = 1;
	
	while(true){
		switch (quadrant){
			case 1:
				openGate(); 
				quadrant++
				break;

			case 2:
				followLine();
				quadrant++;
				break;

			case 3:
				lineMaze(); 
				quadrant++;
				break;

			case 4:	
				wallMaze(); 
				break;

			default:
				break;
		}
	}

	return 0;
}

