#include "string.h"
#include "php.h"
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
#include "php_expandable_mux.h"
#include "php_controller.h"

zend_class_entry *ce_pux_controller;

const zend_function_entry controller_methods[] = {
  PHP_ME(Controller, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
  PHP_ME(Controller, expand, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Controller, getActions, NULL, ZEND_ACC_PUBLIC)
  // PHP_ME(Controller, __destruct,  NULL, ZEND_ACC_PUBLIC|ZEND_ACC_DTOR) 
  PHP_FE_END
};

void pux_init_controller(TSRMLS_D) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Pux\\Controller", controller_methods);
    ce_pux_controller = zend_register_internal_class(&ce TSRMLS_CC);
    zend_class_implements(ce_pux_controller TSRMLS_CC, 1, ce_pux_expandable_mux);
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

int strpos(char *haystack, char *needle)
{
   char *p = strstr(haystack, needle);
   if (p)
      return p - haystack;
   return -1;
}

PHP_METHOD(Controller, getActions)
{
    // get function table hash
    HashTable *function_table = &Z_OBJCE_P(this_ptr)->function_table;
    HashPosition pointer;
    zval **func = NULL;


    zval *funcs;

    ALLOC_INIT_ZVAL(funcs);
    array_init(funcs);

    for(zend_hash_internal_pointer_reset_ex(function_table, &pointer); 
            zend_hash_get_current_data_ex(function_table, (void**) &func, &pointer) == SUCCESS; 
            zend_hash_move_forward_ex(function_table, &pointer)) 
    {
        char *key = NULL;
        uint   key_len = 0;
        ulong   int_key = 0;
        if ( zend_hash_get_current_key_ex(function_table, &key, &key_len, &int_key, 0, &pointer) == HASH_KEY_IS_STRING ) {
            // php_printf("%s\n", key);
            char *pos;
            if ( (strpos(key, "action")) == (key_len - strlen("action") - 1) ) {
                add_next_index_stringl(funcs, key, key_len, 1);
            }
        }
    }
    *return_value = *funcs;
    zval_copy_ctor(return_value);
    return;
}

PHP_METHOD(Controller, expand)
{

}

