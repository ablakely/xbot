#ifndef DB_H
#define DB_H

struct db_hash
{
    char key[85];
    char value[4096];
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
int add_hash(struct db_table *db, char *key, char *value);
int del_hash(struct db_table *db, char *key);
char *get_hash(struct db_table *db, char *key);

#endif
