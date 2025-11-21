/*
 * Stub implementations for YugabyteDB-specific functions
 * These stubs provide minimal implementations to allow the parser to work
 * without the full YugabyteDB runtime.
 *
 * NOTE: With IsMultiThreadedMode() forced to return false in palloc.h,
 * most of these stubs should not be called during normal parser operation.
 * They are kept here for compatibility in case any code paths still reference them.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// Error handling stubs - should not be called in parser-only mode
void yb_reset_error_status(void) {
    // No-op for parser-only use
}

void yb_set_exception_stack(void* stack) {
    // No-op for parser-only use
}

bool yb_run_with_explain_analyze(void) {
    return false;
}

// YugabyteDB C API stubs - should not be called in parser-only mode
void YBCFreeStatus(void* status) {
    // No-op for parser-only use
}

bool YBCGetGFlags(const char* flag_name, char** value) {
    // Return false to indicate the flag is not set
    return false;
}

void* YBCPgGetThreadLocalErrStatus(void) {
    return NULL;
}

void* YBCPgGetThreadLocalJumpBuffer(void) {
    return NULL;
}

void* YBCPgSetThreadLocalCurrentMemoryContext(void* context) {
    return NULL;
}

void YBCPgResetMemctx(void* context) {
    // No-op
}

void YBCPgDestroyMemctx(void* context) {
    // No-op
}

bool YBCPgIsYugaByteEnabled(void) {
    return false;
}

void YBCLogImpl(int level, const char* file, int line, bool suppress_logging, const char* format, ...) {
    // No-op for parser-only use
}

const char* YBCGetStackTrace(void) {
    return "";
}

bool YBCIsTxnAbortedError(void* status) {
    return false;
}

bool YBCIsTxnConflictError(void* status) {
    return false;
}

bool YBCIsTxnDeadlockError(void* status) {
    return false;
}

const char* YBCMessageAsCString(void* status) {
    return "";
}

void* YBCStatusArguments(void* status) {
    return NULL;
}

const char* YBCStatusFilename(void* status) {
    return "";
}

const char* YBCStatusFuncname(void* status) {
    return "";
}

bool YBCStatusIsFatalError(void* status) {
    return false;
}

int YBCStatusLineNumber(void* status) {
    return 0;
}

int YBCStatusPgsqlError(void* status) {
    return 0;
}

uint32_t YBCStatusRelationOid(void* status) {
    return 0;
}

int YBCStatusTransactionError(void* status) {
    return 0;
}

bool YBCTryMemConsume(int64_t bytes) {
    return true;
}

void YBCTryMemRelease(int64_t bytes) {
    // No-op
}
