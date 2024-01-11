/**
 * File              : image2ascii.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 17.05.2023
 * Last Modified Date: 12.01.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef IMAGE_TO_ASCII_H
#define IMAGE_TO_ASCII_H

#include "stb_image.h"
#include "stb_image_resize.h"
#include "stb_image_write.h"

/* 
 * image2ascii
 * callback for each ascii row c string 
 * (with cols and rows) converted from 
 * image data (jpg, png, gif, bmp ...)
 * @filepath - pointer to image filepath
 * @cols - number of columns of ascii string 
 * (0 - to have original width of image)
 * @rows - number of rows of ascii string 
 * (0 to have original resolution)
 */
static int
image2ascii(
		const char *filepath,
		int cols,
		int rows,
		void *userdata,
		int (*callback)(
			void *userdata, int len, const char * error)
		)
{
	const char map[11] = " .,:;ox%#@";// map of image convert
	stbi_uc *image, *buf;             // image pointer
	int w, h, c, x, y, index;
	
	/*read image */
	image = stbi_load(
			filepath, &w, &h,
			&c, 0);
	if (!image)
		return -1;

	/* resize image */
	if (cols < 1 || cols > w)
		cols = w;
	if (rows < 1)
		rows = h * cols / w;

	buf = (stbi_uc *)malloc(cols*rows*c);
	if (!buf)
		return -1;

	stbir_resize_uint8(image, w,
			h, 0,
			buf, cols,
			rows, 0, 
			c);
	stbi_image_free(image);	

	// set image to gray 
	int gc = c == 4 ? 2 : 1; // set channels for gray
	image = (stbi_uc *)malloc(cols*rows*gc);
	if (!image)
		return -1;
	
	stbi_uc *p, *pg;
	for (p  = buf, pg = image; 
			 p != buf + (cols * rows * c);
			 p += c, pg += gc)
	{
		*pg = (uint8_t)((*p + *(p + 1) + *(p + 2)) / 3.0);
		if (c == 4)
			*(pg + 1) = *(p + 3);
	}
	stbi_image_free(buf);	

	//convert to ascii 
	stbi_uc *ptr = image;
	for (y = 0; y < rows; y++)
	{
		int i = 0;
		char row[cols*2 + 1];
		for (x = 0; x < cols; x++)
		{
			index = 
				(int)(*(ptr) / 
						(255 / (sizeof(map) / sizeof(map[0]))));
			index > 9 ? index = 9 : 1;
			index < 0 ? index = 0 : 1;
			row[i++] = map[index];
			row[i++] = map[index];
			ptr++;
		}
		row[i] = '\0';
		if (callback)
			if(callback(userdata, cols, row))
				return 0;
	}
	free(image);
	return 0;
}

#endif /* ifndef IMAGE_TO_ASCII_H */
