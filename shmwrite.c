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

void initial_writer()
{
	key_t msg_key,shm_key;

	/* generate share keys */
	shm_key = ftok(fileshm, PROJID);
	msg_key = ftok(filemsg, PROJID);

	if(shm_key == -1 || msg_key == -1)
	{
		perror ("ftok");
		exit(0);
	}

	/* create shared memory tunnel */
	shm_id = shmget(shm_key, MEMORYSIZE * (sizeof(Data)), IPC_CREAT | IPC_EXCL | 0600); 

	if(shm_id == -1)
	{
		perror("shmget error");
		exit(0);
	}

	/* create message queue */
	msg_id = msgget(msg_key, IPC_CREAT | IPC_EXCL | 0600); 

	if(msg_id == -1)
	{
		perror("msgget error");
		exit(0);
	}
}

void writer_function(uint32_t head, uint32_t tail, Data *p_map)
{
	/* write data to ring buffer */
	FILE *pFile;
	pFile = fopen ("memtrace_gem","r+");

	while( fscanf(pFile,"%u %x %u", &(p_map+tail)->cpuid, &(p_map+tail)->addr, &(p_map+tail)->index) == 3 )
	{
		/* check buffer is full or not */
		while (1)
		{
			head = (p_map + MEMORYSIZE - 2)->index;
			if( (tail + 1 + OFFSET) > MEMORYSIZE - 2 )
			{
				if( head > OFFSET) 
					break;
			}
			else
			{
				if( tail == head || head - tail < OFFSET )
					break;
			}	
		}
 
		/* move tail pointer to next address */
		tail = (tail + 1) % (MEMORYSIZE - 2);
		(p_map + MEMORYSIZE - 1)->index = tail;
	}
	fclose(pFile);
}

int main(int argc, char** argv)
{
	signal(SIGINT, terminate_handler);
	signal(SIGTERM, terminate_handler);

	initial_writer();

	uint32_t head, tail;
	Data *p_map;

	tail = 0;
	head = 0;

	/* initial share memory pointers */
	p_map = (Data*)shmat( shm_id, NULL, 0);
	(p_map + MEMORYSIZE - 2)->cpuid = head;
	(p_map + MEMORYSIZE - 1)->cpuid = tail;

	printf("write start\n");
	writer_function( head, tail, p_map);
	printf("write finish\n");

	/* send terminate signal to reader */
	Signal signal;
	signal.type = TERMINATE;	
	signal.signal = -1;
	if(msgsnd(msg_id, &signal, sizeof(signal), 0) < 0)
	{
		perror("msgsnd");
		return(1);
	}
	printf("transfer finished\n");

	/* receive finish signal from reader */
	msgrcv(msg_id, &signal, sizeof(signal), FINISH, 0);
	printf("finish signal receive\n");

	return 0 ;
}
