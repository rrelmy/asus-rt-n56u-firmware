#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>

#include <signal.h>

#include "rc.h"
#include "dongles.h"

#ifdef USB_MODEM

#define RTN56U

#ifdef RTN56U
#include <nvram/bcmnvram.h>
//#define SUPPORT_USB_DISK
#define SUPPORT_USB_PRINTER
#define SUPPORT_USB_MODEM
#elif defined(RTN66U)
#include <bcmnvram.h>
#define SUPPORT_USB_DISK
#define SUPPORT_USB_PRINTER
#define SUPPORT_USB_MODEM
#endif

#define MAX_RETRY_LOCK 1
#define MAX_WAIT_FILE 5
#define MAX_WAIT_PRINTER_MODULE 10
#define SCAN_PRINTER_NODE 2

#define U2EC_FIFO "/var/u2ec_fifo"

#define USB_MODESWITCH_CONF "/etc/g3.conf"

#define PPP_DIR "/tmp/ppp/peers"
#define PPP_CONF_FOR_3G "/tmp/ppp/peers/3g"

#define LOCK_DEVICE "/var/lock/LOCK.device"
#define LOCK_INTERFACE "/var/lock/LOCK.interface"

#define SYS_MODULE "/sys/module"
#define SYS_BLOCK "/sys/block"
#define SYS_TTY "/sys/class/tty"
#define SYS_PRINTER "/sys/class/usb"
#define USB_DEVICE_PATH "/sys/bus/usb/devices"
#define USB_EHCI_PORT_1 "1-1"
#define USB_EHCI_PORT_2 "1-2"
#define USB_OHCI_PORT_1 "2-1"
#define USB_OHCI_PORT_2 "2-2"

extern char *usb_dev_file;

char *get_usb_port(const char *target_string, char *buf, const int buf_size){
	memset(buf, 0, buf_size);

	if(strstr(target_string, USB_EHCI_PORT_1))
		strcpy(buf, USB_EHCI_PORT_1);
	else if(strstr(target_string, USB_EHCI_PORT_2))
		strcpy(buf, USB_EHCI_PORT_2);
	else if(strstr(target_string, USB_OHCI_PORT_1))
		strcpy(buf, USB_OHCI_PORT_1);
	else if(strstr(target_string, USB_OHCI_PORT_2))
		strcpy(buf, USB_OHCI_PORT_2);
	else
		return NULL;

	return buf;
}

char *get_usb_vid(const char *usb_port, char *buf, const int buf_size){
	FILE *fp;
	char check_usb_port[4], target_file[128];
	int len;

	if(usb_port == NULL || get_usb_port(usb_port, check_usb_port, 4) == NULL || strlen(usb_port) != strlen(USB_EHCI_PORT_1))
		return NULL;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/%s/idVendor", USB_DEVICE_PATH, usb_port);
	if((fp = fopen(target_file, "r")) == NULL)
		return NULL;

	memset(buf, 0, buf_size);
	fgets(buf, buf_size, fp);
	fclose(fp);
	len = strlen(buf);
	buf[len-1] = 0;

	return buf;
}

char *get_usb_pid(const char *usb_port, char *buf, const int buf_size){
	FILE *fp;
	char check_usb_port[4], target_file[128];
	int len;

	if(usb_port == NULL || get_usb_port(usb_port, check_usb_port, 4) == NULL || strlen(usb_port) != strlen(USB_EHCI_PORT_1))
		return NULL;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/%s/idProduct", USB_DEVICE_PATH, usb_port);
	if((fp = fopen(target_file, "r")) == NULL)
		return NULL;

	memset(buf, 0, buf_size);
	fgets(buf, buf_size, fp);
	fclose(fp);
	len = strlen(buf);
	buf[len-1] = 0;

	return buf;
}

char *get_usb_manufacturer(const char *usb_port, char *buf, const int buf_size){
	FILE *fp;
	char check_usb_port[4], target_file[128];
	int len;

	if(usb_port == NULL || get_usb_port(usb_port, check_usb_port, 4) == NULL || strlen(usb_port) != strlen(USB_EHCI_PORT_1))
		return NULL;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/%s/manufacturer", USB_DEVICE_PATH, usb_port);
	if((fp = fopen(target_file, "r")) == NULL)
		return NULL;

	memset(buf, 0, buf_size);
	fgets(buf, buf_size, fp);
	fclose(fp);
	len = strlen(buf);
	buf[len-1] = 0;

	return buf;
}

char *get_usb_product(const char *usb_port, char *buf, const int buf_size){
	FILE *fp;
	char check_usb_port[4], target_file[128];
	int len;

	if(usb_port == NULL || get_usb_port(usb_port, check_usb_port, 4) == NULL || strlen(usb_port) != strlen(USB_EHCI_PORT_1))
		return NULL;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/%s/product", USB_DEVICE_PATH, usb_port);
	if((fp = fopen(target_file, "r")) == NULL)
		return NULL;

	memset(buf, 0, buf_size);
	fgets(buf, buf_size, fp);
	fclose(fp);
	len = strlen(buf);
	buf[len-1] = 0;

	return buf;
}

char *get_usb_serial(const char *usb_port, char *buf, const int buf_size){
	FILE *fp;
	char check_usb_port[4], target_file[128];
	int len;

	if(usb_port == NULL || get_usb_port(usb_port, check_usb_port, 4) == NULL || strlen(usb_port) != strlen(USB_EHCI_PORT_1))
		return NULL;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/%s/serial", USB_DEVICE_PATH, usb_port);
	if((fp = fopen(target_file, "r")) == NULL)
		return NULL;

	memset(buf, 0, buf_size);
	fgets(buf, buf_size, fp);
	fclose(fp);
	len = strlen(buf);
	buf[len-1] = 0;

	return buf;
}

int get_usb_interface_number(const char *usb_port){
	FILE *fp;
	char target_file[128], buf[4];

	if(usb_port == NULL || get_usb_port(usb_port, buf, 4) == NULL || strlen(usb_port) != strlen(USB_EHCI_PORT_1))
		return 0;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/%s/bNumInterfaces", USB_DEVICE_PATH, usb_port);
	if((fp = fopen(target_file, "r")) == NULL)
		return 0;

	memset(buf, 0, 4);
	fgets(buf, 4, fp);
	fclose(fp);

	return atoi(buf);
}

char *get_usb_interface_class(const char *interface_name, char *buf, const int buf_size){
	FILE *fp;
	char check_usb_port[4], target_file[128];
	int retry, len;

	if(interface_name == NULL || get_usb_port(interface_name, check_usb_port, 4) == NULL)
		return NULL;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/%s/bInterfaceClass", USB_DEVICE_PATH, interface_name);
	retry = 0;
	while((fp = fopen(target_file, "r")) == NULL && retry < MAX_WAIT_FILE){
		++retry;
		sleep(1); // Sometimes the class file would be built slowly, so try again.
	}

	if(fp == NULL){
		usb_dbg("(%s): Fail to open the class file really!\n", interface_name);
		return NULL;
	}

	memset(buf, 0, buf_size);
	fgets(buf, buf_size, fp);
	fclose(fp);
	len = strlen(buf);
	buf[len-1] = 0;

	return buf;
}

int get_interface_numendpoints(const char *interface_name){
	FILE *fp;
	char target_file[128], buf[4];

	if(interface_name == NULL || get_usb_port(interface_name, buf, 4) == NULL)
		return 0;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/%s/bNumEndpoints", USB_DEVICE_PATH, interface_name);
	if((fp = fopen(target_file, "r")) == NULL)
		return 0;

	memset(buf, 0, 4);
	fgets(buf, 4, fp);
	fclose(fp);

	return atoi(buf);
}

int get_interface_Int_endpoint(const char *interface_name){
	FILE *fp;
	char interface_path[128], bmAttributes_file[128], buf[4];
	DIR *interface_dir = NULL;
	struct dirent *end_name;
	int bNumEndpoints, end_count, got_Int = 0;

	if(interface_name == NULL || get_usb_port(interface_name, buf, 4) == NULL){
		usb_dbg("(%s): The device is not a interface.\n", interface_name);
		return 0;
	}

	memset(interface_path, 0, 128);
	sprintf(interface_path, "%s/%s", USB_DEVICE_PATH, interface_name);
	if((interface_dir = opendir(interface_path)) == NULL){
		usb_dbg("(%s): Fail to open dir: %s.\n", interface_name, interface_path);
		return 0;
	}

	// Get bNumEndpoints.
	bNumEndpoints = get_interface_numendpoints(interface_name);
	if(bNumEndpoints <= 0){
		usb_dbg("(%s): No endpoints: %d.\n", interface_name, bNumEndpoints);
		return 0;
	}

	end_count = 0;
	while((end_name = readdir(interface_dir)) != NULL){
		if(strncmp(end_name->d_name, "ep_", 3))
			continue;

		++end_count;

		memset(bmAttributes_file, 0, 128);
		sprintf(bmAttributes_file, "%s/%s/bmAttributes", interface_path, end_name->d_name);

		if((fp = fopen(bmAttributes_file, "r")) == NULL){
			usb_dbg("(%s): Fail to open file: %s.\n", interface_name, bmAttributes_file);
			continue;
		}

		memset(buf, 0, 4);
		fgets(buf, 4, fp);
		fclose(fp);

		if(!strncmp(buf, "03", 2)){
			got_Int = 1;
			break;
		}
		else if(end_count == bNumEndpoints)
			break;
	}
	closedir(interface_dir);

	return got_Int;
}

#ifdef SUPPORT_USB_MODEM
int hadSerialModule(){
	char target_file[128];
	DIR *module_dir;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/usbserial", SYS_MODULE);
	if((module_dir = opendir(target_file)) != NULL){
		closedir(module_dir);
		return 1;
	}
	else
		return 0;
}

int hadACMModule(){
	char target_file[128];
	DIR *module_dir;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/cdc_acm", SYS_MODULE);
	if((module_dir = opendir(target_file)) != NULL){
		closedir(module_dir);
		return 1;
	}
	else
		return 0;
}

int isSerialNode(const char *device_name){
	if(strncmp(device_name, "ttyUSB", 6))
		return 0;

	return 1;
}

int isACMNode(const char *device_name){
	if(strncmp(device_name, "ttyACM", 6))
		return 0;

	return 1;
}

int isSerialInterface(const char *interface_name){
	char interface_class[4];

	if(get_usb_interface_class(interface_name, interface_class, 4) == NULL)
		return 0;

	if(strcmp(interface_class, "ff"))
		return 0;

	return 1;
}

int isACMInterface(const char *interface_name){
	char interface_class[4];

	if(get_usb_interface_class(interface_name, interface_class, 4) == NULL)
		return 0;

	if(strcmp(interface_class, "02"))
		return 0;

	return 1;
}
#endif // SUPPORT_USB_MODEM

#ifdef SUPPORT_USB_PRINTER
int hadPrinterModule(){
	char target_file[128];
	DIR *module_dir;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/usblp", SYS_MODULE);
	if((module_dir = opendir(target_file)) != NULL){
		closedir(module_dir);
		return 1;
	}
	else
		return 0;
}

int hadPrinterInterface(const char *usb_port){
	char check_usb_port[4], target_path[128], usb_path[PATH_MAX];
	int printer_order, got_printer = 0;

	for(printer_order = 0; printer_order < SCAN_PRINTER_NODE; ++printer_order){
		memset(target_path, 0, 128);
		sprintf(target_path, "%s/lp%d/device", SYS_PRINTER, printer_order);
		memset(usb_path, 0, PATH_MAX);
		if(realpath(target_path, usb_path) == NULL)
			continue;

		if(get_usb_port(usb_path, check_usb_port, 4) == NULL)
			continue;

		if(!strcmp(usb_port, check_usb_port)){
			got_printer = 1;

			break;
		}
	}

	return got_printer;
}

int isPrinterInterface(const char *interface_name){
	char interface_class[4];

	if(get_usb_interface_class(interface_name, interface_class, 4) == NULL)
		return 0;

	if(strcmp(interface_class, "07"))
		return 0;

	return 1;
}
#endif // SUPPORT_USB_PRINTER

#ifdef SUPPORT_USB_DISK
int isStorageInterface(const char *interface_name){
	char interface_class[4];

	if(get_usb_interface_class(interface_name, interface_class, 4) == NULL)
		return 0;

	if(strcmp(interface_class, "08"))
		return 0;

	return 1;
}
#endif // SUPPORT_USB_DISK

#ifdef SUPPORT_USB_MODEM
int write_3g_conf(FILE *fp, int dno, int aut, char *vid, char *pid){
	switch(dno){
		case SN_MU_Q101:
			fprintf(fp, "DefaultVendor=  0x0408\n");
			fprintf(fp, "DefaultProduct= 0x1000\n");
			fprintf(fp, "TargetVendor=   0x0408\n");
			fprintf(fp, "TargetProduct=  0xea02\n");
			fprintf(fp, "MessageEndpoint=0x05\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
			break;
		case SN_OPTION_ICON225:
			fprintf(fp, "DefaultVendor=  0x0af0\n");
			fprintf(fp, "DefaultProduct= 0x6971\n");
			fprintf(fp, "TargetClass=    0xff\n");
			fprintf(fp, "MessageContent=\"555342431223456780100000080000601000000000000000000000000000000\"\n");
			break;
		case SN_Option_GlobeSurfer_Icon:
			fprintf(fp, "DefaultVendor=  0x05c6\n");
			fprintf(fp, "DefaultProduct= 0x1000\n");
			fprintf(fp, "TargetVendor=   0x0af0\n");
			fprintf(fp, "TargetProduct=  0x6600\n");
			fprintf(fp, "MessageContent=\"55534243123456780000000000000601000000000000000000000000000000\"\n");
			break;
		case SN_Option_GlobeSurfer_Icon72:
			fprintf(fp, "DefaultVendor=  0x05c6\n");
			fprintf(fp, "DefaultProduct= 0x1000\n");
			fprintf(fp, "TargetVendor=   0x0af0\n");
			fprintf(fp, "TargetProduct=  0x6901\n");
			fprintf(fp, "MessageContent=\"55534243123456780000000000000601000000000000000000000000000000\"\n");
			break;
		case SN_Option_GlobeTrotter_GT_MAX36:
			fprintf(fp, "DefaultVendor=  0x05c6\n");
			fprintf(fp, "DefaultProduct= 0x1000\n");
			fprintf(fp, "TargetVendor=   0x0af0\n");
			fprintf(fp, "TargetProduct=  0x6600\n");
			fprintf(fp, "MessageContent=\"55534243123456780000000000000601000000000000000000000000000000\"\n");
			fprintf(fp, "ResponseNeeded=1\n");
			break;
		case SN_Option_GlobeTrotter_GT_MAX72:
			fprintf(fp, "DefaultVendor=  0x05c6\n");
			fprintf(fp, "DefaultProduct= 0x1000\n");
			fprintf(fp, "TargetVendor=   0x0af0\n");
			fprintf(fp, "TargetProduct=  0x6701");
			fprintf(fp, "MessageContent=\"55534243123456780000000000000601000000000000000000000000000000\"\n");
			break;
		case SN_Option_GlobeTrotter_EXPRESS72:
			fprintf(fp, "DefaultVendor=  0x05c6\n");
			fprintf(fp, "DefaultProduct= 0x1000\n");
			fprintf(fp, "TargetVendor=   0x0af0\n");
			fprintf(fp, "TargetProduct=  0x6701\n");
			fprintf(fp, "MessageContent=\"55534243123456780000000000000601000000000000000000000000000000\"\n");
			fprintf(fp, "ResponseNeeded=1\n");
			break;
		case SN_Option_iCON210:
			fprintf(fp, "DefaultVendor=  0x1e0e\n");
			fprintf(fp, "DefaultProduct= 0xf000\n");
			fprintf(fp, "TargetVendor=   0x1e0e\n");
			fprintf(fp, "TargetProduct=  0x9000\n");
			fprintf(fp, "MessageContent=\"555342431234567800000000000006bd000000020000000000000000000000\"\n");
			fprintf(fp, "ResponseNeeded=1\n");
			break;
		case SN_Option_GlobeTrotter_HSUPA_Modem:
			fprintf(fp, "DefaultVendor=  0x0af0\n");
			fprintf(fp, "DefaultProduct= 0x7011\n");
			fprintf(fp, "TargetClass=    0xff\n");
			fprintf(fp, "MessageContent=\"55534243785634120100000080000601000000000000000000000000000000\"\n");
			break;
		case SN_Option_iCON401:
			fprintf(fp, "DefaultVendor=  0x0af0\n");
			fprintf(fp, "DefaultProduct= 0x7401\n");
			fprintf(fp, "TargetClass=    0xff\n");
			fprintf(fp, "MessageContent=\"55534243785634120100000080000601000000000000000000000000000000\"\n");
			fprintf(fp, "ResponseNeeded=1\n");
			break;
		case SN_Vodafone_K3760:
			fprintf(fp, "DefaultVendor=  0x0af0\n");
			fprintf(fp, "DefaultProduct= 0x7501\n");
			fprintf(fp, "TargetClass=    0xff\n");
			fprintf(fp, "MessageContent=\"55534243785634120100000080000601000000000000000000000000000000\"\n");
			break;
		case SN_ATT_USBConnect_Quicksilver:
			fprintf(fp, "DefaultVendor=  0x0af0\n");
			fprintf(fp, "DefaultProduct= 0xd033\n");
			fprintf(fp, "TargetClass=    0xff\n");
			fprintf(fp, "MessageContent=\"55534243785634120100000080000601000000000000000000000000000000\"\n");
			break;
		case SN_Huawei_E169:
			fprintf(fp, "DefaultVendor=  0x12d1\n");
			fprintf(fp, "DefaultProduct= 0x1001\n");
			fprintf(fp, "HuaweiMode=1\n");
			break;
		case SN_Huawei_E220:
			fprintf(fp, "DefaultVendor=  0x12d1\n");
			fprintf(fp, "DefaultProduct= 0x1003\n");
			fprintf(fp, "TargetClass=    0xff\n");
			fprintf(fp, "HuaweiMode=1\n");
			break;
		case SN_Huawei_E180:
			fprintf(fp, "DefaultVendor=  0x12d1\n");
			fprintf(fp, "DefaultProduct= 0x1414\n");
			fprintf(fp, "TargetClass=    0xff\n");
			fprintf(fp, "DetachStorageOnly=1\n");
			fprintf(fp, "HuaweiMode=1\n");
			break;
		case SN_Huawei_E630:
			fprintf(fp, "DefaultVendor=  0x1033\n");
			fprintf(fp, "DefaultProduct= 0x0035\n");
			fprintf(fp, "TargetVendor=   0x12d1\n");
			fprintf(fp, "TargetProduct=  0x1003\n");
			fprintf(fp, "HuaweiMode=1\n");
			fprintf(fp, "DetachStorageOnly=1\n");
			break;
		case SN_Huawei_E270:
			fprintf(fp, "DefaultVendor=  0x12d1\n");
			fprintf(fp, "DefaultProduct= 0x1446\n");
			fprintf(fp, "TargetVendor=   0x12d1\n");
			fprintf(fp, "TargetProduct=  0x14ac\n");
			fprintf(fp, "MessageContent=\"55534243123456780000000000000011060000000000000000000000000000\"\n");
			break;
		case SN_Huawei_E1550:
			fprintf(fp, "DefaultVendor=  0x12d1\n");
			fprintf(fp, "DefaultProduct= 0x1446\n");
			fprintf(fp, "TargetVendor=   0x12d1\n");
			fprintf(fp, "TargetProduct=  0x1001\n");
			fprintf(fp, "MessageContent=\"55534243123456780000000000000011060000000000000000000000000000\"\n");
			break;
		case SN_Huawei_E1612:
			fprintf(fp, "DefaultVendor=  0x12d1\n");
			fprintf(fp, "DefaultProduct= 0x1446\n");
			fprintf(fp, "TargetVendor=   0x12d1\n");
			fprintf(fp, "TargetProduct=  0x1406\n");
			fprintf(fp, "MessageContent=\"55534243123456780000000000000011060000000000000000000000000000\"\n");
			break;
		case SN_Huawei_E1690:
			fprintf(fp, "DefaultVendor=  0x12d1\n");
			fprintf(fp, "DefaultProduct= 0x1446\n");
			fprintf(fp, "TargetVendor=   0x12d1\n");
			fprintf(fp, "TargetProduct=  0x140c\n");
			fprintf(fp, "MessageContent=\"55534243123456780000000000000011060000000000000000000000000000\"\n");
			break;
		case SN_Huawei_K3765:
			fprintf(fp, "DefaultVendor=  0x12d1\n");
			fprintf(fp, "DefaultProduct= 0x1520\n");
			fprintf(fp, "TargetVendor=   0x12d1\n");
			fprintf(fp, "TargetProduct=  0x1465\n");
			fprintf(fp, "MessageContent=\"55534243123456780000000000000011060000000000000000000000000000\"\n");
			break;
		case SN_Huawei_K4505:
			fprintf(fp, "DefaultVendor=  0x12d1\n");
			fprintf(fp, "DefaultProduct= 0x1521\n");
			fprintf(fp, "TargetVendor=   0x12d1\n");
			fprintf(fp, "TargetProduct=  0x1464\n");
			fprintf(fp, "MessageContent=\"55534243123456780000000000000011060000000000000000000000000000\"\n");
			break;
		case SN_ZTE_MF620:
			fprintf(fp, "DefaultVendor=  0x19d2\n");
			fprintf(fp, "DefaultProduct= 0x2000\n");
			fprintf(fp, "TargetVendor=   0x19d2\n");
			fprintf(fp, "TargetProduct=  0x0001\n");
			fprintf(fp, "MessageContent=\"55534243123456780000000000000600000000000000000000000000000000\"\n");
			break;
		case SN_ZTE_MF622:
			fprintf(fp, "DefaultVendor=  0x19d2\n");
			fprintf(fp, "DefaultProduct= 0x2000\n");
			fprintf(fp, "TargetVendor=   0x19d2\n");
			fprintf(fp, "TargetProduct=  0x0002\n");
			fprintf(fp, "MessageContent=\"55534243f8f993882000000080000a85010101180101010101000000000000\"\n");
			break;
		case SN_ZTE_MF628:
			fprintf(fp, "DefaultVendor=  0x19d2\n");
			fprintf(fp, "DefaultProduct= 0x2000\n");
			fprintf(fp, "TargetVendor=   0x19d2\n");
			fprintf(fp, "TargetProduct=  0x0015\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000030000000000000000000000\"\n");
			fprintf(fp, "ResponseNeeded=1\n");
			break;
		case SN_ZTE_MF626:
			fprintf(fp, "DefaultVendor=  0x19d2\n");
			fprintf(fp, "DefaultProduct= 0x2000\n");
			fprintf(fp, "TargetVendor=   0x19d2\n");
			fprintf(fp, "TargetProduct=  0x0031\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000030000000000000000000000\"\n");
			break;
		case SN_ZTE_AC8710:
			fprintf(fp, "DefaultVendor=  0x19d2\n");
			fprintf(fp, "DefaultProduct= 0xfff5\n");
			fprintf(fp, "TargetVendor=   0x19d2\n");
			fprintf(fp, "TargetProduct=  0xffff\n");
			fprintf(fp, "MessageContent=\"5553424312345678c00000008000069f030000000000000000000000000000\"\n");
			break;
		case SN_ZTE_AC2710:
			fprintf(fp, "DefaultVendor=  0x19d2\n");
			fprintf(fp, "DefaultProduct= 0xfff5\n");
			fprintf(fp, "TargetVendor=   0x19d2\n");
			fprintf(fp, "TargetProduct=  0xffff\n");
			fprintf(fp, "MessageContent=\"5553424312345678c00000008000069f010000000000000000000000000000\"\n");
			break;
		case SN_ZTE6535_Z:
			fprintf(fp, "DefaultVendor=  0x19d2\n");
			fprintf(fp, "DefaultProduct= 0x2000\n");
			fprintf(fp, "TargetVendor=   0x19d2\n");
			fprintf(fp, "TargetProduct=  0x0052\n");
			fprintf(fp, "MessageContent=\"55534243123456782000000080000c85010101180101010101000000000000\"\n");
			break;
		case SN_ZTE_K3520_Z:
			fprintf(fp, "DefaultVendor=  0x19d2\n");
			fprintf(fp, "DefaultProduct= 0x2000\n");
			fprintf(fp, "TargetVendor=   0x19d2\n");
			fprintf(fp, "TargetProduct=  0x0055\n");
			fprintf(fp, "MessageContent=\"55534243123456782000000080000c85010101180101010101000000000000\"\n");
			break;
		case SN_ZTE_MF110:
			fprintf(fp, "DefaultVendor=  0x19d2\n");
			fprintf(fp, "DefaultProduct= 0x0053\n");
			fprintf(fp, "TargetVendor=   0x19d2\n");
			fprintf(fp, "TargetProduct=  0x0031\n");
			fprintf(fp, "MessageContent=\"55534243123456782000000080000c85010101180101010101000000000000\"\n");
			break;
		case SN_ZTE_K3565:
			fprintf(fp, "DefaultVendor=  0x19d2\n");
			fprintf(fp, "DefaultProduct= 0x2000\n");
			fprintf(fp, "TargetVendor=   0x19d2\n");
			fprintf(fp, "TargetProduct=  0x0063\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
			break;
		case SN_ONDA_MT503HS:
			fprintf(fp, "DefaultVendor=  0x19d2\n");
			fprintf(fp, "DefaultProduct= 0x2000\n");
			fprintf(fp, "TargetVendor=   0x19d2\n");
			fprintf(fp, "TargetProduct=  0x0002\n");
			fprintf(fp, "MessageContent=\"55534243b0c8dc812000000080000a85010101180101010101000000000000\"\n");
			break;
		case SN_ONDA_MT505UP:
			fprintf(fp, "DefaultVendor=  0x19d2\n");
			fprintf(fp, "DefaultProduct= 0x2000\n");
			fprintf(fp, "TargetVendor=   0x19d2\n");
			fprintf(fp, "TargetProduct=  0x0002\n");
			fprintf(fp, "MessageContent=\"55534243123456780000010080000a28000000001c00002000000000000000\"\n");
			break;
		case SN_Novatel_Wireless_Ovation_MC950D:
			fprintf(fp, "DefaultVendor=  0x1410\n");
			fprintf(fp, "DefaultProduct= 0x5010\n");
			fprintf(fp, "TargetVendor=   0x1410\n");
			fprintf(fp, "TargetProduct=  0x4400\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
			break;
		case SN_Novatel_U727:
			fprintf(fp, "DefaultVendor=  0x1410\n");
			fprintf(fp, "DefaultProduct= 0x5010\n");
			fprintf(fp, "TargetVendor=   0x1410\n");
			fprintf(fp, "TargetProduct=  0x4100\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
			break;
		case SN_Novatel_MC990D:
			fprintf(fp, "DefaultVendor=  0x1410\n");
			fprintf(fp, "DefaultProduct= 0x5020\n");
			fprintf(fp, "Interface=      5\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
			break;
		case SN_Novatel_U760:
			fprintf(fp, "DefaultVendor=  0x1410\n");
			fprintf(fp, "DefaultProduct= 0x5030\n");
			fprintf(fp, "TargetVendor=   0x1410\n");
			fprintf(fp, "TargetProduct=  0x6000\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
			break;
		case SN_Alcatel_X020:
			fprintf(fp, "DefaultVendor=  0x1c9e\n");
			fprintf(fp, "DefaultProduct= 0x1001\n");
			fprintf(fp, "TargetVendor=   0x1c9e\n");
			fprintf(fp, "TargetProduct=  0x6061\n");
			fprintf(fp, "MessageContent=\"55534243123456780000000000000606f50402527000000000000000000000\"\n");
			break;
		case SN_Alcatel_X200:
			fprintf(fp, "DefaultVendor=  0x1bbb\n");
			fprintf(fp, "DefaultProduct= 0xf000\n");
			fprintf(fp, "TargetVendor=   0x1bbb\n");
			fprintf(fp, "TargetProduct=  0x0000\n");
			fprintf(fp, "MessageContent=\"55534243123456788000000080000606f50402527000000000000000000000\"\n");
			break;
		case SN_AnyDATA_ADU_500A:
			fprintf(fp, "DefaultVendor=  0x05c6\n");
			fprintf(fp, "DefaultProduct= 0x1000\n");
			fprintf(fp, "TargetVendor=   0x16d5\n");
			fprintf(fp, "TargetProduct=  0x6502\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
			break;
		case SN_BandLuxe_C120:
			fprintf(fp, "DefaultVendor=  0x1a8d\n");
			fprintf(fp, "DefaultProduct= 0x1000\n");
			fprintf(fp, "TargetVendor=   0x1a8d\n");
			fprintf(fp, "TargetProduct=  0x1002\n");
			fprintf(fp, "MessageContent=\"55534243123456781200000080000603000000020000000000000000000000\"\n");
			fprintf(fp, "ResponseNeeded=1\n");
			break;
		case SN_Solomon_S3Gm660:
			fprintf(fp, "DefaultVendor=  0x1dd6\n");
			fprintf(fp, "DefaultProduct= 0x1000\n");
			fprintf(fp, "TargetVendor=   0x1dd6\n");
			fprintf(fp, "TargetProduct=  0x1002\n");
			fprintf(fp, "MessageContent=\"55534243123456781200000080000603000000020000000000000000000000\"\n");
			fprintf(fp, "ResponseNeeded=1\n");
			break;
		case SN_C_motechD50:
			fprintf(fp, "DefaultVendor=  0x16d8\n");
			fprintf(fp, "DefaultProduct= 0x6803\n");
			fprintf(fp, "TargetVendor=   0x16d8\n");
			fprintf(fp, "TargetProduct=  0x680a\n");
			fprintf(fp, "MessageContent=\"555342431234567824000000800008ff524445564348470000000000000000\"\n");
			break;
		case SN_C_motech_CGU628:
			fprintf(fp, "DefaultVendor=  0x16d8\n");
			fprintf(fp, "DefaultProduct= 0xf000\n");
			fprintf(fp, "TargetVendor=   0x16d8\n");
			fprintf(fp, "TargetProduct=  0x6006\n");
			fprintf(fp, "MessageContent=\"55534243d85dd88524000000800008ff524445564348470000000000000000\"\n");
			break;
		case SN_Toshiba_G450:
			fprintf(fp, "DefaultVendor=  0x0930\n");
			fprintf(fp, "DefaultProduct= 0x0d46\n");
			fprintf(fp, "TargetVendor=   0x0930\n");
			fprintf(fp, "TargetProduct=  0x0d45\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
			break;
		case SN_UTStarcom_UM175:
			fprintf(fp, "DefaultVendor=  0x106c\n");
			fprintf(fp, "DefaultProduct= 0x3b03\n");
			fprintf(fp, "TargetVendor=   0x106c\n");
			fprintf(fp, "TargetProduct=  0x3715\n");
			fprintf(fp, "MessageContent=\"555342431234567824000000800008ff024445564348470000000000000000\"\n");
			break;
		case SN_Hummer_DTM5731:
			fprintf(fp, "DefaultVendor=  0x1ab7\n");
			fprintf(fp, "DefaultProduct= 0x5700\n");
			fprintf(fp, "TargetVendor=   0x1ab7\n");
			fprintf(fp, "TargetProduct=  0x5731\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
			break;
		case SN_A_Link_3GU:
			fprintf(fp, "DefaultVendor=  0x1e0e\n");
			fprintf(fp, "DefaultProduct= 0xf000\n");
			fprintf(fp, "TargetVendor=   0x1e0e\n");
			fprintf(fp, "TargetProduct=  0x9200\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
			break;
		case SN_Sierra_Wireless_Compass597:
			fprintf(fp, "DefaultVendor=  0x1199\n");
			fprintf(fp, "DefaultProduct= 0x0fff\n");
			fprintf(fp, "TargetVendor=   0x1199\n");
			fprintf(fp, "TargetProduct=  0x0023\n");
			fprintf(fp, "SierraMode=1\n");
			break;
		case SN_Sierra881U:
			fprintf(fp, "DefaultVendor=  0x1199\n");
			fprintf(fp, "DefaultProduct= 0x0fff\n");
			fprintf(fp, "TargetVendor=   0x1199\n");
			fprintf(fp, "TargetProduct=  0x6856\n");
			fprintf(fp, "SierraMode=1\n");
			break;
		case SN_Sony_Ericsson_MD400:
			fprintf(fp, "DefaultVendor=  0x0fce\n");
			fprintf(fp, "DefaultProduct= 0xd0e1\n");
			fprintf(fp, "TargetClass=    0x02\n");
			fprintf(fp, "SonyMode=1\n");
			fprintf(fp, "Configuration=2\n");
			break;
		case SN_LG_LDU_1900D:
			fprintf(fp, "DefaultVendor=  0x1004\n");
			fprintf(fp, "DefaultProduct= 0x1000\n");
			fprintf(fp, "MessageContent=\"55534243123456780000000000000aff554d53434847000000000000000000\"\n");
			break;
		case SN_Samsung_SGH_Z810:
			fprintf(fp, "DefaultVendor=  0x05c6\n");
			fprintf(fp, "DefaultProduct= 0x1000\n");
			fprintf(fp, "TargetVendor=   0x04e8\n");
			fprintf(fp, "TargetProduct=  0x6601\n");
			fprintf(fp, "MessageContent=\"55534243123456780000000000000616000000000000000000000000000000\"\n");
			break;
		case SN_MobiData_MBD_200HU:
			fprintf(fp, "DefaultVendor=  0x1c9e\n");
			fprintf(fp, "DefaultProduct= 0xf000\n");
			fprintf(fp, "TargetVendor=   0x1c9e\n");
			fprintf(fp, "TargetProduct=  0x9000\n");
			fprintf(fp, "MessageContent=\"55534243123456788000000080000606f50402527000000000000000000000\"\n");
			break;
		case SN_BSNL_310G:
			fprintf(fp, "DefaultVendor=  0x1c9e\n");
			fprintf(fp, "DefaultProduct= 0xf000\n");
			fprintf(fp, "TargetVendor=   0x1c9e\n");
			fprintf(fp, "TargetProduct=  0x9605\n");
			fprintf(fp, "MessageContent=\"55534243123456788000000080000606f50402527000000000000000000000\"\n");
		case SN_ST_Mobile:
			fprintf(fp, "DefaultVendor=  0x1c9e\n");
			fprintf(fp, "DefaultProduct= 0xf000\n");
			fprintf(fp, "TargetVendor=   0x1c9e\n");
			fprintf(fp, "TargetProduct=  0x9063\n");
			fprintf(fp, "MessageContent=\"55534243123456788000000080000606f50402527000000000000000000000\"\n");
			break;
		case SN_MyWave_SW006:
			fprintf(fp, "DefaultVendor=  0x1c9e\n");
			fprintf(fp, "DefaultProduct= 0x9200\n");
			fprintf(fp, "TargetVendor=   0x1c9e\n");
			fprintf(fp, "TargetProduct=  0x9202\n");
			fprintf(fp, "MessageContent=\"55534243123456780000000000000606f50402527000000000000000000000\"\n");
			break;
		case SN_Cricket_A600:
			fprintf(fp, "DefaultVendor=  0x1f28\n");
			fprintf(fp, "DefaultProduct= 0x0021\n");
			fprintf(fp, "TargetVendor=   0x1f28\n");
			fprintf(fp, "TargetProduct=  0x0020\n");
			fprintf(fp, "MessageContent=\"555342431234567824000000800108df200000000000000000000000000000\"\n");
			break;
		case SN_EpiValley_SEC7089:
			fprintf(fp, "DefaultVendor=  0x1b7d\n");
			fprintf(fp, "DefaultProduct= 0x0700\n");
			fprintf(fp, "TargetVendor=   0x1b7d\n");
			fprintf(fp, "TargetProduct=  0x0001\n");
			fprintf(fp, "MessageContent=\"555342431234567824000000800008FF05B112AEE102000000000000000000\"\n");
			break;
		case SN_Samsung_U209:
			fprintf(fp, "DefaultVendor=  0x04e8\n");
			fprintf(fp, "DefaultProduct= 0xf000\n");
			fprintf(fp, "TargetVendor=   0x04e8\n");
			fprintf(fp, "TargetProduct=  0x6601\n");
			fprintf(fp, "MessageContent=\"55534243123456780000000000000616000000000000000000000000000000\"\n");
			break;
		case SN_D_Link_DWM162_U5:
			fprintf(fp, "DefaultVendor=  0x05c6\n");
			fprintf(fp, "DefaultProduct= 0x2001\n");
			fprintf(fp, "TargetVendor=   0x1e0e\n");
			fprintf(fp, "TargetProduct=  0xce16\n");
			fprintf(fp, "MessageContent=\"55534243e0c26a85000000000000061b000000020000000000000000000000\"\n");
			break;
		case SN_Novatel_MC760:
			fprintf(fp, "DefaultVendor=  0x1410\n");
			fprintf(fp, "DefaultProduct= 0x5031\n");
			fprintf(fp, "TargetVendor=   0x1410\n");
			fprintf(fp, "TargetProduct=  0x6002\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
			break;
		case SN_Philips_TalkTalk:
			fprintf(fp, "DefaultVendor=  0x0471\n");
			fprintf(fp, "DefaultProduct= 0x1237\n");
			fprintf(fp, "TargetVendor=   0x0471\n");
			fprintf(fp, "TargetProduct=  0x1234\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000030000000000000000000000\"\n");
			break;
		case SN_HuaXing_E600:
			fprintf(fp, "DefaultVendor=  0x0471\n");
			fprintf(fp, "DefaultProduct= 0x1237\n");
			fprintf(fp, "TargetVendor=   0x0471\n");
			fprintf(fp, "TargetProduct=  0x1206\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
			fprintf(fp, "Configuration=2\n");
			break;
		case SN_C_motech_CHU_629S:
			fprintf(fp, "DefaultVendor=  0x16d8\n");
			fprintf(fp, "DefaultProduct= 0x700a\n");
			fprintf(fp, "TargetClass=0xff\n");
			fprintf(fp, "MessageContent=\"55534243123456782400000080000dfe524445564348473d4e444953000000\"\n");
			break;
		case SN_Sagem9520:
			fprintf(fp, "DefaultVendor=  0x1076\n");
			fprintf(fp, "DefaultProduct= 0x7f40\n");
			fprintf(fp, "TargetVendor=   0x1076\n");
			fprintf(fp, "TargetProduct=  0x7f00\n");
			fprintf(fp, "GCTMode=1\n");
			break;
		case SN_Nokia_CS15:
			fprintf(fp, "DefaultVendor=  0x0421\n");
			fprintf(fp, "DefaultProduct= 0x0610\n");
			fprintf(fp, "TargetVendor=   0x0421\n");
			fprintf(fp, "TargetProduct=  0x0612\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
			break;
		case SN_Vodafone_MD950:
			fprintf(fp, "DefaultVendor=  0x0471\n");
			fprintf(fp, "DefaultProduct= 0x1210\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
			break;
		case SN_Siptune_LM75:
			fprintf(fp, "DefaultVendor=  0x05c6\n");
			fprintf(fp, "DefaultProduct= 0xf000\n");
			fprintf(fp, "TargetVendor=   0x05c6\n");
			fprintf(fp, "TargetProduct=  0x9000\n");
			fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
			break;
		default:
			fprintf(fp, "\n");
			if(vid && pid){
				fprintf(fp, "DefaultVendor=  0x%s\n", vid);
				fprintf(fp, "DefaultProduct= 0x%s\n", pid);
				if(strcmp(vid, "12d1") == 0){    // huawei
					//fprintf(fp, "DetachStorageOnly=1\n");
					fprintf(fp, "HuaweiMode=1\n");
				}
			}
			break;
	}

	return 0;
}

int init_3g_param(char *vid, char *pid)
{
	FILE *fp;
	int asus_extra_auto = 0;

	unlink(USB_MODESWITCH_CONF);
	fp = fopen(USB_MODESWITCH_CONF, "w+");
	if(!fp)
		return 0;

	if(strstr(nvram_safe_get("Dev3G"), "_AEAUTO"))
		asus_extra_auto = 1;

	if(nvram_match("Dev3G", "AUTO") || (asus_extra_auto == 1)){
		if(asus_extra_auto)
			nvram_set("d3g", nvram_safe_get("Dev3G"));
		else
			nvram_set("d3g", "usb_3g_dongle");

		if(!strcmp(vid, "0408") && (!strcmp(pid, "ea02") || !strcmp(pid, "1000")))
			write_3g_conf(fp, SN_MU_Q101, 1, vid, pid);
		else if((strcmp(vid, "0af0")==0) && (strcmp(pid, "6971")==0))
		{
			nvram_set("d3g", "OPTION-ICON225");
			write_3g_conf(fp, SN_OPTION_ICON225, 1, vid, pid);
		}
		else if((strcmp(vid, "05c6")==0) && (strcmp(pid, "1000")==0)) // also Option-GlobeSurfer-Icon72(may have new fw setting, bug not included here), Option-GlobeTrotter-GT-MAX36.....Option-Globexx series, AnyDATA-ADU-500A, Samsung-SGH-Z810, Vertex Wireless 100 Series
			write_3g_conf(fp, SN_Option_GlobeSurfer_Icon, 1, vid, pid);
		else if((strcmp(vid, "1e0e")==0) && (strcmp(pid, "f000")==0))	// A-Link-3GU
			write_3g_conf(fp, SN_Option_iCON210, 1, vid, pid);
		else if((strcmp(vid, "0af0")==0) && (strcmp(pid, "7011")==0))
		{
			nvram_set("d3g", "Option-GlobeTrotter-HSUPA-Modem");
			write_3g_conf(fp, SN_Option_GlobeTrotter_HSUPA_Modem, 1, vid, pid);
		}
		else if((strcmp(vid, "0af0")==0) && (strcmp(pid, "7401")==0))
		{
			nvram_set("d3g", "Option-iCON-401");
			write_3g_conf(fp, SN_Option_iCON401, 1, vid, pid);
		}
		else if((strcmp(vid, "0af0")==0) && (strcmp(pid, "7501")==0))
		{
			nvram_set("d3g", "Vodafone-K3760");
			write_3g_conf(fp, SN_Vodafone_K3760, 1, vid, pid);
		}
		else if((strcmp(vid, "0af0")==0) && (strcmp(pid, "d033")==0))
		{
			nvram_set("d3g", "ATT-USBConnect-Quicksilver");
			write_3g_conf(fp, SN_ATT_USBConnect_Quicksilver, 1, vid, pid);
		}
		else if((strcmp(vid, "12d1")==0) && (strcmp(pid, "1001")==0))
			write_3g_conf(fp, SN_Huawei_E169, 1, vid, pid);
		else if((strcmp(vid, "12d1")==0) && (strcmp(pid, "1003")==0))
			write_3g_conf(fp, SN_Huawei_E220, 1, vid, pid);
		else if((strcmp(vid, "12d1")==0) && (strcmp(pid, "1414")==0))
			write_3g_conf(fp, SN_Huawei_E180, 1, vid, pid);
		else if((strcmp(vid, "1033")==0) && (strcmp(pid, "0035")==0))
			write_3g_conf(fp, SN_Huawei_E630, 1, vid, pid);
		else if((strcmp(vid, "12d1")==0) && (strcmp(pid, "1446")==0))	// confused w/ E1550, E161, E1612, E1690
			write_3g_conf(fp, SN_Huawei_E270, 1, vid, pid);
		/*else if((strcmp(vid, "19d2")==0) && (strcmp(pid, "2000")==0))	// also ZTE622, 628, 626, 6535-Z, K3520-Z, K3565, ONDA-MT503HS, ONDA-MT505UP
		{
			nvram_set("d3g", "ZTE-MF626");
			write_3g_conf(fp, SN_ZTE_MF626, 1, vid, pid);
		}//*/
		else if((strcmp(vid, "19d2")==0) && (strcmp(pid, "fff5")==0))
			write_3g_conf(fp, SN_ZTE_AC8710, 1, vid, pid);	// 2710
		else if((strcmp(vid, "1410")==0) && (strcmp(pid, "5010")==0))	// U727
			write_3g_conf(fp, SN_Novatel_Wireless_Ovation_MC950D, 1, vid, pid);
		else if((strcmp(vid, "1410")==0) && (strcmp(pid, "5020")==0))
			write_3g_conf(fp, SN_Novatel_MC990D, 1, vid, pid);
		else if((strcmp(vid, "1410")==0) && (strcmp(pid, "5030")==0))
			write_3g_conf(fp, SN_Novatel_U760, 1, vid, pid);
		else if((strcmp(vid, "1c9e")==0) && (strcmp(pid, "1001")==0))
			write_3g_conf(fp, SN_Alcatel_X020, 1, vid, pid);
		else if((strcmp(vid, "1bbb")==0) && (strcmp(pid, "f000")==0))
			write_3g_conf(fp, SN_Alcatel_X200, 1, vid, pid);
		//else if((strcmp(vid, "1a8d")==0) && (strcmp(pid, "1000")==0))
		//	write_3g_conf(fp, SN_BandLuxe_C120, 1, vid, pid);
		else if((strcmp(vid, "1a8d")==0) && (strcmp(pid, "1000")==0))
		{
			nvram_set("d3g", "BandLuxe-C170");
		}
		else if((strcmp(vid, "1a8d")==0) && (strcmp(pid, "1009")==0))
			nvram_set("d3g", "BandLuxe-C170");
		else if((strcmp(vid, "1dd6")==0) && (strcmp(pid, "1000")==0))
			write_3g_conf(fp, SN_Solomon_S3Gm660, 1, vid, pid);
		else if((strcmp(vid, "16d8")==0) && (strcmp(pid, "6803")==0))
			write_3g_conf(fp, SN_C_motechD50, 1, vid, pid);
		else if((strcmp(vid, "16d8")==0) && (strcmp(pid, "f000")==0))
			write_3g_conf(fp, SN_C_motech_CGU628, 1, vid, pid);
		else if((strcmp(vid, "0930")==0) && (strcmp(pid, "0d46")==0))
			write_3g_conf(fp, SN_Toshiba_G450, 1, vid, pid);
		else if((strcmp(vid, "106c")==0) && (strcmp(pid, "3b03")==0))
			write_3g_conf(fp, SN_UTStarcom_UM175, 1, vid, pid);
		else if((strcmp(vid, "1ab7")==0) && (strcmp(pid, "5700")==0))
			write_3g_conf(fp, SN_Hummer_DTM5731, 1, vid, pid);
		else if((strcmp(vid, "1199")==0) && (strcmp(pid, "0fff")==0))	// Sierra881U
			write_3g_conf(fp, SN_Sierra_Wireless_Compass597, 1, vid, pid);
		else if((strcmp(vid, "0fce")==0) && (strcmp(pid, "d0e1")==0))
			write_3g_conf(fp, SN_Sony_Ericsson_MD400, 1, vid, pid);
		else if((strcmp(vid, "1004")==0) && (strcmp(pid, "1000")==0))
			write_3g_conf(fp, SN_LG_LDU_1900D, 1, vid, pid);
		else if((strcmp(vid, "1c9e")==0) && (strcmp(pid, "f000")==0))	// ST-Mobile, MobiData MBD-200HU, // BSNL 310G
			write_3g_conf(fp, SN_BSNL_310G, 1, vid, pid);
		else if((strcmp(vid, "1c9e")==0) && (strcmp(pid, "9605")==0))	// chk BSNL 310G
			write_3g_conf(fp, SN_BSNL_310G, 1, vid, pid);
		else if((strcmp(vid, "1c9e")==0) && (strcmp(pid, "9200")==0))
			write_3g_conf(fp, SN_MyWave_SW006, 1, vid, pid);
		else if((strcmp(vid, "1f28")==0) && (strcmp(pid, "0021")==0))
			write_3g_conf(fp, SN_Cricket_A600, 1, vid, pid);
		else if((strcmp(vid, "1b7d")==0) && (strcmp(pid, "0700")==0))
			write_3g_conf(fp, SN_EpiValley_SEC7089, 1, vid, pid);
		else if((strcmp(vid, "04e8")==0) && (strcmp(pid, "f000")==0))
			write_3g_conf(fp, SN_Samsung_U209, 1, vid, pid);
		else if((strcmp(vid, "05c6")==0) && (strcmp(pid, "2001")==0))
			write_3g_conf(fp, SN_D_Link_DWM162_U5, 1, vid, pid);
		else if((strcmp(vid, "1410")==0) && (strcmp(pid, "5031")==0))
			write_3g_conf(fp, SN_Novatel_MC760, 1, vid, pid);
		else if((strcmp(vid, "19d2")==0) && (strcmp(pid, "0053")==0))
			write_3g_conf(fp, SN_ZTE_MF110, 1, vid, pid);
		else if((strcmp(vid, "0471")==0) && (strcmp(pid, "1237")==0))	// HuaXing E600
			write_3g_conf(fp, SN_Philips_TalkTalk, 1, vid, pid);
		else if((strcmp(vid, "16d8")==0) && (strcmp(pid, "700a")==0))
			write_3g_conf(fp, SN_C_motech_CHU_629S, 1, vid, pid);
		else if((strcmp(vid, "1076")==0) && (strcmp(pid, "7f40")==0))
			write_3g_conf(fp, SN_Sagem9520, 1, vid, pid);
		else if((strcmp(vid, "0421")==0) && (strcmp(pid, "0610")==0))
			write_3g_conf(fp, SN_Nokia_CS15, 1, vid, pid);
		else if((strcmp(vid, "12d1")==0) && (strcmp(pid, "1520")==0))
			write_3g_conf(fp, SN_Huawei_K3765, 1, vid, pid);
		else if((strcmp(vid, "12d1")==0) && (strcmp(pid, "1521")==0))
			write_3g_conf(fp, SN_Huawei_K4505, 1, vid, pid);
		else if((strcmp(vid, "0471")==0) && (strcmp(pid, "1210")==0))
			write_3g_conf(fp, SN_Vodafone_MD950, 1, vid, pid);
		else if((strcmp(vid, "05c6")==0) && (strcmp(pid, "f000")==0))
			write_3g_conf(fp, SN_Siptune_LM75, 1, vid, pid);
		/*else
			write_3g_conf(fp, UNKNOWNDEV, 1, vid, pid);//*/
		else{
			fclose(fp);
			unlink(USB_MODESWITCH_CONF);
			return 0;
		}
	}
	else	/* manaul setting */
	{
		nvram_set("d3g", nvram_safe_get("Dev3G"));

		if(strcmp(nvram_safe_get("Dev3G"), "MU-Q101") == 0){					// on list
			write_3g_conf(fp, SN_MU_Q101, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ASUS-T500") == 0){				// on list
			write_3g_conf(fp, UNKNOWNDEV, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "OPTION-ICON225") == 0){			// on list
			write_3g_conf(fp, SN_OPTION_ICON225, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Option-GlobeSurfer-Icon") == 0){
			write_3g_conf(fp, SN_Option_GlobeSurfer_Icon, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Option-GlobeSurfer-Icon-7.2") == 0){
			write_3g_conf(fp, SN_Option_GlobeSurfer_Icon72, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Option-GlobeTrotter-GT-MAX-3.6") == 0){
			write_3g_conf(fp, SN_Option_GlobeTrotter_GT_MAX36, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Option-GlobeTrotter-GT-MAX-7.2") == 0){
			write_3g_conf(fp, SN_Option_GlobeTrotter_GT_MAX72, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Option-GlobeTrotter-EXPRESS-7.2") == 0){
			write_3g_conf(fp, SN_Option_GlobeTrotter_EXPRESS72, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Option-iCON-210") == 0){
			write_3g_conf(fp, SN_Option_iCON210, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Option-GlobeTrotter-HSUPA-Modem") == 0){
			write_3g_conf(fp, SN_Option_GlobeTrotter_HSUPA_Modem, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Option-iCON-401") == 0){
			write_3g_conf(fp, SN_Option_iCON401, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Vodafone-K3760") == 0){
			write_3g_conf(fp, SN_Vodafone_K3760, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ATT-USBConnect-Quicksilver") == 0){
			write_3g_conf(fp, SN_ATT_USBConnect_Quicksilver, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "HUAWEI-E160G") == 0){			// on list
			write_3g_conf(fp, UNKNOWNDEV, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "HUAWEI-E169") == 0){			// on list
			write_3g_conf(fp, SN_Huawei_E169, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "HUAWEI-E176") == 0){			// on list
			write_3g_conf(fp, UNKNOWNDEV, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-E180") == 0){			// on list
			//write_3g_conf(fp, SN_Huawei_E180, 0, vid, pid);
			write_3g_conf(fp, SN_Huawei_E220, 1, vid, pid);		// E180:12d1/1003 (as E220)
		} else if (strcmp(nvram_safe_get("Dev3G"), "HUAWEI-E220") == 0){			// on list
			write_3g_conf(fp, SN_Huawei_E220, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-E630") == 0){
			write_3g_conf(fp, SN_Huawei_E630, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-E270") == 0){
			write_3g_conf(fp, SN_Huawei_E270, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-E1550") == 0){
			write_3g_conf(fp, SN_Huawei_E1550, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-E161") == 0){
			write_3g_conf(fp, SN_Huawei_E1612, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-E1612") == 0){
			write_3g_conf(fp, SN_Huawei_E1612, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-E1690") == 0){
			write_3g_conf(fp, SN_Huawei_E1690, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-K3765") == 0){
			write_3g_conf(fp, SN_Huawei_K3765, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-K4505") == 0){
			write_3g_conf(fp, SN_Huawei_K4505, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-MF620") == 0){
			write_3g_conf(fp, SN_ZTE_MF620, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-MF622") == 0){
			write_3g_conf(fp, SN_ZTE_MF622, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-MF628") == 0){
			write_3g_conf(fp, SN_ZTE_MF628, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-MF626") == 0){
			write_3g_conf(fp, SN_ZTE_MF626, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-AC8710") == 0){
			write_3g_conf(fp, SN_ZTE_AC8710, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-AC2710") == 0){
			write_3g_conf(fp, SN_ZTE_AC2710, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-6535-Z") == 0){
			write_3g_conf(fp, SN_ZTE6535_Z, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-K3520-Z") == 0){
			write_3g_conf(fp, SN_ZTE_K3520_Z, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-MF110") == 0){
			write_3g_conf(fp, SN_ZTE_MF110, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-K3565") == 0){
			write_3g_conf(fp, SN_ZTE_K3565, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ONDA-MT503HS") == 0){
			write_3g_conf(fp, SN_ONDA_MT503HS, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ONDA-MT505UP") == 0){
			write_3g_conf(fp, SN_ONDA_MT505UP, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Novatel-Wireless-Ovation-MC950D-HSUPA") == 0){
			write_3g_conf(fp, SN_Novatel_Wireless_Ovation_MC950D, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Novatel-U727") == 0){
			write_3g_conf(fp, SN_Novatel_U727, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Novatel-MC990D") == 0){
			write_3g_conf(fp, SN_Novatel_MC990D, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Novatel-U760") == 0){
			write_3g_conf(fp, SN_Novatel_U760, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Alcatel-X020") == 0){
			write_3g_conf(fp, SN_Alcatel_X020, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Alcatel-X200") == 0){
			write_3g_conf(fp, SN_Alcatel_X200, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "AnyDATA-ADU-500A") == 0){
			write_3g_conf(fp, SN_AnyDATA_ADU_500A, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "BandLuxe-C120") == 0){			// on list
			write_3g_conf(fp, SN_BandLuxe_C120, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "BandLuxe-C170") == 0){			// on list
			nvram_set("d3g", "BandLuxe-C170");	// band270
		} else if (strcmp(nvram_safe_get("Dev3G"), "Solomon-S3Gm-660") == 0){
			write_3g_conf(fp, SN_Solomon_S3Gm660, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "C-motechD-50") == 0){
			write_3g_conf(fp, SN_C_motechD50, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "C-motech-CGU-628") == 0){
			write_3g_conf(fp, SN_C_motech_CGU628, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Toshiba-G450") == 0){
			write_3g_conf(fp, SN_Toshiba_G450, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "UTStarcom-UM175") == 0){
			write_3g_conf(fp, SN_UTStarcom_UM175, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Hummer-DTM5731") == 0){
			write_3g_conf(fp, SN_Hummer_DTM5731, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "A-Link-3GU") == 0){
			write_3g_conf(fp, SN_A_Link_3GU, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Sierra-Wireless-Compass-597") == 0){
			write_3g_conf(fp, SN_Sierra_Wireless_Compass597, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Sierra-881U") == 0){
			write_3g_conf(fp, SN_Sierra881U, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Sony-Ericsson-MD400") == 0){
			write_3g_conf(fp, SN_Sony_Ericsson_MD400, 0, vid, pid);
		//} else if (strcmp(nvram_safe_get("Dev3G"), "Sony-Ericsson-W910i") == 0){		// on list
		//	write_3g_conf(fp, UNKNOWNDEV, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "LG-LDU-1900D") == 0){
			write_3g_conf(fp, SN_LG_LDU_1900D, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Samsung-SGH-Z810") == 0){
			write_3g_conf(fp, SN_Samsung_SGH_Z810, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "MobiData-MBD-200HU") == 0){
			write_3g_conf(fp, SN_MobiData_MBD_200HU, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ST-Mobile") == 0){
			write_3g_conf(fp, SN_ST_Mobile, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "MyWave-SW006") == 0){
			write_3g_conf(fp, SN_MyWave_SW006, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Cricket-A600") == 0){
			write_3g_conf(fp, SN_Cricket_A600, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "EpiValley-SEC-7089") == 0){
			write_3g_conf(fp, SN_EpiValley_SEC7089, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Samsung-U209") == 0){
			write_3g_conf(fp, SN_Samsung_U209, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "D-Link-DWM-162-U5") == 0){
			write_3g_conf(fp, SN_D_Link_DWM162_U5, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Novatel-MC760") == 0){
			write_3g_conf(fp, SN_Novatel_MC760, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Philips-TalkTalk") == 0){
			write_3g_conf(fp, SN_Philips_TalkTalk, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "HuaXing-E600") == 0){
			write_3g_conf(fp, SN_HuaXing_E600, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "C-motech-CHU-629S") == 0){
			write_3g_conf(fp, SN_C_motech_CHU_629S, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Sagem-9520") == 0){
			write_3g_conf(fp, SN_Sagem9520, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Nokia-CS-15") == 0){
			write_3g_conf(fp, SN_Nokia_CS15, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Vodafone-MD950") == 0){
			write_3g_conf(fp, SN_Vodafone_MD950, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Siptune-LM-75") == 0){
			write_3g_conf(fp, SN_Siptune_LM75, 0, vid, pid);
		} else{
			fclose(fp);
			unlink(USB_MODESWITCH_CONF);
			return 0;
		}
	}
	fclose(fp);

	return 1;
}

int write_3g_ppp_conf(const char *modem_node){
	FILE *fp;
	char cmd[128];
	char target_path[128], usb_path[PATH_MAX];
	char usb_port[4], vid[8], pid[8];

	if(!isSerialNode(modem_node) && !isACMNode(modem_node))
		return 0;

	unlink(PPP_CONF_FOR_3G);
	if((fp = fopen(PPP_CONF_FOR_3G, "w+")) == NULL){
		memset(cmd, 0, 128);
		sprintf(cmd, "mkdir -p %s", PPP_DIR);
		system(cmd);

		if((fp = fopen(PPP_CONF_FOR_3G, "w+")) == NULL)
			return 0;
	}

	// Get USB port.
	memset(target_path, 0, 128);
	sprintf(target_path, "%s/%s/device", SYS_TTY, modem_node);
	memset(usb_path, 0, PATH_MAX);
	if(realpath(target_path, usb_path) == NULL)
		return 0;

	if(get_usb_port(usb_path, usb_port, 4) == NULL)
		return 0;

	// Get VID.
	if(get_usb_vid(usb_port, vid, 8) == NULL)
		return 0;

	// Get PID.
	if(get_usb_pid(usb_port, pid, 8) == NULL)
		return 0;

	char *user = nvram_safe_get("hsdpa_user");
	char *pass = nvram_safe_get("hsdpa_pass");
	char *isp = nvram_safe_get("hsdpa_isp");
	char *baud = nvram_safe_get("hsdpa_baud");
	char *EVDO_on = nvram_safe_get("EVDO_on");

	fprintf(fp, "/dev/%s\n", modem_node);
	if(strlen(baud) > 0)
		fprintf(fp, "%s\n", baud);
	if(strlen(user) > 0)
		fprintf(fp, "user %s\n", user);
	if(strlen(pass) > 0)
		fprintf(fp, "password %s\n", pass);
	if(!strcmp(isp, "Virgin")){
		fprintf(fp, "refuse-chap\n");
		fprintf(fp, "refuse-mschap\n");
		fprintf(fp, "refuse-mschap-v2\n");
	}
	fprintf(fp, "modem\n");
	fprintf(fp, "crtscts\n");
	fprintf(fp, "noauth\n");
	fprintf(fp, "defaultroute\n");
	fprintf(fp, "noipdefault\n");
	fprintf(fp, "nopcomp\n");
	fprintf(fp, "noaccomp\n");
	fprintf(fp, "novj\n");
	fprintf(fp, "nobsdcomp\n");
	fprintf(fp, "holdoff 10\n");
	fprintf(fp, "usepeerdns\n");
	fprintf(fp, "persist\n");
	fprintf(fp, "nodeflate\n");
	if(!strcmp(EVDO_on, "1")){
		fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s /etc_ro/ppp/3g/EVDO_conn.scr\"\n", modem_node);
		fprintf(fp, "disconnect \"/bin/comgt -d /dev/%s -s /etc_ro/ppp/3g/EVDO_disconn.scr\"\n", modem_node);
	}
	else{
		if(!strcmp(vid, "0b05") && !strcmp(pid, "0302")) // T500
			fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s /etc_ro/ppp/3g/t500_conn.scr\"\n", modem_node);
		else if(!strcmp(vid, "106c") && !strcmp(pid, "3716"))
			fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s /etc_ro/ppp/3g/verizon_conn.scr\"\n", modem_node);
		else if(!strcmp(vid, "1410") && !strcmp(pid, "4400"))
			fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s /etc_ro/ppp/3g/rogers_conn.scr\"\n", modem_node);
		else
			fprintf(fp, "connect \"/bin/comgt -d /dev/%s -s /etc_ro/ppp/3g/Generic_conn.scr\"\n", modem_node);
		
		fprintf(fp, "disconnect \"/bin/comgt -d /dev/%s -s /etc_ro/ppp/3g/Generic_disconn.scr\"\n", modem_node);
	}

	fclose(fp);
	return 1;
}
#endif// SUPPORT_USB_MODEM

#ifdef SUPPORT_USB_DISK
// 201102. James. Move the Jiahao's code from mdev. {
int
check_partition(const char *devname)
{
	FILE *procpt;
	char line[256], ptname[32], ptname_check[32];
	int ma, mi, sz;

	if (devname && (procpt = fopen("/proc/partitions", "r")))
	{
		sprintf(ptname_check, "%s1", devname);

		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, " %d %d %d %[^\n ]", &ma, &mi, &sz, ptname) != 4)
				continue;
			if (!strcmp(ptname, ptname_check))
			{
				fclose(procpt);
				return 1;
			}
		}

		fclose(procpt);
	}

	return 0;
}
// 201102. James. Move the Jiahao's code from mdev. }

// 201102. James. Move the Jiahao's code from rc/service_ex.c. {
int
check_dev_sb_block_count(const char *dev_sd)
{
	FILE *procpt;
	char line[256], ptname[32];
	int ma, mi, sz;

	if ((procpt = fopen_or_warn("/proc/partitions", "r")) != NULL)
	{
		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, " %d %d %d %[^\n ]", &ma, &mi, &sz, ptname) != 4)
				continue;

			if (!strcmp(dev_sd, ptname) && (sz > 1) )
			{
				fclose(procpt);
				return 1;
			}
		}

		fclose(procpt);
	}

	return 0;
}
// 201102. James. Move the Jiahao's code from rc/service_ex.c. }
#endif // SUPPORT_USB_DISK

// 1: add, 0: remove.
int check_hotplug_action(const char *action){
	if(!strcmp(action, "remove"))
		return 0;
	else
		return 1;
}

// 0: unknown device, 1: storage, 2: printer, 3: modem.
int get_device_type_on_name(const char *device_name){
	if(device_name == NULL || strlen(device_name) <= 0){
		usb_dbg("(%s): The device name is not correct.\n", device_name);
		return 0;
	}

	if(!strncmp(device_name, "sd", 2)){
		return 1;
	}

	if(!strncmp(device_name, "lp", 2)){
		return 2;
	}

	if(isSerialNode(device_name) || isACMNode(device_name)){
		return 3;
	}

	return 0;
}

char *get_device_type_on_port(const char *usb_port, char *buf, const int buf_size){
	int interface_num, interface_count;
	char interface_name[8];
#ifdef SUPPORT_USB_PRINTER
	int got_printer = 0;
#endif
#ifdef SUPPORT_USB_MODEM
	int got_modem = 0;
#endif
#ifdef SUPPORT_USB_DISK
	int got_storage = 0;
#endif
	int got_others = 0;

	interface_num = get_usb_interface_number(usb_port);
	if(interface_num <= 0)
		return NULL;

	for(interface_count = 0; interface_count < interface_num; ++interface_count){
		memset(interface_name, 0, 8);
		sprintf(interface_name, "%s:1.%d", usb_port, interface_count);

#ifdef SUPPORT_USB_PRINTER
		if(isPrinterInterface(interface_name))
			++got_printer;
		else
#endif // SUPPORT_USB_PRINTER
#ifdef SUPPORT_USB_MODEM
		if(isSerialInterface(interface_name) || isACMInterface(interface_name))
			++got_modem;
		else
#endif // SUPPORT_USB_MODEM
#ifdef SUPPORT_USB_DISK
		if(isStorageInterface(interface_name))
			++got_storage;
		else
#endif // SUPPORT_USB_DISK
			++got_others;
	}

	if(
#ifdef SUPPORT_USB_PRINTER
			!got_printer
#else
			1
#endif
			&&
#ifdef SUPPORT_USB_MODEM
			!got_modem
#else
			1
#endif
			&&
#ifdef SUPPORT_USB_DISK
			!got_storage
#else
			1
#endif
			)
		return NULL;

	memset(buf, 0, buf_size);
#ifdef SUPPORT_USB_PRINTER
	if(got_printer > 0) // Top priority
		strcpy(buf, "printer");
	else
#endif // SUPPORT_USB_PRINTER
#ifdef SUPPORT_USB_MODEM
	if(got_modem > 0) // 2nd priority
		strcpy(buf, "modem");
	else
#endif // SUPPORT_USB_MODEM
#ifdef SUPPORT_USB_DISK
	if(got_storage > 0)
		strcpy(buf, "storage");
	else
#endif // SUPPORT_USB_DISK
		return NULL;

	return buf;
}

int set_usb_common_nvram(const char *action, const char *usb_port, const char *known_type){
	char *ptr;
	char nvram_name[32];
	char type[16], vid[8], pid[8], manufacturer[256], product[256], serial[256];
	char been_type[16];

	ptr = (char *)(usb_port+2);

	nvram_set("usb_path", ptr);

	if(!check_hotplug_action(action)){
		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%s", ptr);
		nvram_set(nvram_name, "");

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%s_vid", ptr);
		nvram_set(nvram_name, "");

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%s_pid", ptr);
		nvram_set(nvram_name, "");

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%s_manufacturer", ptr);
		nvram_set(nvram_name, "");

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%s_product", ptr);
		nvram_set(nvram_name, "");

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%s_serial", ptr);
		nvram_set(nvram_name, "");
	}
	else{
		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%s", ptr);
		memset(been_type, 0, 16);
		strcpy(been_type, nvram_safe_get(nvram_name));
		if(strlen(been_type) > 0){
#ifdef SUPPORT_USB_PRINTER
			if(!strcmp(been_type, "printer")){ // Top priority
				return 0;
			}
			else
#endif // SUPPORT_USB_PRINTER
#ifdef SUPPORT_USB_MODEM
			if(!strcmp(been_type, "modem")){ // 2nd priority
#ifdef SUPPORT_USB_PRINTER
				if(strcmp(known_type, "printer"))
#endif // SUPPORT_USB_PRINTER
					return 0;
			}
			else
#endif // SUPPORT_USB_MODEM
#ifdef SUPPORT_USB_DISK
			if(!strcmp(been_type, "storage")){
#if defined(SUPPORT_USB_PRINTER) || defined(SUPPORT_USB_MODEM)
				if(
#ifdef SUPPORT_USB_PRINTER
						strcmp(known_type, "printer")
#else
						1
#endif
					 	&&
#ifdef SUPPORT_USB_MODEM
						strcmp(known_type, "modem")
#else
						1
#endif
						)
#endif
					return 0;
			}
			else
#endif // SUPPORT_USB_DISK
			{ // unknown device.
				return 0;
			}
		}
		if(known_type != NULL)
			nvram_set(nvram_name, known_type);
		else if(get_device_type_on_port(usb_port, type, 16) != NULL)
			nvram_set(nvram_name, type);
		else // unknown device.
			return 0;

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%s_vid", ptr);
		if(get_usb_vid(usb_port, vid, 8) == NULL)
			nvram_set(nvram_name, "");
		else
			nvram_set(nvram_name, vid);

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%s_pid", ptr);
		if(get_usb_pid(usb_port, pid, 8) == NULL)
			nvram_set(nvram_name, "");
		else
			nvram_set(nvram_name, pid);

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%s_manufacturer", ptr);
		if(get_usb_manufacturer(usb_port, manufacturer, 256) == NULL)
			nvram_set(nvram_name, "");
		else
			nvram_set(nvram_name, manufacturer);

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%s_product", ptr);
		if(get_usb_product(usb_port, product, 256) == NULL)
			nvram_set(nvram_name, "");
		else
			nvram_set(nvram_name, product);

		memset(nvram_name, 0, 32);
		sprintf(nvram_name, "usb_path%s_serial", ptr);
		if(get_usb_serial(usb_port, serial, 256) == NULL)
			nvram_set(nvram_name, "");
		else
			nvram_set(nvram_name, serial);
	}

	return 0;
}

#ifdef SUPPORT_USB_MODEM
int is_usb_modem_ready(){
	if(nvram_match("modem_enable", "1")
			&& ((!strcmp(nvram_safe_get("usb_path1"), "modem") && strcmp(nvram_safe_get("usb_path1_act"), ""))
					|| (!strcmp(nvram_safe_get("usb_path2"), "modem") && strcmp(nvram_safe_get("usb_path2_act"), ""))
					)
			)
		return 1;
	else
		return 0;
}

int get_usb_modem_state(){
	if(!strcmp(nvram_safe_get("modem_running"), "1"))
		return 1;
	else
		return 0;
}

int set_usb_modem_state(const int flag){
	if(flag != 1 && flag != 0)
		return 0;

	if(flag){
		nvram_set("modem_running", "1");
		return 1;
	}
	else{
		nvram_set("modem_running", "0");
		return 0;
	}
}

int switch_usb_modem(const int flag){
	FILE *fp;
	int retry;

	if(flag != 1 && flag != 0)
		return 0;
	else if(flag){
		if(!is_usb_modem_ready())
			return 0; // No modem in USB ports.
		else if(get_usb_modem_state()) // Already have a running modem.
			return 0;
	}
	else{
		if(!get_usb_modem_state()) // Already have no running modem.
			return 0;
	}

	usb_dbg("(%d): Starting...\n", flag);
	// Set the modem to be running.
	set_usb_modem_state(flag);

	if(flag){
		// Wait the PPP config file to be done.
		retry = 0;
		while((fp = fopen(PPP_CONF_FOR_3G, "r")) == NULL && retry < MAX_WAIT_FILE)
			sleep(1);

		if(fp == NULL){
			usb_dbg("(%d): No pppd conf file and turn off the state of USB Modem.\n", flag);
			set_usb_modem_state(0);
			return 0;
		}
		else
			fclose(fp);
	}

#ifdef RTN56U
	// Wait the original rc_service to be done.
	while(strcmp(nvram_safe_get("rc_service"), "")){
		usb_dbg("(%d): There is the other action with rc_service.\n", flag);
		kill(1, SIGUSR1);
		sleep(1);
	}

	// Notify the init process to swith the connection line.
	usb_dbg("(%d): rc_service=restart_wan_line.\n", flag);
	nvram_set("rc_service", "restart_wan_line");
	kill(1, SIGUSR1);
	sleep(1);

	// Wait the notify to be done.
	while(strcmp(nvram_safe_get("rc_service"), "")){
		usb_dbg("(%d): init don't do rc_service yet, and try again.\n", flag);
		kill(1, SIGUSR1);
		sleep(1);
	}
#elif defined(RTN66U)
	notify_rc_and_wait("restart_wan_line");
#endif

	usb_dbg("(%d): End.\n", flag);
	return 1;
}
#endif // SUPPORT_USB_MODEM

int asus_sd(const char *device_name, const char *action){
#ifdef SUPPORT_USB_DISK
	char target_path[128], usb_path[PATH_MAX];
	char *ptr;
	char disk_name[4], usb_port[4], vid[8];
	char lock_file[128];
	int retry, isLock;
	FILE *fp;
	char nvram_name[32]; // 201102. James. Move the Jiahao's code from ~/drivers/usb/storage.
	int partition_order;
	usb_dbg("(%s): action=%s.\n", device_name, action);

	if(!strcmp(nvram_safe_get("stop_sd"), "1")){
		usb_dbg("(%s): stop_sd be set.\n", device_name);
		return 0;
	}

	if(get_device_type_on_name(device_name) != 1){
		usb_dbg("(%s): The device is not a sd device.\n", device_name);
		return 0;
	}

	// Check Lock.
	memset(lock_file, 0, 128);
	sprintf(lock_file, "%s.%s", LOCK_DEVICE, device_name);
	for(retry = 0, isLock = 0; retry < MAX_RETRY_LOCK; ++retry){
		if((fp = fopen(lock_file, "r")) == NULL){
			if((fp = fopen(lock_file, "w+")) != NULL){
				fputc('1', fp);
				fclose(fp);
				isLock = 1;

				break;
			}
		}
		else
			fclose(fp);

		sleep(1);
	}

	if(!isLock){
		usb_dbg("(%s): Can't set the file lock!\n", device_name);
		return 0;
	}

	// If remove the device?
	if(!check_hotplug_action(action)){
		memset(usb_port, 0, 4);
		if(!strcmp(nvram_safe_get("usb_path1_act"), device_name)){
			strcpy(usb_port, USB_EHCI_PORT_1);

			nvram_set("usb_path1_act", "");
		}
		else if(!strcmp(nvram_safe_get("usb_path2_act"), device_name)){
			strcpy(usb_port, USB_EHCI_PORT_2);

			nvram_set("usb_path2_act", "");
		}

		if(strlen(usb_port) > 0){
			

			usb_dbg("(%s): Remove Storage on USB Port %s.\n", device_name, usb_port);
		}
		else
			usb_dbg("(%s): Remove a unknown-port Storage.\n", device_name);

		unlink(lock_file);
		return 0;
		
	}

	// Get USB port.
	memset(disk_name, 0, 4);
	strncpy(disk_name, device_name, 3);
	memset(target_path, 0, 128);
	sprintf(target_path, "%s/%s/device", SYS_BLOCK, disk_name);
	memset(usb_path, 0, PATH_MAX);
	if(realpath(target_path, usb_path) == NULL){
		usb_dbg("(%s): Fail to get link: %s.\n", device_name, target_path);
		unlink(lock_file);
		return 0;
	}

	if(get_usb_port(usb_path, usb_port, 4) == NULL){
		usb_dbg("(%s): Fail to get usb port: %s.\n", device_name, usb_path);
		unlink(lock_file);
		return 0;
	}

	ptr = (char *)(usb_port+2);

	// Get VID.
	if(get_usb_vid(usb_port, vid, 8) == NULL){
		usb_dbg("(%s): Fail to get VID of USB(%s).\n", device_name, usb_port);
		unlink(lock_file);
		return 0;
	}

	// Don't support the storage of multi-interfaces.
	if(get_usb_interface_number(usb_port) > 1){
		if(strcmp(vid, "0bc2") && strcmp(vid, "1058") && strcmp(vid, "059f")
				&& strcmp(vid, nvram_safe_get("usb_vid_allow"))
				){
			usb_dbg("(%s): We only support single-interface storage device!.\n", device_name);
			unlink(lock_file);
			return 0;
		}
	}

// 201102. James. Move the Jiahao's code from ~/drivers/usb/storage. {
	// set USB common nvram.
	set_usb_common_nvram(action, usb_port, "storage");
// 201102. James. Move the Jiahao's code from ~/drivers/usb/storage. }

// 201102. James. Move the Jiahao's code from mdev. {
	if(strlen(device_name) == 3){ // sda, sdb, sdc...
		for(partition_order = 0; partition_order < 16 ; ++partition_order){
			memset(nvram_name, 0x0, 32);
			sprintf(nvram_name, "usb_path%s_fs_path%d", ptr, partition_order);
			nvram_unset(nvram_name);
		}

		memset(nvram_name, 0x0, 32);
		sprintf(nvram_name, "usb_path%s_fs_path0", ptr);
		nvram_set(nvram_name, device_name);

		memset(nvram_name, 0x0, 32);
		sprintf(nvram_name, "usb_path%s", ptr);
		nvram_set(nvram_name, "storage");

		memset(nvram_name, 0x0, 32);
		sprintf(nvram_name, "usb_path%s_act", ptr);
		nvram_set(nvram_name, device_name);

		memset(nvram_name, 0x0, 32);
		sprintf(nvram_name, "usb_path%s_add", ptr);
		nvram_set(nvram_name, "1");
	}
	else if(strlen(device_name) == 4 && device_name[3] == '1'){ // sda1, sdb1, sdc1...
		memset(nvram_name, 0x0, 32);
		sprintf(nvram_name, "usb_path%s_fs_path0", ptr);
		nvram_set(nvram_name, device_name);
	}
	else if(check_dev_sb_block_count(device_name)){ // sda2, sda3, sdb2, sdb3...
		partition_order = atoi(device_name+3);
		sprintf(nvram_name, "usb_path%s_fs_path%d", ptr, partition_order-1);
		nvram_set(nvram_name, device_name);
	}

	char aidisk_cmd[64];
	char aidisk_path[64];
	memset(aidisk_cmd, 0, sizeof(aidisk_cmd));
	if (device_name[3] == '\0')	// sda, sdb, sdc...
	{
		if (!check_partition(device_name))
		{
			sprintf(aidisk_cmd, "/sbin/automount.sh $MDEV AiDisk_%c%c", device_name[2], '1');
			sprintf(aidisk_path, "/media/AiDisk_%c%c", device_name[2], '1');
		}
		else
			goto No_Need_To_Mount;
	}
	else
	{
		sprintf(aidisk_cmd, "/sbin/automount.sh $MDEV AiDisk_%c%c", device_name[2], device_name[3]);
		sprintf(aidisk_path, "/media/AiDisk_%c%c", device_name[2], device_name[3]);
	}

	umask(0000);
	chmod("/media", 0777);
	chmod("/tmp", 0777);
	if (system(aidisk_cmd) == -1){
		usb_dbg("(%s): can't run '%s'!\n", device_name, aidisk_cmd);
		unlink(lock_file);
		return 0;
	}
	else
		chmod(aidisk_path, 0777);
	sprintf(aidisk_cmd, "/sbin/test_of_var_files_in_mount_path %s", aidisk_path);
	system(aidisk_cmd);
// 201102. James. Move the Jiahao's code from mdev. }

	usb_dbg("(%s): Success!\n", device_name);
	unlink(lock_file);
	return 1;

No_Need_To_Mount:
	usb_dbg("(%s): No need to mount!\n", device_name);
	unlink(lock_file);
#endif // SUPPORT_USB_DISK
	return 0;
}

int asus_lp(const char *device_name, const char *action){
#ifdef SUPPORT_USB_PRINTER
	char target_path[128], usb_path[PATH_MAX];
	char usb_port[4];
	int u2ec_fifo;
	char lock_file[128];
	int retry, isLock;
	FILE *fp;
	char nvram_name[32];
	usb_dbg("(%s): action=%s.\n", device_name, action);

	if(!strcmp(nvram_safe_get("stop_lp"), "1"))
		return 0;

	if(get_device_type_on_name(device_name) != 2){
		usb_dbg("(%s): The device is not a printer.\n", device_name);
		return 0;
	}

	// Check Lock.
	memset(lock_file, 0, 128);
	sprintf(lock_file, "%s.%s", LOCK_DEVICE, device_name);
	for(retry = 0, isLock = 0; retry < MAX_RETRY_LOCK; ++retry){
		if((fp = fopen(lock_file, "r")) == NULL){
			if((fp = fopen(lock_file, "w+")) != NULL){
				fputc('1', fp);
				fclose(fp);
				isLock = 1;

				break;
			}
		}
		else
			fclose(fp);

		sleep(1);
	}

	if(!isLock){
		usb_dbg("(%s): Can't set the file lock!\n", device_name);
		return 0;
	}

	// If remove the device?
	if(!check_hotplug_action(action)){
		memset(usb_port, 0, 4);
		if(!strcmp(nvram_safe_get("usb_path1_act"), device_name)){
			strcpy(usb_port, USB_EHCI_PORT_1);

			nvram_set("usb_path1_act", "");
		}
		else if(!strcmp(nvram_safe_get("usb_path2_act"), device_name)){
			strcpy(usb_port, USB_EHCI_PORT_2);

			nvram_set("usb_path2_act", "");
		}

		if(strlen(usb_port) > 0){
			u2ec_fifo = open(U2EC_FIFO, O_WRONLY|O_NONBLOCK);
			write(u2ec_fifo, "r", 1);
			close(u2ec_fifo);

			usb_dbg("(%s): Remove Printer on USB Port %s.\n", device_name, usb_port);
		}
		else
			usb_dbg("(%s): Remove a unknown-port Printer.\n", device_name);

		unlink(lock_file);
		return 0;
	}

	// Get USB port.
	memset(target_path, 0, 128);
	sprintf(target_path, "%s/%s/device", SYS_PRINTER, device_name);
	memset(usb_path, 0, PATH_MAX);
	if(realpath(target_path, usb_path) == NULL){
		usb_dbg("(%s): Fail to get link: %s.\n", device_name, target_path);
		unlink(lock_file);
		return 0;
	}

	if(get_usb_port(usb_path, usb_port, 4) == NULL){
		usb_dbg("(%s): Fail to get usb port: %s.\n", device_name, usb_path);
		unlink(lock_file);
		return 0;
	}

	// set USB common nvram.
	set_usb_common_nvram(action, usb_port, "printer");

	// Don't support the second printer device on a DUT.
	// Only see the other usb port.
	if((usb_port[2] == '1' && !strcmp(nvram_safe_get("usb_path2"), "printer"))
			|| (usb_port[2] == '2' && !strcmp(nvram_safe_get("usb_path1"), "printer"))
			){
		// We would show the second printer device but didn't let it work.
		// Because it didn't set the nvram: usb_path%d_act.
		usb_dbg("(%s): Already had the printer device in the other USB port!\n", device_name);
		unlink(lock_file);
		return 0;
	}

	// check the current working node of modem.
	memset(nvram_name, 0, 32);
	sprintf(nvram_name, "usb_path%c_act", usb_port[2]);
	nvram_set(nvram_name, device_name);

	u2ec_fifo = open(U2EC_FIFO, O_WRONLY|O_NONBLOCK);
	write(u2ec_fifo, "a", 1);
	close(u2ec_fifo);

	usb_dbg("(%s): Success!\n", device_name);
	unlink(lock_file);
#endif // SUPPORT_USB_PRINTER
	return 1;
}

int asus_sr(const char *device_name, const char *action){
#ifdef SUPPORT_USB_MODEM
	char target_path[128], usb_path[PATH_MAX];
	char usb_port[4], vid[8], pid[8];
	char lock_file[128];
	int retry, isLock;
	FILE *fp;
	char eject_cmd[32];
	usb_dbg("(%s): action=%s.\n", device_name, action);

	if(!strcmp(nvram_safe_get("stop_cd"), "1"))
		return 0;

	if(strncmp(device_name, "sr", 2)){
		usb_dbg("(%s): The device is not a CD one.\n", device_name);
		return 0;
	}

	// If remove the device?
	if(!check_hotplug_action(action)){
		usb_dbg("(%s): Remove CD device.\n", device_name);
		unlink(lock_file);
		return 0;
	}

	// Check Lock.
	memset(lock_file, 0, 128);
	sprintf(lock_file, "%s.%s", LOCK_DEVICE, device_name);
	for(retry = 0, isLock = 0; retry < MAX_RETRY_LOCK; ++retry){
		if((fp = fopen(lock_file, "r")) == NULL){
			if((fp = fopen(lock_file, "w+")) != NULL){
				fputc('1', fp);
				fclose(fp);
				isLock = 1;

				break;
			}
		}
		else
			fclose(fp);

		sleep(1);
	}

	if(!isLock){
		usb_dbg("(%s): Can't set the file lock!\n", device_name);
		return 0;
	}

	// Get USB port.
	memset(target_path, 0, 128);
	sprintf(target_path, "%s/%s/device", SYS_BLOCK, device_name);
	memset(usb_path, 0, PATH_MAX);
	if(realpath(target_path, usb_path) == NULL){
		usb_dbg("(%s): Fail to get link: %s.\n", device_name, target_path);
		unlink(lock_file);
		return 0;
	}

	if(get_usb_port(usb_path, usb_port, 4) == NULL){
		usb_dbg("(%s): Fail to get usb port: %s.\n", device_name, usb_path);
		unlink(lock_file);
		return 0;
	}

	// Get VID.
	if(get_usb_vid(usb_port, vid, 8) == NULL){
		usb_dbg("(%s): Fail to get VID of USB(%s).\n", device_name, usb_port);
		unlink(lock_file);
		return 0;
	}

	// Get PID.
	if(get_usb_pid(usb_port, pid, 8) == NULL){
		usb_dbg("(%s): Fail to get PID of USB(%s).\n", device_name, usb_port);
		unlink(lock_file);
		return 0;
	}

	// initial the config file of usb_modeswitch.
	if(init_3g_param(vid, pid)){
		memset(eject_cmd, 0, 32);
		sprintf(eject_cmd, "usb_modeswitch -c %s", USB_MODESWITCH_CONF);
	}
	else{
		memset(eject_cmd, 0, 32);
		sprintf(eject_cmd, "sdparm --command=eject /dev/%s", device_name);
	}

	system(eject_cmd);

	usb_dbg("(%s): Success!\n", device_name);
	unlink(lock_file);
#endif // SUPPORT_USB_MODEM
	return 1;
}

int asus_tty(const char *device_name, const char *action){
#ifdef SUPPORT_USB_MODEM
	char target_path[128], usb_path[PATH_MAX];
	char *ptr, *interface_name;
	char usb_port[4];
	int got_Int_endpoint = 0;
	char lock_file[128];
	int retry, isLock;
	FILE *fp;
	char nvram_name[32], current_value[16];
	int cur_val, tmp_val;
	usb_dbg("(%s): action=%s.\n", device_name, action);

	if(!strcmp(nvram_safe_get("stop_modem"), "1"))
		return 0;

	if(get_device_type_on_name(device_name) != 3){
		usb_dbg("(%s): The device is not a Modem node.\n", device_name);
		return 0;
	}

	// Check Lock.
	memset(lock_file, 0, 128);
	sprintf(lock_file, "%s.%s", LOCK_DEVICE, device_name);
	for(retry = 0, isLock = 0; retry < MAX_RETRY_LOCK; ++retry){
		if((fp = fopen(lock_file, "r")) == NULL){
			if((fp = fopen(lock_file, "w+")) != NULL){
				fputc('1', fp);
				fclose(fp);
				isLock = 1;

				break;
			}
		}
		else
			fclose(fp);

		sleep(1);
	}

	if(!isLock){
		usb_dbg("(%s): Can't set the file lock!\n", device_name);
		return 0;
	}

	// If remove the device?
	if(!check_hotplug_action(action)){
		memset(usb_port, 0, 4);
		if(!strcmp(nvram_safe_get("usb_path1_act"), device_name)){
			strcpy(usb_port, USB_EHCI_PORT_1);

			nvram_set("usb_path1_act", "");
		}
		else if(!strcmp(nvram_safe_get("usb_path2_act"), device_name)){
			strcpy(usb_port, USB_EHCI_PORT_2);

			nvram_set("usb_path2_act", "");
		}

		if(strlen(usb_port) > 0){
			// Modem remove action.
			unlink(USB_MODESWITCH_CONF);
			unlink(PPP_CONF_FOR_3G);

			if(get_usb_modem_state()){
				unlink("/tmp/nat_rules"); // It's the nat file of USB Modem.

				system("killall pppd");
			}
			system("killall usb_modeswitch");
			system("killall sdparm");

			if(hadSerialModule()){
				system("rmmod option");
				system("rmmod usbserial");
			}

			if(hadACMModule()){
				system("rmmod cdc-acm");
			}

			// Notify wanduck to switch the wan line to WAN port.
			kill_pidfile_s("/var/run/wanduck.pid", SIGINT);

			usb_dbg("(%s): Remove the modem node on USB port %s.\n", device_name, usb_port);
		}
		else
			usb_dbg("(%s): Remove a unknown-port Modem.\n", device_name);

		unlink(lock_file);
		return 0;
	}

	// Get USB port.
	memset(target_path, 0, 128);
	sprintf(target_path, "%s/%s/device", SYS_TTY, device_name);
	memset(usb_path, 0, PATH_MAX);
	if(realpath(target_path, usb_path) == NULL){
		sleep(1); // Sometimes link would be built slowly, so try again.

		if(realpath(target_path, usb_path) == NULL){
			usb_dbg("(%s)(2/2): Fail to get link: %s.\n", device_name, target_path);
			unlink(lock_file);
			return 0;
		}
	}

	if(get_usb_port(usb_path, usb_port, 4) == NULL){
		usb_dbg("(%s): Fail to get usb port: %s.\n", device_name, usb_path);
		unlink(lock_file);
		return 0;
	}

	// Don't support the second modem device on a DUT.
	// Only see the other usb port, because in the same port there are more modem interfaces and they need to compare.
	if((usb_port[2] == '1' && !strcmp(nvram_safe_get("usb_path2"), "modem"))
			|| (usb_port[2] == '2' && !strcmp(nvram_safe_get("usb_path1"), "modem"))
			){
		// We would show the second modem device but didn't let it work.
		// Because it didn't set the nvram: usb_path%d_act.
		usb_dbg("(%s): Already had the modem device in the other USB port!\n", device_name);
		unlink(lock_file);
		return 0;
	}

	// Find the control node of modem.
	// Get Interface name.
	interface_name = strstr(usb_path, usb_port);
	interface_name += strlen(usb_port)+1; // 1 = strlen('/')
	ptr = strchr(interface_name, '/');
	ptr[0] = 0;

	if(isSerialNode(device_name)){
		// Find the endpoint: 03(Int).
		got_Int_endpoint = get_interface_Int_endpoint(interface_name);
		if(!got_Int_endpoint){
			usb_dbg("(%s): No Int endpoint!\n", device_name);
			unlink(lock_file);
			return 0;
		}
	}
	else{ // isACMNode(device_name).
		// Find the control interface of cdc-acm.
		if(!isACMInterface(interface_name)){
			usb_dbg("(%s): Not control interface!\n", device_name);
			unlink(lock_file);
			return 0;
		}
	}

	// check the current working node of modem.
	memset(nvram_name, 0, 32);
	sprintf(nvram_name, "usb_path%c_act", usb_port[2]);
	memset(current_value, 0, 16);
	strncpy(current_value, nvram_safe_get(nvram_name), 4);

	if(!strcmp(current_value, "")){
		nvram_set(nvram_name, device_name);
	}
	else{
		errno = 0;
		cur_val = strtol(&current_value[6], &ptr, 10);
		if(errno != 0 || &current_value[6] == ptr){
			cur_val = -1;
		}

		errno = 0;
		tmp_val = strtol(&device_name[6], &ptr, 10);
		if(errno != 0 || &device_name[6] == ptr){
			usb_dbg("(%s): Can't get the working node.\n", device_name);
			unlink(lock_file);
			return 0;
		}
		else if(cur_val != -1 && cur_val <= tmp_val){
			usb_dbg("(%s): Skip to write PPP's conf.\n", device_name);
			unlink(lock_file);
			return 0;
		}
		else{
			nvram_set(nvram_name, device_name);
		}
	}

	// Write dial config file.
	if(!write_3g_ppp_conf(device_name)){
		usb_dbg("(%s): Fail to write PPP's conf for 3G process!\n", device_name);
		unlink(lock_file);
		return 0;
	}

	// Notify wanduck to switch the wan line to WAN port.
	kill_pidfile_s("/var/run/wanduck.pid", SIGINT);

	usb_dbg("(%s): Success!\n", device_name);
	unlink(lock_file);
#endif // SUPPORT_USB_MODEM
	return 1;
}

int asus_usb_interface(const char *device_name, const char *action){
#if defined(SUPPORT_USB_PRINTER) || defined(SUPPORT_USB_MODEM)
	char usb_port[4];
#ifdef SUPPORT_USB_MODEM
	char vid[8], pid[8];
	char modem_cmd[64];
#endif
	char lock_file[128];
	int retry, isLock;
	FILE *fp;
	char device_type[16];
	usb_dbg("(%s): action=%s.\n", device_name, action);

	if(!strcmp(nvram_safe_get("stop_ui"), "1"))
		return 0;

	if(strncmp(device_name, "1-", 2) && (device_name[2] == '1' || device_name[2] == '2') && strncmp(device_name+3, ":1.", 3)){
		usb_dbg("(%s): The device is not a USB interface.\n", device_name);
		return 0;
	}

	// Check Lock.
	memset(lock_file, 0, 128);
	sprintf(lock_file, "%s.%s", LOCK_INTERFACE, device_name);
	for(retry = 0, isLock = 0; retry < MAX_RETRY_LOCK; ++retry){
		if((fp = fopen(lock_file, "r")) == NULL){
			if((fp = fopen(lock_file, "w+")) != NULL){
				fputc('1', fp);
				fclose(fp);
				isLock = 1;

				break;
			}
		}
		else
			fclose(fp);

		sleep(1);
	}

	if(!isLock){
		usb_dbg("(%s): Can't set the file lock!\n", device_name);
		return 0;
	}

	// Get USB port.
	if(get_usb_port(device_name, usb_port, 4) == NULL){
		usb_dbg("(%s): Fail to get usb port.\n", device_name);
		unlink(lock_file);
		return 0;
	}

	// If remove the device? Handle the remove hotplug of the printer and modem.
	if(!check_hotplug_action(action)){
		memset(device_type, 0, 16);
#ifdef SUPPORT_USB_PRINTER
		if(usb_port[2] == '1' && !strcmp(nvram_safe_get("usb_path1"), "printer"))
			strcpy(device_type, "printer");
		else if(usb_port[2] == '2' && !strcmp(nvram_safe_get("usb_path2"), "printer"))
			strcpy(device_type, "printer");
		else
#endif // SUPPORT_USB_PRINTER
#ifdef SUPPORT_USB_MODEM
		if(usb_port[2] == '1' && !strcmp(nvram_safe_get("usb_path1"), "modem"))
			strcpy(device_type, "modem");
		else if(usb_port[2] == '2' && !strcmp(nvram_safe_get("usb_path2"), "modem"))
			strcpy(device_type, "modem");
		else
#endif // SUPPORT_USB_MODEM
#ifdef SUPPORT_USB_DISK
		if(usb_port[2] == '1' && !strcmp(nvram_safe_get("usb_path1"), "storage"))
			strcpy(device_type, "storage");
		else if(usb_port[2] == '2' && !strcmp(nvram_safe_get("usb_path2"), "storage"))
			strcpy(device_type, "storage");
		else
#endif // SUPPORT_USB_DISK
			strcpy(device_type, "");

		if(strlen(device_type) > 0){
			// Remove USB common nvram.
			set_usb_common_nvram(action, usb_port, NULL);

			usb_dbg("(%s): Remove %s interface on USB Port %s.\n", device_name, device_type, usb_port);
		}
		else
			usb_dbg("(%s): Remove a unknown-type interface.\n", device_name);

		unlink(lock_file);
		return 0;
	}

#ifdef SUPPORT_USB_MODEM
	if(!isSerialInterface(device_name) && !isACMInterface(device_name)){
		usb_dbg("(%s): Not modem interface.\n", device_name);
		unlink(lock_file);
		return 0;
	}
#endif

#ifdef SUPPORT_USB_PRINTER
	// Wait if there is the printer interface.
	retry = 0;
	while(!hadPrinterModule() && retry < MAX_WAIT_PRINTER_MODULE){
		++retry;
		sleep(1); // Wait the printer module to be ready.
	}
	sleep(1); // Wait the printer interface to be ready.

	if(hadPrinterInterface(usb_port)){
		usb_dbg("(%s): Had Printer interface on Port %s.\n", device_name, usb_port);
		unlink(lock_file);
		return 0;
	}
#endif // SUPPORT_USB_PRINTER

#ifdef SUPPORT_USB_MODEM
	// set USB common nvram.
	set_usb_common_nvram(action, usb_port, "modem");

	// Don't support the second modem device on a DUT.
	if(hadSerialModule() || hadACMModule()){
		usb_dbg("(%s): Had inserted the modem module.\n", device_name);
		unlink(lock_file);
		return 0;
	}

	// Modem add action.
	if(isSerialInterface(device_name)){
		// Get VID.
		if(get_usb_vid(usb_port, vid, 8) == NULL){
			usb_dbg("(%s): Fail to get VID of USB.\n", device_name);
			unlink(lock_file);
			return 0;
		}

		// Get PID.
		if(get_usb_pid(usb_port, pid, 8) == NULL){
			usb_dbg("(%s): Fail to get PID of USB.\n", device_name);
			unlink(lock_file);
			return 0;
		}

		memset(modem_cmd, 0, 64);
		sprintf(modem_cmd, "insmod usbserial vendor=0x%s product=0x%s", vid, pid);
		system(modem_cmd);
		system("insmod option");
	}
	else{ // isACMInterface(device_name)
		system("insmod cdc-acm");
	}
#endif // SUPPORT_USB_MODEM

	usb_dbg("(%s): Success!\n", device_name);
	unlink(lock_file);
#endif
	return 1;
}

#endif
