#define BUFFER_LENGTH 256

typedef struct message {
  char data[BUFFER_LENGTH];
  int key;
} Message;
