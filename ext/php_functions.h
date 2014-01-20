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
#include "php_pux.h"
#include "php_mux.h"
#include "php_functions.h"

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

#define PUX_DEBUG 1
#if PUX_DEBUG
#define HT_OK				0
#define HT_IS_DESTROYING	1
#define HT_DESTROYED		2
#define HT_CLEANING			3

static void _zend_is_inconsistent(const HashTable *ht, const char *file, int line)
{
	if (ht->inconsistent==HT_OK) {
		return;
	}
	switch (ht->inconsistent) {
		case HT_IS_DESTROYING:
			zend_output_debug_string(1, "%s(%d) : ht=%p is being destroyed", file, line, ht);
			break;
		case HT_DESTROYED:
			zend_output_debug_string(1, "%s(%d) : ht=%p is already destroyed", file, line, ht);
			break;
		case HT_CLEANING:
			zend_output_debug_string(1, "%s(%d) : ht=%p is being cleaned", file, line, ht);
			break;
		default:
			zend_output_debug_string(1, "%s(%d) : ht=%p is inconsistent", file, line, ht);
			break;
	}
	zend_bailout();
}
#define IS_CONSISTENT(a) _zend_is_inconsistent(a, __FILE__, __LINE__);
#define SET_INCONSISTENT(n) ht->inconsistent = n;
#else
#define IS_CONSISTENT(a)
#define SET_INCONSISTENT(n)
#endif


HashTable * zend_hash_clone_persistent(HashTable* src TSRMLS_DC);

extern inline int persistent_store(char *key, int key_len, int list_type, void * val TSRMLS_DC);
extern inline int pux_persistent_store(char *ns, char *key, void * val TSRMLS_DC) ;

extern inline void * persistent_fetch(char *key, int key_len TSRMLS_DC);
extern inline void * pux_persistent_fetch(char *ns, char *key TSRMLS_DC);

zval * _pux_fetch_mux(char *name TSRMLS_DC);
int mux_loader(char *path, zval *result TSRMLS_DC);
int _pux_store_mux(char *name, zval * mux TSRMLS_DC) ;

PHP_FUNCTION(pux_match);
PHP_FUNCTION(pux_sort_routes);
PHP_FUNCTION(pux_store_mux);
PHP_FUNCTION(pux_fetch_mux);
PHP_FUNCTION(pux_delete_mux);
PHP_FUNCTION(pux_persistent_dispatch);


#endif
