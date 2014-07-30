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
#include "http_request.h"
#include "http_log.h"
#include "unixd.h"
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

#define ENABLE 1
#define DISABLE 0

module AP_MODULE_DECLARE_DATA fileownercheck_module;

typedef struct {
  unsigned int check_suexec;
} foc_dir_config_t;

static void *foc_create_dir_config(apr_pool_t *p, char *d)
{
  foc_dir_config_t *dconf = apr_pcalloc(p, sizeof(foc_dir_config_t));

  dconf->check_suexec = DISABLE;

  return dconf;
}

static int fileownercheck_from_opened_file(request_rec *r, apr_file_t *fd)
{
  apr_finfo_t finfo;
  struct stat st;
  ap_unix_identity_t *ugid = ap_run_get_suexec_identity(r);
  foc_dir_config_t *dconf = ap_get_module_config(r->per_dir_config,
      &fileownercheck_module);

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
      "%s: FILEOWNERCHECK: opened r->filename=%s uid=%d, "
      "current r->filename uid=%d", MODULE_NAME, r->filename, finfo.user,
        st.st_uid);

  if (st.st_uid != finfo.user) {
    ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
        "%s: FILEOWNERCHECK faild: opened r->filename=%s uid=%d but "
        "current r->filename uid=%d, "
        "r->filename symlink to a unauthorized file?",
        MODULE_NAME, r->filename, finfo.user, st.st_uid);
    return HTTP_FORBIDDEN;
  }

  if (dconf->check_suexec == ENABLE && ugid != NULL
      && ugid->uid != finfo.user) {
    ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
        "%s: FILEOWNERCHECK faild: opened r->filename=%s uid=%d but "
        "suexec config uid=%d, "
        "r->filename path includes symlink to a unauthorized dir?",
        MODULE_NAME, r->filename, finfo.user, ugid->uid);
    return HTTP_FORBIDDEN;
  }

  return HTTP_OK;
}

static void create_output_from_status_code(ap_filter_t *f,
    apr_bucket_brigade *bb, int status)
{
  apr_bucket *eb;

  apr_brigade_cleanup(bb);
  eb = ap_bucket_error_create(status, NULL, f->r->pool, f->c->bucket_alloc);
  APR_BRIGADE_INSERT_TAIL(bb, eb);
  eb = apr_bucket_eos_create(f->c->bucket_alloc);
  APR_BRIGADE_INSERT_TAIL(bb, eb);
}

static apr_status_t fileownercheck_filter(ap_filter_t *f,
    apr_bucket_brigade *bb)
{
  int status_code;
  apr_bucket *b = APR_BRIGADE_FIRST(bb);
  apr_bucket_file *file = (apr_bucket_file *)b->data;
  request_rec *r = f->r;

  if (!APR_BUCKET_IS_FILE(b)) {
    return ap_pass_brigade(f->next, bb);
  }

  status_code = fileownercheck_from_opened_file(r, file->fd);
  if (status_code != HTTP_OK) {
    create_output_from_status_code(f, bb, status_code);
  }

  return ap_pass_brigade(f->next, bb);
}

static int fileownercheck_insert_output_filter(request_rec *r)
{
   ap_add_output_filter("FILEOWNERCHECK", NULL, r, r->connection);

   return DECLINED;
}

static void register_hooks(apr_pool_t *p)
{
  ap_register_output_filter("FILEOWNERCHECK", fileownercheck_filter, NULL,
      AP_FTYPE_RESOURCE);
  ap_hook_fixups(fileownercheck_insert_output_filter, NULL, NULL,
      APR_HOOK_LAST);
}

static const char *set_fileownercheck_suexec(cmd_parms *cmd, void *mconfig,
    int flag)
{
    foc_dir_config_t *dconf = (foc_dir_config_t *)mconfig;

    dconf->check_suexec = flag;

    return NULL;
}

static const command_rec fileownercheck_cmds[] = {
  AP_INIT_FLAG("FOCSuexecEnable", set_fileownercheck_suexec, NULL,
      ACCESS_CONF | RSRC_CONF, "Set Enable Owner Check Using suExecUserGgroup "
      " On / Off. (default Off)"),
  {NULL}
};


#if (AP_SERVER_MINORVERSION_NUMBER > 2)
AP_DECLARE_MODULE(fileownercheck) = {
#else
module AP_MODULE_DECLARE_DATA fileownercheck_module = {
#endif
  STANDARD20_MODULE_STUFF,
  foc_create_dir_config, /* create per-directory config structure */
  NULL, /* merge per-directory config structures */
  NULL, /* create per-server config structure */
  NULL, /* merge per-server config structures */
  fileownercheck_cmds, /* command apr_table_t */
  register_hooks /* register hooks */
};

