#include "php.h"
#include "string.h"
#include "main/php_main.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "zend_object_handlers.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_string.h"
#include "pux_functions.h"
#include "php_expandable_mux.h"

zend_class_entry *ce_pux_expandable_mux;

const zend_function_entry expandable_mux_methods[] = {
  PHP_ABSTRACT_ME(ExpandableMux, expand, NULL)
  PHP_FE_END
};

/**
 * TODO: Use zend_class_implements to register Controller class.
 *
 * zend_class_implements(mysqli_result_class_entry TSRMLS_CC, 1, zend_ce_traversable);
 */
static int implement_expandable_mux_interface_handler(zend_class_entry *interface, zend_class_entry *implementor TSRMLS_DC)
{
    if (implementor->type == ZEND_USER_CLASS &&
        !instanceof_function(implementor, ce_pux_expandable_mux TSRMLS_CC)
    ) {
        zend_error(E_ERROR, "Pux\\ExpandableMux can't be implemented by user classes");
    }
    return SUCCESS;
}


void pux_init_expandable_mux(TSRMLS_D)
{
    zend_class_entry ce_interface;
    INIT_CLASS_ENTRY(ce_interface, "Pux\\ExpandableMux", expandable_mux_methods);

    // if(Z_TYPE_PP(current) == IS_OBJECT && instanceof_function(Z_OBJCE_PP(current), curl_CURLFile_class TSRMLS_CC))
    ce_pux_expandable_mux = zend_register_internal_interface(&ce_interface TSRMLS_CC);
    ce_pux_expandable_mux->interface_gets_implemented = implement_expandable_mux_interface_handler;
}

