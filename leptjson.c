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
static int lept_parse_null(lept_context* con, lept_value* val);
static int lept_parse_true(lept_context* con, lept_value* val);
static int lept_parse_false(lept_context* con, lept_value* val);
static int lept_parse_value(lept_context* con, lept_value* val);


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

lept_type lept_get_type(const lept_value* val) 
{
    assert(val != NULL);
    return val->type;
}


/*  让 con->json 指针跳过空白区域 whitespace, ws = *(%x20 / %x09 / %x0A / %x0D)  */
void lept_parse_whitespace(lept_context* con)
{
    const char *p = con->json;
    while(' ' == *p || '\t' == *p || '\n' == *p || '\r'== *p )
        p++;
    con->json = p;
}

/*  con->json 跳过 whitespace，验证 value 部分是否为 null */
int lept_parse_null(lept_context* con, lept_value* val)
{
    /*  验证 *con->json 的值是否为 'null' 的首字母 'n' */
    EXPECT(con, 'n');
    /*  此时只有 null true false 三个值，如果 n 后面不是 ull，则这是无效值 */
    if('u' != con->json[0] || 'l' != con->json[1] || 'l' != con->json[2])
        return LEPT_PARSE_INVALID_VALUE;
    con->json += 3;
    val->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}

/*  验证 value 部分是否为 true */
int lept_parse_true(lept_context* con, lept_value* val)
{
    EXPECT(con, 't');
    if('r' != con->json[0] || 'u' != con->json[1] || 'e' != con->json[2])
        return LEPT_PARSE_INVALID_VALUE;
    con->json += 3;
    val->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

/*  验证 value 部分是否为 false    */
int lept_parse_false(lept_context* con, lept_value* val)
{
    EXPECT(con, 'f');
    if('a' != con->json[0] || 'l' != con->json[1] || 's' != con->json[2] || 'e' != con->json[3])
        return LEPT_PARSE_INVALID_VALUE;
    con->json += 4;
    val->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}


int lept_parse_value(lept_context* con, lept_value* val) 
{
    switch (*con->json) 
    {
        case 'n':
            return lept_parse_null(con, val);
        case 't':
            return lept_parse_true(con, val);
        case 'f':
            return lept_parse_false(con, val);
        case '\0': 
            return LEPT_PARSE_EXPECT_VALUE;
        default:   
            return LEPT_PARSE_INVALID_VALUE;
    }
}



/*
//  JSON 文本由 3 部分组成，首先是空白（whitespace），接着是一个值，最后是空白
JSON-text = ws value ws     
//  所谓空白，是由零或多个空格符（space U+0020）、制表符（tab U+0009）、换行符（LF U+000A）、回车符（CR U+000D）所组成
ws = *(%x20 / %x09 / %x0A / %x0D)   
//  我们现时的值只可以是 null、false 或 true，它们分别有对应的字面值（literal）
value = null / false / true 
null  = "null"
false = "false"
true  = "true" 
*/