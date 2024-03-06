#ifndef DB_H
#define DB_H

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

int db_write(struct db_table *db, char *fname);
struct db_table *db_read(char *fname);

int db_set_hash(struct db_table *db, char *key, void *value);
int db_set_hash_char(struct db_table *db, char *key, char *value);
int db_set_hash_int(struct db_table *db, char *key, int value);
int db_set_hash_float(struct db_table *db, char *key, float value);

int db_del_hash(struct db_table *db, char *key);

void *db_get_hash(struct db_table *db, char *key);
int db_get_hash_type(struct db_table *db, char *key);

char *db_get_hash_char(struct db_table *db, char *key);
int db_get_hash_int(struct db_table *db, char *key);
float db_get_hash_float(struct db_table *db, char *key);

#endif
