#include <stdio.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "keys.h"

STATIC_IF_NOT_TEST
struct dir_keys old_dir_buf;

// Needs to get this from config
STATIC_IF_NOT_TEST
enum SOCD_TYPES socd_method;

// Needs to get this from config
STATIC_IF_NOT_TEST
int key_remap_lut[NUMBER_OF_KEYS];

// Ugly as fuck but I want my 100% coverage
#ifdef TEST
#ifdef printf
#undef printf
#endif
extern int mock_printf(const char *format, ...);
#define printf mock_printf
#endif

void init_keys(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE,
					BT_UP | BT_LEFT | BT_RIGHT | BT_MP | BT_HP | BT_AP |\
		            BT_AK | BT_L3 | BT_R3);
	gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE,
					BT_LK | BT_HK);
	gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE,
					BT_DOWN | BT_LP | BT_MK | BT_START | BT_SELECT |\
					BT_HOME);

	old_dir_buf.down = 1;
	old_dir_buf.up = 1;
	old_dir_buf.left = 1;
	old_dir_buf.right = 1;

	/* Init the remap LUT with identity matrix (no remap) */
	for (int i=0;i<NUMBER_OF_KEYS;i++)
		key_remap_lut[i] = i;
}

STATIC_IF_NOT_TEST
struct dir_keys socd_clean(struct dir_keys *in_keys)
{
	/* Supported SOCD method are:
	 * - None	: no resolution (might result in weird behavior)
	 * - Neutral: opposite sides canceled out
	 * - Absolute priority: up + down = up
	 * - Input priority: the new one wins
	 */
	struct dir_keys ret;
	ret = *in_keys;
	switch (socd_method)
	{
		case SOCD_NEUTRAL:
			if ((in_keys->up == 0) && (in_keys->down==0)) {
				ret.up = 1;
				ret.down = 1;
			}
			if ((in_keys->left == 0) && (in_keys->right==0)) {
				ret.left = 1;
				ret.right = 1;
			}
			break;
		case SOCD_ABSOLUTE_PRIORITY:
			ret = *in_keys;
			if ((in_keys->up == 0) && (in_keys->down==0))
				ret.down = 1;
			if ((in_keys->left == 0) && (in_keys->right==0)) {
				ret.left = 1;
				ret.right = 1;
			}
			break;
		case SOCD_INPUT_PRIORITY:
			if ((in_keys->up == 0) && (in_keys->down == 0)) {
				// Up after down is considered as the dfault
				if (old_dir_buf.down == 0) {
					// Down is the old one
					ret.down = 1;
				} else if (old_dir_buf.up == 0) {
					// Up is the old one
					ret.up = 1;
				}
			}
			if ((in_keys->left == 0) && (in_keys->right == 0)) {
				if (old_dir_buf.left == 0) {
					// Left is the old one
					ret.left = 1;
				} else if (old_dir_buf.right == 0) {
					// Right is the old one
					ret.right = 1;
				}
			}
			break;
		case SOCD_NONE:
		default:
			break;
	}
	old_dir_buf = ret;
	return ret;
}

STATIC_IF_NOT_TEST
void get_raw_keys(int *raw)
{
	uint16_t gpioa_val, gpiob_val, gpioc_val;

	gpioa_val = gpio_port_read(GPIOA);
	gpiob_val = gpio_port_read(GPIOB);
	gpioc_val = gpio_port_read(GPIOC);

	/* Directions */
	raw[BT_UP_MAP_INDEX] = GET_KEY(gpioa_val, BT_UP);
	raw[BT_DOWN_MAP_INDEX] = GET_KEY(gpioc_val, BT_DOWN);
	raw[BT_LEFT_MAP_INDEX] = GET_KEY(gpioa_val, BT_LEFT);
	raw[BT_RIGHT_MAP_INDEX] = GET_KEY(gpioa_val, BT_RIGHT);

	/* Punches */
	raw[BT_LP_MAP_INDEX] = GET_KEY(gpioc_val, BT_LP);
	raw[BT_MP_MAP_INDEX] = GET_KEY(gpioa_val, BT_MP);
	raw[BT_HP_MAP_INDEX] = GET_KEY(gpioa_val, BT_HP);
	raw[BT_AP_MAP_INDEX] = GET_KEY(gpioa_val, BT_AP);

	/* Kicks */
	raw[BT_LK_MAP_INDEX] = GET_KEY(gpiob_val, BT_LK);
	raw[BT_MK_MAP_INDEX] = GET_KEY(gpioc_val, BT_MK);
	raw[BT_HK_MAP_INDEX] = GET_KEY(gpiob_val, BT_HK);
	raw[BT_AK_MAP_INDEX] = GET_KEY(gpioa_val, BT_AK);

	raw[BT_L3_MAP_INDEX] = GET_KEY(gpioa_val, BT_L3);
	raw[BT_R3_MAP_INDEX] = GET_KEY(gpioa_val, BT_R3);

	raw[BT_START_MAP_INDEX] = GET_KEY(gpioc_val, BT_START);
	raw[BT_SELECT_MAP_INDEX] = GET_KEY(gpioc_val, BT_SELECT);
	raw[BT_HOME_MAP_INDEX] = GET_KEY(gpioc_val, BT_HOME);
}

/* It is up to the game subsystem to check if systems without AP/AK
 * should generate press on LP/MP/HP and LK/MK/HK */
void get_keys(struct keys *key_buf)
{
	int raw[NUMBER_OF_KEYS];
	int remaped[NUMBER_OF_KEYS];
	struct dir_keys raw_dir;

	get_raw_keys(raw);

	/* Remap */
	for (int i=0; i<NUMBER_OF_KEYS; i++)
		remaped[i] = *(raw + key_remap_lut[i]);

	raw_dir.up = remaped[BT_UP_MAP_INDEX];
	raw_dir.down = remaped[BT_DOWN_MAP_INDEX];
	raw_dir.left = remaped[BT_LEFT_MAP_INDEX];
	raw_dir.right = remaped[BT_RIGHT_MAP_INDEX];
	key_buf->dir = socd_clean(&raw_dir);

	key_buf->lp = remaped[BT_LP_MAP_INDEX];
	key_buf->mp = remaped[BT_MP_MAP_INDEX];
	key_buf->hp = remaped[BT_HP_MAP_INDEX];
	key_buf->ap = remaped[BT_AP_MAP_INDEX];

	key_buf->lk = remaped[BT_LK_MAP_INDEX];
	key_buf->mk = remaped[BT_MK_MAP_INDEX];
	key_buf->hk = remaped[BT_HK_MAP_INDEX];
	key_buf->ak = remaped[BT_AK_MAP_INDEX];

	key_buf->l3 = remaped[BT_L3_MAP_INDEX];
	key_buf->r3 = remaped[BT_R3_MAP_INDEX];

	key_buf->start = remaped[BT_START_MAP_INDEX];
	key_buf->select = remaped[BT_SELECT_MAP_INDEX];
	key_buf->home = remaped[BT_HOME_MAP_INDEX];
}

void dump_keys(void)
{
	struct keys cur_keys;
	get_keys(&cur_keys);
	printf(" U  D  L  R ");
	printf("LP MP HP 3P ");
	printf("LK MK HK 3K ");
	printf("L3 R3 ");
	printf("ST SL HM ");
	printf("\n");
	printf(" %c  %c  %c  %c ",
		   cur_keys.dir.up ? 'O': 'X',
		   cur_keys.dir.down ? 'O': 'X',
		   cur_keys.dir.left ? 'O': 'X',
		   cur_keys.dir.right ? 'O': 'X'
		);
	printf(" %c  %c  %c  %c ",
		   cur_keys.lp ? 'O': 'X',
		   cur_keys.mp ? 'O': 'X',
		   cur_keys.hp ? 'O': 'X',
		   cur_keys.ap ? 'O': 'X'
		);
	printf(" %c  %c  %c  %c ",
		   cur_keys.lk ? 'O': 'X',
		   cur_keys.mk ? 'O': 'X',
		   cur_keys.hk ? 'O': 'X',
		   cur_keys.ak ? 'O': 'X'
		);
	printf(" %c  %c ",
		   cur_keys.l3 ? 'O': 'X',
		   cur_keys.r3 ? 'O': 'X'
		);
	printf(" %c  %c  %c",
		   cur_keys.start ? 'O': 'X',
		   cur_keys.select ? 'O': 'X',
		   cur_keys.home ? 'O': 'X'
		);
	printf("\n");
}

