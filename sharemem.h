#ifndef SHAREMEM
#define SHAREMEM

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdint.h>
#include <stdlib.h>

#define PROJID 'B'
#define TERMINATE 300
#define FINISH 400
#define MEMORYSIZE 1000
#define OFFSET 2

const char fileshm[] = "/tmp/shm_queue";
const char filemsg[] = "/tmp/msg_queue_signal";

typedef struct _Signal{
	long type ;
    int signal ;
}Signal;

typedef struct _Data{

	/* must have index */ 
	uint32_t index;

	/* you can define other you want here */
	uint32_t addr;
	uint32_t type;
	uint32_t cpuid;
}Data;

#endif //SHAREMEM
