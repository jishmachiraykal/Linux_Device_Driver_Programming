### What is a device driver?

*  Device driver is a piece of code that congigures and manages device. Device driver know how to configure the device, sending data to the device and how to process the request that originate from the device. When the device driver is loaded into the ss such as Linux, it exposes the interface into the user space so that user applucation can communicate with the device. Without device driver OS application will not have clear picture of how to deal with a device

* There are different types of device drivers
    1. Character device: Ex: RTC, keyboard, sensor, serial port, parallel port etc..
    2. Storage devices: Ex: sdmmc, eeprom, flash, harddisk, nand flash, USB camera etc...
    3. Network devices: Ex: Ethernet, wifi, bluetooth

* Character drivers access the data from the device sequentially i.e., byte by byte(like a stream of characters) not as a chunk of data.

* Sophisticated buffering strategies and are not involved in char drivers. Because when we write 1 byte it directly goes to the device without any intermediate buffering, delayed writeback and dirty buffer management.

* Block character device(sdmmc,eeprom) which handles the data in chuck or block i.e., 512 bytes/1024 bytes. This is more complicated than char device because driver should implement the advanced buffering strategies to read and write to the block devices. Here disk caches are involved

* Device file are accessed as files in Unix/Linux system. Device file gets populated in /dev(udev does) directory during kernel boot time or device driver hot plug events. Using device file, user application and driver communicates with each other. Device files are managed as part of VFS(virtual file system) subsystem of the kernel

* Execute "ls -la /dev", Device files with crw-r--r-- are char device file and brw-rw---- are block device files

* Exercise: write a char driver to deal with a pseudo character device. The pseudo device is a memory buffer of some size(not using any h/w here).The driver what we write must support reading, writing and seeking to this device. Test the driver functionality by running the commands such as echo, cat, dd and by writing user level programs

* User application is nothing but open(/dev/rtc)/read(fd,)/write(fd,). The open system call of user application will now talk to open system call of kernel space where driver resides. Same applies to read and write system calls. Now device driver the send/receive data to the hw

* The connection from user application system call to device driver function is done with help of VFS. Driver has to be registered in the VFS

* Driver will be assigned with a device number let's say 4 and device file with 4:0. Here 4 is the number to communicate with the driver and 0 is the instance of device files(4:0=/dev/rtc0 4:1=/dev/rtc1 4:2=/dev/rtc2 4:3=/dev/rtc3). Collectively this number is called device number

* VFS gets the device number from where request is made and checks with the registered drivers and executes. If you do "ls -la /dev", there are 2 col, where we can see the device number(major & minor)

* To create device number, in the driver code we have to user kernel APIs and utilities. Creation shown below will be done in module initialization function while loading the module
```
// Create device number
alloc_chrdev_region();
// Make a char device registration with the VFS
cdev_init();
cdev_add();
// Create device files
class_create();
device_create();
```

* While unloading the module, created module should be deleted and we have to use the below methods
```
// Deleting device number
unregister_chrdev_region();
// Deleting the registration
cdev_del();
class_destroy();
// Deleting device files
device_destroy();
```

* When using these function appropriate header files should be used.

### Dynamically allocating char device numbers

* alloc_chrdev_region() will take 4 i/p shown below
```
dev_t *dev //which should be a pointer for first assigned number and dev_t is typedef data type for unsigned int 32
unsigned_baseminor //first minor number to be created
unsigned_count //count of minor numbers to be created
const *char name //name of associated device or number. This is not a device file name, just a name
```

* Driver number creation:
```
dev_t device_number;
alloc_chrdev_region(&device_number,0,7,"eeprom");
```

* Out of 32 bits in dev_t, 12 bits to store major number and 20 bits to store minor number. We can use the below macros to extract major and minor parts of dev_t variable.
```
dev_t device_number;
int minor_no=MINOR(device_number);
int major_no=MAJOR(device_number);
```

* These macros can be find in Linux/kdev_t.h

### Pseudo char driver implementation

* Go to ldd/custom_drivers, mkdir 002pseudo_char_driver and touch main.c. Also "cp ../001hello_world/Makefile ." Now make changes in main.c in this folder

### Character device registration

* To do a char device registration with the VFS, first we have to initialize a cdev structure
```
void cdev_init(struct cdev *cdev, const struct file_operations *fops); // *cdev is structure to initialize, *fops file operations for this device
```

* cdev_init is a Kernel API which is implemented in fs/char_dev.c

* Next is to add a char device to the Kernel VFS by cdev_add
```
int cdev_add(struct cdev *p, dev_t dev, unsigned count); // *p is cdev structure for the device, dev is first device number for which the device is responsible and unsigned count is number of consecutive minor numbers corresponding to this device
```

### Character driver file operation methods

* struct file_operations which can be found in include/linux/fs.h has support to llseek, read, write, open, release operation etc..

* When fd=open("/dev/pcd",O_RDWR);, control goes to VFS and from there to device driver files. In the Kernel space, we have struct inode, struct cdev, struct file_ops, struct file

* Each file should have its own inode object and it gets created for every func call.

* When open function is called, it calls do_sys_open which then calls do_flip_open where 'file' object allocation happens
```
open --> do_sys_open --> do_flip_open --> do_dentry_open --> chrdev_open --> our_driver_open_method
```

* Details about chrdev_open function can be found in root/fs/char_dev.c

* When device file gets created
```
1. create device file using cdev
2. inode object gets created in the memory and inode's i_rdev field is initialized with device number
3. inode object i_fop field is set to dummy default file operations(def_chr_fops)
```

