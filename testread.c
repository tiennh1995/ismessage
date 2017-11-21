#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include "message_struct.c"

static char receive[BUFFER_LENGTH];

int main(int argc, char *argv[]) {
  int ret, fd;
  if(argc == 2) {
    printf("Starting device test code example...\n");
    fd = open("/dev/ismessage", O_RDWR); // Open the device with read/write access
    if (fd < 0) {
      perror("Failed to open the device...");
      return errno;
    }

    Message *msg;
    msg = (Message*)malloc(sizeof(Message));
    printf("Reading from the device...\n");
    strcpy(receive, argv[1]);
    ret = read(fd, receive, sizeof(Message));   // Read the response from the LKM

    if (ret < 0){
      perror("Failed to read the message from the device.");
      return errno;
    }

    msg = (Message*)receive;
    printf("The received message is: [%s]\n", msg->data);
    printf("The received message is: [%d]\n", msg->key);
    printf("End of the program\n");
  } else {
    printf("please enter parameter [key]...\n");
  }
  return 0;
}
