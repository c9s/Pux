#include "string.h"
#include "ctype.h"
#include "php.h"
#include "main/php_main.h"
#include "Zend/zend_API.h"
#include "Zend/zend_variables.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "Zend/zend_interfaces.h"
#include "zend_object_handlers.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_var.h"

#include "ct_helper.h"
#include "php_pux.h"
#include "pux_mux.h"
#include "pux_functions.h"
#include "php_expandable_mux.h"
#include "pux_controller.h"

zend_class_entry *ce_pux_controller;

const zend_function_entry controller_methods[] = {
  PHP_ME(Controller, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
  PHP_ME(Controller, expand, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Controller, getActionMethods, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Controller, getActionPaths, NULL, ZEND_ACC_PUBLIC)

  PHP_ME(Controller, before, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Controller, after, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Controller, toJson, NULL, ZEND_ACC_PUBLIC)

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

int strpos(const char *haystack, char *needle)
{
   char *p = strstr(haystack, needle);
   if (p)
      return p - haystack;
   return -1;
}

PHP_METHOD(Controller, getActionMethods)
{
    // get function table hash
    HashTable *function_table = &Z_OBJCE_P(this_ptr)->function_table;
    HashPosition pos;

    array_init(return_value);

    zend_function *mptr;
    zend_hash_internal_pointer_reset_ex(function_table, &pos);

    while (zend_hash_get_current_data_ex(function_table, (void **) &mptr, &pos) == SUCCESS) {
        const char * key = mptr->common.function_name;
        size_t   key_len = strlen(mptr->common.function_name);
        int p = strpos(key, "Action");
        if ( p != -1 && (size_t)p == (key_len - strlen("Action")) ) {
            add_next_index_stringl(return_value, key, key_len, 1);
        }
        zend_hash_move_forward_ex(function_table, &pos);
    }
    return;
}


char * translate_method_name_to_path(const char *method_name)
{
    char *p = strstr(method_name, "Action");
    if ( p == NULL ) {
        return NULL;
    }

    char * new_path;
    if ( strncmp(method_name, "indexAction", strlen("indexAction") ) == 0 ) {
        new_path = ecalloc( 1 , sizeof(char) );
        // memcpy(new_path, "", 1);
        return new_path;
    }

    new_path = ecalloc( 128 , sizeof(char) );

    int    len = p - method_name;
    char * c = (char*) method_name;
    int    x = 0;
    new_path[x++] = '/';
    int    new_path_len = 1;
    while( len-- ) {
        if ( isupper(*c) ) {
            new_path[x++] = '/';
            new_path[x++] = tolower(*c);
            new_path_len += 2;
        } else {
            new_path[x++] = *c;
            new_path_len ++;
        }
        c++;
    }
    return new_path;
}

PHP_METHOD(Controller, getActionPaths)
{
    zend_function *fe;
    if ( zend_hash_quick_find( &ce_pux_controller->function_table, "getactionmethods", sizeof("getactionmethods"), zend_inline_hash_func(ZEND_STRS("getactionmethods")), (void **) &fe) == FAILURE ) {
        php_error(E_ERROR, "getActionMethods method not found");
    }
    // call export method
    zval *rv = NULL;
    zend_call_method_with_0_params( &this_ptr, ce_pux_controller, &fe, "getactionmethods", &rv );

    // php_var_dump(rv, 1);

    HashTable *func_list = Z_ARRVAL_P(rv);
    HashPosition pointer;
    zval **func = NULL;


    array_init(return_value);

    for(zend_hash_internal_pointer_reset_ex(func_list, &pointer); 
            zend_hash_get_current_data_ex(func_list, (void**) &func, &pointer) == SUCCESS; 
            zend_hash_move_forward_ex(func_list, &pointer)) 
    {
        const char *method_name = Z_STRVAL_PP(func);
        char * path = translate_method_name_to_path(method_name);
        if ( path ) {
            zval * new_item;
            MAKE_STD_ZVAL(new_item);
            array_init_size(new_item, 2);
            add_next_index_string(new_item, path, 0);
            add_next_index_stringl(new_item, method_name, Z_STRLEN_PP(func) , 1);
            add_next_index_zval(return_value, new_item);
        }
    }
    return;
}

PHP_METHOD(Controller, expand)
{
    zval *new_mux;
    MAKE_STD_ZVAL(new_mux);
    object_init_ex(new_mux, ce_pux_mux);
    CALL_METHOD(Mux, __construct, new_mux, new_mux);


    zval *path_array = NULL;
    zend_call_method_with_0_params( &this_ptr, ce_pux_controller, NULL, "getactionpaths", &path_array );

    if ( Z_TYPE_P(path_array) != IS_ARRAY ) {
        php_error(E_ERROR, "getActionPaths does not return an array.");
        RETURN_FALSE;
    }

    const char *class_name = NULL;
    zend_uint   class_name_len;
    int   dup = zend_get_object_classname(this_ptr, &class_name, &class_name_len TSRMLS_CC);

    // php_printf("%s\n", class_name);

    HashPosition pointer;
    zval **path_entry = NULL;
    for(zend_hash_internal_pointer_reset_ex( Z_ARRVAL_P(path_array), &pointer); 
            zend_hash_get_current_data_ex( Z_ARRVAL_P(path_array), (void**) &path_entry, &pointer) == SUCCESS; 
            zend_hash_move_forward_ex( Z_ARRVAL_P(path_array), &pointer)) 
    {
        zval **z_path;
        zval **z_method;

        if ( zend_hash_index_find(Z_ARRVAL_PP(path_entry), 0, (void**) &z_path) == FAILURE ) {
            continue;
        }
        if ( zend_hash_index_find(Z_ARRVAL_PP(path_entry), 1, (void**) &z_method) == FAILURE ) {
            continue;
        }

        zval *z_callback;
        MAKE_STD_ZVAL(z_callback);
        array_init_size(z_callback, 2);
        add_next_index_stringl(z_callback, class_name, class_name_len, 1);
        add_next_index_zval(z_callback, *z_method);

        zval *rv = NULL;
        zend_call_method_with_2_params(&new_mux, ce_pux_mux, NULL, "add", &rv, *z_path, z_callback );
    }

    zval *rv = NULL;
    zend_call_method_with_0_params(&new_mux, ce_pux_mux, NULL, "sort", &rv );

    *return_value = *new_mux;
    zval_copy_ctor(return_value);
}

PHP_METHOD(Controller, before) {

}

PHP_METHOD(Controller, after) {

}

PHP_METHOD(Controller, toJson)
{
    zval *z_data;
    long options = 0;
    long depth = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|ll", &z_data, &options, &depth) == FAILURE) {
        RETURN_FALSE;
    }

    zval *z_options;
    zval *z_depth;

    MAKE_STD_ZVAL(z_options);
    MAKE_STD_ZVAL(z_depth);

    ZVAL_LONG(z_depth, 0);
    ZVAL_LONG(z_options, 0);

    

    zval *rv = NULL;
    // zend_call_method_with_3_params(NULL, NULL, NULL, "json_encode", sizeof("json_encode"), &rv, 3, z_data, z_options, z_depth TSRMLS_CC );
    zend_call_method_with_2_params(NULL, NULL, NULL, "json_encode", &rv, z_data, z_options);

    *return_value = *rv;
    zval_copy_ctor(return_value);
}



