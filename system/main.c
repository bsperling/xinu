/* Bryce Sperling */
/*  main.c  - main */
/* Lab 2 */

#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>


syscall sendMsg(pid32 pid, umsg32 msg);
uint32 sendMsgs(pid32 pid, umsg32* msgs, uint32 msg_count);
umsg32 receiveMsg(void);
//syscall receiveMsgs(umsg32* msgs, uint32 msgs_count);

uint32 sendnMsg(uint32 pid_count, pid32* pids, umsg32 msg);

void pushItem (pid32 pid, umsg32 msg);
umsg32 popItem (pid32 pid);
void testSender (void);
void testReceiver (void);


umsg32 msgBuffers[10][10] = {{0}}; //10 processes with queues for msgs
uint32 pointers[10][2] = {{0}}; //10 processes with head and tail
pid32 receiver1, receiver2, receiver3, receiver4, receiver5;

# define HEAD pointers[pid][0]
# define TAIL pointers[pid][1]



int main(int argc, char **argv)
{

	receiver1 = create(testReceiver, 4096, 50, "Receiver1", 0, 0);
	receiver2 = create(testReceiver, 4096, 50, "Receiver2", 0, 0);
	receiver3 = create(testReceiver, 4096, 50, "Receiver3", 0, 0);

	kprintf("%d\n", receiver1);
	kprintf("%d\n", receiver2);
	kprintf("%d\n", receiver3);
	resume(receiver1);
	resume(receiver2);
	resume(receiver3);

	resume(create(testSender, 4096, 50, "Sender1", 0, 0));
	//resume(create(testSender, 4096, 50, "Sender2", 0, 0));

	while(TRUE) {
		//Do nothing
	}
	

	return OK;
}

void testSender(void) {
	umsg32 sentMessages = 0;
	while (TRUE) {

		/*Testing single sendMsg() */
/*		if(sendMsg(receiver1, sentMessages++) == SYSERR)
		{
			kprintf("Message not sent.\n");
		}*/

		/*Testing send multiple messages sendMsgs() */
/*		umsg32 msgs[8] = {1, 1, 2, 3, 5, 8, 13, 21};
		if(sendMsgs(receiver2, msgs, 8) == SYSERR)
		{
			kprintf("Multiple message send failed.\n");
		}*/

		/*Testing send a message to multiple receivers */
		pid32 receivers[3] = {receiver1, receiver2, receiver3};
		if (sendnMsg(3, receivers, sentMessages++) != 3)
		{
			kprintf("Multiple receiver send failed.\n");
		}

	}
	
}

void testReceiver(void) {
	while (TRUE) {
		umsg32 msg = receiveMsg();
		if (msg == SYSERR)
		{
			kprintf("Message not received.\n");
		}
	}	
}

void pushItem (pid32 pid, umsg32 msg) {
	msgBuffers[pid][TAIL] = msg;
	TAIL++;
	TAIL = TAIL % 8;
}

umsg32 popItem (pid32 pid) {
	uint32 tempPop = msgBuffers[pid][HEAD];
	msgBuffers[pid][HEAD] = 0; //clear contents of location
	HEAD++;
	HEAD = HEAD % 8;
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

	if (msgBuffers[pid][TAIL] != 0) //Queue of receiver is full
	{
		restore(mask);
		return SYSERR;
	}
	else
	{
		pushItem(pid, msg);
		kprintf("Message ""%d"" sent to process %d\n", msg, pid);
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
	kprintf("Message ""%d"" received by process %d.\n", msg, pid);
	restore(mask);
	return msg;
}

/*------------------------------------------------------------------------
 *  sendMsgs  -  pass multiple messages to a process and start recipient if waiting
 *------------------------------------------------------------------------
 */
uint32 sendMsgs(
	  pid32		pid,		/* ID of recipient process	*/
	  umsg32*	msgs,		/* contents of message		*/
	  uint32	msg_count	/* number of messages in group */
	)
{
	uint32 count = 0; //count of how many messages are sent correctly
	uint32 ret;
	uint32 i;
	for (i=0;i<msg_count;i++)
	{
		ret = sendMsg(pid,msgs[i]);
		if (ret == OK)
			count++;
	}

	if (count > 0)  //return number of correctly sent messages
	{
		kprintf("%d of %d messages sent correctly.\n", count, msg_count);
		return count;
	}	
	else
		return SYSERR;
}


//syscall receiveMsgs(umsg32* msgs, uint32 msgs_count);


/*------------------------------------------------------------------------
 *  sendnMsg  -  pass message to multiple receiver processes
 *------------------------------------------------------------------------
 */
uint32 sendnMsg(
	  uint32	pid_count,	/* number of recipient processes  */
	  pid32*	pids,		/* processes to send messages to  */
	  umsg32	msg			/* message to send  */
	)
{
	uint32 count = 0; //count of how many messages are sent correctly
	uint32 ret;
	uint32 i;
	for (i=0;i<pid_count;i++)
	{
		ret = sendMsg(pids[i],msg);
		if (ret == OK)
			count++;
	}

	if (count > 0)  //return number of correctly sent messages
	{
		kprintf("Message sent correctly to %d of %d processes.\n", count, pid_count);
		return count;
	}
	else
		return SYSERR;
}



