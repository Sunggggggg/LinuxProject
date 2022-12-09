#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>

#define DRAW    2
#define WIN     1
#define LOSE    0

char seg_num[10] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xd8, 0x80, 0x90};
char seg_dnum[10] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x58, 0x00, 0x10};

#define D1 0x01
#define D2 0x02
#define D3 0x04
#define D4 0x08

int FND(int* data) {
    int n = 0;
    int delay_time = 30;

    data[0] = (seg_num[1] << 4) | D1;
    data[1] = (seg_num[2] << 4) | D2;
    data[2] = (seg_num[3] << 4) | D3;
    data[3] = (seg_num[4] << 4) | D4;

    while(n < 4){
        write(dev,&data[n++],2);
        usleep(delay_time);
    }
}


int main(int argc, char **argv){
    

    int dev = open("/dev/cham",O_RDWR);
    if(dev == -1){
        printf("Opening main dev FAIL...!!!\n");
        return -1;
    }
    printf("Opening main dev...!!!\n");

    // Set FND 0:0
    int score[] = {0,0};
    // Motor Set
    Motor(0);

    while(1){
        // Face Detecting...

        // FND ON
        FND(score);

        if( /*한쪽이 3번 먼저 이기면*/){
            printf("Game Set...!!!\n");
            break;
        }

        if( /*button이 눌리면*/){
            /* 참참참 부저 울리기*/
            chamchamcham();

            com_dir = /*컴퓨터 좌우 방향 선택*/;
            Motor(com_dir);
            if( /* 얼굴이 인식 되어 있다면 */){
                user_dir = /* 이용자 좌우 방향 읽기*/;

                result = Compare();


                printf("Compter : %d \t User : %d \t Result : %d", com_dir, user_dir, result);
            }
            else printf("Can't Detect face...\n");
        }
        
        switch (result) {
        case WIN:
            /* 파란색 LED*/ // usleep(delay_time);?
            /* 승리 부저*/
            score[0]++;
            break;
        case LOSE:
            /* 빨간색 LED*/
            /* 패배 부저*/
            score[1]++;
            break;
        case DRAW:
            /* */
            printf("Draw Game\n");
        default:
            break;
        }

        /* 초기화 상태로 만들기*/
        Motor(0);
        LED(0,0);
    }

    write(dev, 0x0000, 2);
    close(dev);

    return 0;
}
