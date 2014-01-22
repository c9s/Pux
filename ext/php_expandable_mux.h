#ifndef PHP_EXPANDABLE_MUX_H
#define PHP_EXPANDABLE_MUX_H 1

#include "php.h"
#include "string.h"
#include "main/php_main.h"
#include "Zend/zend_API.h"
#include "Zend/zend_variables.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "zend_object_handlers.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_string.h"
#include "php_pux.h"
#include "pux_functions.h"

extern zend_class_entry *ce_pux_expandable_mux;

void pux_init_expandable_mux(TSRMLS_D);

#endif
