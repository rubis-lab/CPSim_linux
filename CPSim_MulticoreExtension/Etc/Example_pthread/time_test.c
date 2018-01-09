#include<sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main()
{
	struct timeval mytime;
	int i = 0;
	int count = 0;
	long prev;
	long curr;

	gettimeofday(&mytime, NULL);
	prev = mytime.tv_usec;
        printf("prev : %ld(usec)\n", prev);

	for(i=0; i<1000000000 ; i++)
	{
		count++;
	}
	printf("count : %d\n", count);

	gettimeofday(&mytime, NULL);
	curr = mytime.tv_usec;
        printf("curr : %ld(usec)\n", curr);

	printf("%ld\n", curr-prev);

} 
