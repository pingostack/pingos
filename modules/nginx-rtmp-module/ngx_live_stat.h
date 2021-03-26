#ifndef _NGX_LIVE_STAT_H_INCLUDE_
#define _NGX_LIVE_STAT_H_INCLUDE_

#include "json/cJSON.h"

#include <ngx_config.h>
#include <ngx_core.h>

ngx_int_t ngx_live_stat(cJSON *obj);

#endif
