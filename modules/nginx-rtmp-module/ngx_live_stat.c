#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>
#include "ngx_rtmp.h"
#include "ngx_live.h"
#include "ngx_rtmp_version.h"
#include "ngx_rtmp_live_module.h"
#include "ngx_rtmp_codec_module.h"
#include "json/cJSON.h"

#define NGX_LIVE_STAT_BW            0x01
#define NGX_LIVE_STAT_BYTES         0x02
#define NGX_LIVE_STAT_BW_BYTES      0x03

static void
ngx_live_stat_bw(cJSON *const obj,
    ngx_rtmp_bandwidth_t *bw, ngx_uint_t flags)
{
    ngx_rtmp_update_bandwidth(bw, 0);

    if (flags & NGX_LIVE_STAT_BW) {
        cJSON_AddNumberToObject(obj, "bw", bw->bandwidth * 8);
    }

    if (flags & NGX_LIVE_STAT_BYTES) {
        cJSON_AddNumberToObject(obj, "bytes", bw->bytes);
    }
}

static char cstring_buffer[1024];
inline static const char* CONVERT_CSTRING(ngx_str_t *ngx_str)
{
    if (ngx_str->len == 0) {
        cstring_buffer[0] = '\0';
        return cstring_buffer;
    }

    *ngx_snprintf((u_char *) cstring_buffer, 1023, "%V", ngx_str) = '\0';

    return cstring_buffer;
}

inline static const char* CONVERT_DOUBLE_CSTRING(double in)
{
    *ngx_snprintf((u_char *) cstring_buffer, 1023, "%.1f", in) = '\0';

    return cstring_buffer;
}

static void
ngx_live_stat_client(cJSON *obj, ngx_rtmp_session_t *s)
{
#ifdef NGX_RTMP_POOL_DEBUG
    ngx_rtmp_stat_dump_pool(r, lll, s->pool);
#endif

    cJSON_AddNumberToObject(obj, "id", s->number);
    cJSON_AddStringToObject(obj, "address", CONVERT_CSTRING(s->addr_text));
    cJSON_AddStringToObject(obj, "remote_address", CONVERT_CSTRING(&s->remote_addr_text));
    cJSON_AddNumberToObject(obj, "time", (ngx_int_t) (ngx_current_msec - s->epoch));

    if (s->flashver.len) {
        cJSON_AddStringToObject(obj, "flashver", CONVERT_CSTRING(&s->flashver));
    }

    if (s->page_url.len) {
        cJSON_AddStringToObject(obj, "pageurl", CONVERT_CSTRING(&s->page_url));
    }

    if (s->swf_url.len) {
        cJSON_AddStringToObject(obj, "swfurl", CONVERT_CSTRING(&s->swf_url));
    }
}


static char *
ngx_live_stat_get_aac_profile(ngx_uint_t p, ngx_uint_t sbr, ngx_uint_t ps) {
    switch (p) {
        case 1:
            return "Main";
        case 2:
            if (ps) {
                return "HEv2";
            }
            if (sbr) {
                return "HE";
            }
            return "LC";
        case 3:
            return "SSR";
        case 4:
            return "LTP";
        case 5:
            return "SBR";
        default:
            return "";
    }
}


static char *
ngx_live_stat_get_avc_profile(ngx_uint_t p) {
    switch (p) {
        case 66:
            return "Baseline";
        case 77:
            return "Main";
        case 100:
            return "High";
        default:
            return "";
    }
}

static void
ngx_live_stream_stat(cJSON *obj, ngx_live_server_t *srv)
{
    ngx_live_stream_t              *stream;
    ngx_rtmp_codec_ctx_t           *codec;
    ngx_rtmp_live_ctx_t            *ctx;
    ngx_rtmp_session_t             *s;
    size_t                          n;
    ngx_uint_t                      nclients, total_nclients;
    ngx_uint_t                      nrtmpclients, total_nrtmpclients;
    ngx_uint_t                      nflvclients, total_nflvclients;
    ngx_uint_t                      ntsclients, total_ntsclients;
    ngx_uint_t                      nhlsclients, total_nhlsclients;
    ngx_live_conf_t                *lcf;
    u_char                         *cname;
    cJSON                          *stream_array, *stream_item;
    cJSON                          *client_array, *client_item;
    cJSON                          *meta, *video, *audio;
    cJSON                          *stream_in, *stream_out;
    cJSON                          *stream_video_in, *stream_audio_in;

    lcf = (ngx_live_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                           ngx_live_module);

    total_nrtmpclients = 0;
    total_nflvclients  = 0;
    total_nhlsclients  = 0;
    total_ntsclients   = 0;
    total_nclients     = 0;

    stream_array = cJSON_AddArrayToObject(obj, "stream_array");

    for (n = 0; n < lcf->stream_buckets; ++n) {
        for (stream = srv->streams[n]; stream; stream = stream->next) {
            stream_item = cJSON_CreateObject();
            cJSON_AddStringToObject(stream_item, "name", (char*) stream->name);
            cJSON_AddNumberToObject(stream_item, "time",
                ngx_current_msec - stream->epoch);

            stream_in = cJSON_AddObjectToObject(stream_item, "in");
            ngx_live_stat_bw(stream_in,
                &stream->bw_in, NGX_LIVE_STAT_BW_BYTES);
            stream_video_in = cJSON_AddObjectToObject(stream_item, "video_in");
            stream_audio_in = cJSON_AddObjectToObject(stream_item, "audio_in");

            ngx_live_stat_bw(stream_audio_in,
                &stream->bw_in_audio, NGX_LIVE_STAT_BW_BYTES);
            ngx_live_stat_bw(stream_video_in,
                &stream->bw_in_video, NGX_LIVE_STAT_BW_BYTES);

            stream_out = cJSON_AddObjectToObject(stream_item, "out");
            ngx_live_stat_bw(stream_out,
                &stream->bw_out, NGX_LIVE_STAT_BW_BYTES);

            codec = NULL;
            client_array = cJSON_AddArrayToObject(stream_item, "client_array");

            nrtmpclients = 0;
            nflvclients  = 0;
            nhlsclients  = 0;
            ntsclients   = 0;
            nclients     = 0;

            for (ctx = stream->ctx; ctx; ctx = ctx->next) {
                s = ctx->session;
                client_item = cJSON_CreateObject();
                cJSON_AddItemToArray(client_array, client_item);

                ngx_live_stat_client(client_item, s);

                cJSON_AddNumberToObject(client_item, "dropped", ctx->ndropped);
                cJSON_AddNumberToObject(client_item, "avsync",
                    ctx->cs[1].timestamp - ctx->cs[0].timestamp);
                cJSON_AddNumberToObject(client_item, "timestamp",
                    s->current_time);
                cJSON_AddBoolToObject(client_item, "active",
                    (cJSON_bool) ctx->active);
                cJSON_AddBoolToObject(client_item, "publishing",
                    (cJSON_bool) s->publishing);
                cJSON_AddBoolToObject(client_item, "relay",
                    (cJSON_bool) s->relay);
                cJSON_AddBoolToObject(client_item, "interprocess",
                    (cJSON_bool) s->interprocess);
                cJSON_AddStringToObject(client_item, "protocol",
                    ngx_live_protocol_string[s->live_type]);

                if (ctx->publishing) {
                    codec = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);
                    continue;
                }

                if (s->relay || s->interprocess) {
                    continue;
                }

                switch (s->live_type) {
                    case NGX_RTMP_LIVE:
                    nrtmpclients++;
                    break;

                case NGX_HTTP_FLV_LIVE:
                    nflvclients++;
                    break;

                case NGX_HLS_LIVE:
                    nhlsclients++;
                    break;

                case NGX_MPEGTS_LIVE:
                    ntsclients++;
                    break;
                }
            }

            if (codec) {
                meta = cJSON_AddObjectToObject(stream_item, "meta");
                video = cJSON_AddObjectToObject(meta, "video");

                cJSON_AddNumberToObject(video, "width", codec->width);
                cJSON_AddNumberToObject(video, "height", codec->height);
                cJSON_AddStringToObject(video, "frame_rate",
                    CONVERT_DOUBLE_CSTRING(codec->frame_rate));

                cname = ngx_rtmp_get_video_codec_name(codec->video_codec_id);
                if (*cname) {
                    cJSON_AddStringToObject(video, "codec", (char *) cname);
                }
                if (codec->avc_profile) {
                    cJSON_AddStringToObject(video, "profile",
                        ngx_live_stat_get_avc_profile(codec->avc_profile));
                }
                if (codec->avc_level) {
                    cJSON_AddStringToObject(video, "compat",
                        CONVERT_DOUBLE_CSTRING(codec->avc_level / 10.));
                }
        
                audio = cJSON_AddObjectToObject(meta, "audio");
                cname = ngx_rtmp_get_audio_codec_name(codec->audio_codec_id);
                if (*cname) {
                    cJSON_AddStringToObject(audio, "codec", (char *) cname);
                }
                if (codec->aac_profile) {
                    cJSON_AddStringToObject(audio, "profile",
                        ngx_live_stat_get_aac_profile(codec->aac_profile,
                            codec->aac_sbr, codec->aac_ps));
                }

                if (codec->aac_chan_conf) {
                    cJSON_AddNumberToObject(audio, "channels", codec->aac_chan_conf);
                } else if (codec->audio_channels) {
                    cJSON_AddNumberToObject(audio, "channels", codec->audio_channels);
                }

                if (codec->sample_rate) {
                    cJSON_AddNumberToObject(audio, "sample_rate", codec->sample_rate);
                }
            }

            nclients += nrtmpclients;
            nclients += nflvclients;
            nclients += nhlsclients;
            nclients += ntsclients;

            total_nrtmpclients += nrtmpclients;
            total_nflvclients += nflvclients;
            total_nhlsclients += nhlsclients;
            total_ntsclients += ntsclients;
            total_nclients += nclients;

            cJSON_AddNumberToObject(stream_item, "total_clients", nclients);
            cJSON_AddNumberToObject(stream_item, "rtmp_clients", nrtmpclients);
            cJSON_AddNumberToObject(stream_item, "flv_clients", nflvclients);
            cJSON_AddNumberToObject(stream_item, "hls_clients", nhlsclients);
            cJSON_AddNumberToObject(stream_item, "ts_clients", ntsclients);
            cJSON_AddBoolToObject(stream_item, "active", stream->active);
            cJSON_AddItemToArray(stream_array, stream_item);
        }
    }

    cJSON_AddNumberToObject(obj, "total_clients", total_nclients);
    cJSON_AddNumberToObject(obj, "rtmp_clients", total_nrtmpclients);
    cJSON_AddNumberToObject(obj, "flv_clients", total_nflvclients);
    cJSON_AddNumberToObject(obj, "hls_clients", total_nhlsclients);
    cJSON_AddNumberToObject(obj, "ts_clients", total_ntsclients);
}


ngx_int_t
ngx_live_stat(cJSON *obj)
{
    ngx_live_conf_t     *lcf;
    ngx_live_server_t   *psrv;
    size_t               n;
    cJSON               *server_array;
    cJSON               *all_in;
    cJSON               *all_out;
    cJSON               *item;

    cJSON_AddStringToObject(obj, "server_version", NGINX_VERSION);
    cJSON_AddStringToObject(obj, "compiler", NGX_COMPILER);
    cJSON_AddStringToObject(obj, "built", __DATE__);
    cJSON_AddNumberToObject(obj, "pid", (ngx_uint_t) ngx_getpid());
    cJSON_AddNumberToObject(obj, "timestamp", ngx_cached_time->sec);
    cJSON_AddNumberToObject(obj, "naccepted", ngx_rtmp_naccepted);

    all_in = cJSON_AddObjectToObject(obj, "in");
    all_out = cJSON_AddObjectToObject(obj, "out");
    ngx_live_stat_bw(all_in, &ngx_rtmp_bw_in, NGX_LIVE_STAT_BW_BYTES);
    ngx_live_stat_bw(all_out, &ngx_rtmp_bw_out, NGX_LIVE_STAT_BW_BYTES);

    lcf = (ngx_live_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                           ngx_live_module);

    server_array = cJSON_AddArrayToObject(obj, "server_array");
    for (n = 0; n < lcf->server_buckets; ++n) {
        for (psrv = lcf->servers[n]; psrv; psrv = psrv->next) {
            item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "serverid", (char *) psrv->serverid);

            ngx_live_stream_stat(item, psrv);
            cJSON_AddItemToArray(server_array, item);
        }
    }

    return NGX_OK;
}