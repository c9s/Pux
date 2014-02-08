#ifndef PHP_PUX_H
#define PHP_PUX_H 1

#include "php.h"
#include "string.h"
#include "main/php_main.h"
#include "Zend/zend_API.h"
#include "Zend/zend_variables.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "zend_object_handlers.h"
#include "ext/standard/php_string.h"

#define PHP_PUX_VERSION "1.3.1"
#define PHP_PUX_EXTNAME "pux"


#ifdef ZTS
#include "TSRM.h"
#endif

extern int pux_globals_id;

extern int le_mux_hash_table;

// global variable structure
ZEND_BEGIN_MODULE_GLOBALS(pux)
    // zval *mux_array;
    HashTable * persistent_list;
    // zend_bool direction;
ZEND_END_MODULE_GLOBALS(pux)

#ifdef ZTS
#define PUX_G(v) TSRMG(pux_globals_id, zend_pux_globals *, v)
#else
#define PUX_G(v) (pux_globals.v)
#endif



#define ZEND_HASH_FETCH(hash,key,ret) \
    zend_hash_find(hash, key, sizeof(key), (void**)&ret) == SUCCESS

#define PUSH_PARAM(arg) zend_vm_stack_push(arg TSRMLS_CC)
#define POP_PARAM() (void)zend_vm_stack_pop(TSRMLS_C)
#define PUSH_EO_PARAM()
#define POP_EO_PARAM()
 
#define CALL_METHOD_BASE(classname, name) zim_##classname##_##name
 
#define CALL_METHOD_HELPER(classname, name, retval, thisptr, num, param) \
  PUSH_PARAM(param); PUSH_PARAM((void*)num);                            \
  PUSH_EO_PARAM();                                                      \
  CALL_METHOD_BASE(classname, name)(num, retval, NULL, thisptr, 0 TSRMLS_CC); \
  POP_EO_PARAM();                       \
  POP_PARAM(); POP_PARAM();
 
#define CALL_METHOD(classname, name, retval, thisptr)                  \
  CALL_METHOD_BASE(classname, name)(0, retval, NULL, thisptr, 0 TSRMLS_CC);
 
#define CALL_METHOD1(classname, name, retval, thisptr, param1)         \
  CALL_METHOD_HELPER(classname, name, retval, thisptr, 1, param1);
 
#define CALL_METHOD2(classname, name, retval, thisptr, param1, param2) \
  PUSH_PARAM(param1);                                                   \
  CALL_METHOD_HELPER(classname, name, retval, thisptr, 2, param2);     \
  POP_PARAM();
 
#define CALL_METHOD3(classname, name, retval, thisptr, param1, param2, param3) \
  PUSH_PARAM(param1); PUSH_PARAM(param2);                               \
  CALL_METHOD_HELPER(classname, name, retval, thisptr, 3, param3);     \
  POP_PARAM(); POP_PARAM();

PHP_MINIT_FUNCTION(pux);
PHP_MSHUTDOWN_FUNCTION(pux);
PHP_RINIT_FUNCTION(pux);

zval * php_pux_match(zval *z_routes, char *path, int path_len TSRMLS_DC);

zval * call_mux_method(zval * object , char * method_name , int method_name_len, int param_count, zval* arg1, zval* arg2, zval* arg3 TSRMLS_DC);

zend_class_entry ** get_pattern_compiler_ce(TSRMLS_D);

extern zend_class_entry *ce_pux_exception;

extern zend_module_entry pux_module_entry;

void pux_init_exception(TSRMLS_D);

void pux_mux_le_hash_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);

#define phpext_pux_ptr &pux_module_entry

#endif
