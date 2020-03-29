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
#define MAX_TWEET_LENGTH 140
#define TWEET_BUFFER_SIZE MAX_TWEET_LENGTH * 6

struct tweet_buffer {
  char buffer[TWEET_BUFFER_SIZE + 1 /*For \0*/];
  unsigned int pointer;
};

static int tweet(char *text) {
  char *argv[] = {"/usr/local/bin/usptomo-tweet", text, NULL};
  char *envp[] = {"HOME=/", "TERM=linux",
                  "PATH=/sbin:/usr/sbin:/bin:/usr/bin:/usr/local/bin", NULL};
  if (call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC)) {
    printk(KERN_WARNING "Cannot tweet text.\n");
    return 1;
  }
  return 0;
}

static int open(struct inode *inode, struct file *file) {
  struct tweet_buffer *tw_buf;

  tw_buf = kmalloc(sizeof(struct tweet_buffer), GFP_KERNEL);
  if (tw_buf == NULL) {
    printk(KERN_WARNING "Cannot alloc memory.\n");
    return -ENOMEM;
  }
  tw_buf->pointer = 0;
  file->private_data = tw_buf;
  return 0;
}

static int release(struct inode *inode, struct file *file) {
  struct tweet_buffer *tw_buf;

  tw_buf = file->private_data;
  if (tw_buf) {
    if (tw_buf->pointer > 0) {
      tw_buf->buffer[tw_buf->pointer] = '\0';
      tweet(tw_buf->buffer);
    }
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
  unsigned int read_size;
  unsigned int text_pointer, text_counter;
  char c_buf;
  unsigned int processed_count;

  tw_buf = file->private_data;
  if (tw_buf) {
    processed_count = 0;
    while (count > processed_count) {
      if (count > TWEET_BUFFER_SIZE - tw_buf->pointer) {
        read_size = TWEET_BUFFER_SIZE - tw_buf->pointer;
      } else {
        read_size = count;
      }
      if (raw_copy_from_user(tw_buf->buffer + tw_buf->pointer,
                             buf + processed_count, read_size) != 0) {
        printk(KERN_WARNING "Cannout copy data from buffer.\n");
        return -EFAULT;
      }
      tw_buf->buffer[tw_buf->pointer + read_size] = '\0';
      for (text_counter = 0, text_pointer = 0;
           text_pointer < tw_buf->pointer + read_size; text_pointer++) {
        if (!((unsigned char)tw_buf->buffer[text_pointer] >= 0x80 &&
              0xBF >= (unsigned char)tw_buf->buffer[text_pointer])) {
          text_counter++;
          if (text_counter > MAX_TWEET_LENGTH) {
            c_buf = tw_buf->buffer[text_pointer];
            tw_buf->buffer[text_pointer] = '\0';
            if (tweet(tw_buf->buffer)) {
              return -EFAULT;
            }
            tw_buf->buffer[text_pointer] = c_buf;
            break;
          }
        }
      }
      if (text_counter == MAX_TWEET_LENGTH) {
        if (tweet(tw_buf->buffer)) {
          return -EFAULT;
        }
        tw_buf->pointer = 0;
      } else if (text_counter > MAX_TWEET_LENGTH) {
        memcpy(tw_buf->buffer, tw_buf->buffer + text_pointer,
               tw_buf->pointer + read_size - text_pointer);
        tw_buf->pointer =
            tw_buf->pointer + read_size - text_pointer; // is it ok?
      } else {
        tw_buf->pointer += read_size;
      }
      processed_count += read_size;
    }
  } else {
    printk(KERN_WARNING "Cannot get tweet buffer.\n");
    return -EFAULT;
  }
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
