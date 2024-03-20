#include "db.h"
#include "logger.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int db_write(struct db_table *db, char *fname)
{
    FILE *fp;
    int i;
    char *fullpath;

    if ((fp = fopen(fname, "wb")) == NULL)
    {
        return -1;
    }

    db->db_magic = DB_MAGIC;
    db->db_ver = DB_VER;

    // get the full path to the db file
#ifdef _WIN32
    fullpath = _fullpath(NULL, fname, 0);
#else
    fullpath = realpath(fname, NULL);
#endif

    xlog("Writing db to file: %s\n", fullpath);

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

struct db_table *db_read(char *fname)
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
        xlog("Error: %s incompatible or unknown db file format: Bad Magic\n", fname);

        return NULL;
    }


    // check the version
    if (db->db_ver != DB_VER)
    {
        xlog("Error: %s incompatible or unknown db file format: Incompatible Version\n", fname);

        return NULL;
    }

    tmp = db->count != 0 ? db->count : 1;

    printf("dbug: allocating %d hashes\n", tmp);

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

int db_set_hash(struct db_table *db, char *key, void *value)
{
    int i;

    // check if the key already exists and update it
    for (i = 0; i < db->count; i++)
    {
        if (strcmp(db->hashes[i].key, key) == 0)
        {
            if (db->hashes[i].type == DB_TYPE_CHAR)
            {
                free(db->hashes[i].value);
            }

            db->hashes[i].value = value;

            return 0;
        }
    }

    db->hashes[db->count].key_len = strlen(key) + 1;
    db->hashes[db->count].key = (char *)malloc(sizeof(char) * db->hashes[db->count].key_len);

    memset(db->hashes[db->count].key, 0, sizeof(char) * db->hashes[db->count].key_len);

    strlcpy(db->hashes[db->count].key, key, sizeof(char) * db->hashes[db->count].key_len);

    db->hashes[db->count].value = value;

    db->count++;

    return 0;
}


int db_set_hash_char(struct db_table *db, char *key, char *value)
{
    int i;

    for (i = 0; i < db->count; i++)
    {
        if (strcmp(db->hashes[i].key, key) == 0)
        {
            if (db->hashes[i].type == DB_TYPE_CHAR)
            {
                //db->hashes[i].value = (char *)realloc(db->hashes[i].value, sizeof(char) * (strlen(value) + 1));
                memset(db->hashes[i].value, 0, sizeof(char) * db->hashes[i].value_len);
                free(db->hashes[i].value);
            }

            db->hashes[i].type = DB_TYPE_CHAR;
            db->hashes[i].value_len = strlen(value) + 1;

            memset(db->hashes[i].value, 0, sizeof(char) * db->hashes[i].value_len);
            strlcpy(db->hashes[i].value, value, sizeof(char) * db->hashes[i].value_len);

            return 0;
        }
    }

    db->hashes = (struct db_hash *)realloc(db->hashes, sizeof(struct db_hash) * (db->count + 1));

    // zero out reallocated memory
    memset(&db->hashes[db->count], 0, sizeof(struct db_hash));

    db->hashes[db->count].type = DB_TYPE_CHAR;
    db->hashes[db->count].value_len = strlen(value) + 1;
    db->hashes[db->count].value = (char *)malloc(sizeof(char) * db->hashes[db->count].value_len);

    memset(db->hashes[db->count].value, 0, sizeof(char) * db->hashes[db->count].value_len);
    strlcpy(db->hashes[db->count].value, value, sizeof(char) * db->hashes[db->count].value_len);

    return db_set_hash(db, key, db->hashes[db->count].value);
}

int db_hash_exists(struct db_table *db, char *key)
{
    int i;

    for (i = 0; i < db->count; i++)
    {
        if (strcmp(db->hashes[i].key, key) == 0)
        {
            return 1;
        }
    }

    return 0;
}

int db_set_hash_int(struct db_table *db, char *key, int value)
{
    // check if the key already exists and update it
    int i;

    for (i = 0; i < db->count; i++)
    {
        if (strcmp(db->hashes[i].key, key) == 0)
        {
            if (db->hashes[i].type == DB_TYPE_INT)
            {
                db->hashes[i].value = (int *)realloc(db->hashes[i].value, sizeof(int));
            }

            db->hashes[i].type = DB_TYPE_INT;
            db->hashes[i].value_len = sizeof(int);

            memcpy(db->hashes[i].value, &value, sizeof(int));

            return 0;
        }
    }

    db->hashes = (struct db_hash *)realloc(db->hashes, sizeof(struct db_hash) * (db->count + 1));

    // zero out reallocated memory
    memset(&db->hashes[db->count], 0, sizeof(struct db_hash));

    db->hashes[db->count].type = DB_TYPE_INT;
    db->hashes[db->count].value_len = sizeof(int);

    db->hashes[db->count].value = (int *)malloc(sizeof(int));

    memcpy(db->hashes[db->count].value, &value, sizeof(int));

    return db_set_hash(db, key, db->hashes[db->count].value);
}

int db_set_hash_float(struct db_table *db, char *key, float value)
{
    db->hashes = (struct db_hash *)realloc(db->hashes, sizeof(struct db_hash) * (db->count + 1));

    // zero out reallocated memory
    memset(&db->hashes[db->count], 0, sizeof(struct db_hash));

    db->hashes[db->count].type = DB_TYPE_FLOAT;
    db->hashes[db->count].value_len = sizeof(float);
    db->hashes[db->count].value = (float *)malloc(sizeof(float));

    memcpy(db->hashes[db->count].value, &value, sizeof(float));

    return db_set_hash(db, key, db->hashes[db->count].value);
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

void *db_get_hash(struct db_table *db, char *key)
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

int db_get_hash_type(struct db_table *db, char *key)
{
    int i;

    for (i = 0; i < db->count; i++)
    {
        if (strcmp(db->hashes[i].key, key) == 0)
        {
            return db->hashes[i].type;
        }
    }

    return -1;
}

char *db_get_hash_char(struct db_table *db, char *key)
{
    return (char *)db_get_hash(db, key);
}

int db_get_hash_int(struct db_table *db, char *key)
{
    int value;

    if (db_get_hash_type(db, key) != DB_TYPE_INT)
    {
        return -1;
    }

    memcpy(&value, db_get_hash(db, key), sizeof(int));

    return value;
}

float db_get_hash_float(struct db_table *db, char *key)
{
    float value;

    memcpy(&value, db_get_hash(db, key), sizeof(float));

    return value;
}

