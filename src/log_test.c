#include "tslog.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define THREADS 5
#define ITERATIONS 10

void *worker(void *arg)
{
    int id = *(int *)arg;
    for (int i = 0; i < ITERATIONS; i++)
    {
        tslog_info("Thread %d writing message %d", id, i);
        usleep(100000); // 0.1s
    }
    return NULL;
}

int main(void)
{
    if (tslog_init("test.log") != 0)
    {
        fprintf(stderr, "Failed to init logger\n");
        return 1;
    }

    pthread_t th[THREADS];
    int ids[THREADS];
    for (int i = 0; i < THREADS; i++)
    {
        ids[i] = i;
        pthread_create(&th[i], NULL, worker, &ids[i]);
    }

    for (int i = 0; i < THREADS; i++)
    {
        pthread_join(th[i], NULL);
    }

    tslog_close();
    printf("\n\nTest finished. Check test.log\n");
    return 0;
}
