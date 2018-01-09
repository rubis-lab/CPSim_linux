#include <stdio.h>
#include <math.h>
#include <unistd.h>
#define __USE_GNU
#include <sched.h>

double waste_time(long n)
{
    double res = 0;
    long i = 0;
    while(i <n * 200000) {
        i++;
        res += sqrt (i);
    }
    return res;
}
int main(int argc, char **argv)
{
    unsigned long mask = 1; /* processor 0 */
    /* bind process to processor 0 */
    if (sched_setaffinity(0, sizeof(mask), (cpu_set_t*)&mask) <0) {
        perror("sched_setaffinity");
    }
    /* waste some time so the work is visible with "top" */
    printf ("result: %f\n", waste_time (2000));
    mask = 2; /* process switches to processor 1 now */
    if (sched_setaffinity(0, sizeof(mask), (cpu_set_t*)&mask) <0) {
        perror("sched_setaffinity");
    }
    /* waste some more time to see the processor switch */
    printf ("result: %f\n", waste_time (2000));
    return 0;
 
}
