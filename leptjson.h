/*
 *  leptjson 的头文件（header file），含有对外的类型和 API 函数声明。
 */


//  但由于头文件也可以 #include 其他头文件
//  为避免重复声明，通常会利用宏加入 include 防范（include guard）
 #ifndef LEPTJSON_H__
 #define LEPTJSON_H__

//  JSON 中有 6 种数据类型，如果把 true 和 false 当作两个类型就是 7 种
//  我们为此声明一个枚举类型（enumeration type）
typedef enum LEPT_TYPE {
    LEPT_NULL, 
    LEPT_FALSE, 
    LEPT_TRUE, 
    LEPT_NUMBER, 
    LEPT_STRING, 
    LEPT_ARRAY, 
    LEPT_OBJECT     //  枚举值通常用全大写
} lept_type;

//  声明 JSON 的数据结构
//  JSON 是一个树形结构，我们最终需要实现一个树的数据结构
//  每个节点使用 lept_value 结构体表示，我们会称它为一个 JSON 值（JSON value）
typedef struct LEPT_VALUE {
    lept_type type;
} lept_value;

enum {
    LEPT_PARSE_OK = 0,      //  无错误会返回 LEPT_PARSE_OK
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR
};

//  解析 JSON 的函数，传入只读文本 json 和 JSON 值的指针
int lept_parse(lept_value* v, const char* json);
//  一般用法是：
//  lept_value v;
//  const char json[] = ...;
//  int ret = lept_parse(&v, json);

//  访问结果的函数，获取 JSON 的数据类型
lept_type lept_get_type(const lept_value* v);

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
    number: 一般的浮点数表示方式，在下一单元详细说明
    string: 表示为 "..."
    array: 表示为 [ ... ]
    object: 表示为 { ... }
*/