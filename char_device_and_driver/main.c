#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>

#define DEV_MEM_SIZE 512

char device_buffer[DEV_MEM_SIZE];

/* this holds the device number */
dev_t device_number;

/* cdev variable */
struct cdev pcd_cdev;

/* file operations of the driver */
struct file_operations pcd_fops;

/* pseudo devices memory */
static int __init pcd_driver_init(void)
{
    /* 1. Dynamically allocating a device number */
    alloc_chrdev_region(&device_number,0,1,"pcd");

	/* 2. Initialize the cdev structure with fops */
	cdev_init(&pcd_cdev,&pcd_fops);

    /* 3. Register a device(cdev structure) with VFS*/
    pcd_cdev.owner=THIS_MODULE;
    cdev_add(&pcd_cdev,device_number,1); // registering one device

	return 0;
}

static void __exit pcd_driver_cleanup(void)
{


}


module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JCP");
MODULE_DESCRIPTION("A pseudo character driver");
