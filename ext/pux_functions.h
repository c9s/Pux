#ifndef PHP_PUX_FUNCTIONS_H
#define PHP_PUX_FUNCTIONS_H 1

#define REQUEST_METHOD_GET 1
#define REQUEST_METHOD_POST 2
#define REQUEST_METHOD_PUT 3
#define REQUEST_METHOD_DELETE 4
#define REQUEST_METHOD_PATCH 5
#define REQUEST_METHOD_HEAD 6
#define REQUEST_METHOD_OPTIONS 7

#include "php.h"
#include "string.h"
#include "main/php_main.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "zend_object_handlers.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_string.h"
#include "php_pux.h"
#include "pux_mux.h"
#include "pux_functions.h"

extern zval * php_pux_match(zval *z_routes, char *path, int path_len TSRMLS_DC);
extern int get_current_request_method_const(HashTable * server_vars_hash TSRMLS_DC);
extern int get_current_https(HashTable * server_vars_hash TSRMLS_DC);
extern HashTable * fetch_server_vars_hash(TSRMLS_D);
extern zval * fetch_server_var(HashTable *server_vars_hash, char *key , int key_len TSRMLS_DC);
extern zval * get_current_http_host(HashTable * server_vars_hash TSRMLS_DC);
extern zval * get_current_request_uri(HashTable * server_vars_hash TSRMLS_DC);
extern zval * get_current_request_method(HashTable * server_vars_hash TSRMLS_DC);

extern int validate_request_method(zval **z_route_options_pp, int current_request_method TSRMLS_DC);
extern int validate_domain(zval **z_route_options_pp, zval * http_host TSRMLS_DC);
extern int validate_https(zval **z_route_options_pp, int https TSRMLS_DC);

#if ((PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)) || (PHP_MAJOR_VERSION > 5)
#define PUX_STORE_EG_ENVIRON() \
	{ \
		zval ** __old_return_value_pp   = EG(return_value_ptr_ptr); \
		zend_op ** __old_opline_ptr  	= EG(opline_ptr); \
		zend_op_array * __old_op_array  = EG(active_op_array);

#define PUX_RESTORE_EG_ENVIRON() \
		EG(return_value_ptr_ptr) = __old_return_value_pp;\
		EG(opline_ptr)			 = __old_opline_ptr; \
		EG(active_op_array)		 = __old_op_array; \
	}

#else

#define PUX_STORE_EG_ENVIRON() \
	{ \
		zval ** __old_return_value_pp  		   = EG(return_value_ptr_ptr); \
		zend_op ** __old_opline_ptr 		   = EG(opline_ptr); \
		zend_op_array * __old_op_array 		   = EG(active_op_array); \
		zend_function_state * __old_func_state = EG(function_state_ptr);

#define PUX_RESTORE_EG_ENVIRON() \
		EG(return_value_ptr_ptr) = __old_return_value_pp;\
		EG(opline_ptr)			 = __old_opline_ptr; \
		EG(active_op_array)		 = __old_op_array; \
		EG(function_state_ptr)	 = __old_func_state; \
	}

#endif

#define CHECK(p) { if ((p) == NULL) return NULL; }


zval* my_copy_zval(zval* dst, const zval* src, int persistent TSRMLS_DC);

zval** my_copy_zval_ptr(zval** dst, const zval** src, int persistent TSRMLS_DC);


zval * _pux_fetch_mux(char *name TSRMLS_DC);
int mux_loader(char *path, zval *result TSRMLS_DC);
int _pux_store_mux(char *name, zval * mux TSRMLS_DC) ;

void my_zval_copy_ctor_persistent_func(zval *zvalue ZEND_FILE_LINE_DC);

PHP_FUNCTION(pux_match);
PHP_FUNCTION(pux_sort_routes);
PHP_FUNCTION(pux_store_mux);
PHP_FUNCTION(pux_fetch_mux);
PHP_FUNCTION(pux_delete_mux);
PHP_FUNCTION(pux_persistent_dispatch);


int method_str_to_method_const(char * c_request_method );

#endif
