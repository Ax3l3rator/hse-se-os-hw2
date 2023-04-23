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

    if ((shmid = shm_open(SHM_NAME, O_RDWR, PERMS)) == -1) {
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

    if (sem_init(&shared->sem, 1, 1) == -1) {
        sys_err("Can\'t create gardeners semaphore set");
    }

    int pid = 0;

    for (int i = 0; i < 2; i++) {
        pid = fork();
        if (pid == -1) {
            printf("Error: fork failed.\n");
            return -1;
        } else if (pid == 0) {
            printf("Gardener %d created.\n", i + 1);

            sleep(1);
            while (flg && shared->type != MSG_TYPE_FINISH) {
                for (int j = 0; j < 40; ++j) {
                    if (shared->flower_statuses[j] == FLOWER_WITHERING) {
                        if (sem_wait(&shared->sem) == -1) {
                            sys_err("Can't wait with semaphore");
                        }

                        if (shared->flower_statuses[j] == FLOWER_WATERING) {
                            printf("%d:Skipping flower %d, it is already getting watered\n", i + 1,
                                   j + 1);

                            if (sem_post(&shared->sem) == -1) {
                                sys_err("Can't wait with semaphore");
                            }

                            continue;
                        }
                        shared->flower_statuses[j] = FLOWER_WATERING;

                        if (sem_post(&shared->sem) == -1) {
                            sys_err("Can't wait with semaphore");
                        }

                        usleep(1000 * 240);
                        shared->flower_statuses[j] = FLOWER_FRESH;

                        printf("%d:Watered flower %d\n", i + 1, j + 1);
                    }
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

    if (sem_destroy(&shared->sem) == -1) {
        sys_err("Failed to unlink semaphore\n");
    }

    return 0;
}