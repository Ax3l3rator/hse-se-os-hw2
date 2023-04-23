#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
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

    signal(SIGINT, ctrl_c);

    int id = atoi(argv[1]);

    int shmid;
    int semid;

    if ((semid = semget(ftok(TOKEN, 0), 1, PERMS | IPC_CREAT)) == -1) {
        sys_err("Can\'t create gardeners semaphore set");
    }

    sh_mem* shared;

    if ((shmid = shmget(SHM_ID, sizeof(sh_mem), PERMS)) == -1) {
        char msg[100];
        sprintf(msg, "Cannot get shared memory segment. Shared memory name:%s\n", SHM_NAME);
        sys_err(msg);
    }

    if ((shared = (sh_mem*)shmat(shmid, 0, 0)) == NULL) {
        char msg[100];
        sprintf(msg, "Shared memory attach error. Shared memory id:%#010x\n", shmid);
        sys_err(msg);
    }
    set_sem_values(semid, 1, 1);
    printf("Gardener %d created.\n", id + 1);

    sleep(1);
    while (flg && shared->type != MSG_TYPE_FINISH) {
        for (int j = 0; j < 40; ++j) {
            if (shared->flower_statuses[j] == FLOWER_WITHERING) {
                struct sembuf sem_lock = {0, -1, 0};
                struct sembuf sem_unlock = {0, 1, 0};

                if (semop(semid, &sem_lock, 1) == -1) {
                    sys_err("Can't wait with semaphore");
                }

                if (shared->flower_statuses[j] == FLOWER_WATERING) {
                    printf("%d:Skipping flower %d, it is already getting watered\n", id + 1, j + 1);

                    if (semop(semid, &sem_unlock, 1) == -1) {
                        sys_err("Can't wait with semaphore");
                    }

                    continue;
                }
                shared->flower_statuses[j] = FLOWER_WATERING;

                if (semop(semid, &sem_unlock, 1) == -1) {
                    sys_err("Can't wait with semaphore");
                }

                usleep(1000 * 200);
                shared->flower_statuses[j] = FLOWER_FRESH;

                printf("%d:Watered flower %d\n", id + 1, j + 1);
            }
        }
    }
    shared->type = MSG_TYPE_FINISH;
    exit(0);

    usleep(20000);

    if (semctl(semid, 0, IPC_RMID) == -1) {
        sys_err("Failed to remove semaphore\n");
    }

    return 0;
}