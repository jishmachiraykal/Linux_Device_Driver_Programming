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

* Open method:
```
int pcd_open(struct inode *inode, struct file *flip){
    return 0;
}
// return 0 if open is successful and negative error code if open fails
```

* When the close[close(fd)] is issued, VFS will release the file object. Release method will be trigged when all the open methods trigerred are closed i.e., when f_count field of the file object becomes 0. For every closure of open method f_count decrements and its prototype is given below:
```
int pcd_release(struct inode *inode, struct file *filp){

}
```

* In release method driver can do reverse function of the what open method had done. Basically need to leave the device in its default state, before the open call. Return 0 on success and negative error code on failure. Ex: timeout

* Read system call is read(fd,buff,20) and its prototype is shown below:
```
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos){

}
struct file *flip = pointer of file object
char __user *buff = Pointer of user buffer, optional macro which alerts the programmer that this is a user level pointer so cannot be trusted for direct level dereferencing
size_t count = read count given by user
loff_t *f_pos = pointer of current file position from which the read has to begin
```

* Write is system call used to write some data to the device. And its prototype is shown below:
```
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos){
return 0;
}
struct file *flip = pointer of file object
const char __user *buff = Pointer of user buffer
size_t count = written count given by user
loff_t *f_pos = pointer of current file position from which the write has to begin
```

* Write 'count' bytes into the device starting at the position 'f_pos'. Update the 'f_pos' by adding the number of bytes successfully written. Return the number of bytes successfully written and return approp error code(-ve) if any error

* Next sys call is llseek(llseek[fd,buff,20]) which is used to alter the current file position. It has nothing to do with read/write of a device. Its prototype is given below:
```
loff_t pcd_lseek(struct file *filp, loff_t off, int whence){
 return 0;
}
struct file *flip = pointer of file object
loff_t off = offset val
int whence = origin it can have 3 values i.e.,
SEEK_SET = the file offset is set to 'OFF' values
SEEK_CUR = the file offset is set to its current location + off bytes
SEEK_END = the file offset is set to the size of the file + off bytes
```

* off value depends of the whence value that is set. In the llseek method, driver should update the file pointer using 'off' and 'whence' information. The llseek handler should return newly updated file position or error

### Implementing file operation methods

* Go to ldd/source/linux/include/linux/fs.h and then search for 'struct file_operations'. Here we can see the member elements of the file_operations such as read, write, llseek. Copy these elements and keep it to avoid error regarding prototype:
```
loff_t (*llseek) (struct file *, loff_t, int);
ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
int (*open) (struct inode *, struct file *);
int (*release) (struct inode *, struct file *);
```

* Replace the above function with own methods, variables, semicolon etc as shown below:
```
loff_t pcd_lseek (struct file *filp, loff_t off, int whence){
 return 0;
}
ssize_t pcd_read (struct file *filp, char __user *buff, size_t count, loff_t *f_pos){
  return 0;
}
ssize_t pcd_write (struct file *filp, const char __user *buff, size_t count, loff_t *f_pos){
 return 0;
}
int pcd_open (struct inode *inode, struct file *filp){
 return 0;
}
int pcd_release (struct inode *inode, struct file *filp){
 return 0;
}

*inode, *flip, off, whence, *f_pos, count, *buff etc.. can be any name. Its a variable name
```

* Now go to main.c and paste it before the struct file_operations pcd_fops

### File operations structure initialization

* Next step is to initializa the file operation variable pcd_fops from struct file_operations pcd_fops. This is just like intialising the structure member elements like:
```
struct carModel{
float car_weight;
unsigned int car_number;
uint32_t carPrice;
uint16_t carmaxSpeed;
};

int main()
{
struct carModel carHundai= {.carmaxSpeed=110,.carPrice= 300000,.car_number =100910, .car_weight=120};// C99 method using designated initializers
}
Order is not important here
```

* Instead of :
```
struct carModel{
float car_weight;
unsigned int car_number;
uint32_t carPrice;
uint16_t carmaxSpeed;
};

int main()
{
struct carModel carVW = {2000, 1234, 120000, 100};// order is imp here. C89 method
struct carModel carFord= {1000, 5678, 110000, 120};
}
Order is important here
```

* So implementing it as C99 standard as shown below
struct file_operations pcd_fops={
    .open = pcd_open,
    .write = pcd_write,
    .read = pcd_read,
    .llseek = pcd_lseek,
    .release = pcd_release,
    .owner = THIS_MODULE
};


### Creating device files

* Creating device files using class_create(); and device_create();. In Linux we can create device file dynamically i.e., we need not to manually create the device file under /dev directory under to access the h/w

* User level programs like udevd can populate /dev directory with device files dynamically. udev looks for a file called 'dev' in the /sys/class tree if sysfs to determine what major and minor number is assigned to a device number

* Create and register a class with sysfs
```
struct class * class_create(struct module *owner, const char *name);
struct module *owner = pointer to module that is to 'own' this struct class
const char *name = string for the name of this class
```

* Prototype for device create file is
```
struct device *device_create(struct class *class, struct device *parent, dev_t devt, void *dvrdata, const char *fmt, ...);
```

* Now perform the changes in main.c to add class_create and device_create

### Character driver cleanup func implementation

* In the deletion part, we should do undo of creation part in chronological reverse order.
```
This removed a device that was created with device_create
void device_destroy(struct class *class, dev_t devt);
struct class *class = pointer to the struct class that this device was registered with
dev_t devt = devt of the device that was previously registered
```

* Destroys a struct class structure
```
void class destroy(struct class *cls); // pointer to the struct class that is to be destroyed
```

* Now do the changes in main.c under 'pcd_driver_cleanup' func. We can also change main.c to pcd.c and in the Makefile change it to pcd.o

* make host shoouldn't throw any errors. Now do "sudo insmod main.ko" and after dmesg gives the below o/p:
```
[11287.017057] pcd_driver_init: Device number <major>:<minor> 235:0
[11287.017991] pcd_driver_init: Module init was successful
```

* "ls /sys/class" will has pcd_class under it. dev file can be found under /sys/class/pcd_class/pcd/ directory. cat dev shows the device_number. udev reads dev and cat uevent gives
```
MAJOR=235
MINOR=0
DEVNAME=pcd
```

* "ls -l /dev/pcd" gives crw------- 1 root root 235, 0 Dec 20 17:53 /dev/pcd

* Now remove the module by executing "sudo rmmod main.ko" and dmesg shows module unloaded message