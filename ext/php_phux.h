

#ifndef PHP_PHUX_H
#define PHP_PHUX_H 1
#define PHP_PHUX_VERSION "1.1"
#define PHP_PHUX_EXTNAME "phux"

#define REQ_METHOD_GET 1
#define REQ_METHOD_POST 2
#define REQ_METHOD_PUT 3
#define REQ_METHOD_DELETE 4

PHP_FUNCTION(phux_match);

void phux_init_mux(TSRMLS_D);
 
PHP_METHOD(Mux, __construct);
PHP_METHOD(Mux, add);
PHP_METHOD(Mux, length);
PHP_METHOD(Mux, compile);
PHP_METHOD(Mux, appendRoute);
PHP_METHOD(Mux, appendPCRERoute);
PHP_METHOD(Mux, getRoutes);

PHP_MINIT_FUNCTION(phux);

extern zend_module_entry phux_module_entry;
#define phpext_phux_ptr &phux_module_entry

#endif
