#ifndef PUX_PERSISTENT_H
#define PUX_PERSISTENT_H

#include "php.h"
#include "Zend/zend_compile.h"
#include "Zend/zend_alloc.h"
#include "Zend/zend_operators.h"
#include "Zend/zend_API.h"
#include "Zend/zend_API.h"
#include "string.h"
#include "main/php_main.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "zend_object_handlers.h"

int persistent_store(char *key, int key_len, int list_type, void * val TSRMLS_DC);
int pux_persistent_store(char *ns, char *key, int list_type, void * val TSRMLS_DC) ;

void * persistent_fetch(char *key, int key_len TSRMLS_DC);
void * pux_persistent_fetch(char *ns, char *key TSRMLS_DC);

#endif
