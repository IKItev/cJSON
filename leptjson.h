/*
 *  leptjson 的头文件（header file），含有对外的类型和 API 函数声明。
 */

/*  但由于头文件也可以 include 其他头文件 
    为避免重复声明，通常会利用宏加入 include 防范（include guard）*/
#ifndef LEPTJSON_H__
#define LEPTJSON_H__
#include <stddef.h> /* size_t */
#include <stdlib.h>

/*  JSON 中有 6 种数据类型，如果把 true 和 false 当作两个类型就是 7 种
    我们为此声明一个枚举类型（enumeration type）  
    枚举值通常用全大写   */
typedef enum LEPT_TYPE {
    LEPT_NULL = 0, 
    LEPT_FALSE, 
    LEPT_TRUE, 
    LEPT_NUMBER, 
    LEPT_STRING, 
    LEPT_ARRAY, 
    LEPT_OBJECT
} lept_type; 

/*  声明 JSON 的数据结构
    JSON 是一个树形结构，我们最终需要实现一个树的数据结构
    每个节点使用 lept_value 结构体表示，称为一个 JSON 值（JSON value) */
typedef struct LEPT_VALUE {
    lept_type type;
    /*  一个值只能是数值或只能是字符串，所以用 union 节省内存 */
    union {
        double num; /*  由于没有限制数字的范围和精度，因此使用 double 来存储 JSON 数字较好 */
        struct {
            char* str;
            size_t len; /*  记录字符串的长度 */
        } s;
    } u;   
} lept_value;

/*  无错误会返回 LEPT_PARSE_OK
    一个 JSON 只含有空白，返回 LEPT_PARSE_EXPECT_VALUE
    一个值之后，在空白之后还有其他字符，返回 LEPT_PARSE_INVALID_VALUE
    值不是那三种字面值，返回 LEPT_PARSE_ROOT_NOT_SINGULAR   */
enum {
    LEPT_PARSE_OK = 0, 
    LEPT_PARSE_EXPECT_VALUE, 
    LEPT_PARSE_INVALID_VALUE, 
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_NUMBER_TOO_BIG,   /*  数值过大 */
    LEPT_PARSE_MISS_QUOTATION_MARK,
    LEPT_PARSE_INVALID_STRING_ESCAPE,
    LEPT_PARSE_INVALID_STRING_CHAR,
    LEPT_PARSE_INVALID_UNICODE_HEX,
    LEPT_PARSE_INVALID_UNICODE_SURROGATE

};

/*  初始化类型 */
#define lept_init(val) \
    do { \
        (val)->type = LEPT_NULL; \
    } while(0)


void lept_free(lept_value* val);

/*  解析 JSON 的函数，传入只读文本 json 和 JSON 值的指针
    一般用法是：
    lept_value v;
    const char json[] = ...;
    int ret = lept_parse(&v, json); */
int lept_parse(lept_value* val, const char* json);

/*  访问结果的函数，获取 JSON 的数据类型 */
lept_type lept_get_type(const lept_value* val);

#define lept_set_null(val) lept_free(val)

int lept_get_boolean(const lept_value* val);
void lept_set_boolean(lept_value* val, int boo);

/*  传入 JSON 值的类型，保证类型为 LEPT_NUMBER，才能获取数值 */
double lept_get_number(const lept_value* val);
void lept_set_number(lept_value* val, double num);


const char* lept_get_string(const lept_value* val);
size_t lept_get_string_length(const lept_value* val);
void lept_set_string(lept_value* val, const char* str, size_t len);


 #endif /* LEPTJSON_H__ */


/*  
JSON 文本格式：
{
    "title": "Design Patterns",
    "subtitle": "Elements of Reusable Object-Oriented Software",
    "author": [
        "Erich Gamma",
        "Richard Helm",
        "Ralph Johnson",
        "John Vlissides"
    ],
    "year": 2009,
    "weight": 1.8,
    "hardcover": true,
    "publisher": {
        "Company": "Pearson Education",
        "Country": "India"
    },
    "website": null
}

    JSON 是树状结构，而 JSON 只包含 6 种数据类型：
    null: 表示为 null
    boolean: 表示为 true 或 false
    number: 一般的浮点数表示方式
    string: 表示为 "..."
    array: 表示为 [ ... ]
    object: 表示为 { ... }
*/