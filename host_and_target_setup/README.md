### Host and target setup

* Course repository: https://github.com/niekiran/linux-device-driver-1

* Host setup:
```
sudo apt-get update
sudo apt-get install build-essential lzop u-boot-tools net-tools bison flex libssl-dev libncurses5-dev libncursesw5-dev unzip chrpath xz-utils minicom wget git-core
```

* Workspace setup:
```
ldd
  -->custom_drivers
  -->drivers
  -->patches
  -->source
```

* Download pre-built images from resource attached in sec 1 lecture 3 and download debian OS image from https://www.beagleboard.org/distros/debian-9-9-2019-08-03-4gb-sd-iot place it in downloads folder under ldd

### Toolchain download

* To compile Linux, kernel with ARM architecture for SOC AM335X we need toolchain: https://releases.linaro.org/components/toolchain/binaries/latest-7/arm-linux-gnueabi/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabi.tar.xz

* Confirm whether the host machine is 32 bit/64 bit by running "uname -a" in host

### Toolchain installtion and PATH settings

* Add the absolute path to gcc toolchain /home/user/gcc-linaro/bin in the ~/.bashrc file under export PATH=$PATH:/home/user/gcc-linaro/bin

* Save and source it home directory. Now when we type arm and hit tab it should detect all the toolchain binary

### Target preparation Serial Debug Setup

* SOC is from TI Sitara(am335x) family of SOCs, which is based on Arm cortex A8 processor(ARM V7 architecture)

* USB to TTL serial cable connection
```
Gnd(black) should be connected to pin 1 of BBB
Txd(green) should be connected to the RXD pin 4 of BBB
Rxd(yellow) should be connected to the TXD pin 5 of BBB
```

### Important documents

* AM335X Technical Reference Manual(TRM): https://www.ti.com/lit/ug/spruh73q/spruh73q.pdf

* AM335X data sheet: https://www.ti.com/lit/ds/symlink/am3358.pdf?ts=1763619505526

* Beaglebone black system reference manual: https://cdn-shop.adafruit.com/datasheets/BBB_SRM.pdf

* Device tree specification document: https://github.com/devicetree-org/devicetree-specification/releases/download/v0.3/devicetree-specification-v0.3.pdf

### Understanding booting sequence of beaglebone black hardware

* Bootsequence if S2 is not pressed during power up
```
MMC1(eMMC)
MMC0(uSD)
UART0
USB0
```

* Bootsequence if S2 is pressed during power up
```
SPI0
MMC0(SD)
USB0
UART0
```

* BBB comes up with the pre-built debian image, so when we try to boot the board it boots from eMMC.

* Reset button will resets the board but it will not affect the boot sequence. Boot button(S2) will change the boot sequence during power up of the board

### Prepare SD card for SD boot

* Follow the steps to transfer images to SD card from card reader using https://github.com/jishmachiraykal/Embedded-Linux-Using-Beaglebone-Black/blob/master/Boot_from_SD_card/README.md

### Booting BBB via SD card

* Press and hold S2, press S3 until blue LED's turns off and turns on again. Release S2 after 2-5 seconds

* Execute "uname -r" to see the image version. Execute "uname -h" to see all the info about uname

* Inorder to boot from SD by default, if MLO image are present then perform the following operations inside the target
```
  sudo -s // take to /home/debian
  mkdir /media/tmp1
  mount /dev/mmcblk1p1 /media/tmp1 //eMMC is mmc1 and mounting partition 1(p1), it might throw some error related to volume, ignore it for now
  cd /media/tmp1
  mv MLO MLO.back //it will look for MLO file since it will not be present(MLO.back to be present), it will go to next sequence(mmc0/SD card)
  sync
```

* "shutdown -h now" to power down the board

### Making SD boot default on BBB by erasing eMMC MBR

* If we are using new version of eMMC memory, then we don't see 2 partitions of the eMMC(can be checked using "lsblk" command). There will be no MLO because latest version creates one partition of type ext4 and one more partition MBR of size 1MB which includes u-boot and MLO

* So now we have to erase the MBR to boot by default from SD card. Now perform below operation
```
sudo -s
dd if=/dev/mmcblk1 of=emmcboot.img bs=1M count=1 // if = i/p file and of = o/p file bs=size(1MB here) dd=Copy a file, converting and formatting according to the operands(also be checked using dd --help)
// this creates a file called emmcboot which contains the snapshot of MBR
dd if=/dev/zero of=/dev/mmcblk1 bs=1M count=1 //zeroing out the MBR here
```

* Remove the power(disconnect USB cable) and connect it back. It should boot from SD card without pressing S3/S2

* Now to recover MBR,
```
sudo -s
dd if=emmcboot.img of=/dev/mmcblk1 bs=1 count=1
```

* Power down and now it should boot from emmc

* Throught the course we will be using SD card boot and not from eMMC


### Updating Linux Kernel Image

* Official Linux repo from BBB https://github.com/beagleboard/linux branch 5.10.168-ti-rt-r76

* git clone https://github.com/beagleboard/linux.git linux_bbb_5.10 in the ldd/source directory

### Linux Kernel Compilation

* Compilation steps from https://github.com/niekiran/linux-device-driver-1/blob/master/scripts/kernel_compilation_steps.txt

* Get into linux_bbb_5.10 folder and perform above steps

### Update new boot images and modules in SD card

* Keep the boot and lba flag on BOOT partition, otherwise it boots from rootfs and changes won't be reflected. Can be changed using fdisk

* Connect SD card to host and go to BOOT partition, rename uImage and .dtb files to some other name. Now go to BBB Linux repo where Kernel image was flashed, under arch/arm/boot copy uImage to BOOT partition of SD card and execute sync

* Go to to BBB Linux repo where Kernel image was flashed, under arch/arm/boot/dts/ "cp -a am335x-boneblack.dtb /media/user/BOOT"

* Now in the root directory, go to /lib/modules/ and copy 5.10.168. 'cp -a 5.10.168 media/user/ROOTFS/lib/modules'

* Remove SD card and connect it to BBB and boot the BBB using SD card

* 'uname -r' should should show 5.10.168 kernel release version

### Enabling internet over USB

* Board can connect to the internet over USB cable using PC's internet connection. Doesn't require separate ethernet cable to connect to board to internet

* The required drivers are enabled by default in the kernel & loaded when needed

* Open minicom and execute ifconfig, should see interfaces such as usb0/usb1 etc... Through this it can connect to internet
