
#ifndef _UQMID_LOGGING_H__
#define _UQMID_LOGGING_H__

#include "osmocom/logging.h"

/* TOOD: implement logging */

/* TODO: improve modem_log() */
#define modem_log(modem, level, fmt, args...) fprintf(stderr, "%s:%d " fmt "\n", modem->name, level, ## args)
#define service_log(service, level, fmt, args...) \
	fprintf(stderr, "%s:svc/%d:%d " fmt "\n", service->qmi->modem->name, service->service, level, ## args);


#define LOG_ERROR(fmt, args...) fprintf(stderr, fmt, ## args)
#define LOG_INFO(fmt, args...) LOG_ERROR(fmt, ## args)
#define LOG_DEBUG(fmt, args...) LOG_ERROR(fmt, ## args)

#endif /* _UQMID_LOGGING_H__ */
