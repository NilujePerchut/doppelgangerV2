#pragma once

struct keys {
	int up;
	int down;
	int left;
	int right;
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

void update_key_buff(void);
void dump_keys(void);
void init_keys(void);
