### Understanding read method

* When the read method is called on device file, we shoul transfer data from device memory buffer to user buffer. Data copy happens from kernel side to user side. Write is vice-versa

* Implementation steps are as follows:
```
ssize_t pcd_read (struct file *filp, char __user *buff, size_t count, loff_t *f_pos){
  return 0;
}

1. Check the requested count value against DEV_MEM_SIZE(typically 512 bytes) of the device i.e., if f_pos(current file position) + count > DEV_MEM_SIZE then adjust the count. count = count + DEV_MEM_SIZE
2. Copy "count number of bytes from device memory to user memory
3. Update the f_pos
4. Return the number of bytes successfully read or error code
5. If f_pos is EOF, then 0 return
```

* f_pos is a member element of struct file which gets created for every open method. f_pos tracks the file access.

* Copy to user is used to copy data from Kernel space to user space
```
unsigned long copy_to_user(void __user *to, const void *from, unsigned long n){

}
void __user *to = destination address in user space
const void *from = source address in kernel space
unsigned long n = number of bytes to copy

Returns 0 on success or number of bytes could be read
```

* Copy from user method works similar to copy to user method

### Understanding error codes

* Whenever error happens appropriate error code should be returned from drivers. It travels from Kernel space to the user space. Error codes are defined in "include/uapi/asm-generic/errno-base.h"

### Read Method Implementation

* Go main.c in ldd/custom_drivers/002pseudo_char_driver/main.c and the read method

### Understanding write method

* Copy data from user space to Kernel space i.e., user buffer to device memory buffer. Steps are shown below:
```
ssize_t pcd_write (struct file *filp, const char __user *buff, size_t count, loff_t *f_pos){
    pr_info("Write requested for %zu bytes\n",count);
    return 0;
}
1. Check the requested count value against DEV_MEM_SIZE(typically 512 bytes) of the device i.e., if f_pos(current file position) + count > DEV_MEM_SIZE then adjust the count. count = count + DEV_MEM_SIZE
2. Copy "count number of bytes from user memory memory to user memory
3. Update the f_pos
4. Return the number of bytes successfully written or error code
```

* Should not return 0 if count is 0, then we should return appropriate no memory left error code

### Write method implementation

* Edit the main.c file from ldd/custom_drivers/002pseudo_char_driver/main.c to include write method like read method implementation

### Important Note

* In older kernel version(OSD6) struct class *class_create(struct module *owner, const char *name); was supported but in the latest Kernel(6.+) version(OSD8) is struct class *class_create(const char *name);. So when you pass THIS_MODULE, the compiler complains because it expects a const char *, not a struct module *. To fix this THIS_MODULE is removed from class_pcd = class_create(THIS_MODULE, "pcd_class"); and then compiled

* Also function prototypes were added after the macros def
