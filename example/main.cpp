#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <string> 
using namespace std;

int main(){
    const char* file_addr = "../doc/picture.html";
    struct stat file_stat;
    stat(file_addr, &file_stat);
    printf("file_stat.st_size:%d\n",file_stat.st_size);
    int fd = open(file_addr, O_RDONLY);
    char buf[1024];
    int readcnt = read(fd, buf, file_stat.st_size);
    std::cout << buf << "\n";
    printf("fd : %d\n", fd);
    close(fd);
}