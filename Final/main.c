#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>

#define DRAW    2
#define WIN     1
#define LOSE    0

#define D1 0x01
#define D4 0x08

char seg_num[10] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xd8, 0x80, 0x90};
char seg_dnum[10] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x58, 0x00, 0x10};


char buttonUpdate(int dev, char button_state_now){
    char buff, prev = 'a', tmp;

    read(dev, &buff, 1);
    prev = tmp;
    tmp = buff;
    if (tmp != prev) button_state_now != button_state_now;

    return button_state_now;
}

int FND(int dev, int* score){
    unsigned short data[2];
    int n = 0;

    data[0] = (seg_num[score[0]] << 4) | D1;
    data[1] = (seg_num[score[1]] << 4) | D4;

    write(dev, &data[n], 2);
    n++;
    n = (n + 1) % 2;
}

int main(int argc, char **argv){
    int dev_gpio = open("/dev/gpio_driver",O_RDWR);
    int dev_fnd = open("/dev/fnd_driver",O_RDWR);
 
    if(dev_gpio == -1){
        printf("Opening dev_gpio not Possible!\n");
        return -1;
    }
    if(dev_fnd == -1){
        printf("Opening dev_fnd not Possible!\n");
        return -1;
    }
    printf("Opening main dev...!!!\n");

    int score[] = {0,0};
    char on = WIN, off = LOSE, al = DRAW;
    char button_state_now = 0;
    int delay_time = 1000000;
    int result = 0;
    do {
        printf("Present button State : %d",button_state_now);
        button_state_now = buttonUpdate(dev_gpio, button_state_now);
    } 
    while (!button_state_now);

    while(button_state_now){
        FND(dev_fnd, score);
        button_state_now = buttonUpdate(dev_gpio, button_state_now);

        switch (result) {
        case WIN:
            write(dev_gpio,&on, 1); 
            score[0]++;
            break;
        case LOSE:
            write(dev_gpio,&off, 1);
            score[1]++;
            break;
        default:
            break;
        }
        result = (result + 1)%2
        write(dev_gpio, &al, 1); 

        usleep(delay_time);
    }

    close(dev_gpio);
    close(dev_fnd);
    
    return 0;
}
