/*  main.c  - main */
//Bryce Sperling

#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>


syscall sendMsg(pid32 pid, umsg32 msg);
//uint32 sendMsgs(pid32 pid, umsg32* msgs, uint32 msg_count);
umsg32 receiveMsg(void);
//syscall receiveMsgs(umsg32* msgs, uint32 msgs_count);

//uint32 sendnMsg(uint32 pid_count, pid32* pids, umsg32 msg);

void pushItem (pid32 pid, umsg32 msg);
umsg32 popItem (pid32 pid);

umsg32 msgBuffers[8][10] = {{0}}; //8 processes with queues for msgs
uint32 pointers[8][2] = {{0}}; //8 processes with head and tail

# define HEAD pointers[pid][0]
# define TAIL pointers[pid][1]



int main(int argc, char **argv)
{

	pid32 receiver1 = create(receiveMsg, 4096, 50, "Receiver2", 0, 0);
	pid32 receiver2 = create(receiveMsg, 4096, 50, "Receiver1", 0, 0);
	
	kprintf("%d\n", receiver1);
	kprintf("%d\n", receiver2);
	resume(receiver1);
	resume(receiver2);

	pid32 sender1 = create(sendMsg, 4096, 50, "Sender1", 2, receiver1, 1337);

	umsg32 message1 = resume(sender1);

	kprintf("Receiver 1 priority: %d", message1);

	umsg32 message = sendMsg(receiver2, 7777);

	kprintf("Receier 2: %d", message);

	//resume(create(philosopher, 4096, 50, "Philosopher", 1, ph_num));

	while(TRUE) {
		//Do nothing
	}
	

	return OK;
}

void pushItem (pid32 pid, umsg32 msg) {
	msgBuffers[pid][TAIL] = msg;
	TAIL++;
	kprintf("Push: %d\n", msg);
	TAIL = TAIL % 100;
}

umsg32 popItem (pid32 pid) {
	uint32 tempPop = msgBuffers[pid][HEAD];
	msgBuffers[pid][HEAD] = 0; //clear contents of location
	HEAD++;
	HEAD = HEAD % 100;
	kprintf("Pop: %d\n", tempPop);
	return tempPop;
}

/*------------------------------------------------------------------------
 *  sendMsg  -  pass a message to a process and start recipient if waiting
 *------------------------------------------------------------------------
 */
syscall	sendMsg(
	  pid32		pid,		/* ID of recipient process	*/
	  umsg32	msg		/* contents of message		*/
	)
{
	intmask	mask;			/* saved interrupt mask		*/
	struct	procent *prptr;		/* ptr to process' table entry	*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}

	prptr = &proctab[pid];
	if (prptr->prstate == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	if (msgBuffers[pid][HEAD] != 0) //Queue of receiver is full
	{
		restore(mask);
		return SYSERR;
	}
	else
	{
		pushItem(pid, msg);
	}

	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->prstate == PR_RECV) {
		ready(pid, RESCHED_YES);
	} else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid, RESCHED_YES);
	}
	restore(mask);		/* restore interrupts */
	return OK;
}


/*------------------------------------------------------------------------
 *  receiveMsg  -  wait for a message and return the message to the caller
 *------------------------------------------------------------------------
 */
umsg32	receiveMsg(void)
{
	intmask	mask;			/* saved interrupt mask		*/
	struct	procent *prptr;		/* ptr to process' table entry	*/
	umsg32	msg;			/* message to return		*/

	pid32 pid = getpid(); //Get current process pid

	mask = disable();
	prptr = &proctab[currpid];
	if (HEAD == TAIL) //Queue is empty, No message waiting
	{
		prptr->prstate = PR_RECV;
		resched();		/* block until message arrives	*/
	}
	msg = popItem (pid);	/* retrieve message		*/
	restore(mask);
	return msg;
}


//uint32 sendMsgs(pid32 pid, umsg32* msgs, uint32 msg_count);

//syscall receiveMsgs(umsg32* msgs, uint32 msgs_count);

//uint32 sendnMsg(uint32 pid_count, pid32* pids, umsg32 msg);


