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

mqd_t ftogid, gtofid;

struct mq_attr attr;

char* msg;

void ctrl_c(int blank) {
    flg = 0;
    free(msg);
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

    attr.mq_flags = 0;
    attr.mq_maxmsg = 40;
    attr.mq_msgsize = 1;
    attr.mq_curmsgs = 0;

    int id = atoi(argv[1]);

    ftogid = mq_open("/ftog", O_RDONLY | O_CREAT | O_EXCL, PERMS, &attr);
    if (ftogid == -1) {
        ftogid = mq_open("/ftog", O_RDONLY, PERMS, &attr);
    }

    gtofid = mq_open("/gtof", O_WRONLY | O_CREAT | O_EXCL, PERMS, &attr);
    if (gtofid == -1) {
        gtofid = mq_open("/gtof", O_WRONLY, PERMS, &attr);
    }

    sleep(1);
    msg = (char*)calloc(1, 1);
    printf("Gardener %d created.\n", id + 1);
    unsigned int sz;

    while (flg) {
        if (mq_receive(ftogid, msg, 1, &sz) != -1) {
            if (msg[0] == 100) {
                break;
            }
            usleep(1000 * 200);
            msg[0] = msg[0];
            mq_send(gtofid, msg, 1, 1);
            printf("%d:Watered flower %d\n", id + 1, msg[0]);
        }
    }

    free(msg);
    return 0;
}