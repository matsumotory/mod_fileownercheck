/*
//
// mod_fileownercheck.c - owner check for opened r->filename at output filter
//
// See Copyright Notice in LEGAL
//
*/

#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "http_log.h"
#include "util_filter.h"
#include "sys/types.h"
#include "sys/stat.h"

#if (AP_SERVER_MINORVERSION_NUMBER > 2)
  #include "http_main.h"
#else
  #define ap_server_conf NULL
#endif

#define MODULE_NAME "mod_fileownercheck"
#define MODULE_VERSION "0.0.1"

module AP_MODULE_DECLARE_DATA fileownercheck_module;

static int fileownercheck_from_file(request_rec *r, apr_file_t *fd)
{
  apr_finfo_t finfo;
  struct stat st;

  if (lstat(r->filename, &st) == -1) {
    ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "%s: lstat() failed: %s",
        MODULE_NAME, r->filename);
     return HTTP_FORBIDDEN;
  }

  if (apr_file_info_get(&finfo, APR_FINFO_OWNER, fd) !=  APR_SUCCESS) {
    ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
        "%s: apr_file_info_get() failed: %s", MODULE_NAME, r->filename);
    return HTTP_FORBIDDEN;
  }

  ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, ap_server_conf,
      "%s: FILEOWNERCHECK: opened r->filename=%s uid=%d but "
      "current r->filename uid=%d", MODULE_NAME, r->filename, finfo.user,
        st.st_uid);

  if (st.st_uid != finfo.user) {
    ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
        "%s: FILEOWNERCHECK faild: opened r->filename=%s uid=%d but "
        "current r->filename uid=%d", MODULE_NAME, r->filename, finfo.user,
        st.st_uid);
    return HTTP_FORBIDDEN;
  }

  return HTTP_OK;
}

static apr_status_t fileownercheck_filter(ap_filter_t *f,
    apr_bucket_brigade *bb)
{
  int status_code;
  apr_bucket *b = APR_BRIGADE_FIRST(bb);
  apr_bucket_file *file = b->data;

  if (!APR_BUCKET_IS_FILE(b)) {
    return ap_pass_brigade(f->next, bb);
  }

  status_code = fileownercheck_from_file(f->r, file->fd);
  if (status_code != HTTP_OK) {
    apr_bucket *b;

    apr_brigade_cleanup(bb);
    b = ap_bucket_error_create(status_code, NULL, f->r->pool,
        f->c->bucket_alloc);
    APR_BRIGADE_INSERT_TAIL(bb, b);
    b = apr_bucket_eos_create(f->c->bucket_alloc);
    APR_BRIGADE_INSERT_TAIL(bb, b);

    return ap_pass_brigade(f->next, bb);
  }

  return ap_pass_brigade(f->next, bb);
}

static void register_hooks(apr_pool_t *p)
{
  ap_register_output_filter("FILEOWNERCHECK", fileownercheck_filter, NULL,
      AP_FTYPE_RESOURCE);
}

#if (AP_SERVER_MINORVERSION_NUMBER > 2)
AP_DECLARE_MODULE(fileownercheck) = {
#else
module AP_MODULE_DECLARE_DATA fileownercheck_module = {
#endif
  STANDARD20_MODULE_STUFF,
  NULL, /* create per-directory config structure */
  NULL, /* merge per-directory config structures */
  NULL, /* create per-server config structure */
  NULL, /* merge per-server config structures */
  NULL, /* command apr_table_t */
  register_hooks /* register hooks */
};

