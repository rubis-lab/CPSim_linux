#include <stdio.h>
#include <math.h>
#include <unistd.h>
#define __USE_GNU
#include <pthread.h>
double waste_time(long n)
{
	double res = 0;
	long i = 0;
	while (i <n * 200000) {
		i++;
		res += sqrt(i);
	}
	return res;
}

void *thread_func(void *param)
{
	unsigned long mask = 1; /* processor 0 */
	/* bind process to processor 0 */
	if (pthread_setaffinity_np(pthread_self(), sizeof(mask), (cpu_set_t *)&mask) <0) {
		perror("pthread_setaffinity_np");
	}
	/* waste some time so the work is visible with "top" */
	printf("result: %f\n", waste_time(2000));
	mask = 2;  /* process switches to processor 1 now */
	sleep(2);
	if (pthread_setaffinity_np(pthread_self(), sizeof(mask), (cpu_set_t *)&mask) <0) {
		perror("pthread_setaffinity_np");
	}
	/* waste some more time to see the processor switch */
	printf("result: %f\n", waste_time(2000));
	return 0;
}

int main(int argc, char *argv[])
{
	pthread_t my_thread;
 
	if (pthread_create(&my_thread, NULL, thread_func, NULL) != 0) {
		perror("pthread_create");
	}
	//pthread_exit(NULL);
	pthread_join(my_thread, NULL);
	return 0;
}
