#include "TinyTimber.h"
#include "sciTinyTimber.h"
#include "canTinyTimber.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    Object super;
    int count;
    char c;
	int volume;
	char buf[1000];
} App;

int bJohn[32] = {0,2,4,0,0,2,4,0,4,5,7,4,5,7,7,9,7,5,4,0,7,9,7,5,4,0,0,-5,0,0,-5,0};
int key[11] = {-5,-4,-3,-2,-1,0,1,2,3,4,5};
int period[25] = {2025,1911,1804,1703,1607,1517,1432,1351,1276,1204,1136,1073,1012,956,902,851,804,758,716,676,638,602,568,536,506};

App app = { initObject(), 0, 'X' };

void reader(App*, int);
void receiver(App*, int);

#define DAC_REGISTER (*(volatile uint8_t*)0x4000741C)
Serial sci0 = initSerial(SCI_PORT0, &app, reader);

Can can0 = initCan(CAN_PORT0, &app, receiver);

void printPeriods(int k){
	char str[1000];
	for(int i = 0; i < 32; i++){
		sprintf(str, "Tone number %d: %d \n", i, period[bJohn[i]+(10+k)]);
		SCI_WRITE(&sci0, str);
	}
}

void receiver(App *self, int unused) {
    CANMsg msg;
    CAN_RECEIVE(&can0, &msg);
    SCI_WRITE(&sci0, "Can msg received: ");
    SCI_WRITE(&sci0, msg.buff);
}

void reader(App *self, int c) {
	
	int num;
	
	SCI_WRITE(&sci0, "Rcv: \'");
	SCI_WRITECHAR(&sci0, c);
	SCI_WRITE(&sci0, "\'\n");
	if(c == 'm'){
		
	}	
	if(c == 'e' ){
		self->buf[self->count] = '\0';
		num = atoi(self->buf);
		self->count = 0;
		if(num >= 1 && num <= 20){
		}
	}
	else if(c == '-' || (c >= '0' && c <= '9')){
		self->buf[self->count++] = c;
	}
	/*int num;
	char str[1000];
	
	SCI_WRITE(&sci0, "Rcv: \'");
	SCI_WRITECHAR(&sci0, c);
	SCI_WRITE(&sci0, "\'\n");
		
	if(c == 'e'){
		self->buf[self->count] = '\0';
		num = atoi(self->buf);
		self->myNum = num + self->myNum;
		sprintf(str, "The entered number is %d \nThe running sum is %d \n", num, self->myNum);
		SCI_WRITE(&sci0, str);
		self->count = 0;
	}
	else if(c == 'F'){
		self->myNum = 0;
		sprintf(str, "The running sum is %d \n", self->myNum);
		SCI_WRITE(&sci0, str);
	}
	else if(c == '-' || (c >= '0' && c <= '9')){
		self->buf[self->count++] = c;
	}*/
}

void startApp(App *self, int arg) {
    CANMsg msg;
	self->count = 0;
    CAN_INIT(&can0);
    SCI_INIT(&sci0);
    SCI_WRITE(&sci0, "Hello, hello...\n");

    msg.msgId = 1;
    msg.nodeId = 1;
    msg.length = 6;
    msg.buff[0] = 'H';
    msg.buff[1] = 'e';
    msg.buff[2] = 'l';
    msg.buff[3] = 'l';
    msg.buff[4] = 'o';
    msg.buff[5] = 0;
    CAN_SEND(&can0, &msg);
	printPeriods(0);
}

int main() {
    INSTALL(&sci0, sci_interrupt, SCI_IRQ0);
	INSTALL(&can0, can_interrupt, CAN_IRQ0);
    TINYTIMBER(&app, startApp, 0);
    return 0;
}
