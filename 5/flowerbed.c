#include <errno.h>
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
    int shmid;
    signal(SIGINT, ctrl_c);
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

    if (close(shmid) == -1) {
        sys_err("Failed to close shared memory\n");
    }

    if (shm_unlink(SHM_NAME) == -1) {
        sys_err("Failed to unlink shared memory\n");
        printf("%d", errno);
    }

    return 0;
}