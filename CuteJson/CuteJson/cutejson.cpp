#include "cutejson.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include <cmath>

//------trip white space------
static void cute_parse_whitespace(cute_context* c) {
	const char* p = c->json;
	while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
		++p;
	c->json = p;
}
//------trip white space------

//------access array------
size_t cute_get_array_size(const cute_value* v) {
	assert(v != NULL && v->type == CUTE_ARRAY);
	return v->u.a.size;
}

cute_value* cute_get_array_element(const cute_value* v, size_t index) {
	assert(v != NULL && v->type == CUTE_ARRAY);
	assert(index < v->u.a.size);
	return &v->u.a.e[index];
}
//------access array------

//------free and alloc------
void cute_free(cute_value* v) {
	size_t i;
	assert(v != NULL);
	switch (v->type)
	{
	case CUTE_STRING:
		free(v->u.s.s);
		break;
	case CUTE_ARRAY:
		for (i = 0; i < v->u.a.size; ++i)
			cute_free(&v->u.a.e[i]);
		free(v->u.a.e);
		break;
	case CUTE_OBJECT:
		for (i = 0; i < v->u.o.size; ++i) {
			free(v->u.o.m[i].k);
			cute_free(&v->u.o.m[i].v);
		}
		free(v->u.o.m);
		break;
	default:
		break;
	}
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

//------get set object------
size_t cute_get_object_size(const cute_value* v) {
	assert(v != NULL && v->u.o.m != NULL && v->type == CUTE_OBJECT);
	return v->u.o.size;
}

const char* cute_get_object_key(const cute_value* v, size_t index)
{
	assert(v != NULL && v->u.o.m != NULL && v->type == CUTE_OBJECT);
	assert(v->u.o.size <= index);
	const char* s;

	cute_member* member = v->u.o.m;
	for (size_t i = 0; i < v->u.o.size; ++i)
	{
		++member;
	}
	return member->k;
}

size_t cute_get_object_key_length(const cute_value* v, size_t index)
{
	assert(v != NULL && v->u.o.m != NULL && v->type == CUTE_OBJECT);
	assert(v->u.o.size <= index);
	const char* s;

	cute_member* member = v->u.o.m;
	for (size_t i = 0; i < v->u.o.size; ++i)
	{
		++member;
	}
	return member->klen;
}

cute_value* cute_get_object_value(const cute_value* v, size_t index)
{
	assert(v != NULL && v->u.o.m != NULL && v->type == CUTE_OBJECT);
	assert(v->u.o.size <= index);
	const char* s;

	cute_member* member = v->u.o.m;
	for (size_t i = 0; i < v->u.o.size; ++i)
	{
		++member;
	}
	return &(member->v);
}
//------get set object------

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
//------parse value------

//------parse string------
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
static int cute_parse_string_raw(cute_context* c, char** str, size_t* len) {
	size_t head = c->top;
	//size_t len;

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
		{
			*len = c->top - head;
			*str = (char*)cute_context_pop(c, *len);
			c->json = p;//reset
			return CUTE_PARSE_OK;
		}
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
static int cute_parse_string(cute_context* c, cute_value* v) {
	int ret;
	char* s;
	size_t len;
	if ((ret = cute_parse_string_raw(c, &s, &len)) == CUTE_PARSE_OK)
	{
		cute_set_string(v, s, len);
		//free(s);
	}

	return ret;
}
//------parse string------

//------parse array------
static int cute_parse_array(cute_context* c, cute_value* v) {
	size_t size = 0;
	int ret = 0;
	EXPECT(c, '[');
	cute_parse_whitespace(c);
	if (*c->json == ']') {
		c->json++;
		v->type = CUTE_ARRAY;
		v->u.a.size = 0;
		v->u.a.e = NULL;
		return CUTE_PARSE_OK;
	}
	for (;;) {
		cute_value e;
		cute_init(&e);
		if ((ret = cute_parse_value(c, &e)) != CUTE_PARSE_OK)
			return ret;
		cute_parse_whitespace(c);
		memcpy(cute_context_push(c, sizeof(cute_value)), &e, sizeof(cute_value));
		++size;
		if (*c->json == ',')
		{
			c->json++;
			cute_parse_whitespace(c);
		}
		else if (*c->json == ']') {
			c->json++;
			v->type = CUTE_ARRAY;
			v->u.a.size = size;
			size *= sizeof(cute_value);
			memcpy(v->u.a.e = (cute_value*)malloc(size), cute_context_pop(c, size), size);
			return CUTE_PARSE_OK;
		}
		else
		{
			ret = CUTE_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
			break;
		}
	}
	size_t i;
	/* Pop and free values on the stack */
	for (i = 0; i < size; ++i)
		cute_free((cute_value*)cute_context_pop(c, sizeof(cute_value)));
	return ret;
}
//------parse array------

//------parse object------
static int cute_parse_object(cute_context* c, cute_value* v) {
	size_t size;
	cute_member m;
	int ret;
	EXPECT(c, '{');
	cute_parse_whitespace(c);
	if (*c->json == '}') {
		c->json++;
		v->type = CUTE_OBJECT;
		v->u.o.m = 0;
		v->u.o.size = 0;
		return CUTE_PARSE_OK;
	}
	m.k = NULL;
	size = 0;
	for (;;) {
		char* str;
		cute_init(&m.v);
		/* 1. parse key */
		if (*c->json != '"') {
			ret = CUTE_PARSE_MISS_KEY;
			break;
		}
		if ((ret = cute_parse_string_raw(c, &str, &m.klen)) != CUTE_PARSE_OK)
			break;
		memcpy(m.k = (char*)malloc(m.klen + 1), str, m.klen);
		m.k[m.klen] = '\0';
		/* 2. parse ws colon ws */
		cute_parse_whitespace(c);
		if (*c->json != ':') {
			ret = CUTE_PARSE_MISS_COLON;
			break;
		}
		c->json++;
		cute_parse_whitespace(c);
		/* 3. parse value */
		if ((ret = cute_parse_value(c, &m.v)) != CUTE_PARSE_OK)
			break;
		memcpy(cute_context_push(c, sizeof(cute_member)), &m, sizeof(cute_member));
		++size;
		m.k = NULL;/* parse successfully, set to null, ownership is transferred to member on stack */
		cute_parse_whitespace(c);
		if (*c->json == ',') {
			c->json++;
			cute_parse_whitespace(c);
		}
		else if (*c->json == '}') {
			size_t s = sizeof(cute_member) * size;
			c->json++;
			v->type = CUTE_OBJECT;
			v->u.o.size = size;
			memcpy(v->u.o.m = (cute_member*)malloc(s), cute_context_pop(c, s), s);
			return CUTE_PARSE_OK;
		}
		else {
			ret = CUTE_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
			break;
		}
	}
	/* 5. Pop and free members on the stack */
	free(m.k);
	size_t i;
	for (i = 0; i < size; ++i)
	{
		cute_member* m = (cute_member*)cute_context_pop(c, sizeof(cute_member));
		free(m->k);
		cute_free(&m->v);
	}

	return ret;
}
//------parse object------

//------parse all------
static int cute_parse_value(cute_context* c, cute_value* v) {
	switch (*c->json) {
	case 'n': return cute_parse_null(c, v);
	case 't': return cute_parse_true(c, v);
	case 'f': return cute_parse_false(c, v);
	case '\"': return cute_parse_string(c, v);
	case '[': return cute_parse_array(c, v);
	case '{': return cute_parse_object(c, v);
	default:  return cute_parse_number(c, v);
	case '\0': return CUTE_PARSE_EXPECT_VALUE;
	}
}
//------parse all------

//------parse json------
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
//------parse json------

//------Generator JSON------
static void cute_stringify_string(cute_context* c, const char* s, size_t len) {
	size_t i;
	assert(s != NULL);
	PUTC(c, '"');
	for (i = 0; i < len; ++i) {
		unsigned char ch = (unsigned char)s[i];
		switch (ch) {
		case '\"': PUTS(c, "\\\"", 2); break;
		case '\\': PUTS(c, "\\\\", 2); break;
		case '\b': PUTS(c, "\\b", 2); break;
		case '\f': PUTS(c, "\\f", 2); break;
		case '\n': PUTS(c, "\\n", 2); break;
		case '\r': PUTS(c, "\\r", 2); break;
		case '\t': PUTS(c, "\\t", 2); break;
		default:
			if (ch < 0x20) {
				char buffer[7];
				sprintf(buffer, "\\u%04X", ch);
				PUTS(c, buffer, 6);
			}
			else
				PUTC(c, s[i]);
		}
	}
	PUTC(c, '"');
}

static int cute_stringify_value(cute_context* c, const cute_value* v) {
	size_t i;
	int ret;
	switch (v->type) {
	case CUTE_NULL: PUTS(c, "null", 4); break;
	case CUTE_FALSE: PUTS(c, "false", 5); break;
	case CUTE_TRUE: PUTS(c, "true", 4); break;
	case CUTE_NUMBER:
	{
		char buffer[32];
		int length = sprintf(buffer, "%.17g", v->u.n);
		PUTS(c, buffer, length);
		break;
	}
	case CUTE_STRING:
	{
		cute_stringify_string(c, v->u.s.s, v->u.s.len);
		break;
	}
	case CUTE_ARRAY:
	{
		PUTC(c, '[');
		for (i = 0; i < v->u.a.size; ++i) {
			if (i > 0)
				PUTC(c, ',');
			cute_stringify_value(c, &v->u.a.e[i]);
		}
		PUTC(c, ']');
		break;
	}
	case CUTE_OBJECT:
	{
		PUTC(c, '{');

		for (i = 0; i < v->u.o.size; ++i) {
			if (i > 0)
				PUTC(c, ',');
			cute_stringify_string(c, v->u.o.m[i].k, v->u.o.m[i].klen);
			PUTC(c, ':');
			cute_stringify_value(c, &v->u.o.m[i].v);
		}
		PUTC(c, '}');
		break;
	}
	return CUTE_STRINGFY_OK;
	}
}

//v to temp context's stack, then output to json
int cute_stringify(const cute_value* v, char** json, size_t* length)
{
	cute_context c;
	int ret;
	assert(v != NULL);
	assert(json != NULL);
	c.stack = (char*)malloc(c.size = CUTE_PARSE_STRINGIFY_INIT_SIZE);
	c.top = 0;
	if ((ret = cute_stringify_value(&c, v)) != CUTE_STRINGFY_OK) {
		free(c.stack);
		*json = NULL;
		return ret;
	}
	if (length)
		*length = c.top;
	PUTC(&c, '\0');
	*json = c.stack;
	return CUTE_STRINGFY_OK;

}
//------Generator JSON------