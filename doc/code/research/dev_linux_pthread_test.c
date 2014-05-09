#include "stdio.h"
#include "string.h"
#include "pthread.h"

int exitflag;
int c1 = 0,c2 = 0;

void Sleep(int milisec) {
    usleep(milisec * 1000);
}

void * fun1(void * arg) {
    while (!exitflag) {
        Sleep(1000);
        printf("fun1:%d\n",c1);
        c1++;
    }
}
void * fun2(void * arg) {
    while (!exitflag) {
        Sleep(3000);
        printf("fun2:%d\n",c2);
        c2++;
    }
}
int main() {
    char str[0xff];
    pthread_t id1,id2;
    pthread_attr_t attr;
    str[0] = 0;
    exitflag = 0x00;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    pthread_create(&id1,&attr,fun1,NULL);
    pthread_create(&id2,&attr,fun2,NULL);
    while (!exitflag) {
        fgets(str, 0xff, stdin);
        str[strlen(str) - 1] = 0;
        if (!strcmp(str, "exit")) exitflag = 0x01;
    };
    pthread_attr_destroy(&attr);
    return 0;
}
