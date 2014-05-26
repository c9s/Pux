#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "string.h"
#include "main/php_main.h"
#include "Zend/zend_API.h"
#include "Zend/zend_variables.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "zend_object_handlers.h"
#include "ext/standard/php_string.h"

#include "php_pux.h"
#include "ct_helper.h"
#include "pux_functions.h"
#include "pux_mux.h"
#include "php_expandable_mux.h"
#include "pux_controller.h"

ZEND_DECLARE_MODULE_GLOBALS(pux);


// persistent list entry type for HashTable
int le_mux_hash_table;

// persistent list entry type for boolean
int le_mux_bool;

// persistent list entry type for int
int le_mux_int;

// persistent list entry type for string
int le_mux_string;

zend_class_entry *ce_pux_exception;


// #define DEBUG 1
static const zend_function_entry pux_functions[] = {
    PHP_FE(pux_match, NULL)
    PHP_FE(pux_sort_routes, NULL)
    PHP_FE(pux_delete_mux, NULL)
    PHP_FE_END
};

void pux_init_exception(TSRMLS_D) {
  zend_class_entry e;
  INIT_CLASS_ENTRY(e, "PuxException", NULL);
  ce_pux_exception = zend_register_internal_class_ex(&e, (zend_class_entry*)zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
}

void pux_mux_le_hash_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    HashTable *h = (HashTable*) rsrc->ptr;
    if (h) {
        // zend_hash_destroy(h);
        // pefree(h, 1);
    }
}

zend_module_entry pux_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_PUX_EXTNAME,
    pux_functions,
    PHP_MINIT(pux),
    PHP_MSHUTDOWN(pux),
    PHP_RINIT(pux),
    NULL,
    NULL,
    PHP_PUX_VERSION,
    STANDARD_MODULE_PROPERTIES
};

PHP_INI_BEGIN()
    PHP_INI_ENTRY("pux.fstat", "0", PHP_INI_ALL, NULL)
    // STD_PHP_INI_ENTRY("pux.direction", "1", PHP_INI_ALL, OnUpdateBool, direction, zend_hello_globals, hello_globals)
PHP_INI_END()

#ifdef COMPILE_DL_PUX
ZEND_GET_MODULE(pux)
#endif

static void php_pux_init_globals(zend_pux_globals *pux_globals)
{
    // pux_globals->persistent_list = (HashTable*) 
    // array_init(pux_globals->persistent_list);
}

PHP_MINIT_FUNCTION(pux) {
  ZEND_INIT_MODULE_GLOBALS(pux, php_pux_init_globals, NULL);
  REGISTER_INI_ENTRIES();
  pux_init_mux(TSRMLS_C);
  pux_init_expandable_mux(TSRMLS_C);
  pux_init_controller(TSRMLS_C);
  le_mux_hash_table = zend_register_list_destructors_ex(NULL, pux_mux_le_hash_dtor, "hash table", module_number);

  REGISTER_LONG_CONSTANT("REQUEST_METHOD_GET", REQUEST_METHOD_GET, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("REQUEST_METHOD_POST", REQUEST_METHOD_POST, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("REQUEST_METHOD_DELETE", REQUEST_METHOD_DELETE, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("REQUEST_METHOD_PUT", REQUEST_METHOD_PUT, CONST_CS | CONST_PERSISTENT);
  return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(pux) {
  UNREGISTER_INI_ENTRIES();
  return SUCCESS;
}

PHP_RINIT_FUNCTION(pux) {
  return SUCCESS;
}


