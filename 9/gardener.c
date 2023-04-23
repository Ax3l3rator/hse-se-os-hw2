#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "../message.h"
#include "../queue.h"

int flg = 1;

void ctrl_c(int blank) {
    flg = 0;
    exit(0);
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

    message_buf* mbuf = (message_buf*)malloc(sizeof(message_buf));

    mbuf->mtype = 1;

    int gtofid;
    int ftogid;
    int gtogid;
    int semid;
    int sysid;

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
    printf("Gardener %d created.\n", id + 1);

    sleep(1);
    int sz = 0;
    while (flg) {
        if (msgrcv(ftogid, mbuf, sizeof(int) * 40, MTYPE_FINISH, IPC_NOWAIT) != -1) {
            break;
        }
        for (int flower = 1; flower < 41; ++flower) {
            if ((sz = msgrcv(ftogid, mbuf, sizeof(int) * 40, flower, IPC_NOWAIT)) != -1) {
                mbuf->mtype = flower;

                mbuf->action = ACTION_WATERED;
                usleep(1000 * 200);
                msgsnd(gtofid, mbuf, sizeof(int) * 40, IPC_NOWAIT);
                printf("%d:Watered flower %d\n", id + 1, flower);
            }
        }
    }
    mbuf->mtype = MTYPE_FINISH;
    msgsnd(gtofid, mbuf, sizeof(int) * 40, IPC_NOWAIT);

    return 0;
}