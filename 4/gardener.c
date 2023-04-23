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
    int semid;

    signal(SIGINT, ctrl_c);

    if ((semid = semget(ftok(TOKEN, 0), 1, PERMS | IPC_CREAT)) == -1) {
        sys_err("Can\'t create gardeners semaphore set");
    }

    sh_mem* shared;

    if ((shmid = shmget(SHM_ID, sizeof(sh_mem), PERMS)) == -1) {
        char msg[100];
        sprintf(msg, "Cannot get shared memory segment. Shared memory id:%#010x\n", SHM_ID);
        sys_err(msg);
    }

    if ((shared = (sh_mem*)shmat(shmid, 0, 0)) == NULL) {
        char msg[100];
        sprintf(msg, "Shared memory attach error. Shared memory id:%#010x\n", SHM_ID);
        sys_err(msg);
    }

    set_sem_values(semid, 1, 1);
    int pid = 0;
    for (int i = 0; i < 2; i++) {
        pid = fork();
        if (pid == -1) {
            printf("Error: fork failed.\n");
            return -1;
        } else if (pid == 0) {
            printf("Gardener %d created.\n", i + 1);

            sleep(2);
            while (flg && shared->type != MSG_TYPE_FINISH) {
                for (int j = 0; j < 40; ++j) {
                    if (shared->flower_statuses[j] == FLOWER_WITHERING) {
                        struct sembuf sem_lock = {0, -1, 0};
                        struct sembuf sem_unlock = {0, 1, 0};
                        semop(semid, &sem_lock, 1);

                        if (shared->flower_statuses[j] == FLOWER_WATERING) {
                            printf("%d:Skipping flower %d, it is already getting watered\n", i + 1,
                                   j + 1);
                            semop(semid, &sem_unlock, 1);
                            continue;
                        }
                        shared->flower_statuses[j] = FLOWER_WATERING;

                        semop(semid, &sem_unlock, 1);

                        usleep(1000 * 200);
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
    shmdt(shared);

    if (semctl(semid, 0, IPC_RMID) == -1) {
        sys_err("Failed to remove semaphore\n");
    }
    return 0;
}