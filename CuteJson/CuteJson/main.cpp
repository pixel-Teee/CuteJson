#include <iostream>
#include "cutejson.h"

#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((1) == (actual), 1, actual, "%d")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((0) == (actual), 0, actual, "%d")

#define TEST_NUMBER(expect, json)\
    do{\
        cute_value v;\
        EXPECT_EQ_INT(CUTE_PARSE_OK, cute_parse(&v, json));\
        EXPECT_EQ_INT(CUTE_NUMBER, cute_get_type(&v));\
    }while(0)

static void test_parse_null() {
	cute_value v;
	v.type = CUTE_TRUE;
	EXPECT_EQ_INT(CUTE_PARSE_OK, cute_parse(&v, "null"));
    EXPECT_EQ_INT(CUTE_PARSE_ROOT_NOT_STNGULAR, cute_parse(&v, "null x"));
}

static void test_parse_true() {
    cute_value v;
    v.type = CUTE_FALSE;
    EXPECT_EQ_INT(CUTE_PARSE_OK, cute_parse(&v, "true"));
    EXPECT_EQ_INT(CUTE_TRUE, cute_get_type(&v));
}

static void test_parse_false() {
    cute_value v;
    v.type = CUTE_TRUE;
    EXPECT_EQ_INT(CUTE_PARSE_OK, cute_parse(&v, "false"));
    EXPECT_EQ_INT(CUTE_FALSE, cute_get_type(&v));
}

static void test_parse_number() {
	TEST_NUMBER(0.0, "0");
	TEST_NUMBER(0.0, "-0");
	TEST_NUMBER(0.0, "-0.0");
	TEST_NUMBER(1.0, "1");
	TEST_NUMBER(-1.0, "-1");
	TEST_NUMBER(1.5, "1.5");
	TEST_NUMBER(-1.5, "-1.5");
	TEST_NUMBER(3.1416, "3.1416");
	TEST_NUMBER(1E10, "1E10");
	TEST_NUMBER(1e10, "1e10");
	TEST_NUMBER(1E+10, "1E+10");
	TEST_NUMBER(1E-10, "1E-10");
	TEST_NUMBER(-1E10, "-1E10");
	TEST_NUMBER(-1e10, "-1e10");
	TEST_NUMBER(-1E+10, "-1E+10");
	TEST_NUMBER(1.234E+10, "1.234E+10");
	TEST_NUMBER(1.234E-10, "1.234E-10");
	TEST_NUMBER(0.0, "1e-10000");
}

static void test_parse_string() {
	cute_value v;
    v.type = CUTE_STRING;
    EXPECT_EQ_INT(CUTE_PARSE_OK, cute_parse(&v, "\"tes\uabcdt\""));
    EXPECT_EQ_INT(CUTE_STRING, cute_get_type(&v));
    cute_free(&v);
}

static void test_access_string() {
    cute_value v;
    cute_init(&v);
    cute_set_string(&v, "", 0);
    cute_set_string(&v, "Hello", 5);
    cute_free(&v);
}

static void test_access_boolean() {
    cute_value v;
    cute_init(&v);
    cute_set_string(&v, "a", 1);
    cute_set_boolean(&v, 1);
    EXPECT_TRUE(cute_get_boolean(&v));
    cute_set_boolean(&v, 0);
    EXPECT_FALSE(cute_get_boolean(&v));
    cute_free(&v);
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_string();

    //-----access------
    test_access_string();
    test_access_boolean();
}

int main()
{
#ifdef _WINDOWS
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    //_CrtSetBreakAlloc(91);
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
   
	return 0;
}