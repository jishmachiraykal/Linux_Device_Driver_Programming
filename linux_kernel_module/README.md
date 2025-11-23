### Introduction to Linux Kernel Module

* Linux supports dynamic insertion and removal of code from the kernel while the system is up and running. The code what we add and remove at runtime is called Kernel module

* Once the lkm is loaded into the linux kernel we can start using new features and functionalities exposed by the kernel module without restarting the device

* LKM dynamically extends the functionality of the kernel by introducing new features to the kernel by such as security, device drivers, file system drivers, system calls etc...(modular approach)

* Support for LKM allows our embedded system to have only minimal base Kernel images(less runtime storage) and optional device drivers and other features are supplied on demand via module insertion. Ex: hot-plugabble new device is inserted the device driver which and LKM gets loaded automatically to the kernel

* So when the kernel is built, we can either link modules directly or build them as separate modules that can be loaded into the kernel at some other time

* In the static module when building the Linux kernel, we can make the module statically linked to kernel image. This becomes part of Kernel image. Size will be more. Since module is built-in to kernel image we cannot unload the module. It occupies memory permanently during run time

* In the dynamic module, when building a Linux Kernel, they are compiled and linked separately to produce .ko files(done using modules command in previous section). We can load and unload these modules in the Kernel using userspace program such as insmod, modprobe etc...

### User space vs kernel space

* When the Kernel space code runs on the CPU operational mode of the CPU will be in previleged mode. It will have access to security, linux kernel, subsystems and LKMs

* When user space code runs on the CPU operational mode of the CPU will be in restricted mode. User space programms can't access memory locations, which it is not supposed to touch. To access memory, peripherals it has to request kernel space using system call

* LKM will run in Kernel space

### LKM writing syntax

* All the header files are placed in linux_source_repo/include/include/linux: https://github.com/beagleboard/linux/tree/master/include/linux

* module.h is a kernel header and user header can be stdio.h. No userspace library is linked to kernel module. So we shouldn't use any user space module while writing kernel modules

* No main function.

* Prototype of module initialization function: int fun(void). Must return a value, 0 for success and nonzero means module initialization failed and so the module will not get loaded in the kernel. This is like main function and get called during boot time in the case of static modules. In the cases of dynamic module, this will get called during module insertion.There will be one module initialization point in the module

* We do initialization of devices, device private data structure. Requesting memory dynamically for various kernel data strucures and services. Request for allocation of major-minor numbers & device file creation

* It is made static because it is module specific and never be called from other modules

* Prototype of module clean up function: void fun(void). This is an entry point when the module is removed. Since we cannot remove static modules, cleanup function will get called only incase of dynamic module when it is removed using user space command such as rmmod

* If we sure that it will always be linked statically then no need of having cleanup function. Even if the static module has a cleanup function, kernel build system will remove it during build process if there is __exit marker. Here we undo the init function. Ex: free memory which was requested in the init function, leave the device in proper state etc...

* pr_info is a wrapper around kernels printk

### __init and __exit macros

* __init and __exit are C macros which are defined in init.h file

* When __init is used within a function, function code will be stored in a o/p section called .init and __exit will be stored in .exit. These macros make sense only for static modules

* Once all the initialization function gets executed, it will be freed from the memory by the kernel during boot time. Because this is built-in(static) and loaded only once, so it is better to free the memory to save space until the next reboot

* Similarly, we have __initdata which are used for init variable than functions. If function is not tagged with __init, it will part of .text output section and will not be freed and consume memory permanently

### LKM entry point registration and other macros

* module_init and module_exit are the macros used to register entry point fun with the kernel. It's not a function

* MODULE_LICENSE is a macro to used by the kernel module to annouce its type. GPL(general public license) is the type of license by kernel

### Hello World LKM

* under ldd/custom_driver, "mkdir 001hello_world". Inside this folder create main.c. Copy hello_world.ko file from this directory to main.c

### Building a Linux Kernel Module

* Modules which are already part of linux kernel are called in-tree modules(approved by kernel maintainers and developers). Out-tree means outside of linux kernel tree. When we load out-tree module kernel throws error which can be ignored

* We cannot compile the module against one kernel version and load it into the system which is running in the different kernel version

* Here first we need to link linux kernel source tree to local makefile where external modules to be compiled are stored. This is key point in the out-tree compilation

* make syntax:
```
    make -C <path to linux kernel source tree> M <path to out-tree module> [target]
    target options:
    modules: The default target for external modules. It has the same functionality as if no target was specified
    modules_install: Install the external modules. Default location is /lib/modules/<kernel_release>/extra, but prefix may be added with INSTALL_MOD_PATH
    clean: Remove all the generated files in the modules directory only
    help: list available targets for external modules
```

* Creating the local make file to be executed from linux kernel source tree using below syntax:
```
    obj-m := main.o /kbuild system(obj-m) will build main.o from main.c and after linking, will result in the main.ko file
```

### Compilation and testing of LKM

* In the ldd/custom_drivers/001hello_world, "touch Makefile"

* We can build the kernel module on host machine or directly on target machine

* Check the current kernel version by running "uname -r". We have to build against this version. Go to 001hello_world and execute "make -C /lib/modules/'uname -r version'/build/ M=$PWD modules"

* Now this will enter into main.o and finally generates main.ko and other files

* If clean is used instead of modules in the command everything will be deleted

* Now to insert kernel module(main.ko), into the running kernel of the host. "sudo insmod main.ko". Now to see the o/p run dmesg and will be able to see Hello World

* If "sudo rmmod main.ko" is executed then in dmesg, we can see Bye message


### Testing of LKM on target

* In the last lecture, we have built the kernel modules against host, noe we can do it in target and transfer files to target using below command
```
sudo make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C /home/user/ldd/source/linux M=$PWD modules
```

* Now we might get an error because of using sudo and in "arm-linux-gnueabihf-gcc: Command not found", to resolve that go to /etc/sudoers and in the secure path at end add expot PATH command from bashrc(everything from bashrc to bin). Open sudoers file with "sudo visudo" command

* It didn't work after setting in sudoers file, then gave full path of compiler like below and it worked:
```
sudo make ARCH=arm CROSS_COMPILE=/home/user/downloads/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf- -C /home/user/source/linux/ M=$PWD modules
```

* If we run "file main.ko", should be able to see that it is build for ARM and other information. Executing "modinfo main.ko" gives description, license, author and other info what we added in main.c file

* Running "arm-linux-gnueabihf-objdump -h main.ko" to analyze the various section of the Kernel object module like .init.text, .exit.text, data other section

* Connect board to PC and connect via minicom, inside the target make a dir called "drivers". From 0001hello_world program do "scp main.ko debian@192.168.2.2:/home/debian/drivers"

* From the drivers folder, do "sudo insmod main.ko" it will print Hello World and "sudo rmmod main.ko" will show Good Bye message. dmesg | tail will also show these message

### Makefile

* 001hello_world should have only main.c and Makefile. Open make file with "gedit Makefile" and add below contents
```
obj-m := main.o

ARCH=arm
CROSS_COMPILE=/home/user/downloads/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
TARGET_KERN_DIR=/home/user/source/linux/
all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(TARGET_KERN_DIR) M=$(PWD) modules
clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(TARGET_KERN_DIR) M=$(PWD) clean
help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(TARGET_KERN_DIR) M=$(PWD) help
```

* Execute "make" to load the kernel modules, "file main.ko" should show ARM and "make clean" to clean the modules.

* Now editing the file to build again host unlike target shown before as shown below:
```
obj-m := main.o

ARCH=arm
CROSS_COMPILE=/home/user/downloads/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
TARGET_KERN_DIR=/home/user/source/linux/
HOST_KERN_DIR=/lib/modules/$(shell uname -r)/build/
all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(TARGET_KERN_DIR) M=$(PWD) modules
clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(TARGET_KERN_DIR) M=$(PWD) clean
help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(TARGET_KERN_DIR) M=$(PWD) help
host:
	make -C $(HOST_KERN_DIR) M=$(PWD) modules
```

* Execute "make host" to load against host and "file main.ko" should show x86-64(ARM for target loading) in the output

### In-tree building

* We have to add the Linux kernel module inside the linux kernel source tree and let the linux build system builds that

* If we want to list our kernel module selection in kernel menuconfig, then create and use a kconfig file

* Go to /home/user/ldd/source/linux/drivers/char, create a directory called my_custom_dev and touch kconfig. Copy main.c to this location from previous lecture. Add below entry in the kconfig:
```
menu "my custom modules"
config CUSTOM_HELLOWORLD
	tristate "helloworld support"
	default n
endmenu
```

* Add Kconfig to upper level Kconfig, that's how kernel will identify the local kconfig. Here drivers/char is the upper level directory, go there and open Kconfig before endmenu add "source "drivers/char/my_custom_dev_Kconfig"" and save it

* Go to my_custom_dev and create local Makefile by touch Makefile and add below line:
```
obj-$(CONFIG_CUSTOM_HELLOWORLD) += main.o
```

* Next is to add local level Makefile to upper level Makefile. For that go to drivers/char directory and open Makefile. At the end add:
```
obj-y += my_custom_dev/
```

* Comeback to /home/user/ldd/source/linux and run "make ARCH=arm menuconfig". Menuconfig window will open, there go to device drivers/character drivers/my custom modules(configured in Kconfig)/helloworld support

* Helloworld support will be disabled. To select press spacebar * means statically linked and M means dynamically linked. Select M, save and exit. Open the .config file which is created and search for "my custom modules CONFIG_CUSTOM_HELLOWORLD=m" but don't edit

* Execute "make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules -j4". main.ko file will be present in char/my_custom_dev. Run modinfo main.ko to see in-tree will be set to Y and kernel will not throw any warning related to out-tree like before

### printk

* Instead of printf for debugging, in kernel there is printk where k stands for kernel space printing. This is similar to printf

* When using printk, the message will go into kernel ring buffer and simpy we call "Kernel log", we can print and control the kernel ring buffer using the command dmesg. Printk doesn't support floating-point buffers(%e,%f,%g)

* To get to know more about format specifier for printk, see linux/Documentation/printk-formats.txt. For printk messages we can add priority. Lower number indicates higher log level and vice-versa, default value is 4. Printk(KERN_WARNING "Hello this is abc\n"); KERN_WARNING is a macro. Default value can also be changed using menuconfig

* Printk(KERN_ALERT "Hello this is abc\n"); and Printk(KERN_INFO "Hello this is abc\n"); printk kernel log level is compared with current console log level. If kernel log level is less than current console log level(default=7) then printk will print the message directly on the current console

* To know the current log level status run cat /proc/sys/kernel/printk and log level can be changed using echo 4 > /proc/sys/kernel/printk

* pr_info is a wrapper for KERN_INFO whose default value is 6