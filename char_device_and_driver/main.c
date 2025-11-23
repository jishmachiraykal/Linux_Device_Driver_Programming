#include<linux/module.h>
#include<Linux/fs.h>

#define DEV_MEM_SIZE 512

// array of 512 bytes
char device_buffer[DEV_MEM_SIZE]

dev_t device_number;

static int __init pcd_module_init(void)
{
// allocating device number
alloc_chrdev_region(&device_number,0,1,"pcd");

}

static void __exit pcd_driver_cleanup(void)
{


}

module_init(pcd_module_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JCP");
MODULE_DESCRIPTION("A peseudo character driver");