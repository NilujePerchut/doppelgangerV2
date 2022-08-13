#include <stdint.h>
#include <stdio.h>

#include "config.h"

const uint32_t *configData = (uint32_t *)(0x8008000 - 1024);

static int is_cfg_valid(void)
{
	if ((configData[0] != CFG_MAGIC1) || (configData[1] != CFG_MAGIC2))
		return 0;
	return 1;
}

static inline size_t get_used_cfg()
{
	return configData[2];
}

static inline size_t get_total_cfg()
{
	return configData[3];
}

void display_config_info(void)
{
	int i;
	size_t used_cfg = get_used_cfg();
	const uint32_t *data = configData + 4;

	printf("Cfg base: 0x%p\n", configData);
	printf("Cfg: %c\n", "NY"[is_cfg_valid()]);
	printf("Used cfg entries: %d\n", used_cfg);
	printf("Total cfg entries: %d\n", get_total_cfg());
	for (i=0;i<used_cfg;i++) {
		printf("\t%d --> \t%x\n", data[2*i], data[2*i+1]);		
	}
}
