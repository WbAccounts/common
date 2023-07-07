#include "utils/sqlite3_utils.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "log/log.h"
namespace sqlite3_utils {

int sqlite3CreateDB(const char *db_name, sqlite3 **handle) {
    sqlite3* sqlite_handle = NULL;

    int rc = sqlite3_open(db_name, &sqlite_handle);
    if (sqlite_handle == NULL) {
        return -1;
    }

    if (rc != SQLITE_OK) {
        sqlite3_close(sqlite_handle);
        return -1;
    }

#ifdef SQLITE_HAS_CODEC
    static char szOldBrand[] = {51,54,48,0}; // 360
    char szSqliteKey[64] = {0}; //360safeforlinux_sqlite3
    strcpy(szSqliteKey, szOldBrand);
    strcat(szSqliteKey, "safeforlinux_sqlite3");
    sqlite3_key(sqlite_handle, szSqliteKey, strlen(szSqliteKey));
#endif
    *handle = sqlite_handle;

    return 0;
}

int sqlite3OpenDB(const char* db_name,sqlite3** handle) {
    return sqlite3CreateDB(db_name, handle);
}

int sqlite3OpenDBRW(const char* db_name,const char* key,sqlite3** handle) {
    sqlite3* sqlite_handle = NULL;
    int rc = sqlite3_open_v2(db_name, &sqlite_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(sqlite_handle);
        return -1;
    }

    if(key != NULL) {
#ifdef SQLITE_HAS_CODEC
        sqlite3_key(sqlite_handle, key, std::strlen(key));
#endif
    }

    *handle = sqlite_handle;

    return 0;
}

int sqlite3CloseDB(sqlite3 *handle) {
    sqlite3_close(handle);
    return 0;
}

int sqlite3ExecSql(const char* sql, sqlite3* handle) {
    int rc = SQLITE_OK;
    char* errmsg = NULL;

    rc = sqlite3_exec(handle,
                      sql,NULL,NULL,&errmsg);
    if (rc != SQLITE_OK) {
        LOG_ERROR("exec sql error(sql: %s),because: %s.", sql, errmsg);
        sqlite3_free(errmsg);
    }

    return rc;
}

int sqlite3CreateTable(const char *table_sql, sqlite3 *handle) {
    return sqlite3ExecSql(table_sql,handle);
}

int sqlite3ExecCountSql(const char* count_sql, sqlite3* handle, int* counter) {
    char** result = NULL;
    int row = 0;
    int col = 0;
    char* errmsg = NULL;

    int rc = sqlite3_get_table(handle,
        count_sql,
        &result,
        &row,
        &col,
        &errmsg);
    if (rc != SQLITE_OK) {
        LOG_ERROR("get sql table error(count_sql: %s),because: %s.", count_sql, errmsg);
        sqlite3_free(errmsg);
        rc = -1;
    } else {
        if(result[1]) {
            *counter = std::atoi(result[1]);
        }
    }
    sqlite3_free_table(result);

    return rc;
}

bool sqlite3TableExist(const char* table_name, sqlite3* handle) {
    char sql[1024] = {0};
    bool exist = false;
    int counter = 0;

    snprintf(sql, sizeof(sql), "select count(name) from sqlite_master where name = '%s'", table_name);
    if (SQLITE_OK == sqlite3ExecCountSql(sql, handle, &counter)) {
        exist = counter > 0 ? true : false;
    }

    return exist;
}

std::string sqlite3GetColumnText(sqlite3_stmt* stmt, int column) {
    std::string strVaule;
    if (stmt == NULL) {
        return strVaule;
    }
    const unsigned char* value = sqlite3_column_text(stmt, column);
    int bytes = sqlite3_column_bytes(stmt, column);
    if (bytes > 0) {
        strVaule = std::string(reinterpret_cast<const char*>(value), bytes);
    }
    return strVaule;
}

bool sqlite3ColExist(const char* table_name, const char* col_name, sqlite3* handle) {
    if (!table_name || !col_name || !handle) {
        return false;
    }
    sqlite3_stmt* stmt = NULL;
    int rc = -1;
    bool exist = false;
    do {
        char sql[256] = {0};
        snprintf(sql, sizeof(sql), "SELECT * FROM %s LIMIT 0", table_name);
        rc = sqlite3_prepare(handle, sql, sizeof(sql) - 1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            LOG_ERROR("load [%s] failed, prepare sql[%s] failed, because[%s].", table_name, sql, sqlite3_errmsg(handle));
            break;
        }
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            LOG_ERROR("load [%s] failed, step sql[%s] failed, because[%s].", table_name, sql, sqlite3_errmsg(handle));
            break;
        }
        int nCol = sqlite3_column_count(stmt);
        for (int i = 0; i < nCol; i++) {
            if (!strcmp(sqlite3_column_name(stmt, i), col_name)) {
                exist = true;
                break;
            }
        }
    } while(false);
    sqlite3_finalize(stmt);
    return exist;
}

}
 
 
