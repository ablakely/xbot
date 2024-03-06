#include "db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define FNAME "xbot.db"

int main()
{
    struct db_table *db;
    int i;

    if (access(FNAME, F_OK) == -1)
    {
        printf("Creating db\n");

        db = (struct db_table *)malloc(sizeof(struct db_table));
        db->count = 0;
        db->hashes = NULL;
    }
    else
    {
        db = read_db(FNAME);
    }

    // write some data if db is empty
    if (db->count == 0)
    {
        db_set_hash_char(db, "lua.scripts", "hello.lua,test.lua,youtube.lua");
        db_set_hash_int(db, "lua.scriptcount", 2);
        db_set_hash_float(db, "lua.version", 5.1);
        write_db(db, FNAME);

        return 0;
    }

    for (i = 0; i < db->count; i++)
    {
        if (db->hashes[i].type == DB_TYPE_INT)
        {
            printf("Key: %s, Value: %d\n", db->hashes[i].key, get_hash_int(db, db->hashes[i].key));
        }
        else if (db->hashes[i].type == DB_TYPE_FLOAT)
        {
            printf("Key: %s, Value: %f\n", db->hashes[i].key, get_hash_float(db, db->hashes[i].key));
        }
        else if (db->hashes[i].type == DB_TYPE_CHAR)
        {
            printf("Key: %s, Value: %s\n", db->hashes[i].key, get_hash_char(db, db->hashes[i].key));
        }
    }

    free(db);

    return 0;
}
