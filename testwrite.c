#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#include "message_struct.c"

static char receive[BUFFER_LENGTH];

int main() {
  int ret, fd;
  printf("Starting device test code example...\n");
  fd = open("/dev/ismessage", O_RDWR); //Open the device with read/write access
  if (fd < 0) {
    perror("Failed to open the device...");
    return errno;
  }

  Message *msg;
  msg = (Message*)malloc(sizeof(Message));
  msg->key = 10;
  printf("Type in a short string to send to the kernel module:\n");
  scanf("%[^\n]%*c", msg->data);                // Read in a string (with spaces)
  ret = write(fd, (char*)msg, sizeof(Message)); // Send the string to the LKM
  if (ret < 0){
    perror("Failed to write the message to the device.");
    return errno;
  }
  return 0;
}
