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

*