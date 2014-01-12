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
#include "ext/standard/php_var.h"

#include "php_pux.h"
#include "ct_helper.h"
#include "php_functions.h"
#include "php_controller.h"

zend_class_entry *ce_pux_controller;

const zend_function_entry controller_methods[] = {
  PHP_ME(Controller, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
  // PHP_ME(Controller, __destruct,  NULL, ZEND_ACC_PUBLIC|ZEND_ACC_DTOR) 
  PHP_FE_END
};

void pux_init_controller(TSRMLS_D) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Pux\\Controller", controller_methods);
    ce_pux_controller = zend_register_internal_class(&ce TSRMLS_CC);
}

PHP_METHOD(Controller, __construct) {
    /*
    zval *z_routes = NULL, *z_routes_by_id , *z_subcontroller = NULL, *z_static_routes = NULL;
    ALLOC_INIT_ZVAL(z_routes);
    ALLOC_INIT_ZVAL(z_routes_by_id);
    ALLOC_INIT_ZVAL(z_static_routes);
    ALLOC_INIT_ZVAL(z_subcontroller);

    array_init(z_routes);
    array_init(z_routes_by_id);
    array_init(z_static_routes);
    array_init(z_subcontroller);

    zend_update_property( ce_pux_controller, this_ptr, "routes", sizeof("routes")-1, z_routes TSRMLS_CC);
    zend_update_property( ce_pux_controller, this_ptr, "routesById", sizeof("routesById")-1, z_routes_by_id TSRMLS_CC);
    zend_update_property( ce_pux_controller, this_ptr, "staticRoutes", sizeof("staticRoutes")-1, z_static_routes TSRMLS_CC);
    zend_update_property( ce_pux_controller, this_ptr, "subcontroller", sizeof("subcontroller")-1, z_subcontroller TSRMLS_CC);
    */
}

