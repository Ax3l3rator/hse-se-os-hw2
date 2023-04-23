
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "../message.h"

void sys_err(char* msg) {
    puts(msg);
    exit(1);
}

int main() {
    int gtofid;
    int ftogid;
    int gtogid;

    key_t ftog_key = ftok(TOKEN, 2), gtof_key = ftok(TOKEN, 3), sem_key = ftok(TOKEN, 0);
    if (ftog_key == -1 || gtof_key == -1 || sem_key == -1) {
        printf("%d %d %d\n", ftog_key, gtof_key, sem_key);
        sys_err("Tokenization error:");
    }

    if ((gtofid = msgget(gtof_key, PERMS | IPC_CREAT)) == -1) {
        sys_err("Msg get error");
    }

    if ((ftogid = msgget(ftog_key, PERMS | IPC_CREAT)) == -1) {
        sys_err("Msg get error");
    }

    if (msgctl(gtofid, IPC_RMID, 0) == -1) {
        sys_err("Failed to remove queue\n");
    }
    if (msgctl(ftogid, IPC_RMID, 0) == -1) {
        sys_err("Failed to remove queue\n");
    }
    if (msgctl(gtogid, IPC_RMID, 0) == -1) {
        sys_err("Failed to remove queue\n");
    }
    return 0;
}