#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "../message.h"

int flg = 1;

void ctrl_c(int blank) {
    flg = 0;
}

void sys_err(char* msg) {
    puts(msg);
    exit(1);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        char msg[100];
        sprintf(msg, "Wrong launch format. Program need following format: %s <flower id>\n",
                argv[0]);
        sys_err(msg);
    }

    int id = atoi(argv[1]);

    int shmid;

    sh_mem* shared;

    if ((shmid = shm_open(SHM_NAME, O_CREAT | O_RDWR, PERMS)) == -1) {
        char msg[100];
        sprintf(msg, "Cannot get shared memory segment. Shared memory name:%s\n", SHM_NAME);
        sys_err(msg);
    }

    if (ftruncate(shmid, sizeof(sh_mem)) == -1) {
        char msg[100];
        sprintf(msg, "Cannot size shared memory segment. Shared memory id:%#010x\n", shmid);
        sys_err(msg);
    }

    if ((shared = mmap(0, sizeof(sh_mem), PROT_WRITE | PROT_READ, MAP_SHARED, shmid, 0)) ==
        MAP_FAILED) {
        char msg[100];
        sprintf(msg, "Shared memory attach error. Shared memory id:%#010x\n", shmid);
        sys_err(msg);
    }

    for (int i = 0; i < 40; ++i) {
        shared->flower_statuses[i] = FLOWER_FRESH;
    }

    shared->type = MSG_TYPE_EMPTY;

    printf("Flower %d created.\n", id + 1);
    sleep(1);
    srand(id);
    while (flg && shared->type != MSG_TYPE_FINISH) {
        if (rand() % 40 == id) {
            shared->flower_statuses[id] = false;
            printf("Flower %d is withering\n", id + 1);
            sleep(10);
            if (shared->flower_statuses[id] == FLOWER_WITHERING) {
                printf("Flower %d is dead ☠️ ☠️ ☠️ bruh\n", id + 1);

                return 0;
            }
        } else {
            sleep(1);
        }
    }
    shared->type = MSG_TYPE_FINISH;

    return 0;
}