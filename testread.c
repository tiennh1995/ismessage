#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#include "message_struct.c"
#include "chardev.h"

static char receive[BUFFER_LENGTH];

void ioctl_read_msg(int, char*);

int main(int argc, char *argv[]) {
  int ret, fd;
  if (argc == 2) {
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

    ioctl_read_msg(fd, receive);

    msg = (Message*)receive;
    printf("The received message is: [%s]\n", msg->data);
    printf("The received message is: [%d]\n", msg->key);
    printf("End of the program\n");
  } else {
    printf("Please enter parameter [key]...\n");
    printf("Syntax error!\n");
  }
  return 0;
}

void ioctl_read_msg(int file_desc, char *message) {
  int ret_val;
  ret_val = ioctl(file_desc, IOCTL_GET_MSG, message);
  if (ret_val < 0) {
    printf ("ioctl_get_msg failed: %d\n", ret_val);
    exit(-1);
  }
}
