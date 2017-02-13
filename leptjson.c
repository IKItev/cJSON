/*
 *  leptjson 的实现文件（implementation file），含有内部的类型声明和函数实现。
 *  此文件会编译成库。
 */

#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include "leptjson.h"

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

/*  assert() 宏 assert，如果括号内的为 true，则不做任何动作
    如果为 false，输出标准错误 stderr */
#define EXPECT(con, ch) \
    do {\
        assert((*con->json) == (ch)); \
        con->json++; \
    } while(0)

#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

#define PUTC(con, ch) \
    do { \
        *(char*)lept_context_push(con, sizeof(char)) = (ch); \
    } while(0)

typedef struct LEPT_CONTEST {
    const char* json;
    char* stack;
    size_t size, top;   /*  size 是当前的堆栈容量，top 是栈顶的位置 */
} lept_context;

static void* lept_context_push(lept_context* con, size_t size);
static void* lept_context_pop(lept_context* con, size_t size);

static int lept_parse_value(lept_context* con, lept_value* val);
static void lept_parse_whitespace(lept_context* con); 
static int lept_parse_literal(lept_context* con, lept_value* val, const char* literal, lept_type tpye);
static int lept_parse_number(lept_context* con, lept_value* val);
static int lept_parse_string(lept_context* con, lept_value* val);


/*  入栈，返回数据起始的指针 ret */
void* lept_context_push(lept_context* con, size_t size) 
{
    void* ret;
    assert(size > 0);
    if (con->top + size >= con->size) 
    {
        if(0 == con->size)
            con->size = LEPT_PARSE_STACK_INIT_SIZE;
        while (con->top + size >= con->size)
            con->size += con->size >> 1;  /* c->size * 1.5 */
        con->stack = (char*)realloc(con->stack, con->size);
    }
    ret = con->stack + con->top;
    con->top += size;
    return ret;
}


void* lept_context_pop(lept_context* con, size_t size) 
{
    assert(con->top >= size);
    return con->stack + (con->top -= size);
}


void lept_free(lept_value* val)
{
    assert(NULL != val);
    /*  只有当 val 存储的时字符串才 frre */
    if(LEPT_STRING == val->type)
        free(val->u.s.str);
    /*  free 完设为 null，可以避免 type 仍是 string 导致重复释放 */
    lept_init(val);
}


int lept_parse_value(lept_context* con, lept_value* val) 
{
    switch (*con->json) 
    {
        case '\0': 
            return LEPT_PARSE_EXPECT_VALUE;
        case 'n':
            return lept_parse_literal(con, val, "null", LEPT_NULL);
        case 't':
            return lept_parse_literal(con, val, "true", LEPT_TRUE);
        case 'f':
            return lept_parse_literal(con, val, "false", LEPT_FALSE);
        case '"':
            return lept_parse_string(con, val);
        default:   
            return lept_parse_number(con, val);
            /*  return LEPT_PARSE_INVALID_VALUE; */
    }
}


/*  解析 JSON 的函数 */
int lept_parse(lept_value* val, const char* json)
{
    lept_context con;
    int ret = 0;
    assert(NULL != val);
    con.json = json;
    con.stack = NULL;   /*  初始化栈指针 */
    con.size = con.top = 0; /*  初始化 stack 的容量和位置 */
    lept_init(val);
    lept_parse_whitespace(&con);
    if(LEPT_PARSE_OK == (ret = lept_parse_value(&con, val)))
    {
        lept_parse_whitespace(&con);
        if('\0' != con.json[0])
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    }    

    assert(0 == con.top); /*  确保栈中的所有数据都被弹出 */
    free(con.stack);
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
    /*  使用 stdlib.h 函数 strtod()，把字符串转换成 double 的数值
        double strtod(const char *str, char **endptr)
        如果 endptr 不为空，则指向转换中最后一个字符后的字符 */
    /*  使用 errno.h 定义的宏 erron、ERANGE，测试返回值得知数值是否过大 
        C 库宏 ERANGE 表示一个范围错误
        它在输入参数超出数学函数定义的范围时发生，errno 被设置为 ERANGE */
    const char* p;
    p = con->json;
    errno = 0;
    /*  如果文本中的数值是无效数值，则不需要转换 */
    /*  number = [ "-" ] int [ frac ] [ exp ] */
    /*  符号部分 */
    if('+' == *p)
        return LEPT_PARSE_INVALID_VALUE;
    if('-' == *p) p++;
    /*  整数部分 int = "0" / digit1-9 *digit */
    /*  0 开头只能有且只有 0 */
    if('0' == *p) 
    {
        p++;
        if(*p && '.' != *p)
            return LEPT_PARSE_ROOT_NOT_SINGULAR;          
    }
    else    /*  1-9 开头 */
    {   
        if(ISDIGIT1TO9(*p))
            for(p++; ISDIGIT(*p); p++);
        else
            return LEPT_PARSE_INVALID_VALUE; 
    }
    /*  小数部分 frac = "." 1*digit */
    if('.' == *p)
    {
        p++;
        if(ISDIGIT(*p))
            for(p++; ISDIGIT(*p); p++);
        else
            return LEPT_PARSE_INVALID_VALUE; 
    }
    /*  指数部分 exp = ("e" / "E") ["-" / "+"] 1*digit */
    if('e' == *p || 'E' == *p)
    {
        p++;
        if('+' == *p || '-' == *p) p++;
        if(ISDIGIT(*p))
            for(p++; ISDIGIT(*p); p++);
        else 
            return LEPT_PARSE_INVALID_VALUE;
    }

    /*  把文本中的数值存到 val->u.num 中 */
    val->u.num = strtod(con->json, NULL);

    /*  如果没有正确的数值转换，则指针位置没有发生变化 
    if(p == con->json)
        return LEPT_PARSE_INVALID_VALUE; */
    
    /*  如果结果的幅度太大以致于无法表示，则使用 ekkrrno.h 的宏： errno == ERANGE
        仅使用 errno == ERANGE 判断有可能误判
        如果这个值真的很大，则会返回 math.h 的宏 HUGE_VAL 或 -HUGE_VAL
        如果结果的幅度太小，则会返回零值，但 error 可能为 ERANGE，也有可能不为 ERANGE */
    /*  理论上只 (-)HUGE_VAL == val->u.num 验证过大过小足够 */
    if(errno == ERANGE && (HUGE_VAL == val->u.num || -HUGE_VAL == val->u.num))
        return LEPT_PARSE_NUMBER_TOO_BIG;
        
    con->json = p;
    val->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;

}


int lept_parse_string(lept_context* con, lept_value* val)
{
    size_t head, len;
    char* sta;  
    const char* p;
    char ch;
    head = con->top; 
    EXPECT(con, '\"');
    p = con->json;
    while(1)
    {
        ch = *p++;
        switch(ch)
        {
            /*  JSON 字符串第一个字符一定是 \" */
            case '\"':
                len = con->top - head;
                /*  弹栈，此时字符串存储在您是区域 con->stack 中 
                    再讲字符转义存入 con->json 中 */
                sta = (char*)lept_context_pop(con, len);
                lept_set_string(val, sta, len); 
                con->json = p;
                val->type = LEPT_STRING;
                return LEPT_PARSE_OK;
            case '\\':
                switch (*p++) 
                {
                    case '\"': PUTC(con, '\"'); break;
                    case '\\': PUTC(con, '\\'); break;
                    case '/':  PUTC(con, '/' ); break;
                    case 'b':  PUTC(con, '\b'); break;
                    case 'f':  PUTC(con, '\f'); break;
                    case 'n':  PUTC(con, '\n'); break;
                    case 'r':  PUTC(con, '\r'); break;
                    case 't':  PUTC(con, '\t'); break;
                    default:
                        /*  无效转义字符部分 */
                        con->top = head;
                        return LEPT_PARSE_INVALID_STRING_ESCAPE;
                }
                break;
            case '\0':
                con->top = head;
                return LEPT_PARSE_MISS_QUOTATION_MARK;
            default:
                /*  不合法字符串部分 */
                if ((unsigned char)ch < 0x20) 
                { 
                        con->top = head;
                        return LEPT_PARSE_INVALID_STRING_CHAR;
                }
                PUTC(con, ch); /*   每入栈一次，top 大小 +1 */
        }
        
            
        


    }

}


/*  获取 JSON 的数据类型 */
lept_type lept_get_type(const lept_value* val) 
{
    assert(val != NULL);
    return val->type;
}


/*  boolean part */
int lept_get_boolean(const lept_value* val)
{
    assert((NULL != val) && ((LEPT_TRUE == val->type) || (LEPT_FALSE == val->type)) );
    /*  判断 LEPT_TRUE ?== val->type 返回 0(false) 还是 1(true) */
    return (LEPT_TRUE == val->type);
}


void lept_set_boolean(lept_value* val, int boo)
{
    assert(NULL != val);
    lept_init(val);
    if(boo) val->type = LEPT_TRUE;
    else val->type = LEPT_FALSE;
}


/*  number part */
/*  传入 JSON 值的类型，保证类型为 LEPT_NUMBER，才能获取数值 */
double lept_get_number(const lept_value* val) 
{
    assert((NULL != val) && (LEPT_NUMBER == val->type));
    return val->u.num;
}


void lept_set_number(lept_value* val, double num)
{
    assert(NULL != val);
    lept_init(val);
    val->u.num = num;
    val->type = LEPT_NUMBER;
}


/*  string part */
const char* lept_get_string(const lept_value* val)
{
    assert((NULL != val) && (LEPT_STRING == val->type));
    return val->u.s.str;
}


size_t lept_get_string_length(const lept_value* val)
{
    assert((NULL != val) && (LEPT_STRING == val->type));
    return val->u.s.len;
}


/*  设置一个值为字符串，把 str 传入到 val->u.s.str 中 */
void lept_set_string(lept_value* val, const char* str, size_t len)
{
    assert((NULL != val) && ((NULL != str) || (len == 0)) );
    lept_free(val);
    val->u.s.str = (char*)malloc(len + 1);
    /*  把长度为 len 的字符串 str 复制到 val->u.s.str 中 */
    memcpy(val->u.s.str, str, len);
    val->u.s.len = len;
    /*  设置最后一位为 '\0' */
    val->u.s.str[len] = '\0';
    val->type = LEPT_STRING;
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

/*
字符串
    string = quotation-mark *char quotation-mark
无转义字符 
    char = unescaped /
转义序列：转义序列有 9 种，都是以反斜线 \ 开始    
    escape (
        %x22 /          ; "    quotation mark  U+0022
        %x5C /          ; \    reverse solidus U+005C
        %x2F /          ; /    solidus         U+002F
        %x62 /          ; b    backspace       U+0008
        %x66 /          ; f    form feed       U+000C
        %x6E /          ; n    line feed       U+000A
        %x72 /          ; r    carriage return U+000D
        %x74 /          ; t    tab             U+0009
        %x75 4HEXDIG )  ; uXXXX                U+XXXX   16 进位的 UTF-16 编码
字符串以 \" 开始和结束
    escape = %x5C          ; \
    quotation-mark = %x22  ; "
无转义字符范围
    unescaped = %x20-21 / %x23-5B / %x5D-10FFFF
由上可见：不合法的字符是 %x00 至 %x1F
*/