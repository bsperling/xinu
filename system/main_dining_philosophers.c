/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>


void philosopher(int ph_num);
void pick_up_forks(int ph_num);
void put_down_forks(int ph_num);
void eat(int ph_num);
void think(int ph_num);
void test(int ph_num);

# define N  5
# define LEFT (ph_num+N-1)%N
# define RIGHT (ph_num+1)%N
# define THINKING  0
# define HUNGRY  1
# define EATING  2

int state[N];
//int philosopher_num[N] = {0,1,2,3,4};
sid32 mutex;
sid32 sem[N];


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
     state[ph_num] = HUNGRY;
     kprintf("Philosopher %d is Hungry\n",ph_num+1);
     test(ph_num);
     signal(mutex);
     wait(sem[ph_num]);
}

void put_down_forks(int ph_num)
{
     wait(mutex);
     state[ph_num] = THINKING;
	 kprintf("Philosopher %d puts forks %d and %d down\n",ph_num+1,LEFT+1,ph_num+1);     
	 test(LEFT);
	 test(RIGHT);
     signal(mutex);
}

void think(int ph_num)
{
	kprintf("Philosopher %d is Thinking\n",ph_num+1);
	//sleep(10);
}

void eat(int ph_num)
{
	kprintf("Philosopher %d is Eating\n",ph_num+1);
	//sleep(10);
}


void test(int ph_num)
{
    if (state[ph_num] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING){
        state[ph_num] = EATING;
        sleep(2);
            kprintf("Philosopher %d takes fork %d and %d\n",ph_num+1,LEFT+1,ph_num+1);
            kprintf("Philosopher %d is Eating\n",ph_num+1);
        signal(sem[ph_num]);
    }
}
 

int main(int argc, char **argv)
{
	mutex = semcreate(1);
	int ph_num = 0;
	for(ph_num; ph_num<N; ph_num++)
	{
		sem[ph_num] = semcreate(0);
	}

	ph_num = 0;
	for(ph_num; ph_num<N; ph_num++)
	{
		resume(create(philosopher, 4096, 50, "Philosopher", 1, ph_num));
	}


	while(TRUE) {
		//Do nothing
	}
	

	return OK;
}
