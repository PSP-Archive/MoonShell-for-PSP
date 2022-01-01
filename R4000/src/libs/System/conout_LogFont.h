#pragma once

#include <pspuser.h>

static const u8 LogFont[]={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x00,0x20,0x00,0x50,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x50,0xF8,0x50,0xF8,0x50,0x50,0x00,0x10,0x38,0x50,0x70,0x28,0x28,0x70,0x00,0x00,0x68,0x68,0x10,0x20,0x58,0x58,0x00,0x30,0x28,0x30,0x70,0x48,0x48,0x30,0x00,0x10,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x20,0x20,0x20,0x20,0x20,0x10,0x00,0x20,0x10,0x10,0x10,0x10,0x10,0x20,0x00,0x00,0x20,0x70,0x70,0x70,0x20,0x00,0x00,0x00,0x20,0x20,0x70,0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x08,0x08,0x10,0x10,0x20,0x20,0x40,0x00,0x30,0x48,0x48,0x48,0x48,0x48,0x30,0x00,0x20,0x60,0x20,0x20,0x20,0x20,0x70,0x00,0x30,0x48,0x08,0x10,0x20,0x40,0x78,0x00,0x30,0x48,0x08,0x30,0x08,0x48,0x30,0x00,0x10,0x30,0x30,0x50,0x50,0x78,0x10,0x00,0x78,0x40,0x40,0x70,0x08,0x08,0x70,0x00,0x30,0x48,0x40,0x70,0x48,0x48,0x30,0x00,0x78,0x48,0x08,0x10,0x20,0x20,0x20,0x00,0x30,0x48,0x48,0x30,0x48,0x48,0x30,0x00,0x30,0x48,0x48,0x38,0x08,0x48,0x30,0x00,0x00,0x00,0x20,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x20,0x40,0x00,0x08,0x10,0x20,0x40,0x20,0x10,0x08,0x00,0x00,0x00,0x78,0x00,0x78,0x00,0x00,0x00,0x40,0x20,0x10,0x08,0x10,0x20,0x40,0x00,0x30,0x48,0x08,0x10,0x20,0x00,0x20,0x00,0x30,0x48,0x48,0x58,0x28,0x28,0x38,0x00,0x30,0x48,0x48,0x48,0x78,0x48,0x48,0x00,0x70,0x48,0x48,0x70,0x48,0x48,0x70,0x00,0x30,0x48,0x40,0x40,0x40,0x48,0x30,0x00,0x70,0x48,0x48,0x48,0x48,0x48,0x70,0x00,0x78,0x40,0x40,0x70,0x40,0x40,0x78,0x00,0x78,0x40,0x40,0x70,0x40,0x40,0x40,0x00,0x30,0x48,0x40,0x58,0x48,0x48,0x30,0x00,0x48,0x48,0x48,0x78,0x48,0x48,0x48,0x00,0x70,0x20,0x20,0x20,0x20,0x20,0x70,0x00,0x08,0x08,0x08,0x08,0x48,0x48,0x30,0x00,0x48,0x48,0x50,0x60,0x50,0x48,0x48,0x00,0x40,0x40,0x40,0x40,0x40,0x40,0x78,0x00,0x48,0x78,0x78,0x48,0x48,0x48,0x48,0x00,0x48,0x68,0x68,0x58,0x58,0x48,0x48,0x00,0x78,0x48,0x48,0x48,0x48,0x48,0x78,0x00,0x70,0x48,0x48,0x70,0x40,0x40,0x40,0x00,0x30,0x48,0x48,0x48,0x68,0x58,0x38,0x00,0x70,0x48,0x48,0x70,0x60,0x50,0x48,0x00,0x30,0x48,0x40,0x30,0x08,0x48,0x30,0x00,0x78,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x48,0x48,0x48,0x48,0x48,0x48,0x30,0x00,0x48,0x48,0x48,0x48,0x30,0x30,0x30,0x00,0x48,0x48,0x48,0x48,0x78,0x78,0x48,0x00,0x48,0x48,0x30,0x30,0x30,0x48,0x48,0x00,0x88,0x88,0x50,0x20,0x20,0x20,0x20,0x00,0x78,0x08,0x10,0x30,0x20,0x40,0x78,0x00,0x30,0x20,0x20,0x20,0x20,0x20,0x30,0x00,0x88,0x50,0x20,0xF8,0x20,0xF8,0x20,0x00,0x30,0x10,0x10,0x10,0x10,0x10,0x30,0x00,0x20,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x38,0x48,0x38,0x00,0x40,0x40,0x40,0x70,0x48,0x48,0x70,0x00,0x00,0x00,0x00,0x38,0x40,0x40,0x38,0x00,0x08,0x08,0x08,0x38,0x48,0x48,0x38,0x00,0x00,0x00,0x00,0x30,0x78,0x40,0x30,0x00,0x00,0x10,0x20,0x78,0x20,0x20,0x20,0x00,0x00,0x00,0x00,0x38,0x48,0x38,0x08,0x30,0x40,0x40,0x40,0x70,0x50,0x50,0x50,0x00,0x00,0x00,0x20,0x00,0x20,0x20,0x20,0x00,0x00,0x00,0x10,0x00,0x10,0x10,0x10,0x60,0x40,0x40,0x40,0x58,0x60,0x50,0x48,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x70,0x00,0x00,0x00,0x00,0x78,0x58,0x58,0x58,0x00,0x00,0x00,0x00,0x70,0x48,0x48,0x48,0x00,0x00,0x00,0x00,0x30,0x48,0x48,0x30,0x00,0x00,0x00,0x00,0x70,0x48,0x48,0x70,0x40,0x00,0x00,0x00,0x38,0x48,0x48,0x38,0x08,0x00,0x00,0x00,0x58,0x60,0x40,0x40,0x00,0x00,0x00,0x00,0x38,0x60,0x18,0x70,0x00,0x00,0x00,0x20,0x70,0x20,0x20,0x30,0x00,0x00,0x00,0x00,0x48,0x48,0x48,0x38,0x00,0x00,0x00,0x00,0x48,0x48,0x30,0x30,0x00,0x00,0x00,0x00,0xA8,0xA8,0x70,0x50,0x00,0x00,0x00,0x00,0x48,0x30,0x30,0x48,0x00,0x00,0x00,0x00,0x48,0x48,0x38,0x08,0x30,0x00,0x00,0x00,0x78,0x10,0x20,0x78,0x00,0x30,0x20,0x20,0x40,0x20,0x20,0x30,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x60,0x20,0x20,0x10,0x20,0x20,0x60,0x00,0x28,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

