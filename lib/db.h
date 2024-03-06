#ifndef DB_H
#define DB_H

#define DB_MAGIC 0xdeadbeef
#define DB_VER 0x10

enum db_type
{
    DB_TYPE_CHAR,
    DB_TYPE_INT
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

int write_db(struct db_table *db, char *fname);
struct db_table *read_db(char *fname);

int db_add_hash(struct db_table *db, char *key, void *value);
int db_add_hash_char(struct db_table *db, char *key, char *value);
int db_add_hash_int(struct db_table *db, char *key, int value);

int db_del_hash(struct db_table *db, char *key);

void *get_hash(struct db_table *db, char *key);
char *get_hash_char(struct db_table *db, char *key);
int get_hash_int(struct db_table *db, char *key);

#endif
