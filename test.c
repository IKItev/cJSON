/*  
 *  我们使用测试驱动开发（test driven development, TDD）
 *  此文件包含测试程序，需要链接 leptjson 库。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h" 

/*  传入预期值 expect 和实际值 acactual
    eqequality 为 (expect) == (actual)
    如果预期值不等于实际值，则输出错误信息
    else test_pass++    */
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


static int main_ret = 0;
static int test_pass = 0;
static int test_count = 0;

/*  测试解析    */
static void test_parse();
static void test_parse_null();
static void test_parse_true();
static void test_parse_false();
static void test_parse_expect_value();
static void test_parse_invalid_value();
static void test_parse_root_not_singular();


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
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
}


void test_parse_null()
{
    lept_value val;
    val.type = LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val, "null"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&val));
}

void test_parse_true()
{
    lept_value val;
    val.type = LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val, "true"));
    EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&val));
}

void test_parse_false()
{
    lept_value val;
    val.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val, "false"));
    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&val));
}

/*  只含空白 */
void test_parse_expect_value()
{
    lept_value val;
    val.type = LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&val, ""));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&val));

    val.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&val, " "));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&val));
}

/*  一个值之后，在空白之后还有其他字符   */
void test_parse_invalid_value()
{
    lept_value val;
    val.type = LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&val, "nul"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&val));

    val.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&val, "?"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&val));
}

/*  值不是那三种字面值   */
void test_parse_root_not_singular()
{
    lept_value val;
    val.type = LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&val, "null x"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&val));
}