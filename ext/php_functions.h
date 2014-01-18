#ifndef PHP_PUX_FUNCTIONS_H
#define PHP_PUX_FUNCTIONS_H 1

#define REQ_METHOD_GET 1
#define REQ_METHOD_POST 2
#define REQ_METHOD_PUT 3
#define REQ_METHOD_DELETE 4


#include "php.h"
#include "php_pux.h"
#include "string.h"
#include "main/php_main.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "zend_object_handlers.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_string.h"

extern inline zval * php_pux_match(zval *z_routes, char *path, int path_len TSRMLS_DC);
extern inline int get_current_request_method_const(zval ** server_vars_hash TSRMLS_DC);
extern inline int get_current_https(zval ** server_vars_hash TSRMLS_DC);
extern inline zval ** fetch_server_vars_hash(TSRMLS_D);
extern inline zval * fetch_server_var(zval **server_vars_hash, char *key , int key_len TSRMLS_DC);
extern inline zval * get_current_http_host(zval ** server_vars_hash TSRMLS_DC);
extern inline zval * get_current_request_uri(zval ** server_vars_hash TSRMLS_DC);
extern inline zval * get_current_request_method(zval ** server_vars_hash TSRMLS_DC);

extern inline int validate_request_method(zval **z_route_options_pp, int current_request_method TSRMLS_DC);
extern inline int validate_domain(zval **z_route_options_pp, zval * http_host TSRMLS_DC);
extern inline int validate_https(zval **z_route_options_pp, int https TSRMLS_DC);

static int _pux_store_mux(char *name, zval * mux TSRMLS_DC) {
    zend_rsrc_list_entry new_le;
    char *persistent_key;
    int ret, persistent_key_len;
    persistent_key_len = spprintf(&persistent_key, 0, "mux_%s", name);
    Z_ADDREF_P(mux);
    new_le.type = le_mux_hash_persist;
    new_le.ptr = mux;
    ret = zend_hash_update(&EG(persistent_list), persistent_key, persistent_key_len + 1, &new_le, sizeof(zend_rsrc_list_entry), NULL);
    efree(persistent_key);
    return ret;
}

static zval * _pux_fetch_mux(char *name TSRMLS_DC) {
    zend_rsrc_list_entry *le;
    char *persistent_key;
    int persistent_key_len = spprintf(&persistent_key, 0, "mux_%s", name);
    if ( zend_hash_find(&EG(persistent_list), persistent_key, persistent_key_len + 1, (void**) &le) == SUCCESS) {
        efree(persistent_key);
        return (zval*) le->ptr;
    }
    efree(persistent_key);
    return NULL;
}



PHP_FUNCTION(pux_match);
PHP_FUNCTION(pux_sort_routes);
PHP_FUNCTION(pux_store_mux);
PHP_FUNCTION(pux_fetch_mux);
PHP_FUNCTION(pux_delete_mux);
PHP_FUNCTION(pux_persistent_dispatch);

#endif
