#pragma once

//------expect c's next character == ch------
#define EXPECT(c, ch) do{ assert(*c->json == (ch)); c->json++; } while(0)

#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

#define cute_init(v) do { (v)->type = CUTE_NULL; } while(0)

#define cute_set_null(v) cute_free(v)

#ifndef CUTE_PARSE_STACK_INIT_SIZE
#define CUTE_PARSE_STACK_INIT_SIZE 256
#endif

#ifndef CUTE_PARSE_STRINGIFY_INIT_SIZE
#define CUTE_PARSE_STRINGIFY_INIT_SIZE 256
#endif

#define STRING_ERROR(ret) do { c->top = head; return ret; } while(0)

#define PUTC(c, ch) do { *(char*)cute_context_push(c, sizeof(char)) = (ch); } while(0)
#define PUTS(c, s, len) memcpy(cute_context_push(c, len), s, len)

typedef enum {
	CUTE_NULL,
	CUTE_FALSE,
	CUTE_TRUE,
	CUTE_NUMBER,
	CUTE_STRING,
	CUTE_ARRAY,
	CUTE_OBJECT
}cute_type;

typedef struct cute_value cute_value;
typedef struct cute_member cute_member;

struct cute_value {
	union {
		struct { cute_member* m; size_t size; }o;//member
		struct { char* s; size_t len; }s;//string
		struct { cute_value* e;  size_t size; }a;//array
		double n;//number
	}u;
	cute_type type;
};

struct cute_member {
	char* k;
	size_t klen;//member key string, key string length
	cute_value v;//member value
};

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
	CUTE_PARSE_ROOT_NOT_STNGULAR,
	CUTE_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
	CUTE_PARSE_MISS_OK,
	CUTE_PARSE_MISS_COLON,
	CUTE_PARSE_MISS_COMMA_OR_CURLY_BRACKET,
	CUTE_STRINGFY_OK,
	CUTE_PARSE_MISS_KEY
};

//------trip white space------
static void cute_parse_whitespace(cute_context* c);
//------trip white space------

//------free and alloc------
void cute_free(cute_value* v);
//------free and alloc------

//------access array------
size_t cute_get_array_size(const cute_value* v);
cute_value* cute_get_array_element(const cute_value* v, size_t index);
//------access array------

//------stack push and pop------
static void* cute_context_push(cute_context* c, size_t size);
static void* cute_context_pop(cute_context* c, size_t size);
//------stack push and pop------

//------get type------
cute_type cute_get_type(const cute_value* v);
//------get type------

//------get set number------
double cute_get_number(const cute_value* v);
void cute_set_number(cute_value* v, double n);
//------get set number------

//------get set boolean------
int cute_get_boolean(const cute_value* v);
void cute_set_boolean(cute_value* v, int b);
//------get set boolean------

//------get set string------
void cute_set_string(cute_value* v, const char* s, size_t len);
const char* cute_get_string(const cute_value* v);
size_t cute_get_string_length(const cute_value* v);
//------get set string------

//------get set object------
size_t cute_get_object_size(const cute_value* v);
const char* cute_get_object_key(const cute_value* v, size_t index);
size_t cute_get_object_key_length(const cute_value* v, size_t index);
cute_value* cute_get_object_value(const cute_value* v, size_t index);
//------get set object------

//------parse value------
static int cute_parse_null(cute_context* c, cute_value* v);
static int cute_parse_false(cute_context* c, cute_value* v);
static int cute_parse_number(cute_context* c, cute_value* v);
static int cute_parse_literal(cute_context* c, cute_value* v, const char* literal, cute_type type);
//------parse value------

//------parse string------
static const char* cute_parse_hex4(const char* p, unsigned int* u);
void cute_encode_utf8(cute_context* c, unsigned int u);
static int cute_parse_string_raw(cute_context* c, char** str, size_t* len);
static int cute_parse_string(cute_context* c, cute_value* v);
//------parse string------

//------parse all------
static int cute_parse_value(cute_context* c, cute_value* v);
//------parse all------

//------parse array------
static int cute_parse_array(cute_context* c, cute_value* v);
//------parse array------

//------parse object------
static int cute_parse_object(cute_context* c, cute_value* v);
//------parse object------

/*------parse json text------*/
int cute_parse(cute_value* v, const char* json);
/*------parse json text------*/

/*------Generator JSON Text------*/
static void cute_stringify_string(cute_context* c, const char* s, size_t len);
static int cute_stringify_value(cute_context* c, const cute_value* v);
//v to temp context's stack, then output to json
int cute_stringify(const cute_value* v, char** json, size_t* length);
/*------Generator JSON Text------*/
