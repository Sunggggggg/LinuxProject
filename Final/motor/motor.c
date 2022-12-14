#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int Motor(int dev, char dir){
    
    write(dev,&dir,1);
    
}


int main(int argc, char **argv){
    int dev_motor = open("/dev/my_motor_driver",O_RDWR);
    if(dev == -1){
        printf("motor driver not possible!\n");
        return -1;
    }
    printf("All driver open!\n");

    while(1){
        Motor(dev_motor, dir);
    }

}
