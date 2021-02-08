#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <discodb.h>
#include <fcntl.h>
#include <ddb_internal.h>

#define MAX_KV_SIZE 1<<20

#define FEAT(x) (long long unsigned int)feat[x]

static const char yes[] = "true";
static const char no[] = "false";
const char *boolstr(int boolean) { return boolean ? yes: no; }
static void print_info(struct ddb *db)
{
    ddb_features_t feat;
    ddb_features(db, feat);
    printf("Total size:              %llu bytes\n", FEAT(DDB_TOTAL_SIZE));
    printf("Items size:              %llu bytes\n", FEAT(DDB_ITEMS_SIZE));
    printf("Values size:             %llu bytes\n", FEAT(DDB_VALUES_SIZE));
    printf("Number of keys:          %llu\n", FEAT(DDB_NUM_KEYS));
    printf("Number of items:         %llu\n", FEAT(DDB_NUM_VALUES));
    printf("Number of unique values: %llu\n", FEAT(DDB_NUM_UNIQUE_VALUES));
    printf("Compressed?              %s\n", boolstr(feat[DDB_IS_COMPRESSED]));
    printf("Hashed?                  %s\n", boolstr(feat[DDB_IS_HASHED]));
    printf("Multiset?                %s\n", boolstr(feat[DDB_IS_MULTISET]));
}
static void print_cursor(struct ddb *db, struct ddb_cursor *cur, struct ddb_entry *es)
{
    if (!cur){
        const char *err;
        ddb_error(db, &err);
        fprintf(stderr, "Query failed: %s\n", err);
        exit(1);
    }

    if (ddb_notfound(cur)){
        fprintf(stderr, "Not found\n");
        exit(1);
    }
    int errno, i = 0;
    const struct ddb_entry *e;
    while ((e = ddb_next(cur, &errno))){
        printf("Key: %.*s -- Value:%.*s\n", es->length, es->data, e->length, e->data);
        ++i;
    }
    if (errno){
        fprintf(stderr, "Cursor failed: out of memory\n");
        exit(1);
    }
    ddb_free_cursor(cur);
}
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
    FILE *in;
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

    fprintf(stderr, "Packing the index..\n");

    if (!(data = ddb_cons_finalize(db, &size, flags))){
        fprintf(stderr, "Packing the index failed\n");
        exit(1);
    }

    struct ddb *sddb = ddb_new();
    ddb_loads(sddb, data, size);
    struct ddb_entry ee;
    ee.data = "key";
    ee.length = strlen(ee.data);
    print_cursor(sddb, ddb_getitem(sddb, &ee), &ee);
    ddb_free(sddb);
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
    fprintf(stderr, "Ok! Index written to %s\n", ddbfile);

    return 0;
}