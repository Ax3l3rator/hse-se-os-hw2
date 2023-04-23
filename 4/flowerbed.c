#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
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
    int shmid;
    signal(SIGINT, ctrl_c);
    sh_mem* shared;

    if ((shmid = shmget(SHM_ID, sizeof(sh_mem), PERMS | IPC_CREAT)) == -1) {
        char msg[100];
        sprintf(msg, "Cannot get shared memory segment. Shared memory id:%#010x\n", SHM_ID);
        sys_err(msg);
    }

    if ((shared = (sh_mem*)shmat(shmid, 0, 0)) == NULL) {
        char msg[100];
        sprintf(msg, "Shared memory attach error. Shared memory id:%#010x\n", SHM_ID);
        sys_err(msg);
    }

    int pid;
    for (int i = 0; i < 40; ++i) {
        shared->flower_statuses[i] = FLOWER_FRESH;
    }

    for (int i = 0; i < 40; i++) {
        pid = fork();
        if (pid == -1) {
            printf("Error: fork failed.\n");
            return -1;
        } else if (pid == 0) {
            printf("Flower %d created.\n", i + 1);
            sleep(1);
            srand(i);
            while (flg && shared->type != MSG_TYPE_FINISH) {
                if (rand() % 40 == i) {
                    shared->flower_statuses[i] = false;
                    printf("Flower %d is withering\n", i + 1);
                    sleep(10);
                    if (shared->flower_statuses[i] == FLOWER_WITHERING) {
                        printf("Flower %d is dead ☠️ ☠️ ☠️ bruh\n", i + 1);
                        return 0;
                    }
                } else {
                    sleep(1);
                }
            }
            shared->type = MSG_TYPE_FINISH;
            exit(0);
        }
        usleep(20000);
    }
    printf("All child processes created.\n");
    while (1) {
        pid = wait(NULL);
        if (pid == -1) {
            printf("All child processes terminated.\n");
            break;
        }
    }

    shmdt(shared);

    if (shmctl(shmid, IPC_RMID, (struct shmid_ds*)0) == -1) {
        sys_err("Shared memory remove error");
    }

    return 0;
}