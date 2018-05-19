 //
#include <stdio.h>
#include "E101.h"
#include "networkGate.h"

void openGate() {
	int i = -1;
	char server[] = "130.195.6.196";//Server IP may change
	
	while(i < 0){
		i = connect_to_server(server, 1024);
		printf("%d", i);
	}
	
	send_to_server("Please"); //The uppercase "P" does matter
	
	char password[24]; 
	receive_from_server(password);
	
	printf("%s", password);
	send_to_server(password);
}
