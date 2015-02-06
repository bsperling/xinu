/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>

void myprocess(int a) {
	
	kprintf("Hello world %d\n", a);
	
}

int main(int argc, char **argv)
{
	
	uint32 retval;

	resume(create(shell, 8192, 50, "shell", 1, CONSOLE));

	/* Wait for shell to exit and recreate it */

	resume(create(myprocess, 4096, 50, "helloworld", 1, 1));
	while(TRUE) {
		// Do nothing
	}
	
	recvclr();
	//while (TRUE) {
		//retval = receive();
		//kprintf("\n\n\rMain process recreating shell\n\n\r");
		//resume(create(shell, 4096, 1, "shell", 1, CONSOLE));
	//}
	while (1);

	return OK;
}
