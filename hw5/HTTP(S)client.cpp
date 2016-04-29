#include <cstring>
#include <string>
#include <iostream>
#include <stdio.h>
//#include <cstdlib>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
/* Open SSL headers */
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

using namespace std;

struct url_p{
    const char * host;
    const char * port;
    const char * path;
    bool secure;
};

int main(int argc, char * argv[]){
  url_p parsed;
  char queryHost[256];
  struct addrinfo *first, hints, *i;
  int sockfd, ret;
  if(argc < 2){
    perror("Requires input url to parse.");
    exit(EXIT_FAILURE);
  }
  //Start parseURL
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
    (hostSplit=port.find('/')) != string::npos ? (port = port.substr(0,hostSplit) ) : (port = port) ;
  }else{
    hostSplit = token.find('/');
    hostSplit!=string::npos ? host = token.substr(0,hostSplit) : host = token;
    if(service.compare("https")==0){
      port="443";
      parsed.secure = true;
      SSL_library_init();
      SSL_load_error_strings();
    }else{
      port="80";
      parsed.secure = false;
    }
  }
  (hostSplit=token.find('/')) != string::npos ? path = token.substr(hostSplit) : path="";

  parsed.host = host.c_str();
  parsed.path = path.c_str();
  parsed.port = port.c_str();
  strcpy(queryHost, parsed.host);
  //End parseURL
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
  int len, total = 2147483647; //INTMAX

  cout << "Recieved: " << endl;
  while( (len = recv(sockfd, buf, sizeof(buf)-1, 0)) > 0 && total > 0){
    buf[len] = '\0';
    cout << buf << endl;
    memset(buf, 0, len);
  }
  fclose(stdout);
  close(sockfd);

  return EXIT_SUCCESS;
}
