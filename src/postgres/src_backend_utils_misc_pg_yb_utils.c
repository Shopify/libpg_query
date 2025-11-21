/*--------------------------------------------------------------------
 * Symbols referenced in this file:
 * - IsYugaByteEnabled
 * - yb_debug_report_error_stacktrace
 * - YBShouldLogStackTraceOnError
 * - GetStatusMsgAndArgumentsByCode
 * - FetchUniqueConstraintName
 * - YBPgErrorLevelToString
 * - YbIsConnectedToTemplateDb
 * - yb_connected_to_template_db
 *--------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------
 *
 * pg_yb_utils.c
 *	  Utilities for YugaByte/PostgreSQL integration that have to be defined on
 *	  the PostgreSQL side.
 *
 * Copyright (c) YugaByte, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * IDENTIFICATION
 *	  src/backend/utils/misc/pg_yb_utils.c
 *
 *-------------------------------------------------------------------------
 */

#include "pg_yb_utils.h"

#include <arpa/inet.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "access/heaptoast.h"
#include "c.h"
#include "postgres.h"
#include "miscadmin.h"
#include "access/htup.h"
#include "access/htup_details.h"
#include "access/relation.h"
#include "access/sysattr.h"
#include "access/table.h"
#include "access/tupdesc.h"
#include "access/xact.h"
#include "executor/ybcExpr.h"
#include "catalog/catalog.h"
#include "catalog/index.h"
#include "catalog/indexing.h"
#include "catalog/pg_am.h"
#include "catalog/pg_amop.h"
#include "catalog/pg_amproc.h"
#include "catalog/pg_attrdef.h"
#include "catalog/pg_auth_members.h"
#include "catalog/pg_authid.h"
#include "catalog/pg_cast.h"
#include "catalog/pg_collation.h"
#include "catalog/pg_constraint.h"
#include "catalog/pg_database.h"
#include "catalog/pg_db_role_setting.h"
#include "catalog/pg_inherits.h"
#include "catalog/pg_namespace.h"
#include "catalog/pg_opclass.h"
#include "catalog/pg_operator.h"
#include "catalog/pg_partitioned_table.h"
#include "catalog/pg_policy.h"
#include "catalog/pg_proc.h"
#include "catalog/pg_range_d.h"
#include "catalog/pg_rewrite.h"
#include "catalog/pg_statistic_d.h"
#include "catalog/pg_tablespace.h"
#include "catalog/pg_trigger.h"
#include "catalog/pg_type.h"
#include "catalog/pg_yb_catalog_version.h"
#include "catalog/pg_yb_profile.h"
#include "catalog/pg_yb_role_profile.h"
#include "catalog/yb_catalog_version.h"
#include "catalog/yb_type.h"
#include "commands/dbcommands.h"
#include "commands/defrem.h"
#include "commands/variable.h"
#include "commands/ybccmds.h"
#include "common/ip.h"
#include "common/pg_yb_common.h"
#include "lib/stringinfo.h"
#include "libpq/hba.h"
#include "libpq/libpq.h"
#include "libpq/libpq-be.h"
#include "nodes/makefuncs.h"
#include "optimizer/cost.h"
#include "parser/parse_utilcmd.h"
#include "tcop/utility.h"
#include "utils/builtins.h"
#include "utils/datum.h"
#include "utils/fmgroids.h"
#include "utils/lsyscache.h"
#include "utils/pg_locale.h"
#include "utils/rel.h"
#include "utils/snapshot.h"
#include "utils/spccache.h"
#include "utils/syscache.h"
#include "utils/uuid.h"
#include "utils/jsonb.h"
#include "fmgr.h"
#include "funcapi.h"
#include "mb/pg_wchar.h"

// MODIFIED for libpg_query: Use local YB headers instead of external repo
#include "yb/yql/pggate/util/ybc_util.h"
#include "yb/yql/pggate/ybc_pggate.h"
#include "pgstat.h"
#include "postmaster/interrupt.h"
#include "nodes/readfuncs.h"
#include "yb_ash.h"
#include "yb_query_diagnostics.h"

#ifdef HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#endif


















/** These values are lazily initialized based on corresponding environment variables. */



/* Forward declarations */
static void YBCInstallTxnDdlHook();
static bool YBCanEnableDBCatalogVersionMode();






bool
IsYugaByteEnabled()
{
	/* We do not support Init/Bootstrap processing modes yet. */
	return YBCPgIsYugaByteEnabled();
}































/*
 * Get primary key columns as bitmap of a table,
 * subtracting minattr from attributes.
 */




















#ifdef NDEBUG
#else
#endif





/*
 * Return true if we are in per-database catalog version mode. In order to
 * use per-database catalog version mode, two conditions must be met:
 *   * --FLAGS_ysql_enable_db_catalog_version_mode=true
 *   * the table pg_yb_catalog_version has one row per database.
 * This function takes care of the YSQL upgrade from global catalog version
 * mode to per-database catalog version mode when the default value of
 * --FLAGS_ysql_enable_db_catalog_version_mode is changed to true. In this
 * upgrade procedure --FLAGS_ysql_enable_db_catalog_version_mode is set to
 * true before the table pg_yb_catalog_version is updated to have one row per
 * database.
 * This function does not consider going from per-database catalog version
 * mode back to global catalog version mode.
 */




/*
 * Used to determine whether we should preload certain catalog tables.
 */




const char * FetchUniqueConstraintName(Oid conoid) { return NULL; }


/*
 * GetStatusMsgAndArgumentsByCode - get error message arguments out of the
 * status codes
 *
 * We already have cases when DocDB returns status with SQL code and
 * relation Oid, but without error message, assuming the message is generated
 * on Postgres side, with relation name retrieved by Oid. We have to keep
 * the functionality for backward compatibility.
 *
 * Same approach can be used for similar cases, when status is originated from
 * DocDB: by known SQL code the function may set or amend the error message and
 * message arguments.
 */
void
GetStatusMsgAndArgumentsByCode(const uint32_t pg_err_code,
							   uint16_t txn_err_code, YBCStatus s,
							   const char **msg_buf, size_t *msg_nargs,
							   const char ***msg_args, const char **detail_buf,
							   size_t *detail_nargs, const char ***detail_args)
{
	const char	*status_msg = YBCMessageAsCString(s);
	size_t		 status_nargs;
	const char **status_args = YBCStatusArguments(s, &status_nargs);


	// Initialize message and detail buffers with default values
	*msg_buf = status_msg;
	*msg_nargs = status_nargs;
	*msg_args = status_args;
	*detail_buf = NULL;
	*detail_nargs = 0;
	*detail_args = NULL;
	elog(DEBUG2,
			 "status_msg=%s txn_err_code=%d pg_err_code=%d", status_msg, txn_err_code, pg_err_code);

	switch(pg_err_code)
	{
		case ERRCODE_T_R_SERIALIZATION_FAILURE:
			if(YBCIsTxnConflictError(txn_err_code))
			{
				*msg_buf = "could not serialize access due to concurrent update";
				*msg_nargs = 0;
				*msg_args = NULL;

				*detail_buf = status_msg;
				*detail_nargs = status_nargs;
				*detail_args = status_args;
			}
			else if(YBCIsTxnAbortedError(txn_err_code))
			{
				*msg_buf = "current transaction is expired or aborted";
				*msg_nargs = 0;
				*msg_args = NULL;

				*detail_buf = status_msg;
				*detail_nargs = status_nargs;
				*detail_args = status_args;
			}
			break;
		case ERRCODE_UNIQUE_VIOLATION:
			*msg_buf = "duplicate key value violates unique constraint \"%s\"";
			*msg_nargs = 1;
			*msg_args = (const char **) palloc(sizeof(const char *));
			(*msg_args)[0] = FetchUniqueConstraintName(YBCStatusRelationOid(s));
			break;
		case ERRCODE_T_R_DEADLOCK_DETECTED:
			if (YBCIsTxnDeadlockError(txn_err_code)) {
				*msg_buf = "deadlock detected";
				*msg_nargs = 0;
				*msg_args = NULL;

				*detail_buf = status_msg;
				*detail_nargs = status_nargs;
				*detail_args = status_args;
			}
			break;
		default:
			break;
	}
}









/*
 * Ensure we've defined the correct postgres Oid values. This function only
 * contains compile-time assertions. It would have been made 'static' but it is
 * not called anywhere and making it 'static' caused compiler warning which
 * broke the build.
 */


/*
 * Holds the RPC/Storage execution stats for the session. A handle to this
 * struct is passed down to pggate which record updates to the stats as they
 * happen. This model helps avoid making copies of the stats and passing it
 * back/forth.
 */
typedef struct YbSessionStats
{
	YBCPgExecStatsState current_state;
	YBCPgExecStats		latest_snapshot;
} YbSessionStats;























static __thread bool yb_connected_to_template_db = false;




bool
YbIsConnectedToTemplateDb()
{
	return yb_connected_to_template_db;
}











bool
YBShouldLogStackTraceOnError()
{
	static int cached_value = -1;
	if (cached_value != -1)
	{
		return cached_value;
	}

	cached_value = YBCIsEnvVarTrue("YB_PG_STACK_TRACE_ON_ERROR");
	return cached_value;
}

const char*
YBPgErrorLevelToString(int elevel) {
	switch (elevel)
	{
		case DEBUG5: return "DEBUG5";
		case DEBUG4: return "DEBUG4";
		case DEBUG3: return "DEBUG3";
		case DEBUG2: return "DEBUG2";
		case DEBUG1: return "DEBUG1";
		case LOG: return "LOG";
		case LOG_SERVER_ONLY: return "LOG_SERVER_ONLY";
		case INFO: return "INFO";
		case WARNING: return "WARNING";
		case ERROR: return "ERROR";
		case FATAL: return "FATAL";
		case PANIC: return "PANIC";
		default: return "UNKNOWN";
	}
}



















/*
 * With PG upstream commit 86dc90056dfdbd9d1b891718d2e5614e3e432f35, UPDATE's
 * child node only returns the columns being updated along with junk columns. PG
 * then fetches the pre-existing old tuple to reconstruct the new tuple. This is
 * be an expensive operation in YB. To workaround this problem, YB stores the
 * old tuple as "wholerow" junk column when required. This function
 * returns true when this should be done.
 */


//------------------------------------------------------------------------------
// YB GUC variables.














	/* 5 min */










//------------------------------------------------------------------------------
// YB Debug utils.

__thread bool yb_debug_report_error_stacktrace = false;


























/*
 * These two GUC variables are used together to control whether DDL atomicity
 * is enabled. See comments for the gflag --ysql_yb_enable_ddl_atomicity_infra
 * in common_flags.cc.
 */






















/*---------------------------------------------------------------------------*/
/* Transactional DDL support                                                 */
/*---------------------------------------------------------------------------*/


typedef struct CatalogModificationAspects
{
	uint64_t applied;
	uint64_t pending;

} CatalogModificationAspects;

typedef struct DdlTransactionState
{
	int nesting_level;
	MemoryContext mem_context;
	CatalogModificationAspects catalog_modification_aspects;
	bool is_global_ddl;
	NodeTag original_node_tag;
	const char *original_ddl_command_tag;
} DdlTransactionState;







/*
 * Release all space allocated in the yb_memctx of a context and all of
 * its descendants, but don't delete the yb_memctx themselves.
 */




















#ifdef YB_TODO /* utils/syscache.h has YbInitPinnedCacheIfNeeded removed. */
#endif

;





























/*
 * For backward compatibility, this function dynamically adapts to the number
 * of output columns defined in pg_proc.
 */
















/*
 * For backward compatibility, this function dynamically adapts to the number
 * of output columns defined in pg_proc.
 */


/*
 * This function is adapted from code of PQescapeLiteral() in fe-exec.c.
 * If use_quote_strategy_token is false, the string value will be converted
 * to an SQL string literal and appended to the given StringInfo.
 * If use_quote_strategy_token is true, the string value will be enclosed in
 * double quotes, backslashes will be escaped and the value will be appended
 * to the given StringInfo.
 */


/*
 * This function is adapted from code in pg_dump.c.
 * It converts an internal raw datum value to a output string based on
 * column type, and append the string to the StringInfo input parameter.
 * Datum of all types can be generated in a quoted string format
 * (e.g., '100' for integer 100), and rely on PG's type cast to function
 * correctly. Here, we specifically handle some cases to ignore quotes to
 * make the generated string look better.
 */


/*
 * This function gets range relations' split point values as PG datums.
 * It also stores key columns' data types in input parameters: pkeys_atttypid.
 */


/*
 * This function constructs SPLIT AT VALUES clause for range-partitioned tables
 * with more than one tablet.
 */


/*
 * This function is used to retrieve a range partitioned table's split points
 * as a list of list of Exprs.
 */














/*
 * This PG function takes one optional bool input argument (legacy).
 * If the input argument is not specified or its value is false, this function
 * returns whether the current database is a colocated database.
 * If the value of the input argument is true, this function returns whether the
 * current database is a legacy colocated database.
 */


/*
 * This function serves mostly as a helper for YSQL migration to introduce
 * pg_yb_catalog_version table without breaking version continuity.
 */












#define YB_TABLET_INFO_COLS 8
#undef YB_TABLET_INFO_COLS

 

#define YB_SERVERS_METRICS_COLS 4
#undef YB_SERVERS_METRICS_COLS

/*---------------------------------------------------------------------------*/
/* Deterministic DETAIL order                                                */
/*---------------------------------------------------------------------------*/



typedef struct {
	char **lines;
	int length;
} DetailSorter;









/*
 * This function is adapted from code in varlena.c.
 */
#ifdef USE_ICU
#endif
#ifdef USE_ICU
#endif
#ifdef USE_ICU
#endif
#ifdef HAVE_LOCALE_T
#endif
#ifdef USE_ICU
#endif















#ifdef HAVE_SYS_PRCTL_H
#endif







































































/*
 * ```ysql_conn_mgr_sticky_object_count``` is the count of the database objects
 * that requires the sticky connection
 * These objects are
 * 1. WITH HOLD CURSORS
 * 2. TEMP TABLE
 */


/*
 * `yb_ysql_conn_mgr_sticky_guc` is used to denote stickiness of a connection
 * due to the setting of GUC variables that cannot be directly supported
 * by Connection Manager.
 */


/*
 * ```YbIsConnectionMadeStickyUsingGUC()``` returns whether or not the a
 * connection is made sticky using via specific GUC variables.
 */


/*
 * ```YbIsStickyConnection(int *change)``` updates the number of objects that requires a sticky
 * connection and returns whether or not the client connection
 * requires stickiness. i.e. if there is any `WITH HOLD CURSOR` or `TEMP TABLE`
 * at the end of the transaction.
 *
 * Also check if any GUC variable is set that requires a sticky connection.
 */




/*
 * This function is almost equivalent to the `read_whole_file` function of
 * src/postgres/src/backend/commands/extension.c. It differs only in its error
 * handling. The original read_whole_file function logs errors elevel ERROR
 * while this function accepts the elevel as the argument for better control
 * over error handling.
 */


/*
 * Needed to support the guc variable yb_use_tserver_key_auth, which is
 * processed before authentication i.e. before setting this variable.
 */




/*
 * Copies the primary key of a relation to a create stmt intended to clone that
 * relation.
 */


/*
 * In YB, a "relfilenode" corresponds to a DocDB table.
 * This function creates a new DocDB table for the given index,
 * with UUID corresponding to the given relfileNodeId. It is used when a
 * user index is re-indexed.
 */








/*
 * In YB, a "relfilenode" corresponds to a DocDB table.
 * This function creates a new DocDB table for the given table,
 * with UUID corresponding to the given relfileNodeId.
 */












// TODO(#22370): the method will be used to make Const Based Optimizer to be aware of
// fast backward scan capability.




/* Used in YB to check if an attribute is a key column. */


/* Retrieve the sort ordering of the first key element of an index. */

