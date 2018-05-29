#include <stdio.h>
#include <time.h>
#include "E101.h"

int main() {
    init();

    int adc1;
    int adc2;
    int adc3;

    for (int i = 0; i < 500; i++){
        adc1 = read_analog(5);
        adc2 = read_analog(6);
        adc3 = read_analog(7);
        printf("1:%d\t2:%d\t3:%d\n", adc1, adc2, adc3);
    }
}
