#pragma once

/* from osmocore/logging.h */

/*! different log levels */
#define LOGL_DEBUG  1   /*!< debugging information */
#define LOGL_INFO   3   /*!< general information */
#define LOGL_NOTICE 5   /*!< abnormal/unexpected condition */
#define LOGL_ERROR  7   /*!< error condition, requires user action */
#define LOGL_FATAL  8   /*!< fatal, program aborted */

/* logging subsystems defined by the library itself */
#define DLGLOBAL    -1  /*!< global logging */

/*! Log a new message through the Osmocom logging framework
 *  \param[in] ss logging subsystem (e.g. \ref DLGLOBAL)
 *  \param[in] level logging level (e.g. \ref LOGL_NOTICE)
 *  \param[in] fmt format string
 *  \param[in] args variable argument list
 */
#define LOGP(ss, level, fmt, args...) \
    LOGPSRC(ss, level, NULL, 0, fmt, ## args)

/*! Log through the Osmocom logging framework with explicit source.
 *  If caller_file is passed as NULL, __FILE__ and __LINE__ are used
 *  instead of caller_file and caller_line (so that this macro here defines
 *  both cases in the same place, and to catch cases where callers fail to pass
 *  a non-null filename string).
 *  \param[in] ss logging subsystem (e.g. \ref DLGLOBAL)
 *  \param[in] level logging level (e.g. \ref LOGL_NOTICE)
 *  \param[in] caller_file caller's source file string (e.g. __FILE__)
 *  \param[in] caller_line caller's source line nr (e.g. __LINE__)
 *  \param[in] fmt format string
 *  \param[in] args variable argument list
 */
#define LOGPSRC(ss, level, caller_file, caller_line, fmt, args...) \
	LOGPSRCC(ss, level, caller_file, caller_line, 0, fmt, ##args)

/* TODO: implement proper logging */
#define LOGPSRCC(ss, level, caller_file, caller_line, cont, fmt, args...) \
	do { \
		if (caller_file) \
			fprintf(stderr, "%d: %s:%d: " fmt, level, (char *) caller_file, caller_line, ##args); \
		else \
			fprintf(stderr, "%d: %s:%d: " fmt, level, __FILE__, __LINE__, ##args); \
	} while(0)

#define osmo_log_backtrace(ss, level) fprintf(stderr, "%s:%d: backtrace not compiled in.", __FILE__, __LINE__);
