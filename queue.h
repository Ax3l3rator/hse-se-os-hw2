#define ACTION_WATERED 1
#define ACTION_WITHERING 2
#define ACTION_WATERING 3
#define MTYPE_FINISH 127

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>

typedef struct msgbuf {
    long mtype;
    int action;
} message_buf;
