#include "php.h"
#include "string.h"
#include "main/php_main.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "zend_object_handlers.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_string.h"
#include "php_functions.h"
#include "php_expandable_mux.h"

zend_class_entry *ce_pux_expandable_mux;

const zend_function_entry expandable_mux_methods[] = {
  PHP_ABSTRACT_ME(ExpandableMux, expand, NULL)
  PHP_FE_END
};

void pux_init_expandable_mux(TSRMLS_D)
{
    zend_class_entry ce_interface;
    INIT_CLASS_ENTRY(ce_interface, "Pux\\ExpandableMux", expandable_mux_methods);
    ce_pux_expandable_mux = zend_register_internal_interface(&ce_interface TSRMLS_CC);
}
