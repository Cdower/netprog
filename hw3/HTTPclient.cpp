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

using namespace std;

struct url_p{
    const char * host;
    const char * port;
    const char * path;
    //const char * service;
};

int parseURL(char * URL, url_p& parsed){
    int hostSplit;
    string host, port, token, path;//, service;
    string sURL(URL);
    token = sURL.substr(sURL.find("www"));
    /*service = sURL.substr(0,sURL.find("://"));
    cout << service << endl;/**/
    hostSplit = token.find(":");

    if(hostSplit != string::npos){
        host = token.substr(0,hostSplit);
        port = token.substr(hostSplit+1);
        port = port.substr(0,port.find('/'));
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
            cout << "Connect Failure" << endl;
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
    string query = "GET ";
    query.append(path);
    query.append(" HTTP/1.1\r\nHost: ");
    query.append(host); query.append(":"); query.append(port);
    query.append("\r\n");
    query.append("User-Agent: dowerc-netprog-hw3/1.0\r\n\r\n\0");
    if(send(sockfd, query.c_str(), sizeof(query.c_str()), 0) < 0){
        cout << "Not written: " << query << endl;
    }else{cout << "Sent:\n" << query << endl;}
    return 0;
}

int listenResponse(int sockfd){
    char buf[1024];
    char * headerEnd;
    int len, flags = 0;
    int total = 2147483647; //INTMAX

    cout << "Recieved: " << endl;
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
        cout << buf << endl;
        cout.write(buf, sizeof(len));
        cout << endl;
        memset(&buf[0], 0, sizeof(buf));
        recv(sockfd,buf, 255,0);
    }/**/
    cout << endl;
    return 0;
}

int main(int argc, char * argv[]){
  url_p parsed;
  char queryPath[512], queryHost[256], queryPort[20];
  struct addrinfo *first, hints, *i;
  int sockfd, ret;
  if(argc < 2){
    perror("Requires input url to parse.");
    exit(EXIT_FAILURE);
  }
  //Start parseURL
  //parseURL(argv[1], parsed);
  size_t hostSplit;
  string host, port, token, path, service;
  string sURL(argv[1]);

  service = sURL.substr(0,sURL.find("://"));
  hostSplit = sURL.find("www");
  hostSplit!= string::npos ? (token=sURL.substr(hostSplit)) : (token = token);
  hostSplit = 0;
  hostSplit = token.find(":");

  if(hostSplit != string::npos){
    host = token.substr(0,hostSplit);
    port = token.substr(hostSplit+1);
    hostSplit=port.find('/');
    hostSplit != string::npos ? (port = port.substr(0,hostSplit) ) : (port = port) ;
  }else{
    hostSplit = token.find('/');
    hostSplit!=string::npos ? host = token.substr(0,hostSplit) : host = token;
    service.compare("https")==0 ? port="443" : port="80";
    //true ? cout << "True" << endl : cout << "false" << endl;
    //cout <<  host << endl;
    //host = token.substr(0,hostSplit);
  }
  hostSplit=token.find('/');
  hostSplit!=string::npos ? path = token.substr(hostSplit) : path="";
  //cout << path << endl;
  parsed.host = host.c_str();
  parsed.path = path.c_str();
  parsed.port = port.c_str();
  //End parseURL

  strcpy(queryPath, parsed.path);
  strcpy(queryHost, parsed.host);
  strcpy(queryPort, parsed.port);
  //sockfd = connectToHost(parsed, first);
  //Begin connect to host
  int lenHost = strlen(parsed.host), lenPort = strlen(parsed.port);
  char *host_c  = new char[lenHost+1](), *port_c = new char[lenPort+1]();
  strncpy(host_c, parsed.host, lenHost);
  strncpy(port_c, parsed.port, lenPort);
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  ///Initialize & Connect TCP Socket
  if(ret = getaddrinfo(host_c, port_c, &hints, &first) != 0){
      fprintf(stderr, "getaddrinfo() failed: %s\n", gai_strerror(ret));
      exit(1);
  }
  //cout << "getaddr success" << cout;
  for(i=first; i!= NULL; i=i->ai_next){
    if((sockfd = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == -1 ){
      perror("socket");
      continue;
    }
    bind(sockfd, (struct sockaddr *) &i, sizeof(i));
    if( connect( sockfd, (i->ai_addr), i->ai_addrlen) == -1){
      cout << "Connect Failure" << endl;
      close(sockfd);
      perror("connect");
      continue;
    }
    break;
  }
/*    struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);
  getpeername(sockfd, (struct sockaddr *)&addr, &addrlen);
  printf("%s\n", inet_ntoa(addr.sin_addr));
  //end connect to host
  if(sockfd == -1){
      close(sockfd);
      exit(EXIT_FAILURE);
  }/**/

  //sendRequest(queryPath, queryHost, queryPort, sockfd);
  ///Begin sendRequest to build and send GET request
  char request[1024];
  sprintf(request, "GET %s HTTP/1.1\r\nHost: %s:%s\r\nUser-Agent: dowerc-netprog-hw3/1.0\r\n\r\n", parsed.path, queryHost, parsed.port);
  if(send(sockfd, request, sizeof(request), 0) < 0){
    cout << "Not written: " << request << endl;
  }else{cout << "Sent:\n" << request << endl;}
  ///End sendRequest
  ///Begin listenResponse//listenResponse(sockfd);
  char buf[1024];
  char * headerEnd = NULL;
  int len, flags = 0;
  int total = 2147483647; //INTMAX

  cout << "Recieved: " << endl;
  while( (len = recv(sockfd, buf, sizeof(buf)-1, 0)) > 0 && total > 0){
    buf[len] = '\0';
    cout << buf << endl;
    headerEnd = strstr(buf, "\r\n\r\n");/*
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
      if(contentSize == NULL) { exit(1); }
      contentSize = contentSize + strlen("Content-Size: ");
      cout << "Content-Length: " << contentSize << endl;
      char * rnIndex = strstr(contentSize, "\r\n");
      char length[rnIndex - contentSize + 1];
      strncpy(length, contentSize, sizeof(length));
      total = atoi(length);

      //
      fwrite(headerEnd+4, 1, len-headerSize-4, stdout);
      fflush(stdout);
      cout << headerSize << endl;
      total = total-len;
      flags = 1;
    }
    else{
      fwrite(buf, 1, len, stdout);
      fflush(stdout);
      total=total-len;
    }/**/
    memset(buf, 0, len);
  }/**/
  ///End listenResponse
  fclose(stdout);
  close(sockfd);

  return EXIT_SUCCESS;
}
