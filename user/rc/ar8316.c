/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <stdio.h>	     
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/config.h>
#include <linux/types.h>

#include <ralink_gpio.h>
#include <ra_ioctl.h>

#define AR8316_DEV	"/dev/ar8316"

#define RAETH_MII_READ	0x89F3
#define RAETH_MII_WRITE	0x89F4

int ar8316_ioctl(int val)
{
	int fd;
	ra_mii_ioctl_data mii;

	fd = open(AR8316_DEV, O_RDONLY);
	if (fd < 0) {
		perror(AR8316_DEV);
		return -1;
	}

	switch (val)
	{
	case 0:
		mii.phy_id = (unsigned short int)2;
		mii.reg_num = (unsigned short int)2;
		if(ioctl(fd, RAETH_MII_READ, &mii) < 0){
			perror("ar8316 ioctl");
			close(fd);
			return -1;
		}
		fprintf(stderr, "Get: phy[%d].reg[%d] = %04x\n", mii.phy_id, mii.reg_num, mii.val_out);
		break;
	case 2:
		mii.phy_id = (unsigned short int)2;
		mii.reg_num = (unsigned short int)2;
        	mii.val_in = (unsigned short int)0x1234;
		if(ioctl(fd, RAETH_MII_WRITE, &mii) < 0){
			perror("ar8316 ioctl");
			close(fd);
			return -1;
		}
		fprintf(stderr, "Set: phy[%d].reg[%d] = %04x\n", mii.phy_id, mii.reg_num, mii.val_in);
		break;
	default:
		printf("wrong ioctl cmd: %d\n", val);
	}

	close(fd);
	return 0;
}

void config8316(char *cmd)
{
	int val = atoi(cmd);
	ar8316_ioctl(val);
}

typedef struct {
	unsigned int idx;
	unsigned int value;
} asus_gpio_info;

int
ralink_gpio_write_bit(int idx, int value)
{
	int fd;
	asus_gpio_info info;

	fd = open(AR8316_DEV, O_RDONLY);
	if (fd < 0) {
		perror(AR8316_DEV);
		return -1;
	}

	info.idx = idx;
	info.value = value;

	if (ioctl(fd, RALINK_GPIO_WRITE_BIT, &info) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

int
ralink_gpio_read_bit(int idx)
{
	int fd;
	int value;

	fd = open(AR8316_DEV, O_RDONLY);
	if (fd < 0) {
		perror(AR8316_DEV);
		return -1;
	}

	value = idx;

	if (ioctl(fd, RALINK_GPIO_READ_BIT, &value) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}

	close(fd);

	return value;
}

int
ralink_gpio_init(unsigned int idx, int dir)
{
	int fd;
	asus_gpio_info info;

	fd = open(AR8316_DEV, O_RDONLY);
	if (fd < 0) {
		perror(AR8316_DEV);
		return -1;
	}

	info.idx = idx;
	info.value = dir;

	if (ioctl(fd, RALINK_GPIO_SET_DIR, &info) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

int
rtl8367m_wanPort_phyStatus()
{
	return 1;
}

int
ar8316_wanPort_phyStatus()
{
	return 1;

/*
	int fd;
	int value;

	fd = open(AR8316_DEV, O_RDONLY);
	if (fd < 0) {
		perror(AR8316_DEV);
		return -1;
	}

	if (ioctl(fd, 0, &value) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}

	close(fd);

	return value;
*/
}
