/*
 *  leptjson 的实现文件（implementation file），含有内部的类型声明和函数实现。
 *  此文件会编译成库。
 */

#include <assert.h>
#include <stdlib.h>
#include "leptjson.h"

#define EXPECT(con, ch) \
    do {\
        assert((*con->json) == (ch)); \
        con->json++; \
        } while(0)


typedef struct LEPT_CONTEST {
    const char* json;
} lept_context;


int lept_parse(lept_value* val, const char* json);
static void lept_parse_whitespace(lept_context* con); 
static int lept_parse_literal(lept_context* con, lept_value* val, const char* literal, lept_type tpye);
static int lept_parse_number(lept_context* con, lept_value* val);
static int lept_parse_value(lept_context* con, lept_value* val);


/*  获取 JSON 的数据类型 */
lept_type lept_get_type(const lept_value* val) 
{
    assert(val != NULL);
    return val->type;
}


/*  传入 JSON 值的类型，保证类型为 LEPT_NUMBER，才能获取数值 */
double lept_get_number(const lept_value* val) 
{
    assert((NULL != val) && (LEPT_NUMBER == val->type));
    return val->num;
}


/*  解析 JSON 的函数 */
int lept_parse(lept_value* val, const char* json)
{
    lept_context con;
    int ret;
    assert(NULL != val);
    con.json = json;
    val->type = LEPT_NULL;
    lept_parse_whitespace(&con);
    if(LEPT_PARSE_OK == (ret = lept_parse_value(&con, val)))
    {
        lept_parse_whitespace(&con);
        if('\0' != con.json[0])
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    }    
    return ret;
}


/*  让 con->json 指针跳过空白区域 whitespace, ws = *(%x20 / %x09 / %x0A / %x0D) */
void lept_parse_whitespace(lept_context* con)
{
    const char *p = con->json;
    while(' ' == *p || '\t' == *p || '\n' == *p || '\r'== *p )
        p++;
    con->json = p;
}


/*  重构，合并代码相似的 null true false 部分 */
int lept_parse_literal(lept_context* con, lept_value* val, const char* literal, lept_type type)
{
    size_t i = 0;
    EXPECT(con, literal[0]);
    /*  literal 是传入的字符串，定长，拿来做 for 的中止条件
        即判断到字符串结束后跳出循环 */
    for(i = 0; literal[i+1]; i++)
    {
        if(literal[i+1] != con->json[i])
            return LEPT_PARSE_INVALID_VALUE;
    }
    con->json += i;
    val->type = type;
    return LEPT_PARSE_OK;
}


int lept_parse_number(lept_context* con, lept_value* val)
{
    /*  使用 stdlib 函数 strtod()，把字符串转换成 double 的数值
        double strtod(const char *str, char **endptr)
        如果 endptr 不为空，则指向转换中最后一个字符后的字符 */
    char* endptr;
    /*  把文本中的数值存到 val->num 中 */
    val->num = strtod(con->json, &endptr);
    /*  如果没有正确的数值转换，则指针位置没有发生变化 */
    if (endptr == con->json)
        return LEPT_PARSE_INVALID_VALUE;
    con->json = endptr;
    val->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;

}



int lept_parse_value(lept_context* con, lept_value* val) 
{
    switch (*con->json) 
    {
        case 'n':
            return lept_parse_literal(con, val, "null", LEPT_NULL);
        case 't':
            return lept_parse_literal(con, val, "true", LEPT_TRUE);
        case 'f':
            return lept_parse_literal(con, val, "false", LEPT_FALSE);
        case '\0': 
            return LEPT_PARSE_EXPECT_VALUE;
        default:   
            return lept_parse_number(con, val);
            /*  return LEPT_PARSE_INVALID_VALUE; */
    }
}



/*
JSON 文本由 3 部分组成，首先是空白（whitespace），接着是一个值，最后是空白。
    JSON-text = ws value ws     
空白，是由零或多个空格符（space U+0020）、制表符（tab U+0009）、换行符（LF U+000A）、回车符（CR U+000D）所组成。
    ws = *(%x20 / %x09 / %x0A / %x0D)   
我们现时的值只可以是 null、false 或 true，它们分别有对应的字面值（literal）。
    value = null / false / true 
    null  = "null"
    false = "false"
    true  = "true" 
*/

/*
number 是以十进制表示，它主要由 4 部分顺序组成：负号、整数、小数、指数。
只有整数是必需部分。正号是不合法的。
    number = [ "-" ] int [ frac ] [ exp ]
整数部分：0 开始的数，有且只有 0 自身。1-9 则任意。
    int = "0" / digit1-9 *digit
小数部分：小数点后一个或多个数字（0-9）。
    frac = "." 1*digit
指数部分：由大写 E 或小写 e 开始，然后可有正负号，之后是一或多个数字（0-9）。
    exp = ("e" / "E") ["-" / "+"] 1*digit
*/