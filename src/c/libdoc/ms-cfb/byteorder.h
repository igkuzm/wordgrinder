/**
 * File              : byteorder.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 20.02.2023
 * Last Modified Date: 27.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef BYTEORDER_H_
#define BYTEORDER_H_

/* all data in CFB MUST
 * be stored in little-endian byte order. The only exception
 * is in user-defined data streams, where the
 * compound file structure does not impose any restrictions.
 */

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <byteswap.h>

static bool is_little_endian()
{
	int x = 1;
	return *(char*)&x;
}

// host to cfb
static uint16_t htocs (uint16_t x)
{
	if (!is_little_endian())
		return bswap_16(x);
	return x;
}

static uint32_t htocl (uint32_t x)
{
	if (!is_little_endian())
		return bswap_32(x);
	return x;
}

static uint64_t htocll (uint64_t x)
{
	if (!is_little_endian())
		return bswap_64(x);
	return x;
}

// cfb to host
static uint16_t ctohs (uint16_t x)
{
	if (!is_little_endian())
		return bswap_16(x);
	return x;
}

static uint32_t ctohl (uint32_t x)
{
	if (!is_little_endian())
		return bswap_32(x);
	return x;
}

static uint64_t ctohll (uint64_t x)
{
	if (!is_little_endian())
		return bswap_64(x);
	return x;
}

#ifdef __cplusplus
}
#endif

#endif //BYTEORDER_H_

// vim:ft=c	
