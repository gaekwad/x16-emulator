// Commander X16 Emulator
// Copyright (c) 2019 Michael Steil
// All rights reserved. License: 2-clause BSD

#include <stdio.h>
#include <stdbool.h>
#include "vera_spi.h"
#include "sdcard.h"

static bool    autotx;
static bool    ss;
static bool    busy;
static uint8_t sending_byte, received_byte;
static int     outcounter;

void
vera_spi_init()
{
	ss            = false;
	busy          = false;
	received_byte = 0xff;
}

void
vera_spi_step()
{
	if (busy) {
		outcounter++;
		if (outcounter == 8) {
			busy = false;
			if (sdcard_file) {
				received_byte = sdcard_handle(sending_byte);
			} else {
				received_byte = 0xff;
			}
		}
	}
}

uint8_t
vera_spi_read(uint8_t reg)
{
	uint8_t result = 0;
	switch (reg) {
		case 0:
			result = received_byte;
			if (autotx) {
				vera_spi_write(0, 0xFF);
			}
			break;
		case 1:
			result = busy << 7 | ss;
			break;
	}
	return result;
}

void
vera_spi_write(uint8_t reg, uint8_t value)
{
	switch (reg) {
		case 0:
			if (ss && !busy) {
				sending_byte = value;
				busy         = true;
				outcounter   = 0;
			}
			break;
		case 1:
			ss     = (value & 1) != 0;
			autotx = (value & 4) != 0;
			sdcard_select(ss);
			break;
	}
}
