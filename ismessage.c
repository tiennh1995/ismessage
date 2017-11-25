#include <linux/init.h>             // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>           // Core header for loading LKMs into the kernel
#include <linux/device.h>           // Header to support the kernel Driver Model
#include <linux/kernel.h>           // Contains types, macros, functions for the kernel
#include <linux/fs.h>               // Header for the Linux file system support
#include <asm/uaccess.h>            // Required for the copy to user function
#include <linux/string.h>
#include "chardev.h"

#define  DEVICE_NAME "ismessage"    ///< The device will appear at /dev/ismessage using this value
#define  CLASS_NAME  "is"           ///< The device class -- this is a character device driver

#include <linux/string.h>
#include "message_struct.c"
#define MAX_LENGTH 50
#define IOCTL_WRITE_MSG 0
#define IOCTL_READ_MSG 1

MODULE_LICENSE("GPL");              ///< The license type -- this affects available functionality
MODULE_AUTHOR("IS TEAM");           ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A Linux char driver for the Message");  ///< The description -- see modinfo
MODULE_VERSION("0.1");              ///< A version number to inform users

static int majorNumber;
static short message_size = MESSAGE_SIZE; ///< Stores the device number -- determined automatically
static int numberOpens = 0;         ///< Counts the number of times the device is opened
static struct class* ismessageClass = NULL; ///< The device-driver class struct pointer
static struct device* ismessageDevice = NULL; ///< The device-driver device struct pointer
static char messages[MAX_LENGTH][MESSAGE_SIZE];
static int check[MAX_LENGTH];

// The prototype functions for the character driver -- must come before the struct definition
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static long dev_ioctl (struct file *, unsigned int, unsigned long);
int dev_find_valid_message(int);
int dev_copy_to_user(char *);
int find_0(void);

static struct file_operations fops = {
  .open = dev_open,
  .read = dev_read,
  .write = dev_write,
  .release = dev_release,
  .unlocked_ioctl = dev_ioctl,
};

// Khi bat dau dang ki thiet bi
static int __init ismessage_init(void) {
  printk(KERN_INFO "ismessage: Initializing the ismessage LKM\n");

  // Khoi tao tu dong 1 majorNumber
  majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
  if (majorNumber < 0) {
    printk(KERN_ALERT "ismessage failed to register a major number\n");
    return majorNumber;
  }
  printk(KERN_INFO "ismessage: registered correctly with major number %d\n",
    majorNumber);

  // Dang ki lop thiet bi
  ismessageClass = class_create(THIS_MODULE, CLASS_NAME);
  if (IS_ERR(ismessageClass)) {
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_ALERT "Failed to register device class\n");
    return PTR_ERR(ismessageClass);
  }
  printk(KERN_INFO "ismessage: device class registered correctly\n");

  // Dang ki driver cua thiet bi
  ismessageDevice = device_create(ismessageClass, NULL, MKDEV(majorNumber, 0),
    NULL, DEVICE_NAME);
  if (IS_ERR(ismessageDevice)) {
    class_destroy(ismessageClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_ALERT "Failed to create the device\n");
    return PTR_ERR(ismessageDevice);
  }

  printk(KERN_INFO "ismessage: device class created correctly\n");
  return 0;
}

// Khi go bo dang ki thiet bi
static void __exit ismessage_exit(void) {
  device_destroy(ismessageClass, MKDEV(majorNumber, 0)); //Go bo thiet bi
  class_unregister(ismessageClass);                      // Huy dang ki lop thiet bi
  class_destroy(ismessageClass);                         // Xoa lop thiet bi
  unregister_chrdev(majorNumber, DEVICE_NAME);           // Huy dang ki majorNumber
  printk(KERN_INFO "ismessage: Goodbye from the LKM!\n");
}

// Khi co yeu cau mo thiet bi
static int dev_open(struct inode *inodep, struct file *filep) {
  numberOpens++;
  printk(KERN_INFO "ismessage: Device has been opened %d time(s)\n",
    numberOpens);
  return 0;
}

static long dev_ioctl(struct file *filep, unsigned int ioctl_num, unsigned long ioctl_param) {
  switch(ioctl_num) {
    case IOCTL_SET_MSG:
      if(dev_write(filep, (char*)ioctl_param, sizeof(Message), 0) < 0) {
        return -1;
      }
      break;
    case IOCTL_GET_MSG:
      if(dev_read(filep, (char*)ioctl_param, sizeof(Message), 0) < 0) {
        return -1;
      }
      break;
  }
  return 0;
}

// Khi co yeu cau doc thiet bi
static ssize_t dev_read(struct file *filep, char *buffer, size_t len,
  loff_t *offset) {
  int error_count = 0;
  error_count = dev_copy_to_user(buffer);

  if(error_count == 0) {            // if true then have success
    printk(KERN_INFO "ismessage: Sent %d characters to the user\n",message_size);
  } else {
    printk(KERN_INFO "ismessage: Failed to send %d characters to the user\n",
      error_count);
    return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
  }
}

// Khi co yeu cau ghi vao thiet bi
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len,
  loff_t *offset) {
  int i = find_0();
  if(i == -1) return -1;
  copy_from_user(messages[i], buffer, len);
  check[i] = 1;
  printk(KERN_INFO "ismessage: Received %zu characters from the user\n", len);
  return len;
}

// Khi chuong trinh UD ket thuc hoac giai phong
static int dev_release(struct inode *inodep, struct file *filep) {
  printk(KERN_INFO "ismessage: Device successfully closed\n");
  return 0;
}

// Tim vi tri trong trong messages
int find_0() {
  int i;
  for(i = 0; i < MAX_LENGTH; i++) {
    if(check[i] == 0) return i;
  }
  return -1;
}

// Tim kiem vi tri message thich hop de tra ve
int dev_find_valid_message(int key) {
  int i;
  Message* msg;
  for(i = 0; i < MAX_LENGTH; i++) {
    if(check[i] == 1) {
      msg = (Message*) messages[i];
      if(msg->key == key) {
        return i;
      }
    }
  }
  return -1;
}

// Ho tro viec copy tu kernel space sang vung user space
int dev_copy_to_user(char* buffer) {
  int error_count = 0;
  int key = simple_strtol(buffer, NULL, key);
  int index = dev_find_valid_message(key);
  if(index != -1) {
    error_count = copy_to_user(buffer, messages[index], message_size);
    check[index] = 0;
  } else {
    return -EFAULT;
  }
  return error_count;
}

// module_exit and module_init
module_init(ismessage_init);
module_exit(ismessage_exit);
