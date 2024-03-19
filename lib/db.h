#ifndef DB_H
#define DB_H

#include "util.h"

#define DB_MAGIC 0xdeadbeef
#define DB_VER 0x10

enum db_type
{
    DB_TYPE_CHAR,
    DB_TYPE_INT,
    DB_TYPE_FLOAT
};

struct db_hash
{
    int key_len;
    int value_len;
    int type;

    char *key;
    void *value;
};

struct db_table
{
    int db_magic;
    int db_ver;
    int count;

    struct db_hash *hashes;
};

MY_API int db_write(struct db_table *db, char *fname);
MY_API struct db_table *db_read(char *fname);

MY_API int db_set_hash(struct db_table *db, char *key, void *value);
MY_API int db_set_hash_char(struct db_table *db, char *key, char *value);
MY_API int db_set_hash_int(struct db_table *db, char *key, int value);
MY_API int db_set_hash_float(struct db_table *db, char *key, float value);

MY_API int db_del_hash(struct db_table *db, char *key);

MY_API void *db_get_hash(struct db_table *db, char *key);
MY_API int db_get_hash_type(struct db_table *db, char *key);
MY_API int db_hash_exists(struct db_table *db, char *key);

MY_API char *db_get_hash_char(struct db_table *db, char *key);
MY_API int db_get_hash_int(struct db_table *db, char *key);
MY_API float db_get_hash_float(struct db_table *db, char *key);

#endif
