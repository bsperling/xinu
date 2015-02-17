/*  main.c  - main */
//Bryce Sperling

#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>


void philosopher(int ph_num);
void pick_up_forks(int ph_num);
void put_down_forks(int ph_num);
void eat(int ph_num);
void think(int ph_num);
void test(int ph_num);

# define NUM_PHILOSOPHERS  5

# define LEFT (ph_num+NUM_PHILOSOPHERS-1)%NUM_PHILOSOPHERS
# define RIGHT (ph_num+1)%NUM_PHILOSOPHERS

# define THINKING 0
# define HUNGRY	1
# define EATING	2

int state[NUM_PHILOSOPHERS];
sid32 mutex;
sid32 sem[NUM_PHILOSOPHERS];


int main(int argc, char **argv)
{
	mutex = semcreate(1);
	int i = 0;
	for(i; i<NUM_PHILOSOPHERS; i++)
	{
		sem[i] = semcreate(0);
	}

	intmask mask = disable();

	int ph_num = 0;
	for(ph_num; ph_num<NUM_PHILOSOPHERS; ph_num++) //ERR?
	{
		resume(create(philosopher, 4096, 50, "Philosopher", 1, ph_num));
	}

	restore(mask);

	while(TRUE) {
		//Do nothing
	}
	

	return OK;
}


void philosopher(int ph_num) 
{
	while(TRUE)
	{
		think(ph_num); 
		pick_up_forks(ph_num);
		eat(ph_num);
		put_down_forks(ph_num);
	}
}

void pick_up_forks(int ph_num)
{
    wait(mutex);
    kprintf("Philosopher %d is blocking the waiter\n",ph_num);

   		state[ph_num] = HUNGRY;
    	kprintf("Philosopher %d is Hungry\n",ph_num);
     
    test(ph_num);

    kprintf("Philosopher %d stopped blocking the waiter\n",ph_num);
    signal(mutex);

    if (state[ph_num] != EATING) {
    	kprintf("Philosopher %d is waiting for %d and/or %d to stop eating\n",ph_num, LEFT, RIGHT);
    }
    wait(sem[ph_num]);
}

void put_down_forks(int ph_num)
{
	wait(mutex);
	kprintf("Philosopher %d is blocking the waiter\n",ph_num);

		state[ph_num] = THINKING;
		kprintf("Philosopher %d puts forks %d and %d down\n",ph_num,ph_num,RIGHT);

		test(RIGHT);
		test(LEFT);

		kprintf("Philosopher %d stopped blocking the waiter\n",ph_num);
	signal(mutex);
}

void test(int ph_num)
{
    if (state[ph_num] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING)
    {

        kprintf("Philosopher %d takes forks %d and %d\n",ph_num,ph_num,RIGHT);

        state[ph_num] = EATING;
        signal(sem[ph_num]);
    }
}

void think(int ph_num)
{

		kprintf("Philosopher %d is Thinking\n",ph_num);
		sleepms(500);

}

void eat(int ph_num)
{

		kprintf("Philosopher %d is Eating\n",ph_num);
		//sleepms(500);
		kprintf("Philosopher %d is Finished Eeating \n",ph_num);

}
