#include <stdio.h>
#include <stdlib.h>
#include "pg_query.h"

int main() {
    printf("Testing basic PostgreSQL SELECT...\n");
    const char *sql = "SELECT * FROM users WHERE id = 1";
    printf("SQL: %s\n", sql);

    PgQueryProtobufParseResult result = pg_query_parse_protobuf(sql);

    if (result.error) {
        printf("✗ Error: %s\n", result.error->message);
        printf("  at %s:%d\n", result.error->filename, result.error->lineno);
        return 1;
    } else {
        printf("✓ Success! Parsed %zu bytes\n", result.parse_tree.len);
    }

    pg_query_free_protobuf_parse_result(result);
    printf("Test completed successfully!\n");
    return 0;
}
