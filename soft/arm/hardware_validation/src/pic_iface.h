#pragma once

void init_pic_usart(void);
void init_pic_gpio(void);
void LVP_enter(void);
void LVP_exit(void);
bool flash_pic_from_uart(void);
void pic_debug(void);
