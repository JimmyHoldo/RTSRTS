#include "TinyTimber.h"
#include "sciTinyTimber.h"
#include "canTinyTimber.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    Object super;
    int count;
    char c;
	int tone;
	int volume;
	int muteFlag;
	int deadlineFlag;
	char buf[1000];
} App;

typedef struct {
	Object super;
	int background_loop_range;
	int deadlineFlag;
} Background;

int bJohn[32] = {0,2,4,0,0,2,4,0,4,5,7,4,5,7,7,9,7,5,4,0,7,9,7,5,4,0,0,-5,0,0,-5,0};
int key[11] = {-5,-4,-3,-2,-1,0,1,2,3,4,5};
int period[25] = {2025,1911,1804,1703,1607,1517,1432,1351,1276,1204,1136,1073,1012,956,902,851,804,758,716,676,638,602,568,536,506};
int test[3] = {500,650,931};

App app = { initObject(), 0, 'X', 0,0,0,1 };
Background background = { initObject(), 1000,1 };

void reader(App*, int);
void receiver(App*, int);

#define DAC_REGISTER (*(char *)0x4000741C)

Serial sci0 = initSerial(SCI_PORT0, &app, reader);

Can can0 = initCan(CAN_PORT0, &app, receiver);

void printPeriods(int k){
	char str[1000];
	for(int i = 0; i < 32; i++){
		sprintf(str, "Tone number %d: %d \n", i, period[bJohn[i]+(10+k)]);
		SCI_WRITE(&sci0, str);
	}
}

void checkToneTime(App *self){
	for(int i = 0; i < 1000; i++){
		DAC_REGISTER = self->volume;
	}
}

void checkToneBack(Background *self){
	for(int i = 0; i < 13500; i++){
		
	}
}

void Controller(App *self, int arg){
	Time Start, Diff, sum;
	Time m = 0;
	Time times[1000];
	int a;
	
	for(int i = 0; i < 500; i++) {
		Start = CURRENT_OFFSET();
		checkToneBack(&background);
		Diff = CURRENT_OFFSET() - Start;
		times[i] = Diff;
	}
	for(int j = 0; j < 500; j++){
		sum += times[j];
		if(times[j] > m)
			m = times[j];
	}
	a = USEC_OF(sum)/500;
	char str[1000];
	sprintf(str, "Sum: %ld \nMax: %ld \nAvg: %d",USEC_OF(sum), USEC_OF(m), a );
	SCI_WRITE(&sci0, str);
}

void backgoundLoopTask(Background *self, int arg) {
	for(int i = 0; i < self->background_loop_range; i++){
		
	}
	if(self->deadlineFlag)
		SEND(USEC(1300),USEC(1300),self,backgoundLoopTask, 0);
	else
		AFTER(USEC(1300),self,backgoundLoopTask, 0);
		
}

void writeOneToDAC(App *self, int t);

void writeZeroToDAC(App *self, int t) {
	DAC_REGISTER = 0x0;
	if(self->deadlineFlag)
		SEND(USEC(test[self->tone]),USEC(100), self, writeOneToDAC, t);
	else
		AFTER(USEC(test[self->tone]), self, writeOneToDAC, t);}

void writeOneToDAC(App *self, int t) {
	if(!self->muteFlag){
		DAC_REGISTER = self->volume;
	}else{
		DAC_REGISTER = 0x0;
	}
	if(self->deadlineFlag)
		SEND(USEC(test[self->tone]),USEC(100), self, writeZeroToDAC, t);
	else
		AFTER(USEC(test[self->tone]), self, writeZeroToDAC, t);

}

void toneGenerator(App *self, int t) {
	writeOneToDAC(self, t);
}

void receiver(App *self, int unused) {
    CANMsg msg;
    CAN_RECEIVE(&can0, &msg);
    SCI_WRITE(&sci0, "Can msg received: ");
    SCI_WRITE(&sci0, msg.buff);
}

void mute(App *self) {
	if(!self->muteFlag){
		self->muteFlag = 1;
	}else {
		self->muteFlag = 0;
	}
}

void changeLoad(Background *self, int c){
	if(c == 'u'){
		self->background_loop_range += 500;			
	}
	else{
		self->background_loop_range -= 500;
	}
	char str[1000];
	sprintf(str, "%d \n", self->background_loop_range);
	SCI_WRITE(&sci0, str);
} 
void toggleDeadline(Background *self){
	if(self->deadlineFlag == 0){
		self->deadlineFlag = 1;
		self->deadlineFlag = 1;
	}else{
		self->deadlineFlag = 0;
		self->deadlineFlag = 0;
	}
}

void reader(App *self, int c) {
		
	SCI_WRITE(&sci0, "Rcv: \'");
	SCI_WRITECHAR(&sci0, c);
	SCI_WRITE(&sci0, "\'\n");
	if(c == 'm'){
		mute(self);
	}else if(c == '+') {
		if(self->volume < 20)
			self->volume++;
	}else if(c == '-') {
		if(self->volume > 1 )
			self->volume--;
	}else if(c == 'u' || c == 'j') {
		changeLoad(&background, c);
	}else if(c == 'd') {
		toggleDeadline(&background);
	}else if(c >= '1' && c <='3') {
		self->tone = c-'1';
	}
	
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
	self->volume = 0x5;
	Controller(self,0);
	//ASYNC(self, toneGenerator, 0);
	//ASYNC(&background, backgoundLoopTask, 0);
}

int main() {
    INSTALL(&sci0, sci_interrupt, SCI_IRQ0);
	INSTALL(&can0, can_interrupt, CAN_IRQ0);
    TINYTIMBER(&app, startApp, 0);
    return 0;
}
