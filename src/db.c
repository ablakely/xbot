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

    db->db_magic = 0xdeadbeef;
    db->db_ver = 0x10;

    fwrite(db, sizeof(struct db_table) + (sizeof(struct db_hash) * db->count), 1, fp);

    fclose(fp);

    return 0;
}

struct db_table *read_db(char *fname)
{
    FILE *fp;
    struct db_table *db;

    if ((fp = fopen(fname, "rb")) == NULL)
    {
        return NULL;
    }

    db = (struct db_table *)malloc(sizeof(struct db_table));

    fread(db, sizeof(struct db_table), 1, fp);

    // check the magic value
    if (db->db_magic != 0xdeadbeef)
    {
        printf("Error: %s incompatible or unknown db file format\n", fname);

        return NULL;
    }


    // check the version
    if (db->db_ver != 0x10)
    {
        printf("Error: %s incompatible or unknown db file format\n", fname);

        return NULL;
    }

    db->hashes = (struct db_hash *)malloc(sizeof(struct db_hash) * db->count);

    // skip padding
    fseek(fp, 8, SEEK_CUR);

    fread(db->hashes, sizeof(struct db_hash), db->count, fp);

    fclose(fp);

    return db;
}

int add_hash(struct db_table *db, char *key, char *value)
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

    memset(db->hashes[db->count].key, 0, sizeof(db->hashes[db->count].key));
    memset(db->hashes[db->count].value, 0, sizeof(db->hashes[db->count].value));

    strlcpy(db->hashes[db->count].key, key, sizeof(db->hashes[db->count].key));
    strlcpy(db->hashes[db->count].value, value, sizeof(db->hashes[db->count].value));
    db->count++;

    return 0;
}

int del_hash(struct db_table *db, char *key)
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
