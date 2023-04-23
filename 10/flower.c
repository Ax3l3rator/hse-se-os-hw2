#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
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

    int ftogid, gtogid, gtofid;
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 40;
    attr.mq_msgsize = 1;
    attr.mq_curmsgs = 0;

    ftogid = mq_open("/ftog", O_WRONLY, PERMS, &attr);
    gtofid = mq_open("/gtof", O_RDONLY, PERMS, &attr);
    if (ftogid == -1) {
        sys_err("AAAAAAAA");
    }

    char* msg = (char*)malloc(1 * sizeof(char));
    printf("%d %d\n", ftogid, gtofid);
    printf("Flower %d created.\n", id + 1);
    // srand(id);

    while (flg) {
        if (rand() % 40 == id) {
            msg[0] = id + 1;
            if (mq_send(ftogid, msg, 1, 1) == -1) {
                sys_err("cant send message");
            }
            printf("Flower %d is withering\n", id + 1);
            sleep(10);
            mq_receive(gtofid, msg, 1, NULL);
            if (msg[0] == 100) {
                break;
            }
            if (msg[0] != id + 1) {
                printf("Flower %d is dead ☠️ ☠️ ☠️ bruh\n", id + 1);
                return 0;
            } else {
                printf("Flower %d is watered", msg[0]);
            }
        } else {
            sleep(1);
        }
    }
    msg[0] = 100;
    mq_send(ftogid, msg, 1, 50);
    mq_close(ftogid);
    mq_close(gtofid);
    return 0;
}