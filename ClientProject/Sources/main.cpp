#include "TestInfo.h"


int main() {

    pthread_t tid;

    initDBusCommunicationForClient();

    pthread_create(&tid,NULL,run,NULL);

    clientLoop();
    return 0;
}
