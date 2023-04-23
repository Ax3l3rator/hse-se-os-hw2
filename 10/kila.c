#include <mqueue.h>

int main() {
    mq_unlink("ftog");
    mq_unlink("gtog");
    mq_unlink("gtof");
}