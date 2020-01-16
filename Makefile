obj-m := dev_twitter.o

RM := rm
CP := cp
CHMOD := chmod
MKNOD := mknod
INSMOD := insmod
RMMOD := rmmod

defalut:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

load:
	$(INSMOD) dev_twitter.ko
	$(MKNOD) /dev/twitter c 62 1
	$(CHMOD) 222 /dev/twitter
	$(CP) usptomo-tweet /usr/local/bin/
	$(CHMOD) +x /usr/local/bin/usptomo-tweet
	$(CP) key /etc/dev_twitter
unload:
	$(RMMOD) dev_twitter.ko
	$(RM) -f /dev/twitter
	$(RM) -f /usr/local/usptomo-tweet
	$(RM) -f /etc/dev_twitter
clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
