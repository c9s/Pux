#ifndef PHP_MUX_H
#define PHP_MUX_H 1

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

static zend_class_entry *pux_ce_mux;

void pux_init_mux(TSRMLS_D);


PHP_METHOD(Mux, __construct);
PHP_METHOD(Mux, getId);
PHP_METHOD(Mux, add);
PHP_METHOD(Mux, length);
PHP_METHOD(Mux, compile);
PHP_METHOD(Mux, sort);
PHP_METHOD(Mux, appendRoute);
PHP_METHOD(Mux, appendPCRERoute);
PHP_METHOD(Mux, getRoutes);
PHP_METHOD(Mux, matchRoute);
PHP_METHOD(Mux, dispatch);
PHP_METHOD(Mux, getSubMux);
PHP_METHOD(Mux, export);
PHP_METHOD(Mux, mount);

PHP_METHOD(Mux, get);
PHP_METHOD(Mux, post);
PHP_METHOD(Mux, put);
PHP_METHOD(Mux, delete);

PHP_METHOD(Mux, __set_state);

// static method
PHP_METHOD(Mux, generate_id);


#endif
