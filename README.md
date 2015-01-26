# cntouch_driver
CNTouch driver for linux.


## Install

Install your kernel's header, for instance on Ubuntu/Mint type:

```
sudo apt-get install linux-headers-generic
```

Then, `make` and to load the driver: `sudo insmod ./cntouch.ko`.



## Note

This driver has been inspired from `usbmouse.c`.


