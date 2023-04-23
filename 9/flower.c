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

    signal(SIGINT, ctrl_c);
    message_buf* mbuf = (message_buf*)malloc(sizeof(message_buf));

    int gtofid, gtogid, ftogid;

    if ((gtofid = msgget(ftok(TOKEN, 3), PERMS | IPC_CREAT)) == -1) {
        sys_err("Msg get error");
    }

    if ((ftogid = msgget(ftok(TOKEN, 2), PERMS | IPC_CREAT)) == -1) {
        sys_err("Msg get error");
    }

    key_t sys_key = ftok(TOKEN, 4);

    int sysid;

    if ((sysid = msgget(sys_key, PERMS | IPC_CREAT)) == -1) {
        sys_err("Msg get error");
    }

    int pid;

    // for (int i = 0; i < 40; ++i) {
    //     mbuf->flower_statuses[i] = FLOWER_FRESH;
    // }

    printf("Flower %d created.\n", id + 1);
    sleep(1);
    srand(id);
    while (flg && msgrcv(gtofid, mbuf, sizeof(int) * 40, MTYPE_FINISH, IPC_NOWAIT) == -1) {
        if (rand() % 40 == id) {
            mbuf->mtype = id + 1;
            mbuf->action = ACTION_WITHERING;
            msgsnd(ftogid, mbuf, sizeof(int) * 40, IPC_NOWAIT);

            printf("Flower %d is withering\n", id + 1);
            sleep(10);
            msgrcv(gtofid, mbuf, sizeof(int) * 40, id + 1, IPC_NOWAIT);
            // for (int i = 0; i < 40; ++i) {
            //     printf("%d ", mbuf->flower_statuses[i]);
            // }
            if (mbuf->action != ACTION_WATERED) {
                printf("Flower %d is dead ☠️ ☠️ ☠️ bruh\n", id + 1);
                return 0;
            }
        } else {
            sleep(1);
        }
    }
    mbuf->mtype = MTYPE_FINISH;
    msgsnd(ftogid, mbuf, sizeof(int) * 40, IPC_NOWAIT);
    if (msgctl(gtofid, IPC_RMID, 0) == -1) {
        sys_err("Failed to remove queue\n");
    }
    if (msgctl(ftogid, IPC_RMID, 0) == -1) {
        sys_err("Failed to remove queue\n");
    }
    return 0;
}