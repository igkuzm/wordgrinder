/**
 * File              : image2rtf.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 05.09.2023
 * Last Modified Date: 15.01.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef IMAGE_TO_RTF
#define IMAGE_TO_RTF

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "stb_image.h"
#include "stb_image_write.h"

struct image_jpg_write_s {
	unsigned char *data;
	size_t len;
};

static void 
image_jpg_write_func(void *context, void *data, int size){
	struct image_jpg_write_s *s = 
		(struct image_jpg_write_s *)context; 
	
	// realloc data
	s->data = (unsigned char *)realloc(s->data, s->len + size);
	if (!s->data){
				perror("realloc");
		return;
	}
	
	// copy
	memcpy(&(s->data[s->len]), data, size);
	s->len += size;
}

static unsigned char * bin_to_strhex(
		const unsigned char *bin,
		unsigned int binsz,
		unsigned char **result)
{
	unsigned char hex_str[] = "0123456789abcdef";
	unsigned int  i;

	if (!binsz)
		return NULL;
	
	if (!(*result = (unsigned char *)malloc(binsz * 2 + 1)))
		return NULL;

	(*result)[binsz * 2] = 0;

	for (i = 0; i < binsz; ++i)
	{
		(*result)[i * 2 + 0] = hex_str[(bin[i] >> 4) & 0x0F];
		(*result)[i * 2 + 1] = hex_str[(bin[i]     ) & 0x0F];
	}

	return (*result);
}

static unsigned char *image2hex(const char *filename){
	// try to load file
	int w=0, h=0, c;
	stbi_uc *image = 
		stbi_load(filename, &w, &h, &c, 0);

	if (!image){
				perror("can't open image");
		return NULL;
	}

	struct image_jpg_write_s s;
	s.data = (unsigned char *)malloc(1);
	if (!s.data){
		perror("allocate");
		exit(EXIT_FAILURE);
	}
	s.len = 0;

	if (stbi_write_jpg_to_func(
			image_jpg_write_func,
			&s, 
			w, h, c, 
			image, 90) == 0) 
	{
		perror("convert image to jpeg");
		stbi_image_free(image);
		return NULL;
	}
		
	stbi_image_free(image);

	unsigned char *str;
	bin_to_strhex(s.data, s.len, &str);
	free(s.data);

	return str;
}

static int image2rtf(
		const char *filename,
		void *userdata,
		void (*callback)(
			void *userdata, const char *rtf))
{
	unsigned char *str = image2hex(filename);
	if (str){
		callback(userdata, (char *)str);
		free(str);
	}
	return 0;
}

#ifdef __cplusplus
}
#endif
#endif /* ifndef IMAGE_TO_RTF */
