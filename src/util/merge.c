
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <discodb.h>
#include <ddb_internal.h>
#define MAX_KV_SIZE 1<<20


static struct ddb *open_discodb(const char *file)
{
    struct ddb *db;
    int fd;

    if (!(db = ddb_new())){
        fprintf(stderr, "Couldn't initialize discodb: Out of memory\n");
        exit(1);
    }
    if ((fd = open(file, O_RDONLY)) == -1){
        fprintf(stderr, "Couldn't open discodb %s\n", file);
        exit(1);
    }
    if (ddb_load(db, fd)){
        const char *err;
        ddb_error(db, &err);
        fprintf(stderr, "Invalid discodb in %s: %s\n", file, err);
        exit(1);
    }
    return db;
}

int main(int argc, char **argv)
{
    if (argc < 2){
        fprintf(stderr, "%s", "Usage:\n");
        fprintf(stderr, "%s %s", argv[0], "disco_merged.ddb disco_input1.ddb [ disco_input2.ddb, ... ]\n");
        exit(1);
    }
    FILE *out;
    uint64_t size;
    char *data;
    struct ddb_cons *db = ddb_cons_new();
    uint64_t flags = 0;
    char * ddbfile = argv[1];
    uint64_t ddb_i = 2;  // arg2 starts input files
    flags |= getenv("DONT_COMPRESS") ? DDB_OPT_DISABLE_COMPRESSION: 0;
    flags |= getenv("UNIQUE_ITEMS") ? DDB_OPT_UNIQUE_ITEMS: 0;
    flags |= DDB_OPT_UNIQUE_ITEMS;

    if (!db){
        fprintf(stderr, "DB init failed\n");
        exit(1);
    }

    for (ddb_i; ddb_i < argc; ddb_i++){
        struct ddb * ddb_i_p = open_discodb(argv[ddb_i]);
        ddb_cons_merge(db, ddb_i_p, NULL);
        ddb_free(ddb_i_p);
    }


    if (!(data = ddb_cons_finalize(db, &size, flags))){
        fprintf(stderr, "Packing the index failed\n");
        exit(1);
    }
    ddb_cons_free(db);


    if (!(out = fopen(ddbfile, "w"))){
        fprintf(stderr, "Opening file %s failed\n", ddbfile);
        exit(1);
    }
    if (!fwrite(data, size, 1, out)){
        fprintf(stderr, "Writing file %s failed\n", ddbfile);
        exit(1);
    }
    fflush(out);
    fclose(out);
    free(data);

    return 0;
}
