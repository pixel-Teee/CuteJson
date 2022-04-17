#pragma once

#include <assert.h>

//-----expect c's next character == ch
#define EXPECT(c, ch) do{ assert(*c->json == (ch)); c->json++; } while(0)

#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

#define cute_init(v) do { (v)->type = CUTE_NULL; } while(0)

#define cute_set_null(v) cute_free(v)

#ifndef CUTE_PARSE_STACK_INIT_SIZE
#define CUTE_PARSE_STACK_INIT_SIZE 256
#endif

#define STRING_ERROR(ret) do { c->top = head; return ret; } while(0)

typedef enum {
	CUTE_NULL,
	CUTE_FALSE,
	CUTE_TRUE,
	CUTE_NUMBER,
	CUTE_STRING,
	CUTE_ARRAY,
	CUTE_OBJECT
}cute_type;

typedef struct {
	union {
		struct { char* s; size_t len; }s;//string
		double n;//number
	}u;
	cute_type type;
}cute_value;

typedef struct {
	const char* json;
	char* stack;//stack
	size_t size, top;//stack
}cute_context;

enum {
	CUTE_PARSE_OK = 0,
	CUTE_PARSE_EXPECT_VALUE,
	CUTE_PARSE_INVALID_VALUE,
	CUTE_PARSE_NUMBER_TOO_BIG,
	CUTE_PARSE_MISS_QUOTATION_MARK,
	CUTE_PARSE_INVALID_STRING_CHAR,
	CUTE_PARSE_INVALID_STRING_ESCAPE,
	CUTE_PARSE_INVALID_UNICODE_SURROGATE,
	CUTE_PARSE_INVALID_UNICODE_HEX,
	CUTE_PARSE_ROOT_NOT_STNGULAR
};

//------free and alloc------
void cute_free(cute_value* v) {
	assert(v != NULL);
	if (v->type == CUTE_STRING)
		free(v->u.s.s);
	v->type = CUTE_NULL;
}
//------free and alloc------

//------stack push and pop------
static void* cute_context_push(cute_context* c, size_t size) {
	void* ret;
	assert(size > 0);
	if (c->top + size >= c->size)
	{
		if (c->size == 0)
			c->size = CUTE_PARSE_STACK_INIT_SIZE;
		while (c->top + size >= c->size)
			c->size += c->size >> 1;/*c->size * 1.5*/
		c->stack = (char*)realloc(c->stack, c->size);
	}
	ret = c->stack + c->top;
	c->top += size;
	return ret;
}

static void* cute_context_pop(cute_context* c, size_t size) {
	assert(c->top >= size);
	return c->stack + (c->top -= size);
}
//------stack push and pop------

//------get type------
cute_type cute_get_type(const cute_value* v) {
	return v->type;
}
//------get type------

//------get set number------
double cute_get_number(const cute_value* v) {
	assert(v != NULL && v->type == CUTE_NUMBER);
	return v->u.n;
}

void cute_set_number(cute_value* v, double n) {
	cute_free(v);
	v->u.n = n;
	v->type = CUTE_NUMBER;
}
//------get set number------

//------get set boolean------
int cute_get_boolean(const cute_value* v) {
	assert(v != NULL && (v->type == CUTE_TRUE || v->type == CUTE_FALSE));
	return v->type == CUTE_TRUE;
}

void cute_set_boolean(cute_value* v, int b) {
	cute_free(v);
	v->type = b ? CUTE_TRUE : CUTE_FALSE;
}
//------get set boolean------

//------get set string------
void cute_set_string(cute_value* v, const char* s, size_t len)
{
	assert(v != NULL && (s != NULL || len == 0));
	cute_free(v);
	v->u.s.s = (char*)malloc(len + 1);
	memcpy(v->u.s.s, s, len);
	v->u.s.s[len] = '\0';
	v->u.s.len = len;
	v->type = CUTE_STRING;
}

const char* cute_get_string(const cute_value* v) {
	assert(v != NULL && v->u.s.s != NULL && v->type == CUTE_STRING);
	return v->u.s.s;
}

size_t cute_get_string_length(const cute_value* v) {
	assert(v != NULL && v->u.s.s != NULL && v->type == CUTE_STRING);
	return v->u.s.len;
}
//------get set string------

//------parse value------
static int cute_parse_null(cute_context* c, cute_value* v) {
	EXPECT(c, 'n');
	if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
		return CUTE_PARSE_INVALID_VALUE;
	c->json += 3;
	v->type = CUTE_NULL;
	return CUTE_PARSE_OK;
}

static int cute_parse_true(cute_context* c, cute_value* v) {
	EXPECT(c, 't');
	if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
		return CUTE_PARSE_INVALID_VALUE;
	c->json += 3;
	v->type = CUTE_TRUE;
	return CUTE_PARSE_OK;
}

static int cute_parse_false(cute_context* c, cute_value* v) {
	EXPECT(c, 'f');
	if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
		return CUTE_PARSE_INVALID_VALUE;
	c->json += 4;
	v->type = CUTE_FALSE;
	return CUTE_PARSE_OK;
}

static int cute_parse_number(cute_context* c, cute_value* v)
{
	const char* p = c->json;
	char* end;
	/*负号 ... */
	if (*p == '-') ++p;
	/*整数 ... */
	if (*p == '0') ++p;
	else {
		if (!ISDIGIT1TO9(*p)) return CUTE_PARSE_INVALID_VALUE;
		for (p++; ISDIGIT(*p); p++);
	}
	/*小数 ... */
	if (*p == '.') {
		++p;
		if (!ISDIGIT(*p)) return CUTE_PARSE_INVALID_VALUE;
		for (p++; ISDIGIT(*p); p++);
	}
	/*指数 ... */
	if (*p == 'e' || *p == 'E') {
		++p;
		if (*p == '+' || *p == '-') ++p;
		if (!ISDIGIT(*p)) return CUTE_PARSE_INVALID_VALUE;
		for (p++; ISDIGIT(*p); p++);
	}
	errno = 0;//global scope variable
	v->u.n = strtod(c->json, &end);
	if (errno == ERANGE && v->u.n == HUGE_VAL) return CUTE_PARSE_NUMBER_TOO_BIG;
	if (c->json == end)
		return CUTE_PARSE_INVALID_VALUE;
	c->json = end;
	v->type = CUTE_NUMBER;
	return CUTE_PARSE_OK;
}

static int cute_parse_literal(cute_context* c, cute_value* v, const char* literal, cute_type type) {
	size_t i;
	EXPECT(c, literal[0]);
	for (i = 0; literal[i + 1]; ++i)
		if (c->json[i] != literal[i + 1])
			return CUTE_PARSE_INVALID_VALUE;
	c->json += i;
	v->type = type;
	return CUTE_PARSE_OK;
}

//------parse string------
#define PUTC(c, ch) do { *(char*)cute_context_push(c, sizeof(char)) = (ch); } while(0)

static const char* cute_parse_hex4(const char* p, unsigned int* u)
{
	int i;
	*u = 0;
	for (i = 0; i < 4; ++i)
	{
		char ch = *p++;
		*u <<= 4;
		if (ch >= '0' && ch <= '9') *u |= ch - '0';
		else if (ch >= 'A' && ch <= 'F') *u |= ch - ('A' - 10);
		else if (ch >= 'a' && ch <= 'f') *u |= ch - ('a' - 10);
		else return NULL;
	}
	return p;
}

void cute_encode_utf8(cute_context* c, unsigned int u)
{
	if (u <= 0x7F)
		PUTC(c, u & 0xFF);
	else if (u <= 0x7FF) {
		PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
		PUTC(c, 0x80 | (u & 0x3F));
	}
	else if (u <= 0xFFFF) {
		PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
		PUTC(c, 0x80 | ((u >> 6) & 0x3F));
		PUTC(c, 0x80 | (u & 0x3F));
	}
	else {
		assert(u <= 0x10FFFF);
		PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
		PUTC(c, 0x80 | ((u >> 12) & 0x3F));
		PUTC(c, 0x80 | ((u >> 6) & 0x3F));
		PUTC(c, 0x80 | (u & 0x3F));
	}
}

static int cute_parse_string(cute_context* c, cute_value* v) {
	size_t head = c->top;
	size_t len;

	unsigned u, u2;

	const char* p;
	EXPECT(c, '\"');
	p = c->json;
	for (;;) {
		char ch = *p++;
		switch (ch) {
		case '\\':
			switch (*p++) {
				case '\"': PUTC(c, '\"'); break;
				case '\\': PUTC(c, '\\'); break;
				case '/':  PUTC(c, '/'); break;
				case 'b':  PUTC(c, '\b'); break;
				case 'f':  PUTC(c, '\f'); break;
				case 'n':  PUTC(c, '\n'); break;
				case 'r':  PUTC(c, '\r'); break;
				case 't':  PUTC(c, '\t'); break;
				case 'u':
					if (!(p = cute_parse_hex4(p, &u)))
						STRING_ERROR(CUTE_PARSE_INVALID_UNICODE_HEX);
					if (u >= 0xD800 && u <= 0xDBFF) {
						if (*p++ != '\\')
							STRING_ERROR(CUTE_PARSE_INVALID_UNICODE_SURROGATE);
						if (*p++ != 'u')
							STRING_ERROR(CUTE_PARSE_INVALID_UNICODE_SURROGATE);
						if (!(p = cute_parse_hex4(p, &u2)))
							STRING_ERROR(CUTE_PARSE_INVALID_UNICODE_HEX);
						if (u2 < 0xDC00 || u2 > 0xDFFF)
							STRING_ERROR(CUTE_PARSE_INVALID_UNICODE_SURROGATE);
						u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
					}
					cute_encode_utf8(c, u);
					break;
				default:
				c->top = head;
				return CUTE_PARSE_INVALID_STRING_ESCAPE;
			}
			break;
		case '\"':
			len = c->top - head;
			cute_set_string(v, (const char*)cute_context_pop(c, len), len);
			c->json = p;//reset
			return CUTE_PARSE_OK;
		case '\0':
			c->top = head;
			return CUTE_PARSE_MISS_QUOTATION_MARK;
		default:
			if ((unsigned char)ch < 0x20) {
				c->top = head;
				return CUTE_PARSE_INVALID_STRING_CHAR;
			}
			PUTC(c, ch);
		}
	}
}
//------parse string------

static int cute_parse_value(cute_context* c, cute_value* v) {
	switch (*c->json) {
		case 'n': return cute_parse_null(c, v);		
		case 't': return cute_parse_true(c, v);
		case 'f': return cute_parse_false(c, v);
		case '\"': return cute_parse_string(c, v);
		default:  return cute_parse_number(c, v);
		case '\0': return CUTE_PARSE_EXPECT_VALUE;
	}
}

//------parse value------

static void cute_parse_whitespace(cute_context* c) {
	const char* p = c->json;
	while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
		++p;
	c->json = p;
}

/*------JSON-text = ws value ws------*/
int cute_parse(cute_value* v, const char* json) {
	cute_context c;
	int ret;
	assert(v != NULL);
	c.json = json;
	c.stack = NULL;//<-
	c.size = c.top = 0;//<-
	cute_init(v);
	v->type = CUTE_NULL;
	//------trip white space------
	cute_parse_whitespace(&c);
	if ((ret = cute_parse_value(&c, v)) == CUTE_PARSE_OK)
	{
		cute_parse_whitespace(&c);
		if (*c.json != '\0')
			ret = CUTE_PARSE_ROOT_NOT_STNGULAR;
	}
	//------trip white space------
	assert(c.top == 0);//<-
	free(c.stack);//<-
	return ret;
}
