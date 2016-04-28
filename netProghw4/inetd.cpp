//Chris Dower
//Network Progamming HW 4

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <arpa/inet.h>
#include <map>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

using namespace std;
//Need to connect to socket.
//connect( sockfd, (i->ai_addr), i->ai_addrlen)
/*int startOnTCP(string input, fd_set set, int fd_max){
  int ret, i, fd;
  ret = select(fd_max+1, &set, &set, NULL, NULL);
  if(ret >0){
    for(i=0;i<fd_max+1;i++){
      if(FD_ISSET(i, &set)){
        pid_t pid;
        fork();
        pid = getpid();
        if(pid == 0){
          fd = i;
          dup2(fd, STDIN_FILENO);
          dup2(fd, STDOUT_FILENO);
          cout << "Input: " << input << endl;
          execl("/bin/sh", "/bin/sh", "-c", input.c_str(), NULL);
        }
      }
    }
  }
  return 0;
}
*/
int main(){
  map<int, struct addrinfo*> fdaddr;
  string input, port;
  pid_t pid;
  int fd, split, ret, i,  newFD, fd_max = 0;
  fd_set set;
  FD_ZERO(&set);
  socklen_t client_size;
  struct addrinfo *sock_con, *it;
  struct sockaddr_in client_addr;
  ifstream config("config.txt");

  while(1){
    if(config.is_open()){
      label:
      if(getline(config, input) < 0){
        config.close();
      }
      if(input.size() <= 0){
        goto label;
      }
      cout << input << endl;
    }else{ getline(cin, input); }
    split = input.find(" ");
    port = input.substr(0,split);
    input = input.substr(split+1);
    getaddrinfo("localhost", port.c_str(), NULL, &sock_con);
    for(it=sock_con; it!=NULL;it=it->ai_next){
      if( (fd = socket(it->ai_family, SOCK_STREAM, 0)) != -1){
        if(bind(fd,(it->ai_addr), (it->ai_addrlen)) < 0){
          perror("Error on Binding");
        }else{
          FD_SET(fd, &set);
          listen(fd, 1);
          fdaddr[fd] = it;
          fd_max = fd;
          cout << "Socket Found: " << fd << endl;
        }
      }
    }

    ret = select(fd_max+1, &set, NULL, NULL, NULL);
    if(ret >0){
      for(i=0;i<fd_max+1;i++){
        if(FD_ISSET(i, &set)){
          //cout << "FD " << i << " of " << fd_max << " is available" << endl;
          client_size = sizeof(client_addr);
          newFD = accept4(i, (struct sockaddr *) &client_addr, &client_size, 0);
          if(newFD < 1){ perror("accept"); }
          pid = fork();
          //cout << "Fork Successful fd:" << i << " newFD:" << newFD << endl;
          if(pid == 0){
            dup2(newFD, STDIN_FILENO);
            dup2(newFD, STDOUT_FILENO);
            //cout << "Input: " << input << endl;
            execl("/bin/sh", "/bin/sh", "-c", input.c_str(), NULL);
            close(newFD);
            break;
          }
        }//else {cout << "FD " << i << " of " << fd_max << " is not available" << endl;}
      }
    }else{ perror("select: "); }
    input = ""; port = ""; FD_ZERO(&set); fd = 0;
  }
  return 0;
}
