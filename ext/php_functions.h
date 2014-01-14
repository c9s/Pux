#ifndef PHP_PUX_FUNCTIONS_H
#define PHP_PUX_FUNCTIONS_H 1

#define REQ_METHOD_GET 1
#define REQ_METHOD_POST 2
#define REQ_METHOD_PUT 3
#define REQ_METHOD_DELETE 4


#include "php.h"
#include "string.h"
#include "main/php_main.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "zend_object_handlers.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_string.h"

extern inline zval * php_pux_match(zval *z_routes, char *path, int path_len TSRMLS_DC);
extern inline int get_current_request_method_const(TSRMLS_D);
extern inline int get_current_https(TSRMLS_D);
extern inline zval * fetch_server_var( char *key , int key_len );
extern inline zval * get_current_http_host(TSRMLS_D);
extern inline zval * get_current_request_uri(TSRMLS_D);
extern inline zval * get_current_request_method(TSRMLS_D);

extern inline int validate_request_method(zval **z_route_options_pp, int current_request_method TSRMLS_DC);
extern inline int validate_domain(zval **z_route_options_pp, zval * http_host TSRMLS_DC);
extern inline int validate_https(zval **z_route_options_pp, int https TSRMLS_DC);

PHP_FUNCTION(pux_match);
PHP_FUNCTION(pux_sort_routes);

#endif
