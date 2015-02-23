/* Bryce Sperling */
/*  main.c  - main */
/* Lab 2 */

#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>


syscall sendMsg(pid32 pid, umsg32 msg);
uint32 sendMsgs(pid32 pid, umsg32* msgs, uint32 msg_count);
umsg32 receiveMsg(void);
syscall receiveMsgs(umsg32* msgs, uint32 msgs_count);

uint32 sendnMsg(uint32 pid_count, pid32* pids, umsg32 msg);

void pushItem (pid32 pid, umsg32 msg);
umsg32 popItem (pid32 pid);
void testSender (void);
void testReceiverSingle (void);
void testReceiverMultiple (uint32 msg_count);

# define QUEUE_SIZE 10
# define HEAD pointers[pid][0]
# define TAIL pointers[pid][1]
# define SIZE (QUEUE_SIZE - (pointers[pid][0] - pointers[pid][1])) % QUEUE_SIZE

umsg32 msgBuffers[10][QUEUE_SIZE] = {{0}}; //10 processes with queues for msgs
uint32 pointers[10][2] = {{0}}; //10 processes with head and tail
pid32 receiver1, receiver2, receiver3, receiver4, receiver5;




int main(int argc, char **argv)
{

	receiver1 = create(testReceiverMultiple, 4096, 50, "Receiver1", 1, 2);
	receiver2 = create(testReceiverSingle, 4096, 50, "Receiver2", 0, 0);
	receiver3 = create(testReceiverMultiple, 4096, 50, "Receiver3", 1, 3);

	//kprintf("%d\n", receiver1);
	//kprintf("%d\n", receiver2);
	//kprintf("%d\n", receiver3);
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
	umsg32 sentSingleMessages = 0;
	umsg32 sentMultiReceiverMessages = 1000;
	//while (TRUE) {
	uint32 testCount;
	for(testCount = 0; testCount<3; testCount++) {

		/* Testing single sendMsg() {1} */
		if(sendMsg(receiver1, sentSingleMessages++) == SYSERR)
		{
			kprintf("Message not sent.\n");
		}

		/* Testing send multiple messages sendMsgs() {3} */
		umsg32 msgs[8] = {1, 1, 2, 3, 5, 8, 13, 21};
		//umsg32 msgs[4] = {1, 3, 5, 7};
		if(sendMsgs(receiver3, msgs, 8) == SYSERR)
		{
			kprintf("Multiple message send failed.\n");
		}

		/* Testing send a message to multiple receivers {1,2,3} */
		pid32 receivers[3] = {receiver1, receiver2, receiver3};
		if (sendnMsg(3, receivers, sentMultiReceiverMessages++) != 3)
		{
			kprintf("Multiple receiver send failed.\n");
		}
	}
	
}

/* Tests receiving individual messages */
void testReceiverSingle(void) {
	while (TRUE) {
	//uint32 testCount;
	//for(testCount = 0; testCount<1; testCount++) {
		umsg32 msg = receiveMsg();
		if (msg == SYSERR)
		{
			kprintf("Message not received.\n");
		}
	}	
}

/*Tests receiving multiple messages at a time */
void testReceiverMultiple(uint32 msg_count) {
	while (TRUE) {
	//uint32 testCount;
	//for(testCount = 0; testCount<1; testCount++) {
		umsg32 msgs[msg_count];
		if (receiveMsgs(msgs, msg_count) == SYSERR) //Shouldn't ever happen
		{
			kprintf("%d messages not received.\n", msg_count);
		}
	}	
}

/* Tests receiving multiple messages at a time sent via different means*/
/*void testReceiver3(void) {
	while (TRUE) {
		umsg32 msgs = receiveMsg();
		if (msg == SYSERR)
		{
			kprintf("Message not received.\n");
		}
	}	
}*/

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
		kprintf("BAD PID FOR PROCESS %d\t", pid);
		return SYSERR;
	}

	prptr = &proctab[pid];
	if (prptr->prstate == PR_FREE) {
		restore(mask);
		kprintf("FREE PROCESS\t");
		return SYSERR;
	}

	if (msgBuffers[pid][TAIL] != 0) //Queue of receiver is full
	{
		restore(mask);
		kprintf("FULL QUEUE\t");
		return SYSERR;
	}
	else
	{
		pushItem(pid, msg);
		kprintf("Message ""%d"" sent to process %d.\n", msg, pid);
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


/*------------------------------------------------------------------------
 *  receiveMsgs  -  wait then receive messages when queue reaches a certain size
 *------------------------------------------------------------------------
 */
syscall	receiveMsgs(
	  umsg32*	msgs,		/* ID of recipient process	*/
	  uint32	msg_count	/* contents of message		*/
	)
{
	intmask	mask;			/* saved interrupt mask		*/
	struct	procent *prptr;	/* ptr to process' table entry	*/

	pid32 pid = getpid(); //Get current process pid
	mask = disable();
	prptr = &proctab[currpid];


	if (msg_count > 10)	//Check if msg_count is larger than the queue size
		return SYSERR;
	else if (HEAD == TAIL) //Queue is empty, No message waiting
	{
		prptr->prstate = PR_RECV;
		resched();		/* block until messages arrives	*/
	}

	while (SIZE < msg_count)
	{
		prptr->prstate = PR_RECV;
		resched();		/* block until messages arrives	*/
	}


	int i;
	for(i=0;i<msg_count;i++)	/* retrieve messages in order  */
	{
		msgs[i] = receiveMsg();
	}

	kprintf("%d messages received by process %d.\n", msg_count, pid);

	restore(mask);
	return OK;
}
