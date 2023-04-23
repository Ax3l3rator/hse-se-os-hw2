#include <fcntl.h>
#include <mqueue.h>
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
    mqd_t ftogid, gtofid;

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 40;
    attr.mq_msgsize = 1;
    attr.mq_curmsgs = 0;

    ftogid = mq_open("/ftog", O_RDONLY | O_CREAT | O_EXCL, PERMS, &attr);
    if (ftogid == -1) {
        ftogid = mq_open("/ftog", O_RDONLY, PERMS, &attr);
    }

    gtofid = mq_open("/gtof", O_WRONLY | O_CREAT | O_EXCL, PERMS, &attr);
    if (gtofid == -1) {
        gtofid = mq_open("/gtof", O_WRONLY, PERMS, &attr);
    }

    if (mq_close(ftogid) == -1) {
        sys_err("Can't close q");
    }
    if (mq_close(gtofid) == -1) {
        sys_err("Can't close q");
    }
    if (mq_unlink("/ftog") == -1) {
        sys_err("Can't close q");
    }
    if (mq_unlink("/gtof") == -1) {
        sys_err("Can't close q");
    }
    return 0;
}