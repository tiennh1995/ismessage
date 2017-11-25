#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#if CONFIG_MODVERSIONS==1
#define MODVERSIONS
#include <linux/modversions.h>
#endif
#include <linux/fs.h>
#include <linux/wrapper.h>
#include "chardev.h"
#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) ((a)*65536+(b)*256+(c))
#endif
#include <asm/uaccess.h>  /* for get_user and put_user */
#define SUCCESS 0

#define DEVICE_NAME "char_dev"
#define BUF_LEN 80
static int Device_Open = 0;
static char Message[BUF_LEN];
static char *Message_Ptr;

static int device_open(struct inode *inode, struct file *file) {
  if (Device_Open)
    return -EBUSY;
  Device_Open++;
  Message_Ptr = Message;
  MOD_INC_USE_COUNT;
  return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) {
  Device_Open --;
  MOD_DEC_USE_COUNT;
  return 0;
}

static ssize_t device_read(struct file *file, char *buffer, size_t length, loff_t *offset) {
  int bytes_read = 0;
  if (*Message_Ptr == 0)
    return 0;
  while (length && *Message_Ptr)  {
    put_user(*(Message_Ptr++), buffer++);
    length --;
    bytes_read ++;
  }

  return bytes_read;
}

static ssize_t device_write(struct file *file, const char *buffer, size_t length, loff_t *offset) {
  int i;
  for(i=0; i<length && i<BUF_LEN; i++)
      get_user(Message[i], buffer+i);
  Message_Ptr = Message;
  return i;
}

int device_ioctl(struct inode *inode, struct file *file, unsigned int ioctl_num, unsigned long ioctl_param) {
  int i;
  char *temp;
  char ch;

  switch (ioctl_num) {
    case IOCTL_SET_MSG:
      temp = (char *) ioctl_param;
      get_user(ch, temp);
      for (i=0; ch && i<BUF_LEN; i++, temp++)
        get_user(ch, temp);
      device_write(file, (char *) ioctl_param, i, 0);
      break;
    case IOCTL_GET_MSG:
      i = device_read(file, (char *) ioctl_param, 99, 0);
      put_user('\0', (char *) ioctl_param+i);
      break;
    case IOCTL_GET_NTH_BYTE:
      return Message[ioctl_param];
      break;
  }
  return SUCCESS;
}

struct file_operations Fops = {
  NULL,   /* seek */
  device_read,
  device_write,
  NULL,   /* readdir */
  NULL,   /* select */
  device_ioctl,   /* ioctl */
  NULL,   /* mmap */
  device_open,
  NULL,  /* flush */
  device_release  /* a.k.a. close */
};

int init_module()
{
  int ret_val;
  ret_val = module_register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops);
  if (ret_val < 0) {
    printk ("%s failed with %d\n", "Sorry, registering the character device ", ret_val);
    return ret_val;
  }
  return 0;
}

void cleanup_module()
{
  int ret;
  ret = module_unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
  if (ret < 0)
    printk("Error in module_unregister_chrdev: %d\n", ret);
}
