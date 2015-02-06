/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>


int myQueue[100];
int head = 0;
int tail = 0;
int data = 1;
sid32 psem;
sid32 csem;

void producer () {
	while(TRUE)
	{
		wait(psem);
		//pushItem();
		myQueue[tail] = data;
		tail++;
		tail = tail % 100;
		data++;
		kprintf("Produced: %d\n", data-1);
		signal(csem);
		
	}
}

/*void pushItem () {
	myQueue[tail] = data;
	tail++;
	tail = tail % 100;
}*/

void consumer () {
	while(TRUE)
	{
		wait(csem);
		int tempPop = myQueue[head];
		myQueue[head] = 0; //clear contents of location
		head++;
		head = head % 100;
		kprintf("Consumed: %d\n", tempPop);
		signal(psem);
		
	}
}

/*int popItem () {
	int tempPop = myQueue[head];
	myQueue[head] = 0; //clear contents of location
	head++;
	head = head % 100;
	return tempPop;
}*/

int main(int argc, char **argv)
{
	csem = semcreate(0);
	psem = semcreate(100);

	resume(create(producer, 4096, 50, "Producer", 1, 0));
	resume(create(consumer, 4096, 50, "Consumer", 1, 0));


	while(TRUE) {
		//Do nothing
	}
	

	return OK;
}
