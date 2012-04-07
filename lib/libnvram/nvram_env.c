#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nvram.h"
#include "nvram_env.h"
#include "flash_api.h"

#include <linux/autoconf.h>

char libnvram_debug = 0;
#define LIBNV_PRINT(x, ...) do { if (libnvram_debug) printf("%s %d: " x, __FILE__, __LINE__, ## __VA_ARGS__); } while(0)
#define LIBNV_ERROR(x, ...) do { printf("%s %d: ERROR! " x, __FILE__, __LINE__, ## __VA_ARGS__); } while(0)


typedef struct environment_s {
	unsigned long crc;		//CRC32 over data bytes
	char *data;
} env_t;

typedef struct cache_environment_s {
	char *name;
	char *value;
} cache_t;

#define MAX_CACHE_ENTRY 500
typedef struct block_s {
	char *name;
	env_t env;			//env block
	cache_t	cache[MAX_CACHE_ENTRY];	//env cache entry by entry
	unsigned long flash_offset;
	unsigned long flash_max_len;	//ENV_BLK_SIZE

	char valid;
	char dirty;
} block_t;

#ifdef CONFIG_DUAL_IMAGE
#define FLASH_BLOCK_NUM	5
#else
#define FLASH_BLOCK_NUM	4
#endif

static block_t fb[FLASH_BLOCK_NUM] =
{
#ifdef CONFIG_DUAL_IMAGE
	{
		.name = "uboot",
		.flash_offset =  0x0,
		.flash_max_len = ENV_UBOOT_SIZE,
		.valid = 0
	},
#endif
	{
		.name = "2860",
		.flash_offset =  0x2000,
		.flash_max_len = ENV_BLK_SIZE*4,
		.valid = 0
	},
	{
		.name = "rtdev",
		.flash_offset = 0x6000,
		.flash_max_len = ENV_BLK_SIZE*2,
		.valid = 0
	},
	{
		.name = "cert",
		.flash_offset = 0x8000,
		.flash_max_len = ENV_BLK_SIZE*2,
		.valid = 0
	},
	{
		.name = "wapi",
		.flash_offset = 0xa000,
		.flash_max_len = ENV_BLK_SIZE*5,
		.valid = 0
	}
};


//x is the value returned if the check failed
#define LIBNV_CHECK_INDEX(x) do { \
	if (index < 0 || index >= FLASH_BLOCK_NUM) { \
		LIBNV_PRINT("index(%d) is out of range\n", index); \
		return x; \
	} \
} while (0)

#define LIBNV_CHECK_VALID() do { \
	if (!fb[index].valid) { \
		LIBNV_PRINT("fb[%d] invalid, init again\n", index); \
		nvram_init(index); \
	} \
} while (0)

#define FREE(x) do { if (x != NULL) {free(x); x=NULL;} } while(0)

/*
 * 1. read env from flash
 * 2. parse entries
 * 3. save the entries to cache
 */
void nvram_init(int index)
{
	unsigned long from;
	int i, len;
	char *p, *q;

	LIBNV_PRINT("--> nvram_init %d\n", index);
	LIBNV_CHECK_INDEX();
	if (fb[index].valid)
		return;

	//read crc from flash
	from = fb[index].flash_offset;
	len = sizeof(fb[index].env.crc);
	flash_read((char *)&fb[index].env.crc, from, len);

	//read data from flash
	from = from + len;
	len = fb[index].flash_max_len - len;
	fb[index].env.data = (char *)malloc(len);
	flash_read(fb[index].env.data, from, len);

	//check crc
	//printf("crc shall be %08lx\n", crc32(0, (unsigned char *)fb[index].env.data, len));
	if (crc32(0, (unsigned char *)fb[index].env.data, len) != fb[index].env.crc) {
		LIBNV_PRINT("Bad CRC %x, ignore values in flash.\n", fb[index].env.crc);
		FREE(fb[index].env.data);
		//empty cache
		fb[index].valid = 1;
		fb[index].dirty = 0;
		return;
	}

	//parse env to cache
	p = fb[index].env.data;
	for (i = 0; i < MAX_CACHE_ENTRY; i++) {
		if (NULL == (q = strchr(p, '='))) {
			LIBNV_PRINT("parsed failed - cannot find '='\n");
			break;
		}
		*q = '\0'; //strip '='
		fb[index].cache[i].name = strdup(p);
		//printf("  %d '%s'->", i, p);

		p = q + 1; //value
		if (NULL == (q = strchr(p, '\0'))) {
			LIBNV_PRINT("parsed failed - cannot find '\\0'\n");
			break;
		}
		fb[index].cache[i].value = strdup(p);
		//printf("'%s'\n", p);

		p = q + 1; //next entry
		if (p - fb[index].env.data + 1 >= len) //end of block
			break;
		if (*p == '\0') //end of env
			break;
	}
	if (i == MAX_CACHE_ENTRY)
		LIBNV_PRINT("run out of env cache, please increase MAX_CACHE_ENTRY\n");

	FREE(fb[index].env.data); //free it to save momery
	fb[index].valid = 1;
	fb[index].dirty = 0;
}

void nvram_close(int index)
{
	int i;

	LIBNV_PRINT("--> nvram_close %d\n", index);
	LIBNV_CHECK_INDEX();
	if (!fb[index].valid)
		return;
	if (fb[index].dirty)
		nvram_commit(index);

	//free env
	FREE(fb[index].env.data);

	//free cache
	for (i = 0; i < MAX_CACHE_ENTRY; i++) {
		FREE(fb[index].cache[i].name);
		FREE(fb[index].cache[i].value);
	}

	fb[index].valid = 0;
}

/*
 * return idx (0 ~ iMAX_CACHE_ENTRY)
 * return -1 if no such value or empty cache
 */
static int cache_idx(int index, char *name)
{
	int i;

	for (i = 0; i < MAX_CACHE_ENTRY; i++) {
		if (!fb[index].cache[i].name)
			return -1;
		if (!strcmp(name, fb[index].cache[i].name))
			return i;
	}
	return -1;
}

char *nvram_get(int index, char *name)
{
	//LIBNV_PRINT("--> nvram_get\n");
	nvram_close(index);
	nvram_init(index);
	return nvram_bufget(index, name);
}

int nvram_set(int index, char *name, char *value)
{
	//LIBNV_PRINT("--> nvram_set\n");
	if (-1 == nvram_bufset(index, name, value))
		return -1;
	return nvram_commit(index);
}

char const *nvram_bufget(int index, char *name)
{
	int idx;
	static char const *ret;

	//LIBNV_PRINT("--> nvram_bufget %d\n", index);
	LIBNV_CHECK_INDEX("");
	LIBNV_CHECK_VALID();
	idx = cache_idx(index, name);

	if (-1 != idx) {
		if (fb[index].cache[idx].value) {
			//duplicate the value in case caller modify it
			//ret = strdup(fb[index].cache[idx].value);
			ret = fb[index].cache[idx].value;
			LIBNV_PRINT("bufget %d '%s'->'%s'\n", index, name, ret);
			return ret;
		}
	}

	//no default value set?
	//btw, we don't return NULL anymore!
	LIBNV_PRINT("bufget %d '%s'->''(empty) Warning!\n", index, name);
	return "";
}

int nvram_bufset(int index, char *name, char *value)
{
	int idx;

	//LIBNV_PRINT("--> nvram_bufset\n");
	LIBNV_CHECK_INDEX(-1);
	LIBNV_CHECK_VALID();
	idx = cache_idx(index, name);

	if (-1 == idx) {
		//find the first empty room
		for (idx = 0; idx < MAX_CACHE_ENTRY; idx++) {
			if (!fb[index].cache[idx].name)
				break;
		}
		//no any empty room
		if (idx == MAX_CACHE_ENTRY) {
			LIBNV_ERROR("run out of env cache, please increase MAX_CACHE_ENTRY\n");
			return -1;
		}
		fb[index].cache[idx].name = strdup(name);
		fb[index].cache[idx].value = strdup(value);
	}
	else {
		//abandon the previous value
		FREE(fb[index].cache[idx].value);
		fb[index].cache[idx].value = strdup(value);
	}
	LIBNV_PRINT("bufset %d '%s'->'%s'\n", index, name, value);
	fb[index].dirty = 1;
	return 0;
}

void nvram_buflist(int index)
{
	int i;

	//LIBNV_PRINT("--> nvram_buflist %d\n", index);
	LIBNV_CHECK_INDEX();
	LIBNV_CHECK_VALID();

	for (i = 0; i < MAX_CACHE_ENTRY; i++) {
		if (!fb[index].cache[i].name)
			break;
		printf("  '%s'='%s'\n", fb[index].cache[i].name, fb[index].cache[i].value);
	}
}

/*
 * write flash from cache
 */
int nvram_commit(int index)
{
	unsigned long to;
	int i, len;
	char *p;

	//LIBNV_PRINT("--> nvram_commit %d\n", index);
	LIBNV_CHECK_INDEX(-1);
	LIBNV_CHECK_VALID();

	if (!fb[index].dirty) {
		LIBNV_PRINT("nothing to be committed\n");
		return 0;
	}

	//construct env block
	len = fb[index].flash_max_len - sizeof(fb[index].env.crc);
	fb[index].env.data = (char *)malloc(len);
	bzero(fb[index].env.data, len);
	p = fb[index].env.data;
	for (i = 0; i < MAX_CACHE_ENTRY; i++) {
		int l;
		if (!fb[index].cache[i].name || !fb[index].cache[i].value)
			break;
		l = strlen(fb[index].cache[i].name) + strlen(fb[index].cache[i].value) + 2;
		if (p - fb[index].env.data + 2 >= fb[index].flash_max_len) {
			LIBNV_ERROR("ENV_BLK_SIZE 0x%x is not enough!", ENV_BLK_SIZE);
			FREE(fb[index].env.data);
			return -1;
		}
		snprintf(p, l, "%s=%s", fb[index].cache[i].name, fb[index].cache[i].value);
		p += l;
	}
	*p = '\0'; //ending null

	//calculate crc
	fb[index].env.crc = (unsigned long)crc32(0, (unsigned char *)fb[index].env.data, len);
	printf("Commit crc = %x\n", (unsigned int)fb[index].env.crc);

	//write crc to flash
	to = fb[index].flash_offset;
	len = sizeof(fb[index].env.crc);
	flash_write((char *)&fb[index].env.crc, to, len);

	//write data to flash
	to = to + len;
	len = fb[index].flash_max_len - len;
	flash_write(fb[index].env.data, to, len);
	FREE(fb[index].env.data);
	fb[index].dirty = 0;
	return 0;
}

/*
 * clear flash by writing all 1's value
 */
int nvram_clear(int index)
{
	unsigned long to;
	int len;

	LIBNV_PRINT("--> nvram_clear %d\n", index);
	LIBNV_CHECK_INDEX(-1);
	nvram_close(index);

	//construct all 1s env block
	len = fb[index].flash_max_len - sizeof(fb[index].env.crc);
	fb[index].env.data = (char *)malloc(len);
	memset(fb[index].env.data, 0xFF, len);

	//calculate and write crc
	fb[index].env.crc = (unsigned long)crc32(0, (unsigned char *)fb[index].env.data, len);
	to = fb[index].flash_offset;
	len = sizeof(fb[index].env.crc);
	flash_write((char *)&fb[index].env.crc, to, len);

	//write all 1s data to flash
	to = to + len;
	len = fb[index].flash_max_len - len;
	flash_write(fb[index].env.data, to, len);
	FREE(fb[index].env.data);
	LIBNV_PRINT("clear flash from 0x%x for 0x%x bytes\n", (unsigned int *)to, len);
	fb[index].dirty = 0;
	return 0;
}

#if 0
//WARNING: this fuunction is dangerous because it erases all other data in the same sector
int nvram_erase(int index)
{
	int s, e;

	LIBNV_PRINT("--> nvram_erase %d\n", index);
	LIBNV_CHECK_INDEX(-1);
	nvram_close(index);

	s = fb[index].flash_offset;
	e = fb[index].flash_offset + fb[index].flash_max_len - 1;
	LIBNV_PRINT("erase flash from 0x%x to 0x%x\n", s, e);
	FlashErase(s, e);
	return 0;
}
#endif

int getNvramNum(void)
{
	return FLASH_BLOCK_NUM;
}

unsigned int getNvramOffset(int index)
{
	LIBNV_CHECK_INDEX(0);
	return fb[index].flash_offset;
}

char *getNvramName(int index)
{
	LIBNV_CHECK_INDEX(NULL);
	return fb[index].name;
}

unsigned int getNvramBlockSize(int index)
{
	LIBNV_CHECK_INDEX(0);
	return fb[index].flash_max_len;
}

unsigned int getNvramIndex(char *name)
{
	int i;
	for (i = 0; i < FLASH_BLOCK_NUM; i++) {
		if (!strcmp(fb[i].name, name)) {
			return i;
		}
	}
	return -1;
}

void toggleNvramDebug()
{
	if (libnvram_debug) {
		libnvram_debug = 0;
		printf("%s: turn off debugging\n", __FILE__);
	}
	else {
		libnvram_debug = 1;
		printf("%s: turn ON debugging\n", __FILE__);
	}
}

