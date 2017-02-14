/*  
 *  我们使用测试驱动开发（test driven development, TDD）
 *  此文件包含测试程序，需要链接 leptjson 库。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h" 

/*  传入预期值 expect 和实际值 actual
    eqequality 为 (expect) == (actual)
    如果预期值不等于实际值，则输出错误信息
    else test_pass++ */
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

#define EXPECT_EQ_INT(expect, actual) \
    EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

#define EXPECT_EQ_DOUBLE(expect, actual) \
    EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%g")
    /*  f	以小数形式输出单、双精度实数
        e,E	以指数形式输出单、双精度实数
        g,G	以 %f 或 %e 中较短的输出宽度输出单、双精度实数 */

#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == alength && memcmp(expect, actual, alength) == 0, expect, actual, "%s")


/*  测试数值的宏 */
#define TEST_NUMBER(expect, json) \
    do { \
        lept_value val; \
        lept_init(&val); \
        EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val, json)); \
        EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&val)); \
        EXPECT_EQ_DOUBLE(expect, lept_get_number(&val)); \
        lept_free(&val); \
    } while(0)


/*  测试字符串的宏 */
#define TEST_STRING(expect, json) \
    do { \
        lept_value val; \
        lept_init(&val); \
        EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val, json)); \
        EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&val)); \
        EXPECT_EQ_STRING(expect, lept_get_string(&val), lept_get_string_length(&val)); \
        lept_free(&val); \
    } while(0)


/*  仅对集中无效部分的代码进行宏定义替换重构
    由于有小部分的测试将来要有所添加
    无效值类型都是 null */
#define TEST_ERROR(error, json) \
    do { \
        lept_value val; \
        lept_init(&val); \
        EXPECT_EQ_INT(error, lept_parse(&val, json)); \
        EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&val)); \
        lept_free(&val); \
    } while(0)


static int main_ret = 0;
static int test_pass = 0;
static int test_count = 0;

/*  测试解析 */
static void test_parse();
static void test_parse_null();
static void test_parse_true();
static void test_parse_false();
static void test_parse_number();
static void test_parse_string();

static void test_parse_expect_value();
static void test_parse_invalid_value();
static void test_parse_root_not_singular();
static void test_parse_number_too_big();

static void test_parse_missing_quotation_mark(); 
static void test_parse_invalid_string_escape(); 
static void test_parse_invalid_string_char();
static void test_parse_invalid_unicode_hex();
static void test_parse_invalid_unicode_surrogate();

static void test_access_null();
static void test_access_boolean();
static void test_access_number();
static void test_access_string();

int main(int argc, char **argv)
{
    test_parse();
    printf("%d / %d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    
    return main_ret;
}


void test_parse()
{
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_string();

    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();

    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();
    
    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();

}


void test_parse_null()
{
    lept_value val;
    lept_init(&val);
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val, "null"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&val));
    lept_free(&val);
}


void test_parse_true()
{
    lept_value val;
    lept_init(&val);
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val, "true"));
    EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&val));
    lept_free(&val);
}


void test_parse_false()
{
    lept_value val;
    lept_init(&val);
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val, "false"));
    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&val));
    lept_free(&val);
}


void test_parse_number() 
{
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
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(5.0E-324,"5e-324"); /*Min subnormal positive double*/ 
    TEST_NUMBER(1.7976931348623157E308,"1.7976931348623157E308"); /*Max Double*/ 
    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}


void test_parse_string() 
{
    TEST_STRING("", "\"\"");
    TEST_STRING("/", "\"\\/\"");
    TEST_STRING("/", "\"/\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}


/*  只含空白 */
void test_parse_expect_value()
{
    TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, " ");
}


/*  无效值 */
void test_parse_invalid_value()
{
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "?");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "a");

    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */

    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nan");

}


/*  一个值之后，在空白之后还有其他字符 */
void test_parse_root_not_singular()
{
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "null x");
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "nullx");
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x123");
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0e123");  
}


/*  数值过大 */
void test_parse_number_too_big() 
{
    TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "-1e309");
    
}

void test_parse_missing_quotation_mark() 
{
    TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

void test_parse_invalid_string_escape() 
{
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

void test_parse_invalid_string_char() 
{
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}


void test_parse_invalid_unicode_hex() 
{
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}


void test_parse_invalid_unicode_surrogate() 
{
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

/*  测试写入部分 */

void test_access_null()
{
    lept_value val;
    lept_init(&val);
    lept_set_null(&val);
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&val));
    lept_free(&val);
}


void test_access_boolean()
{
    lept_value val;
    lept_init(&val);
    lept_set_boolean(&val, 1);
    EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&val));
    lept_set_boolean(&val, 0);
    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&val));
    lept_free(&val);
}


void test_access_number()
{   
    lept_value val;
    lept_init(&val);
    lept_set_number(&val, 1.234);
    EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&val));
    EXPECT_EQ_DOUBLE(1.234, lept_get_number(&val));
    lept_free(&val);
}


void test_access_string() 
{
    lept_value val;
    lept_init(&val);
    lept_set_string(&val, "", 0);
    EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&val));
    EXPECT_EQ_STRING("", lept_get_string(&val), lept_get_string_length(&val));
    lept_set_string(&val, "Hello", 5);
    EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&val));    
    EXPECT_EQ_STRING("Hello", lept_get_string(&val), lept_get_string_length(&val));
    lept_free(&val);
}


