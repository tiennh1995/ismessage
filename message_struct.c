#define BUFFER_LENGTH 256
#define MESSAGE_SIZE 260

typedef struct message {
  char data[BUFFER_LENGTH];
  int key;
} Message;
