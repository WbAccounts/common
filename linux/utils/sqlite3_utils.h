#ifndef UTILS_SQLITE3_DB_H_
#define UTILS_SQLITE3_DB_H_

#include "sqlite3.h"
#include <string>

namespace sqlite3_utils {
    int sqlite3CreateDB(const char* db_name, sqlite3** handle);
    int sqlite3OpenDB(const char* db_name, sqlite3** handle);
    int sqlite3OpenDBRW(const char* db_name, const char* key, sqlite3** handle);
    int sqlite3CloseDB(sqlite3* handle);
    int sqlite3CreateTable(const char* table_sql, sqlite3* handle);
    int sqlite3ExecCountSql(const char* count_sql, sqlite3* handle, int* counter);
    int sqlite3ExecSql(const char* sql, sqlite3* handle);
    bool sqlite3TableExist(const char* table_name, sqlite3* handle);
    std::string sqlite3GetColumnText(sqlite3_stmt* stmt, int column);
    bool sqlite3ColExist(const char* table_name, const char* col_name, sqlite3* handle);
}

#endif /* UTILS_SQLITE3_DB_H_ */
 
 
