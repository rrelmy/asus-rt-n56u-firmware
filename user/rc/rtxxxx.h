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
#ifndef RTXXXXH
#define RTXXXXH

#define LED_CONTROL(led, flag)	ralink_gpio_write_bit(led, flag)

#define LED_ON		0	// low active (all 5xx series)
#define LED_OFF		1

#define LED_POWER	0
#define LED_USB		24
#ifndef SR3
#define BTN_RESET	25
#else
#define BTN_RESET	13
#endif
#define BTN_WPS		26
#define LED_WAN		27
#ifndef SR3
#define LED_LAN		31
#else
#define LED_LAN		19
#endif
/*
#define RTN13U_SW1	9
#define RTN13U_SW2	13
#define RTN13U_SW3	11
*/

#define GPIO_DIR_OUT	1
#define GPIO_DIR_IN	0

#define GPIO0		0x0001
#define GPIO1		0x0002
#define GPIO2		0x0004
#define GPIO3		0x0008
#define GPIO4		0x0010
#define GPIO5		0x0020
#define GPIO6		0x0040
#define GPIO7		0x0080
#define GPIO15		0x8000

#define TASK_HTTPD       0
#define TASK_UDHCPD      1
#define TASK_LLD2D       2
#define TASK_WANDUCK     3
#define TASK_UDHCPC      4
#define TASK_NETWORKMAP  5
#define TASK_DPROXY      6
#define TASK_NTP         7
#define TASK_U2EC        8
#define TASK_OTS         9
#define TASK_LPD         10
#define TASK_UPNPD       11
#define TASK_WATCHDOG    12
#define TASK_INFOSVR     13
#define TASK_SYSLOGD     14
#define TASK_KLOGD       15
#define TASK_PPPD        16
#define TASK_PPPOE_RELAY 17
#define TASK_IGMP	 18

unsigned long task_mask;

int switch_init(void);

void switch_fini(void);

int ra3052_reg_read(int offset, int *value);

int ra3052_reg_write(int offset, int value);

int config_3052(int type);

int restore_3052();

void ra_gpio_write_spec(bit_idx, flag);

int check_all_tasks();

int ra_gpio_set_dir(int dir);

int ra_gpio_write_int(int value);

int ra_gpio_read_int(int *value);

int ra_gpio_write_bit(int idx, int value);

#endif
