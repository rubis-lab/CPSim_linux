#include <stdio.h>
#include <math.h>
#include <unistd.h>
#define __USE_GNU
#include <pthread.h>
#include <sys/time.h>

double waste_time(long n)
{
	double res = 0;
	long i = 0;
	while (i <n * 400000) {
		i++;
		res += sqrt(i);
	}
	return res;
}

void *thread_func1(void *param)
{
	unsigned long mask = 1; /* processor 0 */
	/* bind process to processor 0 */
	if (pthread_setaffinity_np(pthread_self(), sizeof(mask), (cpu_set_t *)&mask) <0) {
		perror("pthread_setaffinity_np");
	}
	/* waste some time so the work is visible with "top" */
	printf("result of thread 1: %f\n", waste_time(2000));
	

	return 0;
}
void *thread_func2(void *param)
{
	unsigned long mask = 1; /* processor 1 */
	/* bind process to processor 1 */
	if (pthread_setaffinity_np(pthread_self(), sizeof(mask), (cpu_set_t *)&mask) <0) {
		perror("pthread_setaffinity_np");
	}
	/* waste some time so the work is visible with "top" */
	printf("result of thread 2: %f\n", waste_time(2000));

	return 0;
}

/*
int main(int argc, char *argv[])
{
	struct timeval mytime;
	long prev, curr;
	gettimeofday(&mytime, NULL);
	prev = mytime.tv_sec;

	pthread_t p_thread[2];
 
	if (pthread_create(&p_thread[0], NULL, thread_func1, NULL) != 0) {
		perror("pthread_create 1");
	}
	if (pthread_create(&p_thread[1], NULL, thread_func2, NULL) != 0) {
		perror("pthread_create 2");
	}


	//pthread_exit(NULL);
	pthread_join(p_thread[0], NULL);
	pthread_join(p_thread[1], NULL);


	gettimeofday(&mytime, NULL);
	curr = mytime.tv_sec;
	
	printf("Execution time : %ld(sec)\n",	(curr-prev));
	

	return 0;
}
*/

int main(int argc, char *argv[])
{
	struct timeval mytime;
	long prev, curr;
	gettimeofday(&mytime, NULL);
	prev = mytime.tv_sec;

	pthread_t p_thread[2];
	struct sched_param param;
	pthread_attr_t thread_attrs;
	

	pthread_attr_init(&thread_attrs);
	pthread_attr_setschedpolicy(&thread_attrs, SCHED_RR);
	param.sched_priority = 50;
	pthread_attr_setschedparam(&thread_attrs, &param);
	if (pthread_create(&p_thread[0], NULL, thread_func1, NULL) != 0) {
		perror("pthread_create 1");
	}

	pthread_attr_init(&thread_attrs);
	pthread_attr_setschedpolicy(&thread_attrs, SCHED_RR);
	param.sched_priority = 55;
	pthread_attr_setschedparam(&thread_attrs, &param);
	if (pthread_create(&p_thread[1], NULL, thread_func2, NULL) != 0) {
		perror("pthread_create 2");
	}

	//pthread_exit(NULL);
	pthread_join(p_thread[0], NULL);
	pthread_join(p_thread[1], NULL);


	gettimeofday(&mytime, NULL);
	curr = mytime.tv_sec;
	
	printf("Execution time : %ld(sec)\n",	(curr-prev));
	

	return 0;
}
