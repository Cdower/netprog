#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

typedef int bool;
#define true 1
#define false 0
#define MSGSIZ 1024

void fail(char* s){
    perror(s);
    exit(1);
}

int connectToHost(int argc, char *argv[]){
    char *host, *port, *responsePort, *responseHost;
    struct addrinfo *first, *i, *newPeer;
    int ret, sockfd;
    bool verbose;
    char node[80];
    char service[80];
    char *mbuf, personalID[MSGSIZ], connectID[MSGSIZ], ackvar[MSGSIZ];
    verbose = false;

///Initialize tokens to send
    if(argv[4] != NULL){
        host = argv[1];
        strcpy(personalID, "REGISTER ");
        strcat(personalID, argv[2]);
        strcpy(connectID, "GET_ADDR ");
        strcat(connectID, argv[3]);
        mbuf = argv[4];
    }else{
        fprintf(stderr, "Not Enough Arguments\n");
        exit(0);
    }
    if(argv[5] != NULL &&  strcmp(argv[5], "-v")==0 ){ verbose = true; }
    if(verbose){
        printf("pID:%s, cID:%s, msg:%s\n", personalID, connectID, mbuf);
    }
///Intialize & Connect to Sending Address
    port = "12345";
    //i->ai_family = AF_INET6;
    ret = getaddrinfo(host, port, NULL, &first);
    if(ret != 0){
        fprintf(stderr, "getaddrinfo() failed: \n%s", gai_strerror(ret));
    }else if(verbose){
        printf("Connected to UDP server %s:%s\n", host,port);
    }
    i=first;

///Send Messages to server
    sockfd = socket(i->ai_family, SOCK_DGRAM, 0);
    sendto(sockfd, personalID, strlen(personalID), 0, i->ai_addr, i->ai_addrlen);
    if(verbose){
        printf("Sent pID token: %s\n", personalID);
    }
    i=i->ai_next;
    sendto(sockfd, connectID, strlen(connectID), 0, i->ai_addr, i->ai_addrlen);
    if(verbose){
        printf("Sent cID token: %s\nListening for response.\n", connectID);
    }
    if(recvfrom(sockfd, &ackvar, MSGSIZ, 0, i->ai_addr, &(i->ai_addrlen)) < 0){
        fprintf(stderr, "Server Error: Recieving ack");
    }else if(verbose){
        printf("Recieved: %s\n", ackvar);
    }
    if(strstr(ackvar, "NOT FOUND")==NULL){
    ///Connect to new contact Peer
        /*strcpy(responseHost, "::FFFF:");
        char*tmpChar;
        tmpChar = strtok(ackvar, " ");

        strcat(responseHost, tmpChar);*/
        responseHost = strtok(ackvar, " ");
        responsePort = strtok(NULL, " ");
        if(verbose){
            printf("Connecting to: %s:%s\n", responseHost, responsePort);
        }
        i->ai_family = AF_UNSPEC;
        i->ai_socktype = 0;
        i->ai_protocol = 0;
        ret = getaddrinfo(responseHost, responsePort, i, &newPeer);
        if(ret != 0){
            fprintf(stderr, "getaddrinfo() failed: \n%s", gai_strerror(ret));
        }else if(verbose){
            printf("Connected to UDP server %s:%s\n", responseHost,responsePort);
        }
    ///Send Message to new Contact Peer
        i=i->ai_next;
        //ackvar[0]='\0';
        memset(&ackvar[0], 0, sizeof(ackvar));
        sendto(sockfd, mbuf, strlen(mbuf), 0, newPeer->ai_addr, newPeer->ai_addrlen);
        if(verbose){
            printf("Sent msg token: %s\n", mbuf);
        }
    }else{
        newPeer = first;
    }
    //newPeer = first;
///Loop for responses
    while(1){
        if(verbose){
            printf("Listening for packets...\n");
            for(i=newPeer; i!= NULL; i=i->ai_next){
                printf("newperaifamily: %d\n", i->ai_family==AF_INET6);
            }
        }
        if(recvfrom(sockfd, &ackvar, MSGSIZ, 0, newPeer->ai_addr, &(newPeer->ai_addrlen)) < 0){
            fprintf(stderr, "Server Error: Recieving ack");
        }
        else{
            getnameinfo(newPeer->ai_addr, newPeer->ai_addrlen, node, sizeof(node), service, sizeof(service), NI_NUMERICHOST);
/*
            if(newPeer->ai_family == AF_INET){
                printf("recieved from ::ffff:%s (%s): %s\n", node,service, ackvar);
            }else if(newPeer->ai_family == AF_INET6){
                printf("recieved from %s (%s): %s\n", node,service, ackvar);
            }
*/
            bool printed = false;
            for(i=newPeer; i!= NULL; i=i->ai_next){
                if(newPeer->ai_family == AF_INET){
                    printf("recieved from ::ffff:%s (%s): %s\n", node,service, ackvar);
                    printed = true;
                    break;
                }else if(newPeer->ai_family == AF_INET6){
                    printf("recieved from %s (%s): %s\n", node,service, ackvar);
                    printed = true;
                    break;
                }
            }
            if(!printed){
                printf("recieved from ::ffff:%s (%s): %s\n", node,service, ackvar);
            }
        }
    }

    freeaddrinfo(first);
    return 0;
}

int main(int argc, char *argv[]){
    return connectToHost(argc, argv);
}
