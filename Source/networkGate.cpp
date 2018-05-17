#include <stdio.h>
#include "E101.h"
#include "networkGate.h"

void openGate() {
	int i = -1;
	
	while(i < 0){
		i = connect_to_server("130.195.6.196", 1024);
		printf("%d", i);
	}
	
	send_to_server("Please");
	
	char password[24]; 
	receive_from_server(password);
	
	printf("%s", password);
	send_to_server(password);
}
