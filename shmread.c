#include <stdio.h>
#include <string.h>
#include "sharemem.h"

int shm_id,msg_id;

void terminate_handler()
{
	msgctl(msg_id,IPC_RMID,NULL);
	shmctl(shm_id,IPC_RMID,NULL);
	exit(0);
}


void initial_reader()
{
	key_t msg_key,shm_key;

	/* generate share keys */
	shm_key = ftok(fileshm,PROJID);
	msg_key = ftok(filemsg, PROJID);

	if(shm_key == -1 || msg_key == -1)
	{
		perror("ftok");
		exit(0);
	}

	/* get share memory tunnel */
	shm_id = shmget(shm_key, 0, 0);  

	while(shm_id == -1)
	{
		sleep(1);
		shm_id = shmget(shm_key, 0, 0);
	}

	/* get message queue */
	msg_id = msgget(msg_key, 0);

	while(msg_id == -1)
	{
		sleep(1);
		msg_id = msgget(msg_key, 0);
	}

}

void reader_function(uint32_t head, uint32_t tail, Data *p_map)
{
	/* read data from ring buffer */
	int terminate = 0;

	Signal signal;
	
	while(!terminate || head != tail)
	{
		/* if buffer is empty then check terminate signal */
		while( head == tail )
		{
			tail = (p_map + MEMORYSIZE - 1)->index;
			int ret = msgrcv(msg_id, &signal, sizeof(signal), TERMINATE, IPC_NOWAIT);
			if( ret != -1)
			{
				terminate = 1;
				printf("terminate signal receive\n");
				break;
			}
		}
		
		/* if terminate signal is received and buffer is empty then */
		/* end the read loop */
		if(terminate == 1 && head == tail)
			break;

		/* read the data from share memory pointer */
		printf("%u 0x%x %u \n", (*(p_map + head)).cpuid, (*(p_map + head)).addr, (*(p_map + head)).index);

		/* move head pointer to next address */
		head = (head + 1)%(MEMORYSIZE - 2);
		(p_map + MEMORYSIZE - 2)->index = head;
	}
}

int main(int argc, char** argv)
{
	signal(SIGINT, terminate_handler);
	signal(SIGTERM, terminate_handler);

	initial_reader();
		
	Data *p_map;
	uint32_t head, tail;

	/* read share memory pointers */
	p_map = (Data *)shmat(shm_id, NULL, 0);
	head = (p_map + MEMORYSIZE - 2)->index;
	tail = (p_map + MEMORYSIZE - 1)->index;

	reader_function( head, tail, p_map);

	/* send finish signal to writer */
	Signal signalbuf;
	signalbuf.type = FINISH;	
	signalbuf.signal = -1;
	if(msgsnd(msg_id, &signalbuf, sizeof(signalbuf), 0) < 0)
	{
		perror("msgsnd");
		return(1);
	}

	/* remove the ipc id */
	msgctl(msg_id, IPC_RMID, NULL);
	shmctl(shm_id, IPC_RMID, NULL);

	return 0 ;
}
