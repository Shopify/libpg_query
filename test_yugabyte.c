#include <stdio.h>
#include <stdlib.h>
#include "pg_query.h"

int main() {
    // Test 1: Basic PostgreSQL SQL
    printf("Test 1: Basic PostgreSQL SELECT\n");
    printf("=====================================\n");
    const char *basic_sql = "SELECT * FROM users WHERE id = 1";
    printf("SQL: %s\n", basic_sql);

    PgQueryProtobufParseResult result1 = pg_query_parse_protobuf(basic_sql);

    if (result1.error) {
        printf("✗ Error: %s\n", result1.error->message);
        printf("  at %s:%d\n", result1.error->filename, result1.error->lineno);
    } else {
        printf("✓ Success! Parsed %zu bytes\n", result1.parse_tree.len);
    }

    pg_query_free_protobuf_parse_result(result1);

    // Test 2: YugabyteDB-specific SQL with HASH and SPLIT INTO
    printf("\n\nTest 2: YugabyteDB CREATE INDEX with HASH and SPLIT INTO\n");
    printf("==========================================================\n");
    const char *yb_sql = "CREATE INDEX idx_test ON core.test (shop_id HASH, code) SPLIT INTO 4 TABLETS";
    printf("SQL: %s\n", yb_sql);

    PgQueryProtobufParseResult result2 = pg_query_parse_protobuf(yb_sql);

    if (result2.error) {
        printf("✗ Error: %s\n", result2.error->message);
        printf("  at %s:%d\n", result2.error->filename, result2.error->lineno);
        if (result2.error->context) {
            printf("  context: %s\n", result2.error->context);
        }
    } else {
        printf("✓ Success! Parsed %zu bytes\n", result2.parse_tree.len);
        if (result2.parse_tree.len > 0) {
            printf("  First 50 bytes of protobuf: ");
            for (int i = 0; i < 50 && i < result2.parse_tree.len; i++) {
                printf("%02x ", (unsigned char)result2.parse_tree.data[i]);
            }
            printf("\n");
        }
    }

    pg_query_free_protobuf_parse_result(result2);

    // Test 3: Simple CREATE INDEX (no YB-specific syntax)
    printf("\n\nTest 3: Simple CREATE INDEX\n");
    printf("=====================================\n");
    const char *simple_index = "CREATE INDEX idx_test ON users (email)";
    printf("SQL: %s\n", simple_index);

    PgQueryProtobufParseResult result3 = pg_query_parse_protobuf(simple_index);

    if (result3.error) {
        printf("✗ Error: %s\n", result3.error->message);
        printf("  at %s:%d\n", result3.error->filename, result3.error->lineno);
    } else {
        printf("✓ Success! Parsed %zu bytes\n", result3.parse_tree.len);
    }

    pg_query_free_protobuf_parse_result(result3);

    printf("\n\nAll tests completed!\n");
    return 0;
}
