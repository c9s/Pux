

#ifndef PHP_ROLLER_H
#define PHP_ROLLER_H 1
#define PHP_ROLLER_VERSION "1.1"
#define PHP_ROLLER_EXTNAME "phux"

PHP_FUNCTION(phux_dispatch);

extern zend_module_entry phux_module_entry;
#define phpext_phux_ptr &phux_module_entry

#endif
