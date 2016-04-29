#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <map>

#define PORT 23456
#define HOST "239.255.24.25"
#define WAIT 60
#define NUM_THREADS 2

///Intialize User Table
std::map<std::string, struct sockaddr_in> userTable;
pthread_mutex_t lock;

typedef struct _thread_data_t {
    int tid;
    struct sockaddr_in addr;
    int sockfd;
    char * usrName;
    //std::map<std::string, sockaddr_in> *userTable;
} thread_data_t;

void *announce(void* arg){
    int cnt;
    char msg[512];
    thread_data_t *data = (thread_data_t *)arg;
    struct ip_mreqn mreq;
    mreq.imr_multiaddr = data->addr.sin_addr;
    if(setsockopt(data->sockfd, INADDR_ANY, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
    {
      perror("Setting announce interface error");
      exit(1);
    }
    else{
        printf("Setting the announce interface...OK\n");
    }/**/
    std::string message = "ANNOUNCE ";
    message += std::string(data->usrName);
    std::strcpy(msg, message.c_str());
    //printf("s:%s\n", msg);

    while(1){
        printf("Sending data: %s\n", msg);
        cnt = sendto(data->sockfd, msg, sizeof(msg),0,(sockaddr *) &(data->addr), sizeof(data->addr));
        if(cnt < 0){
            perror("sendTo");
            pthread_exit(NULL);
        }
        //printf("SENT: %s\n", msg);
        sleep(60);
    }
    //printf("hello from announce, thread id: %d\n", data->tid);
    //free(arg);
    pthread_exit(NULL);
}

void* recieve(void* arg){
    ///Intialize local variables
    char buf[512];
    thread_data_t *data = (thread_data_t *)arg;
    struct sockaddr_in from;// = data->addr;
    unsigned int addr_len;
    addr_len = sizeof(from);
    //printf("from: %d\naddr: %d", from.sin_family, data->addr.sin_family); //sin_family 0 by default, will be != 0 if initialized
    ///Create recieve multicasts
    while(1){
        //printf("Waiting to recieve message in thread %d..\n", data->tid);
        if( recvfrom(data->sockfd, &buf, sizeof(buf), 0, (sockaddr *) &(from), &addr_len) < 0){
            fprintf(stderr, "Server Error: Recieving ack");
            pthread_exit(NULL);
        }else{
            printf("%s\n", buf);
            std::string buffer(buf), searchString("ANNOUNCE ");
            std::size_t pos = buffer.find(searchString);
            if(pos!=std::string::npos){
                std::cout << "Adding " << buffer.substr(pos+searchString.size()) << " to map" << std::endl;
                pthread_mutex_lock(&lock);
                userTable[buffer.substr(pos+searchString.size())] = from;
                pthread_mutex_unlock(&lock);
            }
            /*char * token;
            char *buffer;
            const char * tmp = (char *) buf;
            strcpy(buffer,tmp);
            token = strtok(buffer, ' ');
            printf("%s", token);/**/
            memset(&buf[0], 0, sizeof(buf));
        }
    }
    //free(arg);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]){
//Create UPD IP4 socket and bind to 23456 PORT
    if(argc < 2){
        return EXIT_FAILURE;
    }
    int i, rc, sockfd;
    struct sockaddr_in addr;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_aton(HOST, &addr.sin_addr);//addr.sin_addr.s_addr = inet_addr(HOST);


///Set socketoptions before bind
    struct ip_mreqn mreq;
    mreq.imr_multiaddr = addr.sin_addr;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &mreq, sizeof(mreq)) < 0)
    {
      perror("Setting announce interface error");
      exit(1);
    }
    else{
        printf("Setting the announce interface...OK\n");
    }/**/

    bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));

///Initialize thread stuff(variables etc...)
    pthread_t thread[NUM_THREADS];
    thread_data_t thread_data[NUM_THREADS];
    thread_data[0].tid = 0;
    thread_data[1].tid = 1;
    thread_data[0].sockfd = sockfd;
    thread_data[1].sockfd = sockfd;
    thread_data[0].addr = addr;
    thread_data[1].addr = addr;
    thread_data[0].usrName = argv[1];
    thread_data[1].usrName = argv[1];

    if ((rc = pthread_create(&thread[0], NULL, announce, &thread_data[0]))) {
        fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
        return EXIT_FAILURE;
    }
    if ((rc = pthread_create(&thread[1], NULL, recieve, &thread_data[1]))) {
        fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
        return EXIT_FAILURE;
    }
///Listen for user input
    std::string input, message;
    //char msgbuf[512];
    while(1){
        getline(std::cin, input);

        if(input.find("/") == 0){///PRIVATE MESSAGE
            std::string pm_rec = input.substr(1,input.find(" "));
            message= "FROM:"+std::string(argv[1]) + ' ' + input.substr(input.find(" ")+1);
            std::cout << "PMing " << pm_rec << ": " << message << std::endl;
            pthread_mutex_lock(&lock); //Mutex enabled when using table
            sendto(sockfd, message.c_str(), message.length(), 0, (sockaddr *) &userTable[pm_rec], sizeof(userTable[pm_rec]));
            pthread_mutex_unlock(&lock); //Mutex unlocked when table not in use
        }else{///Public Messages
            message= "FROM:"+std::string(argv[1]) + ' ' + input;
            sendto(sockfd, message.c_str(), message.length(),0,(sockaddr *) &(addr), sizeof(addr));
        }
        message.clear();
        //std::strcpy(msgbuf, message.c_str());
        //sendto(sockfd, msgbuf, sizeof(msgbuf),0,(sockaddr *) &(addr), sizeof(addr));
        //memset(&msgbuf[0], 0, sizeof(msgbuf));
    }

//Block for threads to finish
    for (i = 0; i < NUM_THREADS; ++i) {
        pthread_join(thread[i], NULL);
    }
    pthread_mutex_destroy(&lock);
    return EXIT_SUCCESS;

}
