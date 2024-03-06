#include "util.h"
#include "irc.h"
#include "db.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int write_db(struct db_table *db, char *fname)
{
    FILE *fp;
    int i;

    if ((fp = fopen(fname, "wb")) == NULL)
    {
        return -1;
    }

    db->db_magic = DB_MAGIC;
    db->db_ver = DB_VER;

    // write the header
    fwrite(db, sizeof(struct db_table), 1, fp);

    // write the hashes
    fwrite(db->hashes, sizeof(struct db_hash), db->count, fp);

    // write the keys and values
    for (i = 0; i < db->count; i++)
    {
        fwrite(db->hashes[i].key, sizeof(char), db->hashes[i].key_len, fp);
        fwrite(db->hashes[i].value, sizeof(char), db->hashes[i].value_len, fp);
    }

    fclose(fp);

    return 0;
}

struct db_table *read_db(char *fname)
{
    FILE *fp;
    struct db_table *db;
    int tmp;

    if ((fp = fopen(fname, "rb")) == NULL)
    {
        return NULL;
    }

    db = (struct db_table *)malloc(sizeof(struct db_table));

    fread(db, sizeof(struct db_table), 1, fp);

    // check the magic value
    if (db->db_magic != DB_MAGIC)
    {
        printf("Error: %s incompatible or unknown db file format: Bad Magic\n", fname);

        return NULL;
    }


    // check the version
    if (db->db_ver != DB_VER)
    {
        printf("Error: %s incompatible or unknown db file format: Incompatible Version\n", fname);

        return NULL;
    }

    tmp = db->count != 0 ? db->count : sizeof(struct db_hash);
    db->hashes = (struct db_hash *)malloc(sizeof(struct db_hash) * tmp);

    fread(db->hashes, sizeof(struct db_hash), db->count, fp);

    // read the keys and values
    for (tmp = 0; tmp < db->count; tmp++)
    {
        db->hashes[tmp].key = (char *)malloc(sizeof(char) * db->hashes[tmp].key_len);
        db->hashes[tmp].value = (char *)malloc(sizeof(char) * db->hashes[tmp].value_len);

        fread(db->hashes[tmp].key, sizeof(char), db->hashes[tmp].key_len, fp);
        fread(db->hashes[tmp].value, sizeof(char), db->hashes[tmp].value_len, fp);
    }

    fclose(fp);

    return db;
}

int db_add_hash(struct db_table *db, char *key, char *value)
{
    int i;

    for (i = 0; i < db->count; i++)
    {
        if (strcmp(db->hashes[i].key, key) == 0)
        {
            return -1;
        }
    }

    db->hashes = (struct db_hash *)realloc(db->hashes, sizeof(struct db_hash) * (db->count + 1));

    // zero out reallocated memory
    memset(&db->hashes[db->count], 0, sizeof(struct db_hash));

    db->hashes[db->count].key_len = strlen(key) + 1;
    db->hashes[db->count].value_len = strlen(value) + 1;

    printf("key_len: %d, value_len: %d\n", sizeof(char) * db->hashes[db->count].key_len, sizeof(char) *  db->hashes[db->count].value_len);

    db->hashes[db->count].key = (char *)malloc(sizeof(char) * db->hashes[db->count].key_len);
    db->hashes[db->count].value = (char *)malloc(sizeof(char) * db->hashes[db->count].value_len);

    memset(db->hashes[db->count].key, 0, sizeof(char) * db->hashes[db->count].key_len);
    memset(db->hashes[db->count].value, 0, sizeof(char) * db->hashes[db->count].value_len);

    strlcpy(db->hashes[db->count].key, key, sizeof(char) * db->hashes[db->count].key_len);
    strlcpy(db->hashes[db->count].value, value, sizeof(char) * db->hashes[db->count].value_len);
    db->count++;

    return 0;
}

int db_del_hash(struct db_table *db, char *key)
{
    int i;

    for (i = 0; i < db->count; i++)
    {
        if (strcmp(db->hashes[i].key, key) == 0)
        {
            free(db->hashes[i].key);
            free(db->hashes[i].value);
            memmove(&db->hashes[i], &db->hashes[i + 1], sizeof(struct db_hash) * (db->count - i));
            db->count--;

            return 0;
        }
    }

    return -1;
}

char *get_hash(struct db_table *db, char *key)
{
    int i;

    for (i = 0; i < db->count; i++)
    {
        if (strcmp(db->hashes[i].key, key) == 0)
        {
            return db->hashes[i].value;
        }
    }

    return NULL;
}
