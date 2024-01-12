/**
 * File              : unrtf.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 12.01.2024
 * Last Modified Date: 12.01.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#ifndef UNRTF_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* dynamic string structure */
struct str {
	char *str;   //null-terminated c string
	int   len;   //length of string (without last null char)
	int   size;  //allocated size
};

/* init string - return non-null on error */
static int str_init(struct str *s, size_t size);

/* append c string */
static void str_append(
		struct str *s, const char *str, int len);

/* append fprint-like formated c string */
#define str_appendf(s, ...)

/* IMPLIMATION */
#include <string.h>
#include <stdlib.h>

int str_init(struct str *s, size_t size)
{
	// allocate data
	s->str = (char*)malloc(size);
	if (!s->str)
		return -1;

	// set dafaults
	s->str[0]  = 0;
	s->len     = 0;
	s->size    = size;

	return 0;
}

static int _str_realloc(
		struct str *s, int new_size)
{
	while (s->size < new_size){
		// do realloc
		void *p = realloc(s->str, s->size + BUFSIZ);
		if (!p)
			return -1;
		s->str = (char*)p;
		s->size += BUFSIZ;
	}
	return 0;
}

void str_append(
		struct str *s, const char *str, int len)
{
	if (!str || len < 1)
		return;

	int new_size, i;
	
	new_size = s->len + len + 1;
	// realloc if not enough size
	if (_str_realloc(s, new_size))
		return;

	// append string
	for (i = 0; i < len; ++i)
		s->str[s->len++] = str[i];
  
	s->str[s->len] = 0;
}

#undef  str_appendf
#define str_appendf(s, ...)\
	({\
	 char str[BUFSIZ];\
	 snprintf(str, BUFSIZ-1, __VA_ARGS__);\
	 str[BUFSIZ-1] = 0;\
	 str_append(s, str, strlen(str));\
	 })

static uint8_t *_utf32_to_utf8(
		uint32_t c, uint8_t s[5])
{
	//more than 00000000 00000100 00000000 00000000
	if (c > 0x040000){ //4-byte

		//get first byte - first 3 bit 00000000 00011100 00000000 00000000
		//and mask with 11110000 
		s[0] = ((c & 0x1C0000) >> 18) | 0xF0;

		//get second - 00000000 00000011 11110000 00000000
		//and mask with 10000000 
		s[1] = ((c & 0x03F000) >> 12) | 0x80;
		
		//get third - 00000000 00000000 00001111 11000000
		//and mask with 10000000 
		s[2] = ((c & 0x0FC0) >> 6 )   | 0x80;

		//get last - 00000000 00000000 00000000 00111111
		//and mask with 10000000 
		s[3] = ( c & 0x3F)            | 0x80;

		s[4] = 0;
	}
	//more than 00000000 00000000 00010000 00000000
	else if (c > 0x1000){ //3-byte
		
		//get first byte - first 4 bit 00000000 00000000 11110000 00000000
		//and mask with 11100000 
		s[0] = ((c & 0xF000) >> 12) | 0xE0;

		//get second - 00000000 00000000 00001111 11000000
		//and mask with 10000000 
		s[1] = ((c & 0x0FC0) >> 6 ) | 0x80;
		
		//get last - 00000000 00000000 00000000 00111111
		//and mask with 10000000 
		s[2] = ( c & 0x3F)          | 0x80;

		s[3] = 0;
	}
	//more than 000000000 00000000 00000000 1000000
	else if (c > 0x80){ //2-byte
		//get first byte - first 5 bit 00000000 00000000 00000111 11000000
		//and mask with 11000000 
		s[0] = ((c & 0x7C0)>> 6) | 0xC0;

		//get last - 00000000 00000000 00000000 00111111 
		//and mask with 10000000 
		s[1] = ( c & 0x3F)       | 0x80;

		s[2] = 0;
	}
	else { //ANSY
		s[0] = c;
		s[1] = 0;
	}

	return s;
}

static int unrtf_isinword(int ch)
{
	if ( 	
		ch == '\n' ||
		ch == '\r' ||
		ch == ' '  ||
		ch == '\t' ||
		ch == '\\'
		)
		return 0;
	return 1;
}

static int unrtf_readword(FILE *fp, char *buf)
{
	int ch = 1;
	int blen = 0;
	while (1)
	{
		ch = fgetc(fp);
		if (ch == EOF)
			break;
		if (unrtf_isinword(ch))
			buf[blen++] = ch;
		else
			break;;
	}
	buf[blen] = 0;
	return ch;
}

static int unrtf_isservice(char *buf)
{
	if (
			(buf[0] >= 'A' && buf[0] <= 'Z') ||
			(buf[0] >= 'a' && buf[0] <= 'z') 
		 )
		return 1;
	return 0;
}

static int unrtf_isutf(char *buf)
{
	if (buf[0] == 'u')
		if (buf[1] >= '0' && buf[1] <= '9') 
			return 1;
	return 0;
}

static int unrtf_isstyle(char *buf)
{
	if (buf[0] == 's')
		if (buf[1] >= '0' && buf[1] <= '9') 
			return 1;
	return 0;
}

static int unrtf_isli(char *buf)
{
	if (
		buf[0] == 'l' &&
		buf[1] == 'i' &&
		(buf[2] >= '0' || buf[2] <= '9')
		)
		return 1;
	return 0;
}

static int unrtf_isfs(char *buf)
{
	if (
		buf[0] == 'f' &&
		buf[1] == 's' &&
		(buf[2] >= '0' || buf[2] <= '9')
		)
		return 1;
	return 0;
}

static int unrtf_islist(char *buf)
{
	if (
		buf[0] == 'l' &&
		buf[1] == 's' &&
		(buf[2] >= '0' || buf[2] <= '9')
		)
		return 1;
	return 0;
}

static int unrtf_iscolwidth(char *buf)
{
	if (
		buf[0] == 'c' &&
		buf[1] == 'e' &&
		buf[2] == 'l' &&
		buf[3] == 'l' &&
		buf[4] == 'x' &&
		(buf[5] >= '0' || buf[2] <= '9')
		)
		return 1;
	return 0;
}



static int unrtf_isvalid(int ch)
{
	if ( 	
		ch == '\n' ||
		ch == '\r' ||
		ch == ' '  ||
		ch == '\t' ||
		ch == ';'  ||
		ch == ','  ||
		ch == ':'  ||
		ch == '.'  ||
		ch == '/'  ||
		ch == '['  ||
		ch == ']'  ||
		ch == '#'  ||
		ch == '@'  ||
		ch == '!'  ||
		ch == '$'  ||
		ch == '%'  ||
		ch == '^'  ||
		ch == '*'  ||
		ch == '('  ||
		ch == ')'  ||
		ch == '-'  ||
		ch == '+'  ||
		ch == '='  ||
		ch == '"'  ||
		ch == '|'  ||
		ch == '\'' ||
		ch == '<'  ||
		ch == '>'  ||
		ch == '`'  ||
		ch == '~'  ||
		ch == '?'  ||
		(ch >= 'A' && ch <= 'Z') ||
		(ch >= 'a' && ch <= 'z') ||
		(ch >= '0' && ch <= '9')
		)
		return 1;

	return 0;
}

static int unrtf_flushstr(
		struct str *str,
		void *userdata,
		int (*text)(void *userdata, const char *text, int len))
{
	// flush str
	if (text)
		if (text(userdata, str->str, str->len))
			return 0;

	free(str->str);
	str_init(str, BUFSIZ);
	return 0;
}

struct unrtf_style {
	int n;
	char name[32];
};

static int unrtf_parse(
		const char *filename,
		void *userdata,
		int (*paragraph_start)(void *userdata),
		int (*paragraph_end)(void *userdata),
		int (*bold_start)(void *userdata),
		int (*bold_end)(void *userdata),
		int (*italic_start)(void *userdata),
		int (*italic_end)(void *userdata),
		int (*underline_start)(void *userdata),
		int (*underline_end)(void *userdata),
		int (*table_start)(void *userdata),
		int (*table_end)(void *userdata),
		int (*tablerow_width)(void *userdata, int i, int w),
		int (*tablerow_start)(void *userdata, int n),
		int (*tablerow_end)(void *userdata, int n),
		int (*tablecell_start)(void *userdata, int n),
		int (*tablecell_end)(void *userdata, int n),
		int (*style)(void *userdata, const char *style),
		int (*text)(void *userdata, const char *text, int len)
		)
{
	FILE *fp = fopen(filename, "r");
	if (!fp)
		return -1;

	int paragraph = 0;
	int stylesheet = 0;
	int stylesheetlevel = 0;
	int level = 0;
	
	struct unrtf_style styles[100];
	int current_style = 0;
	int current_style_number = 0;

	int istable = 0;
	int colwidth[100];
	int colwidthn = 0;
	int row = 0;
	int cell = 0;

	struct str str;
	str_init(&str, BUFSIZ);

	int ch = 1;
	while(ch != EOF)
	{
		ch = fgetc(fp);

unrtf_parse_start:
			
		if (ch == '{')
			level++;
		if (ch == '}'){
			level--;
			if (stylesheetlevel > level)
				stylesheet = 0;
		}

		if (ch == '\\'){ // starts with dash
			char buf[BUFSIZ];	
			ch = unrtf_readword(fp, buf);

			// check if not service word
			if (!unrtf_isservice(buf)){
				// print it if in paragraph
				if (paragraph){
					str_append(&str, buf, strlen(buf));
				}
				goto unrtf_parse_start;
			}

			// handle with service word
			if (strcmp(buf, "stylesheet") == 0){
				// table of styles
				stylesheet = 1;
				stylesheetlevel = level;
			}
			
			if (unrtf_isstyle(buf) && stylesheet){
				// handle with table of slyles
				char *s = buf + 1;
				current_style_number = atoi(s);
			}

			if (strcmp(buf, "qc") == 0 && stylesheet)
			{
				struct unrtf_style style =
					{current_style_number, "CENTER"};
				styles[current_style++] = style; 
			}
			
			if (strcmp(buf, "qr") == 0 && stylesheet)
			{
				struct unrtf_style style =
					{current_style_number, "RIGHT"};
				styles[current_style++] = style; 
			}
			
			if (strcmp(buf, "ql") == 0 && stylesheet)
			{
				struct unrtf_style style =
					{current_style_number, "LEFT"};
				styles[current_style++] = style; 
			}

			if (stylesheet && unrtf_isli(buf))
			{
				struct unrtf_style style =
					{current_style_number, "Q"};
				styles[current_style++] = style; 
			}

			if (stylesheet && unrtf_islist(buf))
			{
				struct unrtf_style style =
					{current_style_number, "LN"};
				styles[current_style++] = style; 
			}

			if (unrtf_isstyle(buf) && paragraph){
				// handle with slyles
				char *s = buf + 1;
				int n = atoi(s);
				int i;
				for (i = 0; i < current_style; ++i) {
					if (n == styles[i].n){
						if (style)
							if (style(userdata, styles[i].name))
								return 0;
						break;
					}	
				}
			}

			if (strcmp(buf, "pard") == 0)
			{
				// new paragraph
				if (paragraph_start)
					if(paragraph_start(userdata))
						return 0;
				paragraph = 1;
				
							if (istable){
					colwidthn = 0;
					if (table_end)
						if (table_end(userdata))
							return 0;
				}
			}
			if (strcmp(buf, "par") == 0)
			{
				unrtf_flushstr(&str, userdata, text);
				// end paragraph
				if (paragraph_end)
					if(paragraph_end(userdata))
						return 0;
				paragraph = 0;
			}

			// UTF
			if (unrtf_isutf(buf)){
				// handle utf
				char unicode[5] = 
				{
					buf[1], 
					buf[2], 
					buf[3], 
					buf[4], 
					0
				};
				
				uint32_t u;
				sscanf(unicode, "%u", &u);			
				uint8_t s[5];
				_utf32_to_utf8(u, s);					
				str_append(&str, (char*)s,
					 	strlen((char*)s));
				
				// if next char is space or ? - drop it
				if (ch == ' ' || ch == '?')
					continue;
			}

			// bold
			if (strcmp(buf, "b") == 0 && paragraph){
				unrtf_flushstr(&str, userdata, text);
				if (bold_start)
					if (bold_start(userdata))
						return 0;
			}
			if (strcmp(buf, "b0") == 0 && paragraph){
				unrtf_flushstr(&str, userdata, text);
				if (bold_end)
					if (bold_end(userdata))
						return 0;
			}

			// italic
			if (strcmp(buf, "i") == 0 && paragraph){
				unrtf_flushstr(&str, userdata, text);
				if (italic_start)
					if (italic_start(userdata))
						return 0;
			}
			if (strcmp(buf, "i0") == 0 && paragraph){
				unrtf_flushstr(&str, userdata, text);
				if (italic_end)
					if (italic_end(userdata))
						return 0;
			}
			
			// underline
			if (strcmp(buf, "ul") == 0 && paragraph){
				unrtf_flushstr(&str, userdata, text);
				if (underline_start)
					if (underline_start(userdata))
						return 0;
			}
			if (strcmp(buf, "ul0") == 0 && paragraph){
				unrtf_flushstr(&str, userdata, text);
				if (underline_end)
					if (underline_end(userdata))
						return 0;
			}

			if (strcmp(buf, "qc") == 0)
				if (style && paragraph)
					if (style(userdata, "CENTER"))
						return 0;
			
			if (strcmp(buf, "ql") == 0)
				if (style && paragraph)
					if (style(userdata, "LEFT"))
						return 0;
			
			if (strcmp(buf, "qr") == 0)
				if (style && paragraph)
					if (style(userdata, "RIGHT"))
						return 0;

			if (unrtf_isli(buf) && paragraph)
				if (style)
					if (style(userdata, "Q"))
						return 0;

			if (unrtf_islist(buf) && paragraph)
				if (style)
					if (style(userdata, "LN"))
						return 0;

			if (strcmp(buf, "trowd") == 0){
				paragraph = 1;
				istable = 1;
				row = 0;
				cell = 0;
				colwidthn = 0;
				if (table_start)
					if(table_start(userdata))
						return 0;
				if (tablerow_start)
					if (tablerow_start(userdata, row))
						return 0;
			}
			
			if (strcmp(buf, "lastrow") == 0){
				istable = 0;
				colwidthn = 0;
				if (table_end)
					if (table_end(userdata))
						return 0;
				if (row)
					if (tablerow_end)
						if (tablerow_end(userdata, row))
							return 0;
				paragraph = 0;
			}
			
			if (strcmp(buf, "row") == 0){
				row = 0;
				if (tablerow_end)
					if (tablerow_end(userdata, row))
						return 0;
				row++;
				cell = 0;
			}
			
			if (strcmp(buf, "intbl") == 0){
				if (tablecell_start)
					if (tablecell_start(userdata, cell))
						return 0;
			}
			
			if (strcmp(buf, "cell") == 0){
				unrtf_flushstr(&str, userdata, text);
				if (tablecell_end)
					if (tablecell_end(userdata, cell))
						return 0;
				cell++;
			}

			if (unrtf_iscolwidth(buf)){
				char *s = buf + 5;
				colwidth[colwidthn] = atoi(s);
				if (tablerow_width)
					if (tablerow_width(userdata, 
								colwidthn, colwidth[colwidthn]))
						return 0;
				
				colwidthn++;
			}
			
			// handle with ch again
			goto unrtf_parse_start;

		} else {
			if (paragraph && unrtf_isvalid(ch)){
				// callback plain text
				char s[2] = {(char)ch, 0};
				str_append(&str, (char*)s, 1);
			}
		}
	}

	return 0;
}
#define UNRTF_H_
#endif /* ifndef UNRTF_H_ */
