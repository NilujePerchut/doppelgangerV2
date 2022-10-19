#pragma once

#ifdef TEST
#define STATIC_IF_NOT_TEST
#else
#define STATIC_IF_NOT_TEST static
#endif

#define NUMBER_OF_KEYS 17

/* Keys definition */
/* UP */
#define BT_UP_PORT GPIOA
#define BT_UP GPIO10
#define BT_UP_MAP_INDEX 0

/* DOWN */
#define BT_DOWN_PORT GPIOC
#define BT_DOWN GPIO1
#define BT_DOWN_MAP_INDEX 1

/* LEFT */
#define BT_LEFT_PORT GPIOA
#define BT_LEFT GPIO4
#define BT_LEFT_MAP_INDEX 2

/* RIGHT */
#define BT_RIGHT_PORT GPIOA
#define BT_RIGHT GPIO1
#define BT_RIGHT_MAP_INDEX 3

/* LP (SQUARE) */
#define BT_LP_PORT GPIOC
#define BT_LP GPIO5
#define BT_LP_MAP_INDEX 4

/* MP (TRIANGLE) */
#define BT_MP_PORT GPIOA
#define BT_MP GPIO5
#define BT_MP_MAP_INDEX 5

/* HP (R1) */
#define BT_HP_PORT GPIOA
#define BT_HP GPIO6
#define BT_HP_MAP_INDEX 6

/* AP (L1) */
#define BT_AP_PORT GPIOA
#define BT_AP GPIO3
#define BT_AP_MAP_INDEX 7

/* LK (CROSS) */
#define BT_LK_PORT GPIOB
#define BT_LK GPIO0
#define BT_LK_MAP_INDEX 8

/* MK (CIRCLE) */
#define BT_MK_PORT GPIOC
#define BT_MK GPIO4
#define BT_MK_MAP_INDEX 9

/* HK (R2) */
#define BT_HK_PORT GPIOB
#define BT_HK GPIO1
#define BT_HK_MAP_INDEX 10

/* AK (L2) */
#define BT_AK_PORT GPIOA
#define BT_AK GPIO2
#define BT_AK_MAP_INDEX 11

/* L3 */
#define BT_L3_PORT GPIOA
#define BT_L3 GPIO0
#define BT_L3_MAP_INDEX 12

/* R3 */
#define BT_R3_PORT GPIOA
#define BT_R3 GPIO7
#define BT_R3_MAP_INDEX 13

/* START */
#define BT_START_PORT GPIOC
#define BT_START GPIO15
#define BT_START_MAP_INDEX 14

/* SELECT */
#define BT_SELECT_PORT GPIOC
#define BT_SELECT GPIO13
#define BT_SELECT_MAP_INDEX 15

/* HOME */
#define BT_HOME_PORT GPIOC
#define BT_HOME GPIO14
#define BT_HOME_MAP_INDEX 16

#define GET_KEY(P, I) ((P&I)?1:0)

struct dir_keys {
	int up;
	int down;
	int left;
	int right;
};

struct keys {
	struct dir_keys dir;
	int lp;
	int mp;
	int hp;
	int ap;
	int lk;
	int mk;
	int hk;
	int ak;
	int l3;
	int r3;
	int start;
	int select;
	int home;
};

enum SOCD_TYPES {
	SOCD_NONE = 0,						/* a.k.a. : no resoltion */
	SOCD_NEUTRAL,						/* opposite directions cancel out */
	SOCD_ABSOLUTE_PRIORITY,				/* up + down -> up  anytime*/
	SOCD_INPUT_PRIORITY,    			/* last wins */
};

void init_keys(void);
void get_keys(struct keys *key_buf);
void dump_keys(void);
