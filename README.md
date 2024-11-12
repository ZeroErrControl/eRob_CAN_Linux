
# CAN driver (Linux)
---

# Installing CAN Driver on Linux

## 1. Check the Linux Version and System Type (32/64 bit)
Run the following command to check the Linux version and system type:

```bash
uname -a
```

From the output, we can see that the system is 64-bit

```bash
Linux erobman 6.5.0-rt5 #1 SMP PREEMPT_RT Wed Oct 16 10:34:26 
CST 2024 x86_64 x86_64 x86_64 GNU/Linux

```

## 2. Copy the Corresponding USBCAN Driver Files to the System
After confirming the system type, copy the corresponding USBCAN driver files to the system.

(For this example, place the driver files in /home/eRob_CAN_driver/)

## 3. Get Administrator Privileges
To perform further operations, including installing the driver, acquire administrator privileges.
``` bash
sudo su
```
(Enter the root password after the su command to gain admin access)

## 4. Copy Driver Libraries to the GCC Compilation Directory
Navigate to the USBCAN driver folder and copy libusb.so, libusb-1.0.so, and libECanVci.so.1 to the GCC compilation directory (default path is /usr/lib).

``` bash
cd lib
cp libusb.so libusb-1.0.so libECanVci.so.1 /usr/lib
```
## 5. Link the Libraries
Navigate to the GCC library folder and link libECanVci.so.1 to libECanVci.so.
``` bash
ln -sv libECanVci.so.1 libECanVci.so
```
## 6. Compile the USBCAN Driver
Go to the USBCAN driver folder and compile the driver.

```bash
cd ..
mkdir build && cd build
cmake ..
make
```
:::tip
If you encounter the following error, install the required library:

```bash
error while loading shared libaries: libusb-0.1.so.4: cannot open share ....
```
You can fix it by :

``` bash
sudo apt-get install libusb-0.1-4
```
:::

## 7. Run the Test Program
Run the test program to test USBCAN transmission.

``` bash
sudo ./test
```
After installation, run the test program again. If successful, the output should be as follows:

- The **first parameter** `16`: 
    - Device type. For single-channel devices, input 3, for dual-channel input 4.
- The **second parameter** `0`: 
    - Device index. Input 0 if only one USBCAN device is connected.
- The **third parameter** `3`: 
    - Channel to open. Input 1 to open CAN1, 2 to open CAN2, and 3 to open both CAN1 and CAN2.
- The **fourth parameter** `0x1400`: 
    - CAN bus baud rate. 0x1400 represents a 1000K baud rate. For other rates, refer to the "EcanVCI Dynamic Library Manual."
- The **fifth parameter** `0`: 
    - Operating mode. 0 is normal mode. For other modes, see the "EcanVCI Dynamic Library Manual."
- The **sixth parameter** `1`: 
    - Sending interval in ms.
- The **seventh parameter** `1000`: 
    - Number of transmissions.

## 8. Connect the USBCAN Device
Use the lsusb command to see the connected USBCANI-V503.

To run the test (with admin privileges):
``` bash
sudo ./test 4 0 3 0x1400 2 0 3 1000
```

## 9. Modify Udev Rules for Permanent User Access
To permanently grant regular users access to the USBCAN device, modify the udev configuration:
``` bash
sudo gedit /etc/udev/rules.d/50-usbcan.rules
```

Add the following content (modify the values marked in red based on the USB's ID):
``` t
SUBSYSTEMS=="usb" 
ATTRS{idVendor}=="0c66" 
ATTRS{idProduct}=="000c" 
GROUP="users" 
MODE="0666"
```
Reload `udev` rules and reconnect the device to apply the new permissions:

``` bash
sudo udevadm control --reload
```
## 10. Recompile the Test Program
Recompile the test program with admin privileges, then run:

``` bash
make
./test 4 0 3 0x1400 2 0 3 1000
```
The installation of the eRob driver is now complete.

## 11. Compile and Run the eRobCAN Test
Go to the `eRob_CAN_Linux` folder, create a build directory, and compile the program:

``` bash
cd build
cmake ..
make
```
After compiling, you will see three executable files.

If the USBCAN device doesn't work, try reconnecting it and run the following command:

``` bash
./eRobCAN_sendCOB 0x641 "{01 00 00 00 00 01}"
```

If the return value is `0x3e`, the command was sent successfully.

If you encounter connection failure issues, you may need to add `sudo` before the command.

For CAN message commands, refer to the **eRunner User Manual**: [Download link](https://www.zeroerr.cn/d/file/download/eRunner%E7%94%A8%E6%88%B7%E6%89%8B%E5%86%8C_Ver1.4.pdf)

## 12.Example (Velocity Mode)
Here is an example of sending commands in velocity mode:

``` bash
./eRobCAN_sendCOB 0x641 "{00 4E 00 00 00 02}"
./eRobCAN_sendCOB 0x641 "{01 12 00 00 00 00}"
./eRobCAN_sendCOB 0x641 "{01 FD 00 00 00 00}"
./eRobCAN_sendCOB 0x641 "{00 88 00 00 75 30}"
./eRobCAN_sendCOB 0x641 "{00 89 00 00 75 30}"
./eRobCAN_sendCOB 0x641 "{01 FE 00 00 27 10}"
./eRobCAN_sendCOB 0x641 "{01 00 00 00 00 01}"
./eRobCAN_sendCOB 0x641 "{00 08}"
./eRobCAN_sendCOB 0x641 "{00 05 00 01}"
./eRobCAN_sendCOB 0x641 "{00 02}"
./eRobCAN_sendCOB 0x641 "{01 00 00 00 00 00}"
```
