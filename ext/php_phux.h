

#ifndef PHP_PHUX_H
#define PHP_PHUX_H 1
#define PHP_PHUX_VERSION "1.1"
#define PHP_PHUX_EXTNAME "phux"

PHP_FUNCTION(phux_match);

extern zend_module_entry phux_module_entry;
#define phpext_phux_ptr &phux_module_entry

#endif
