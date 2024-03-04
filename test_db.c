#include "db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FNAME "test.db"

int main()
{
    struct db_table *db;
    int i;

    db = (struct db_table *)malloc(sizeof(struct db_table));
    db->count = 0;
    db->hashes = NULL;

    add_hash(db, "lua.scripts", "hello.lua,test.lua,youtube.lua");
    add_hash(db, "lua.scriptcount", "2");

    if (write_db(db, FNAME) == -1)
    {
        printf("Error writing db\n");
    }

    free(db);

    db = read_db(FNAME);

    for (i = 0; i < db->count; i++)
    {
        printf("Key: %s, Value: %s\n", db->hashes[i].key, db->hashes[i].value);
    }

    printf("test: lua.scripts: %s\n", get_hash(db, "lua.scripts"));

    free(db);

    return 0;
}
