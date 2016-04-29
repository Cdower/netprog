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
  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();
  BIO *bio, *out;
  SSL_CTX *ctx;
  SSL *ssl;
  url_p parsed;
  char hostPort[512], request[1024], buf[1024];
  int len;
  size_t hostSplit;
  string host, port, token, path="", service;
  string sURL(argv[1]);
  if(argc < 2){
    perror("Requires input url to parse.");
    exit(EXIT_FAILURE);
  }
  //Start parseURL
  service = sURL.substr(0,sURL.find("://"));
  hostSplit = sURL.find("www");
  hostSplit!= string::npos ? (token=sURL.substr(hostSplit)) : (token = token);
  hostSplit = 0;
  hostSplit = token.find(":");

  if(hostSplit != string::npos){
    host = token.substr(0,hostSplit);
    port = token.substr(hostSplit+1);
    (hostSplit=port.find('/')) != string::npos ? (port = port.substr(0,hostSplit) ) : (port = port);
    if(port.compare("443")!=0){
      parsed.secure = true;
    }else { parsed.secure = false; }
  }else{
    hostSplit = token.find('/');
    hostSplit!=string::npos ? host = token.substr(0,hostSplit) : host = token;
    if(service.compare("https")==0){
      port="443";
      parsed.secure = true;
    }else{
      port="80";
      parsed.secure = false;
    }
  }
  (hostSplit=token.find('/')) != string::npos ? path = token.substr(hostSplit) : path=path;
  parsed.host = host.c_str();
  parsed.path = path.c_str();
  parsed.port = port.c_str();
  sprintf(request, "GET %s HTTP/1.1\nHost: %s:%s\nUser-Agent: dowerc-netprog-hw3/1.0\n\n", parsed.path, parsed.host, parsed.port);
  //End parseURL
  ///Intialize OpenSSL

  if(parsed.secure){ //do SSL
    strcpy(hostPort, (host + ":" + port).c_str());
    ctx = SSL_CTX_new(SSLv23_client_method());
    bio = BIO_new_ssl_connect(ctx);
    BIO_get_ssl(bio, &ssl);
    if(!ssl) { fprintf(stderr, "Can't locate SSL pointer\n"); }
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
    BIO_set_conn_hostname(bio, hostPort);
    out = BIO_new_fp(stdout, BIO_CLOSE);
    if(BIO_do_connect(bio) <= 0){
      fprintf(stderr, "Error connecting to server\n");
       ERR_print_errors_fp(stderr);
    }
    if(BIO_do_handshake(bio) <= 0){
      fprintf(stderr, "Error establishing SSL connection\n");
      ERR_print_errors_fp(stderr);
    }
    cout << request << endl;
    BIO_puts(bio, request);
    for(;;){
      len = BIO_read(bio, buf, 1024);
      if(len>0){
        BIO_write(out, buf, len);
      }else{
        cout << "\n\nLEN:: " << len << endl;
        break;
      }
    }
    BIO_free_all(bio);
    BIO_free(out);
  }else{ // do regular connect
    strcpy(hostPort, (host + ":" + port).c_str());
    bio = BIO_new_connect( hostPort );
    out = BIO_new_fp(stdout, BIO_CLOSE);
    if(BIO_do_connect(bio) <= 0){
      fprintf(stderr, "Error connecting to server\n");
      ERR_print_errors_fp(stderr);
    }
    cout << request << endl;
    BIO_puts(bio, request);
    for(;;){
      len = BIO_read(bio, buf, 1024);
      if(len>0){
        BIO_write(out, buf, len);
      }else{
        cout << "\n\nLEN:: " << len << endl;
        break;
      }
    }
    BIO_free_all(bio);
    BIO_free(out);
  }
  return EXIT_SUCCESS;
}
