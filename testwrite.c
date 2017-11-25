#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#include "chardev.h"
#include "message_struct.c"

static char receive[BUFFER_LENGTH];
void ioctl_write_msg(int, char*);

int main(int argc, char *argv[]) {
  int ret, fd;
  if(argc == 2) {
    printf("Starting device test code example...\n");
    fd = open("/dev/ismessage", O_RDWR); //Open the device with read/write access
    if (fd < 0) {
      perror("Failed to open the device...");
      return errno;
    }

    Message *msg;
    msg = (Message*)malloc(sizeof(Message));
    msg->key = atoi(argv[1]);
    printf("Type in a short string to send to the kernel module:\n");
    scanf("%[^\n]%*c", msg->data);                // Read in a string (with spaces)

    ioctl_write_msg(fd, (char*)msg);

    // ret = write(fd, (char*)msg, sizeof(Message)); // Send the string to the LKM
    // if (ret < 0){
    //   perror("Failed to write the message to the device.");
    //   return errno;
    // }
  } else {
    printf("please enter parameter [key]...\n");
  }
  return 0;
}

void ioctl_write_msg(int file_desc, char *message) {
  int ret_val;
  printf("file_desc: %d\n", file_desc);
  printf("file_desc: %d\n", IOCTL_SET_MSG);
  printf("file_desc: %s\n", message);
  ret_val = ioctl(file_desc, IOCTL_SET_MSG, message);
  if (ret_val < 0) {
    printf ("ioctl_set_msg failed:%d\n", ret_val);
    exit(-1);
  }
}
