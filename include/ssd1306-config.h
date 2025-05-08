#pragma once

#define N_PAGES 0x4
#define N_COLUMNS 0x80

#define USE_UDP     false

#ifndef __weak
#ifdef __GNUC__
#define __weak __attribute__((weak))
#else
#define __weak
#endif
#endif