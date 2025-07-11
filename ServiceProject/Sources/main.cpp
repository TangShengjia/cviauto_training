// ServerProject/Sources/main.cpp
#include <pthread.h>
#include <string>
#include "TestInfo.h"



int main() {
    pthread_t tid;
    initDBusCommunicationForServer();

    pthread_create(&tid, NULL, run, NULL);

    pthread_join(tid, nullptr);
    return 0;
}