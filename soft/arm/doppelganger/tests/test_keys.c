#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <cmocka.h>
#include <stdarg.h>

#include <libopencm3/stm32/gpio.h>
#include "keys.h"

/*
 * Mocking Setup
 */

static char temporary_buffer[256];
int mock_printf(const char *format, ...)
{
	int return_value;
	va_list args;
	va_start(args, format);
	return_value = vsnprintf(temporary_buffer, sizeof(temporary_buffer),
							 format, args);
	check_expected(temporary_buffer);
	va_end(args);
	return return_value;
}

void __wrap_rcc_periph_clock_enable(uint32_t port)
{
	check_expected(port);
}

void __wrap_gpio_mode_setup(uint32_t port, uint8_t mode, uint8_t cfn,
							uint32_t io)
{
	check_expected(port);
	check_expected(mode);
	check_expected(cfn);
	check_expected(io);
}

uint16_t __wrap_gpio_port_read(uint32_t port)
{
	check_expected(port);
	return mock();
}

/*
 * Real tests
 */

struct dir_keys socd_clean(struct dir_keys *in_keys);
extern enum SOCD_TYPES socd_method;
extern int key_remap_lut[NUMBER_OF_KEYS];

static void init_env(enum SOCD_TYPES socd_type, int *key_remap)
{
	int i;

	socd_method = socd_type;
	if (key_remap) {
		for (i=0;i<NUMBER_OF_KEYS;i++)
			key_remap_lut[i] = key_remap[i];
	} else {
		/* Init as no remap */
		for (i=0;i<NUMBER_OF_KEYS;i++)
			key_remap_lut[i] = i;
	}
}

#define DIR_KEYS_SET(s, u, d, l, r) do {				\
										   s.up = u;    \
										   s.down = d;  \
										   s.left = l;  \
										   s.right = r; \
									   } while(0)

#define DIR_KEYS_CHECK(s, u, d, l, r) do {								\
										assert_int_equal(s.up, u);		\
										assert_int_equal(s.down, d);	\
										assert_int_equal(s.left, l);	\
										assert_int_equal(s.right, r);	\
									} while(0)

extern struct dir_keys old_dir_buf;

void test_init_keys(void **state)
{
	expect_value(__wrap_rcc_periph_clock_enable, port, 0x600);
	expect_value(__wrap_rcc_periph_clock_enable, port, 0x601);
	expect_value(__wrap_rcc_periph_clock_enable, port, 0x602);

	/* Port A */
	expect_value(__wrap_gpio_mode_setup, port, 0x50000000);
	expect_value(__wrap_gpio_mode_setup, mode, 0);
	expect_value(__wrap_gpio_mode_setup, cfn, 0);
	expect_value(__wrap_gpio_mode_setup, io, 0x4FF);

	/* Port B */
	expect_value(__wrap_gpio_mode_setup, port, 0x50000400);
	expect_value(__wrap_gpio_mode_setup, mode, 0);
	expect_value(__wrap_gpio_mode_setup, cfn, 0);
	expect_value(__wrap_gpio_mode_setup, io, 0x3);

	/* Port C */
	expect_value(__wrap_gpio_mode_setup, port, 0x50000800);
	expect_value(__wrap_gpio_mode_setup, mode, 0);
	expect_value(__wrap_gpio_mode_setup, cfn, 0);
	expect_value(__wrap_gpio_mode_setup, io, 0xE032);

	init_keys();

	DIR_KEYS_CHECK(old_dir_buf, 1, 1, 1, 1);
}


void test_socd_clean_socd_none(void **state)
{
	struct dir_keys new, res;
	init_env(SOCD_NONE, NULL);
	DIR_KEYS_SET(new, 0, 0, 0, 0);
	res = socd_clean(&new);
	DIR_KEYS_CHECK(res, 0, 0, 0, 0);
}

void test_socd_clean_socd_neutral(void **state)
{
	struct dir_keys new, res;
	init_env(SOCD_NEUTRAL, NULL);
	DIR_KEYS_SET(new, 0, 0, 0, 0);
	res = socd_clean(&new);
	DIR_KEYS_CHECK(res, 1, 1, 1, 1);
}

void test_socd_clean_socd_abs_priority(void **state)
{
	struct dir_keys new, res;
	init_env(SOCD_ABSOLUTE_PRIORITY, NULL);
	DIR_KEYS_SET(new, 0, 0, 0, 0);
	res = socd_clean(&new);
	DIR_KEYS_CHECK(res, 0, 1, 1, 1);
}

void test_socd_clean_socd_input_priority(void **state)
{
	struct dir_keys new, res;
	init_env(SOCD_INPUT_PRIORITY, NULL);

	/* up after down + right after left */
	DIR_KEYS_SET(old_dir_buf, 1, 0, 0, 1);
	DIR_KEYS_SET(new, 0, 0, 0, 0);
	res = socd_clean(&new);
	DIR_KEYS_CHECK(res, 0, 1, 1, 0);

	/* down after up + left after right */
	DIR_KEYS_SET(old_dir_buf, 0, 1, 1, 0);
	DIR_KEYS_SET(new, 0, 0, 0, 0);
	res = socd_clean(&new);
	DIR_KEYS_CHECK(res, 1, 0, 0, 1);
}

void test_get_keys(void **state)
{
	/* Sets (all other are set to zero):
	 *	Up, Left, LP, HP, LK, HK, L3, START, HOME
	 */
	struct keys res;

	init_env(SOCD_NONE, NULL);
	expect_value(__wrap_gpio_port_read, port, 0x50000000);
	will_return(__wrap_gpio_port_read,
				BT_RIGHT | BT_MP | BT_AP | BT_AK | BT_R3); /* GPIOA */
	expect_value(__wrap_gpio_port_read, port, 0x50000400);
	will_return(__wrap_gpio_port_read, 0x0); /* GPIOB */
	expect_value(__wrap_gpio_port_read, port, 0x50000800);
	will_return(__wrap_gpio_port_read,
				BT_DOWN | BT_MK | BT_SELECT); /* GPIOC */

	get_keys(&res);

	assert_int_equal(res.dir.up, 0);
	assert_int_equal(res.dir.down, 1);
	assert_int_equal(res.dir.left, 0);
	assert_int_equal(res.dir.right, 1);
	assert_int_equal(res.lp, 0);
	assert_int_equal(res.mp, 1);
	assert_int_equal(res.hp, 0);
	assert_int_equal(res.ap, 1);
	assert_int_equal(res.lk, 0);
	assert_int_equal(res.mk, 1);
	assert_int_equal(res.hk, 0);
	assert_int_equal(res.ak, 1);
	assert_int_equal(res.l3, 0);
	assert_int_equal(res.r3, 1);
	assert_int_equal(res.start, 0);
	assert_int_equal(res.select, 1);
	assert_int_equal(res.home, 0);
}

void test_get_keys_remap(void **state)
{
	/* Sets (all other are set to zero):
	 *	Up, Left, LP, HP, LK, HK, L3, START, HOME
	 */
	struct keys res;
	/* Just invert by 2 pack (Up<=>Down, Left<=>Right, ...) */
	int remap[] = { 1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14,
					16};

	init_env(SOCD_NONE, remap);
	expect_value(__wrap_gpio_port_read, port, 0x50000000);
	will_return(__wrap_gpio_port_read,
				BT_RIGHT | BT_MP | BT_AP | BT_AK | BT_R3); /* GPIOA */
	expect_value(__wrap_gpio_port_read, port, 0x50000400);
	will_return(__wrap_gpio_port_read, 0x0); /* GPIOB */
	expect_value(__wrap_gpio_port_read, port, 0x50000800);
	will_return(__wrap_gpio_port_read,
				BT_DOWN | BT_MK | BT_SELECT); /* GPIOC */

	get_keys(&res);

	assert_int_equal(res.dir.up, 1);
	assert_int_equal(res.dir.down, 0);
	assert_int_equal(res.dir.left, 1);
	assert_int_equal(res.dir.right, 0);
	assert_int_equal(res.lp, 1);
	assert_int_equal(res.mp, 0);
	assert_int_equal(res.hp, 1);
	assert_int_equal(res.ap, 0);
	assert_int_equal(res.lk, 1);
	assert_int_equal(res.mk, 0);
	assert_int_equal(res.hk, 1);
	assert_int_equal(res.ak, 0);
	assert_int_equal(res.l3, 1);
	assert_int_equal(res.r3, 0);
	assert_int_equal(res.start, 1);
	assert_int_equal(res.select, 0);
	assert_int_equal(res.home, 0);
}

void test_dump_keys_1(void **state)
{
	init_env(SOCD_NONE, NULL);
	expect_value(__wrap_gpio_port_read, port, 0x50000000);
	will_return(__wrap_gpio_port_read,
				BT_RIGHT | BT_MP | BT_AP | BT_AK | BT_R3); /* GPIOA */
	expect_value(__wrap_gpio_port_read, port, 0x50000400);
	will_return(__wrap_gpio_port_read, 0); /* GPIOB */
	expect_value(__wrap_gpio_port_read, port, 0x50000800);
	will_return(__wrap_gpio_port_read,
				BT_DOWN | BT_MK | BT_SELECT); /* GPIOC */

	expect_string(mock_printf, temporary_buffer, " U  D  L  R ");
	expect_string(mock_printf, temporary_buffer, "LP MP HP 3P ");
	expect_string(mock_printf, temporary_buffer, "LK MK HK 3K ");
	expect_string(mock_printf, temporary_buffer, "L3 R3 ");
	expect_string(mock_printf, temporary_buffer, "ST SL HM ");
	expect_string(mock_printf, temporary_buffer, "\n");
	expect_string(mock_printf, temporary_buffer, " X  O  X  O ");
	expect_string(mock_printf, temporary_buffer, " X  O  X  O ");
	expect_string(mock_printf, temporary_buffer, " X  O  X  O ");
	expect_string(mock_printf, temporary_buffer, " X  O ");
	expect_string(mock_printf, temporary_buffer, " X  O  X");
	expect_string(mock_printf, temporary_buffer, "\n");
	dump_keys();
}

void test_dump_keys_2(void **state)
{
	init_env(SOCD_NONE, NULL);
	expect_value(__wrap_gpio_port_read, port, 0x50000000);
	will_return(__wrap_gpio_port_read,
				BT_UP | BT_LEFT | BT_HP | BT_L3); /* GPIOA */
	expect_value(__wrap_gpio_port_read, port, 0x50000400);
	will_return(__wrap_gpio_port_read, BT_LK | BT_HK); /* GPIOB */
	expect_value(__wrap_gpio_port_read, port, 0x50000800);
	will_return(__wrap_gpio_port_read, BT_LP | BT_START | BT_HOME); /* GPIOC */

	expect_string(mock_printf, temporary_buffer, " U  D  L  R ");
	expect_string(mock_printf, temporary_buffer, "LP MP HP 3P ");
	expect_string(mock_printf, temporary_buffer, "LK MK HK 3K ");
	expect_string(mock_printf, temporary_buffer, "L3 R3 ");
	expect_string(mock_printf, temporary_buffer, "ST SL HM ");
	expect_string(mock_printf, temporary_buffer, "\n");
	expect_string(mock_printf, temporary_buffer, " O  X  O  X ");
	expect_string(mock_printf, temporary_buffer, " O  X  O  X ");
	expect_string(mock_printf, temporary_buffer, " O  X  O  X ");
	expect_string(mock_printf, temporary_buffer, " O  X ");
	expect_string(mock_printf, temporary_buffer, " O  X  O");
	expect_string(mock_printf, temporary_buffer, "\n");
	dump_keys();
}

const struct CMUnitTest tests_keys[] = {
	cmocka_unit_test(test_init_keys),
	cmocka_unit_test(test_socd_clean_socd_none),
	cmocka_unit_test(test_socd_clean_socd_neutral),
	cmocka_unit_test(test_socd_clean_socd_abs_priority),
	cmocka_unit_test(test_socd_clean_socd_input_priority),
	cmocka_unit_test(test_get_keys),
	cmocka_unit_test(test_get_keys_remap),
	cmocka_unit_test(test_dump_keys_1),
	cmocka_unit_test(test_dump_keys_2),
};

int main(void)
{
	return cmocka_run_group_tests(tests_keys, NULL, NULL);
}

