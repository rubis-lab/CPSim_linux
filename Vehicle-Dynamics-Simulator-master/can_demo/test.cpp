#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#include <errno.h>
#include <signal.h>
#include <string.h>

int main(int argc, char *argv[])
{
	pid_t pid;
	pid = fork();
	if(pid == 0)
		execl("/home/jckim/stop.sh", "/home/jckim/stop.sh", (char*) 0);

	return 0;
}

