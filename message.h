#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>

#define SHM_ID 0x779
#define SHM_NAME "shared-memory-gardener-flowers"
#define SEM_NAME "semaphore-gardener-flowers"
#define TOKEN "../message.h"
#define PERMS 0666

#define MSG_TYPE_EMPTY 0
#define MSG_TYPE_FINISH 1
#define FLOWER_WITHERING 0
#define FLOWER_FRESH 1
#define FLOWER_WATERING 2

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void set_sem_values(int semid, int value, unsigned int size) {
    for (int i = 0; i < size; ++i) {
        if ((semctl(semid, i, SETVAL, value)) < 0) {
            printf("Can't set value of %d semaphore by id:%#010x\n", i, semid);
        }
    }
}

typedef struct {
    int type;
    short flower_statuses[40];
    sem_t sem;
} sh_mem;