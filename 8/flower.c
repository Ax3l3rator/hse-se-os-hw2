#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/sem.h>
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
    signal(SIGINT, ctrl_c);
    sh_mem* shared;

    if ((shmid = shmget(SHM_ID, sizeof(sh_mem), PERMS | IPC_CREAT)) == -1) {
        char msg[100];
        sprintf(msg, "Cannot get shared memory segment. Shared memory name:%s\n", SHM_NAME);
        sys_err(msg);
    }

    if ((shared = (sh_mem*)shmat(shmid, 0, 0)) == NULL) {
        char msg[100];
        sprintf(msg, "Shared memory attach error. Shared memory id:%#010x\n", shmid);
        sys_err(msg);
    }

    int pid;

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