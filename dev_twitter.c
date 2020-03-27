#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <asm/current.h>
#include <asm/uaccess.h>
#include <linux/kmod.h>

#define DRIVER_NAME "DEV_TWITTER"
#define DRIVER_MAJOR 62
#define INITAL_TWEET_BUFFER_SIZE 1024
#define MAX_TWEET_LENGTH 140

struct tweet_buffer {
  char *buffer;
  unsigned int buffer_size;
  unsigned int pointer;
};

static int open(struct inode *inode, struct file *file) {
  struct tweet_buffer *tw_buf;

  tw_buf = kmalloc(sizeof(struct tweet_buffer), GFP_KERNEL);
  if (tw_buf == NULL) {
    printk(KERN_WARNING "Cannot alloc memory.\n");
    return -ENOMEM;
  }
  tw_buf->pointer = 0;
  tw_buf->buffer =
      kmalloc_array(INITAL_TWEET_BUFFER_SIZE, sizeof(char), GFP_KERNEL);
  if (tw_buf->buffer == NULL) {
    printk(KERN_WARNING "Cannot alloc memory.\n");
    kfree(tw_buf);
    return -ENOMEM;
  }
  tw_buf->buffer_size = INITAL_TWEET_BUFFER_SIZE;
  file->private_data = tw_buf;
  return 0;
}

static int release(struct inode *inode, struct file *file) {

  if (file->private_data) {
    struct tweet_buffer *tw_buf;
    unsigned int text_counter, pointer, start_pointer;
    char c_buf;

    tw_buf = file->private_data;
    tw_buf->buffer[tw_buf->pointer] = '\0';
    pointer = 0;
    start_pointer = 0;
    c_buf = 0;
    while (tw_buf->buffer[pointer] != '\0') {
      for (text_counter = 0;; pointer++) {
        if (tw_buf->pointer <= pointer) {
          pointer = tw_buf->pointer;
          c_buf = '\0';
          tw_buf->buffer[pointer] = c_buf;
          break;
        }
        if (!((unsigned char)tw_buf->buffer[pointer] >= 0x80 &&
              0xBF >= (unsigned char)tw_buf->buffer[pointer])) {
          text_counter++;
          if (text_counter > MAX_TWEET_LENGTH) {
            c_buf = tw_buf->buffer[pointer];
            tw_buf->buffer[pointer] = '\0';
            break;
          }
        }
      }
      char *argv[] = {"/usr/local/bin/usptomo-tweet",
                      tw_buf->buffer + start_pointer, NULL};
      char *envp[] = {"HOME=/", "TERM=linux",
                      "PATH=/sbin:/usr/sbin:/bin:/usr/bin:/usr/local/bin",
                      NULL};
      if (call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC)) {
        printk(KERN_WARNING "Cannot tweet text.\n");
        break;
      }
      //後片付け
      start_pointer = pointer;
      tw_buf->buffer[pointer] = c_buf;
    }

    kfree(tw_buf->buffer);
    kfree(tw_buf);
    file->private_data = NULL;
  }
  return 0;
}

static ssize_t dummy_read(struct file *file, char __user *buf, size_t count,
                          loff_t *f_pos) {
  return 0;
}

static ssize_t write(struct file *file, const char __user *buf, size_t count,
                     loff_t *f_pos) {
  struct tweet_buffer *tw_buf;
  char *new_buffer;
  unsigned int new_buffer_size;

  tw_buf = file->private_data;
  if (count + tw_buf->pointer >= tw_buf->buffer_size) {
    new_buffer_size = count + tw_buf->pointer + INITAL_TWEET_BUFFER_SIZE;
    new_buffer = krealloc(tw_buf->buffer, new_buffer_size, GFP_KERNEL);
    if (new_buffer == NULL) {
      printk(KERN_WARNING "Cannout alloc Buffer.\n");
      return -EFAULT;
    }
    tw_buf->buffer = new_buffer;
    tw_buf->buffer_size = new_buffer_size;
  }
  if (raw_copy_from_user(tw_buf->buffer + tw_buf->pointer, buf, count) != 0) {
    return -EFAULT;
  }
  tw_buf->pointer += count;
  return count;
}

struct file_operations fops = {
    .open = open,
    .release = release,
    .read = dummy_read,
    .write = write,
};

static int dev_twitter_init(void) {
  printk(KERN_INFO "Starting dev_twitter\n");
  register_chrdev(DRIVER_MAJOR, DRIVER_NAME, &fops);
  return 0;
}

static void dev_twitter_exit(void) {
  printk(KERN_INFO "Removing dev_twitter\n");
  unregister_chrdev(DRIVER_MAJOR, DRIVER_NAME);
}

module_init(dev_twitter_init);
module_exit(dev_twitter_exit);
MODULE_LICENSE("MIT");
