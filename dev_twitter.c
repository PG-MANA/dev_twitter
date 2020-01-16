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
#define MAX_TWEET_TEXT 1024

struct tweet_buffer {
  char buf[MAX_TWEET_TEXT];
  unsigned int pointer;
};

static int open(struct inode *inode, struct file *file) {
  struct tweet_buffer *buf;

  buf = kcalloc(1, sizeof(struct tweet_buffer), GFP_KERNEL);
  if (buf == NULL) {
    printk(KERN_WARNING "Cannot alloc memory\n");
    return -ENOMEM;
  }
  file->private_data = buf;
  return 0;
}

static int release(struct inode *inode, struct file *file) {

  if (file->private_data) {
    struct tweet_buffer *tweet_buf;

    tweet_buf = file->private_data;
    char *argv[] = {"/usr/local/bin/usptomo-tweet", tweet_buf->buf, NULL};
    char *envp[] = {"HOME=/", "TERM=linux",
                    "PATH=/sbin:/usr/sbin:/bin:/usr/bin:/usr/local/bin", NULL};
    printk("%s", tweet_buf->buf);
    if (call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC)) {
      printk(KERN_WARNING "Cannot tweet text.\n");
    }
    kfree(file->private_data);
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
  struct tweet_buffer *tweet_buf;

  tweet_buf = file->private_data;
  if (count + tweet_buf->pointer > MAX_TWEET_TEXT) {
    printk(KERN_WARNING "Buffer was overflowed.");
    return -EFAULT;
  }
  if (raw_copy_from_user(tweet_buf->buf + tweet_buf->pointer, buf, count) !=
      0) {
    return -EFAULT;
  }
  tweet_buf->pointer += count;
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
MODULE_LICENSE("Apache-2.0");
