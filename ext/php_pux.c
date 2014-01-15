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
#include "php_functions.h"
#include "php_mux.h"
#include "php_expandable_mux.h"
#include "php_controller.h"

zend_class_entry *ce_pux_exception;

ZEND_DECLARE_MODULE_GLOBALS(pux);

// #define DEBUG 1
static const zend_function_entry php_functions[] = {
    PHP_FE(pux_match, NULL)
    PHP_FE(pux_sort_routes, NULL)
    PHP_FE_END
};

void pux_init_exception(TSRMLS_D) {
  zend_class_entry e;
  INIT_CLASS_ENTRY(e, "PuxException", NULL);
  ce_pux_exception = zend_register_internal_class_ex(&e, (zend_class_entry*)zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
}

zend_module_entry pux_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_PUX_EXTNAME,
    php_functions,
    PHP_MINIT(pux),
    PHP_MSHUTDOWN(pux),
    PHP_RINIT(pux),
    NULL,
    NULL,
    PHP_PUX_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_PUX
ZEND_GET_MODULE(pux)
#endif

PHP_MINIT_FUNCTION(pux) {
  pux_init_mux(TSRMLS_C);
  pux_init_expandable_mux(TSRMLS_C);
  pux_init_controller(TSRMLS_C);
  return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(pux) {
  return SUCCESS;
}

PHP_RINIT_FUNCTION(pux) {
  return SUCCESS;
}


