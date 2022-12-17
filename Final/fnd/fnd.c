// Score FND display example
// use only side fnd (D1 | D4)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define D1 0x01
#define D4 0x08

char seg_num[10] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xd8, 0x80, 0x90};

int FND(int dev, char* score){
    unsigned short data[2];
    int n = 0;

    data[0] = (seg_num[score[0]] << 4) | D1;
    data[1] = (seg_num[score[1]] << 4) | D4;

    write(dev, &data[n], 2);
    n++;
    n = (n + 1) % 2;

    return 0;
}

char buttonUpdate(int dev){
    char buff = 0;
    read(dev, &buff, 1);

    return buff;
}

int main(int argc, char **argv){
    int dev_fnd = open("/dev/fnd_driver", O_RDWR);
    if(dev_fnd == -1){
        printf("fnd driver not possible!\n");
        return -1;
    }

    int dev_gpio = open("/dev/gpio_driver", O_RDWR);
    if(dev_gpio == -1){
        printf("gpio driver not possible!\n");
        return -1;
    }
    printf("All driver open!\n");

    char score[2] = {0, 0};
    char ButtonState = '0';
    char prevButtonState = '0';
    int n = 0;

    while(1){
        n = (n + 10) % 2;
        prevButtonState = ButtonState;
        ButtonState = buttonUpdate(dev_gpio);
        
        if(ButtonState != prevButtonState){
            if(n%2) score[0]++;
            else score[1]++;
        }

        FND(dev_fnd, score);
    }
}
