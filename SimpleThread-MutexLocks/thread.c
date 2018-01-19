#define _XOPEN_SOURCE	600
//#define PTHREAD_SYNC


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "syscall.h"


int SharedVariable = 0;

#ifdef PTHREAD_SYNC
pthread_barrier_t barr;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
#endif
void* SimpleThread(void* which) {
        int num, val;
        for(num = 0; num < 20; num++) {
                if (rand() > RAND_MAX / 2)
                        usleep(10);
                #ifdef PTHREAD_SYNC
                pthread_mutex_lock( &mutex1 );
                #endif
                val = SharedVariable;
                printf("*** thread %d sees value %d\n", (int) which, val);
                SharedVariable = val + 1;
                #ifdef PTHREAD_SYNC
                pthread_mutex_unlock( &mutex1 );
                #endif

        }
        #ifdef PTHREAD_SYNC
        int rc = pthread_barrier_wait(&barr);
        if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD){
                printf("Could not wait on barrier\n");
                exit(-1);
        }
        #endif
        val = SharedVariable;
        printf("Thread %d sees final value %d\n", (int) which, val);
}

int main(int argc, char* argv[]){
        int counter, input = 0;
 	if (argc > 2){
                printf("Need one int as input\n");
                return 1;
        }else if(atoi(argv[1]) == 0){
                printf("Need one int as input\n");
                return 1;
         }else{
                input = atoi(argv[1]);
                pthread_t threads[input];

                #ifdef PTHREAD_SYNC
                if(pthread_barrier_init(&barr, NULL, input)){
                        printf("Could not create barrier\n");
                        return -1;
                }
                #endif

                for(counter = 0; counter < input; counter++){
                        if(pthread_create(&threads[counter], NULL, &SimpleThread, (void*) counter)){
                                printf("Could not create thread %d\n", counter);
                                return -1;
                        }
                }
		printf("Syscall success, check dmesg|tail \n\n", syscall(__NR_print_tasks_mark_cameron));
		
		for(counter = 0; counter < input; counter++){
                        if(pthread_join(threads[counter], NULL)){
                                printf("Could not join thread %d\n", counter);
                                return -1;
                        }
                }
         }
        return 0;
}
