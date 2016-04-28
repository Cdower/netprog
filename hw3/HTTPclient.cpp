#include <cstring>
#include <string>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

struct url_p{
    const char * host;
    const char * port;
    const char * path;
    //const char * service;
};

int parseURL(char * URL, url_p& parsed){
    int portNum, hostSplit;
    std::string host, port, token, path;//, service;
    std::string sURL(URL);
    token = sURL.substr(sURL.find("www"));
    /*service = sURL.substr(0,sURL.find("://"));
    std::cout << service << std::endl;/**/
    hostSplit = token.find(":");

    if(hostSplit != std::string::npos){
        host = token.substr(0,hostSplit);
        port = token.substr(hostSplit+1);
        port = port.substr(0,port.find('/'));
        std::stringstream convert(port);
        if(!(convert >> portNum)){
            portNum=80;
        }
    }else{
        port = "80";
        host = token.substr(0,token.find('/'));
    }
    path = token.substr(token.find('/'));

    parsed.host = host.c_str();
    parsed.path = path.c_str();
    parsed.port = port.c_str();

    return EXIT_SUCCESS;
}

///Makes TCP Socket Connection
//Returns addrinfo of connection by reference
int connectToHost(url_p parsed, struct addrinfo *first){
    int sockfd, ret, lenHost, lenPort;
    char *host, *port;
    struct addrinfo hints, *i;
    lenHost = strlen(parsed.host);
    lenPort = strlen(parsed.port);
    host = new char[lenHost+1]();
    port = new char[lenPort+1]();
    strncpy(host, parsed.host, lenHost);
    strncpy(port, parsed.port, lenPort);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    ///Initialize & Connect TCP Socket
    ret = getaddrinfo(host, port, &hints, &first);
    if(ret != 0){
        fprintf(stderr, "getaddrinfo() failed: %s\n", gai_strerror(ret));
        exit(1);
    }

    for(i=first; i!= NULL; i=i->ai_next){
        if((sockfd = socket(i->ai_family, SOCK_DGRAM, 0)) == -1 ){
            perror("socket");
            continue;
        }
        bind(sockfd, (struct sockaddr *) i, sizeof(*i));
        if( connect( sockfd, (i->ai_addr), i->ai_addrlen) == -1){
            std::cout << "Connect Failure" << std::endl;
            close(sockfd);
            perror("connect");
            continue;
        }
        break;
    }
    //ret = connect(sockfd, first->ai_addr, first->ai_addrlen);
    return sockfd;
}

int sendRequest(char* path, char* host, char* port, int sockfd){
    std::string query = "GET ";
    query.append(path);
    query.append(" HTTP/1.1\r\nHost: ");
    query.append(host); query.append(":"); query.append(port);
    query.append("\r\n");
    query.append("User-Agent: dowerc-netprog-hw3/1.0\r\n\r\n\0");
    if(send(sockfd, query.c_str(), sizeof(query.c_str()), 0) < 0){
        std::cout << "Not written: " << query << std::endl;
    }else{std::cout << "Sent:\n" << query << std::endl;}
    return 0;
}

int listenResponse(int sockfd){
    char buf[1024];
    char * headerEnd;
    int len, flags = 0;
    int total = 2147483647; //INTMAX

    std::cout << "Recieved: " << std::endl;
    while( (len = recv(sockfd, buf, sizeof(buf)-1, 0)) > 0 && total > 0){
        buf[len] = '\0';
        headerEnd = strstr(buf, "\r\n\r\n");
        if(headerEnd != NULL && flags == 0){
            //get headers to properrly set stderrr
            int headerSize = headerEnd - buf;
            char headers[headerSize + 5];
            strncpy(headers, buf, headerSize);
            snprintf(headers+headerSize, 5, "\r\n\r\n");
            headers[headerSize+4] = '\0';
            fwrite(headers, 1, sizeof(headers), stderr);

            //
            char* contentSize = strstr(headers, "Content-Size: ");
            if(contentSize == NULL){ exit(1);}
            contentSize = contentSize + strlen("Content=Size: ");
            char * rnIndex = strstr(contentSize, "\r\n");
            char length[rnIndex - contentSize + 1];
            strncpy(length, contentSize, sizeof(length));
            total = atoi(length);

            //
            fwrite(headerEnd+4, 1, len-headerSize-4, stdout);
            fflush(stdout);
            total = total-len;
            flags = 1;
        }
        else{
        //
        fwrite(buf, 1, len, stdout);
        fflush(stdout);
        total=total-len;
        }
        memset(buf, 0, len);
    }
    /*len = recv(sockfd,buf, sizeof(buf)-1,0);
    while(len > 0){
        std::cout << buf << std::endl;
        std::cout.write(buf, sizeof(len));
        std::cout << std::endl;
        memset(&buf[0], 0, sizeof(buf));
        recv(sockfd,buf, 255,0);
    }/**/
    std::cout << std::endl;
    return 0;
}

int main(int argc, char * argv[]){
    url_p parsed;
    char queryPath[512], queryHost[256], queryPort[20];
    struct addrinfo *first;
    int sockfd, ret;
    if(argc < 2){
        perror("Requires input url to parse.");
        exit(EXIT_FAILURE);
    }
    //Start parseURL
    //parseURL(argv[1], parsed);
    int portNum, hostSplit;
    std::string host, port, token, path;//, service;
    std::string sURL(argv[1]);
    token = sURL.substr(sURL.find("www"));
    /*service = sURL.substr(0,sURL.find("://"));
    std::cout << service << std::endl;/**/
    hostSplit = token.find(":");

    if(hostSplit != std::string::npos){
        host = token.substr(0,hostSplit);
        port = token.substr(hostSplit+1);
        port = port.substr(0,port.find('/'));
        std::stringstream convert(port);
        if(!(convert >> portNum)){
            portNum=80;
        }
    }else{
        port = "80";
        host = token.substr(0,token.find('/'));
    }
    path = token.substr(token.find('/'));
    //std::cout << path << std::endl;
    parsed.host = host.c_str();
    parsed.path = path.c_str();
    parsed.port = port.c_str();
    //End parseURL

    strcpy(queryPath, parsed.path);
    strcpy(queryHost, parsed.host);
    strcpy(queryPort, parsed.port);
    //sockfd = connectToHost(parsed, first);
    //Begin connect to host
    int lenHost, lenPort;
    char *host_c, *port_c;
    struct addrinfo hints, *i;
    lenHost = strlen(parsed.host);
    lenPort = strlen(parsed.port);
    host_c = new char[lenHost+1]();
    port_c = new char[lenPort+1]();
    strncpy(host_c, parsed.host, lenHost);
    strncpy(port_c, parsed.port, lenPort);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    ///Initialize & Connect TCP Socket
    std::cout << host_c << ", " << port_c << std::endl;
    ret = getaddrinfo(host_c, port_c, &hints, &first);
    if(ret != 0){
        fprintf(stderr, "getaddrinfo() failed: %s\n", gai_strerror(ret));
        exit(1);
    }
    //std::cout << "getaddr success" << std::cout;
    for(i=first; i!= NULL; i=i->ai_next){
        if((sockfd = socket(i->ai_family, SOCK_DGRAM, 0)) == -1 ){
            perror("socket");
            continue;
        }
        bind(sockfd, (struct sockaddr *) i, sizeof(*i));
        if( connect( sockfd, (i->ai_addr), i->ai_addrlen) == -1){
            std::cout << "Connect Failure" << std::endl;
            close(sockfd);
            perror("connect");
            continue;
        }
        break;
    }
/*
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    getpeername(sockfd, (struct sockaddr *)&addr, &addrlen);
    printf("%s\n", inet_ntoa(addr.sin_addr));/**/
    //end connect to host
    /*if(sockfd == -1){
        close(sockfd);
        exit(EXIT_FAILURE);
    }/**/

    //sendRequest(queryPath, queryHost, queryPort, sockfd);
    ///Begin sendRequest
    char query_c[1024];
    sprintf(query_c, "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: dowerc-netprog-hw/1.0\r\n\r\n", parsed.path, queryHost);
    if(send(sockfd, query_c, sizeof(query_c), 0) < 0){
        std::cout << "Not written: " << query_c << std::endl;
    }else{std::cout << "Sent:\n" << query_c << std::endl;}
    ///End sendRequest
    //listenResponse(sockfd);
    ///Begin listenResponse
    char buf[1024];
    char * headerEnd;
    int len = 0, flags = 0;
    int total = 2147483647; //INTMAX

    std::cout << "Recieved: " << std::endl;
    while( (len = recv(sockfd, buf, sizeof(buf)-1, 0)) > 0 && total > 0){
        std::cout << len << ", " << total << std::endl;
        buf[len] = '\0';
        headerEnd = strstr(buf, "\r\n\r\n");
        if(headerEnd != NULL && flags == 0){
            //get headers to properrly set stderrr
            int headerSize = headerEnd - buf;
            char headers[headerSize + 5];
            strncpy(headers, buf, headerSize);
            snprintf(headers+headerSize, 5, "\r\n\r\n");
            headers[headerSize+4] = '\0';
            fwrite(headers, 1, sizeof(headers), stderr);

            //
            char* contentSize = strstr(headers, "Content-Size: ");
            if(contentSize == NULL){ exit(1);}
            contentSize = contentSize + strlen("Content=Size: ");
            char * rnIndex = strstr(contentSize, "\r\n");
            char length[rnIndex - contentSize + 1];
            strncpy(length, contentSize, sizeof(length));
            total = atoi(length);

            //
            fwrite(headerEnd+4, 1, len-headerSize-4, stdout);
            fflush(stdout);
            std::cout << headerSize << std::endl;
            total = total-len;
            flags = 1;
        }
        else{
        //
            fwrite(buf, 1, len, stdout);
            fflush(stdout);
            total=total-len;
        }
        memset(buf, 0, len);
    }
    std::cout << len << ", " << total << std::endl;
    ///End listenResponse
    fclose(stdout);
    close(sockfd);

    return EXIT_SUCCESS;
}
