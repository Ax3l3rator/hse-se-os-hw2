#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "../message.h"

void sys_err(char* msg) {
    puts(msg);
    exit(1);
}

int main() {
    int shmid;

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

    if (shared->type == MSG_TYPE_FINISH) {
        return 0;
    }

    shared->type = MSG_TYPE_FINISH;
    sleep(1);

    shmdt(shared);

    if (shmctl(shmid, IPC_RMID, (struct shmid_ds*)0) == -1) {
        sys_err("Shared memory remove error");
    }
    return 0;
}