#ifndef PHP_CONTROLLER_H
#define PHP_CONTROLLER_H 1

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
#include "php_functions.h"

extern zend_class_entry *ce_pux_controller;

void pux_init_controller(TSRMLS_D);

PHP_METHOD(Controller, __construct);
PHP_METHOD(Controller, expand);

#endif
