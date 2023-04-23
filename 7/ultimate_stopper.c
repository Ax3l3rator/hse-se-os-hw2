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

    if ((shmid = shm_open(SHM_NAME, O_RDWR, PERMS)) == -1) {
        char msg[100];
        sprintf(msg, "Cannot get shared memory segment. Shared memory name:%s\n", SHM_NAME);
        sys_err(msg);
    }

    if ((shared = mmap(0, sizeof(sh_mem), PROT_WRITE | PROT_READ, MAP_SHARED, shmid, 0)) ==
        MAP_FAILED) {
        char msg[100];
        sprintf(msg, "Shared memory attach error. Shared memory id:%#010x\n", shmid);
        sys_err(msg);
    }

    shared->type = MSG_TYPE_FINISH;
    sleep(2);
    if (close(shmid) == -1) {
        sys_err("Failed to close shared memory\n");
    }

    if (shm_unlink(SHM_NAME) == -1) {
        sys_err("Failed to unlink shared memory\n");
    }
    return 0;
}