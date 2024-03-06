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

    /*
    db = (struct db_table *)malloc(sizeof(struct db_table));
    db->count = 0;
    db->hashes = NULL;

    db_add_hash(db, "lua.scripts", "hello.lua,test.lua,youtube.lua");
    db_add_hash(db, "lua.scriptcount", "2");

    if (write_db(db, FNAME) == -1)
    {
        printf("Error writing db\n");
    }

    free(db);
    */

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
        db_add_hash(db, "lua.scripts", "hello.lua,test.lua,youtube.lua");
        db_add_hash(db, "lua.scriptcount", "2");
        write_db(db, FNAME);

        return 0;
    }

    for (i = 0; i < db->count; i++)
    {
        printf("Key: %s, Value: %s\n", db->hashes[i].key, db->hashes[i].value);
    }

    free(db);

    return 0;
}
