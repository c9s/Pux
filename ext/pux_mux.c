
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
#include "ext/standard/php_smart_str.h"
#include "ext/standard/php_array.h"

#include "php_pux.h"
#include "ct_helper.h"
#include "pux_functions.h"
#include "pux_controller.h"
#include "pux_mux.h"

zend_class_entry *ce_pux_mux;

const zend_function_entry mux_methods[] = {
  PHP_ME(Mux, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
  PHP_ME(Mux, __destruct,  NULL, ZEND_ACC_PUBLIC|ZEND_ACC_DTOR) 
  PHP_ME(Mux, getId, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, add, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, any, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, compile, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, sort, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, dispatch, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, length, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, mount, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, appendRoute, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, appendPCRERoute, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, match, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, getRoutes, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, setRoutes, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, getRoute, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, getSubMux, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, getRequestMethodConstant, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, export, NULL, ZEND_ACC_PUBLIC)

  PHP_ME(Mux, get, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, post, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, put, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, delete, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, head, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, patch, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, options, NULL, ZEND_ACC_PUBLIC)

  PHP_ME(Mux, __set_state, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
  PHP_ME(Mux, generate_id, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
  PHP_FE_END
};

void pux_init_mux(TSRMLS_D) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Pux\\Mux", mux_methods);
    ce_pux_mux = zend_register_internal_class(&ce TSRMLS_CC);
    zend_declare_property_null(ce_pux_mux, "id", sizeof("id")-1, ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(ce_pux_mux, "routes", sizeof("routes")-1, ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(ce_pux_mux, "routesById", sizeof("routesById")-1, ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(ce_pux_mux, "staticRoutes", sizeof("staticRoutes")-1, ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(ce_pux_mux, "submux", sizeof("submux")-1, ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_bool(ce_pux_mux, "expand", sizeof("expand")-1, 1, ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_long(ce_pux_mux, "id_counter", sizeof("id_counter")-1, 0, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC TSRMLS_CC);
}

zend_class_entry ** get_pattern_compiler_ce(TSRMLS_D) {
    zend_class_entry **ce_pattern_compiler = NULL;
    if ( zend_lookup_class( "Pux\\PatternCompiler", sizeof("Pux\\PatternCompiler")-1 , &ce_pattern_compiler TSRMLS_CC) == FAILURE ) {
        php_error(E_ERROR, "Class Pux\\PatternCompiler not found.");
        return NULL;
    }
    if ( ce_pattern_compiler == NULL || *ce_pattern_compiler == NULL ) {
        php_error(E_ERROR, "Class Pux\\PatternCompiler not found.");
        return NULL;
    }
    return ce_pattern_compiler;
}

// Returns compiled route zval
zval * compile_route_pattern(zval *z_pattern, zval *z_options, zend_class_entry **ce_pattern_compiler TSRMLS_DC)
{
    // zend_class_entry **ce_pattern_compiler;
    if ( ce_pattern_compiler == NULL ) {
        ce_pattern_compiler = get_pattern_compiler_ce(TSRMLS_C);
        if ( ce_pattern_compiler == NULL ) {
            return NULL;
        }
    }

    zval *z_compiled_route = NULL; // will be an array
    zend_call_method( NULL, *ce_pattern_compiler, NULL, "compile", sizeof("compile")-1, &z_compiled_route, 2, z_pattern, z_options TSRMLS_CC );

    if ( z_compiled_route == NULL ) {
        return NULL;
    } else if ( Z_TYPE_P(z_compiled_route) == IS_NULL ) {
        zval_ptr_dtor(&z_compiled_route);
        return NULL;
    }
    if ( Z_TYPE_P(z_compiled_route) != IS_ARRAY ) {
        zval_ptr_dtor(&z_compiled_route);
        return NULL;
    }
    INIT_PZVAL(z_compiled_route); // set ref = 1
    return z_compiled_route;
}

static void var_export(zval *return_value, zval *what TSRMLS_DC)
{
    smart_str buf = {0};
    php_var_export_ex(&what, 0, &buf TSRMLS_CC);
    smart_str_0 (&buf);
    ZVAL_STRINGL(return_value, buf.c, buf.len, 0);
}

PHP_METHOD(Mux, __construct) {
    zval *z_routes = NULL, *z_routes_by_id , *z_submux = NULL, *z_static_routes = NULL;

    MAKE_STD_ZVAL(z_routes);
    MAKE_STD_ZVAL(z_routes_by_id);
    MAKE_STD_ZVAL(z_static_routes);
    MAKE_STD_ZVAL(z_submux);

    array_init(z_routes);
    array_init(z_routes_by_id);
    array_init(z_static_routes);
    array_init(z_submux);

    zend_update_property(ce_pux_mux, this_ptr, "routes", sizeof("routes")-1, z_routes TSRMLS_CC);
    zend_update_property(ce_pux_mux, this_ptr, "routesById", sizeof("routesById")-1, z_routes_by_id TSRMLS_CC);
    zend_update_property(ce_pux_mux, this_ptr, "staticRoutes", sizeof("staticRoutes")-1, z_static_routes TSRMLS_CC);
    zend_update_property(ce_pux_mux, this_ptr, "submux", sizeof("submux")-1, z_submux TSRMLS_CC);
}

PHP_METHOD(Mux, __destruct) {
}

PHP_METHOD(Mux, generate_id) {
    zval * z_counter = NULL;
    long counter = 0;
    z_counter = zend_read_static_property(ce_pux_mux, "id_counter", sizeof("id_counter")-1, 0 TSRMLS_CC);
    if ( z_counter != NULL ) {
        counter = Z_LVAL_P(z_counter);
    }
    counter++;
    Z_LVAL_P(z_counter) = counter;
    RETURN_LONG(counter);
}

PHP_METHOD(Mux, __set_state) {
    zval *z_array;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &z_array) == FAILURE) {
        RETURN_FALSE;
    }

    object_init_ex(return_value, ce_pux_mux);
    // XXX: put this back if we have problem
    // CALL_METHOD(Mux, __construct, return_value, return_value);

    zval **z_id = NULL;
    zval **z_routes = NULL;
    zval **z_static_routes = NULL;
    zval **z_routes_by_id = NULL;
    zval **z_submux = NULL;
    zval **z_expand = NULL;

    if ( zend_hash_quick_find(Z_ARRVAL_P(z_array), "id", sizeof("id"), zend_inline_hash_func(ZEND_STRS("id")), (void**)&z_id) == SUCCESS ) {
        zend_update_property(ce_pux_mux, return_value, "id", sizeof("id")-1, *z_id TSRMLS_CC);
    }


    if ( zend_hash_quick_find(Z_ARRVAL_P(z_array), "routes", sizeof("routes"), zend_inline_hash_func(ZEND_STRS("routes")), (void**)&z_routes) == SUCCESS ) {
        Z_ADDREF_PP(z_routes);
        zend_update_property(ce_pux_mux, return_value, "routes", sizeof("routes")-1, *z_routes TSRMLS_CC);
    }

    if ( zend_hash_quick_find(Z_ARRVAL_P(z_array), "staticRoutes", sizeof("staticRoutes"), zend_inline_hash_func(ZEND_STRS("staticRoutes")), (void**)&z_static_routes) == SUCCESS ) {
        Z_ADDREF_PP(z_static_routes);
        zend_update_property(ce_pux_mux, return_value, "staticRoutes", sizeof("staticRoutes")-1, *z_static_routes TSRMLS_CC);
    }

    if ( zend_hash_quick_find(Z_ARRVAL_P(z_array), "routesById", sizeof("routesById"), zend_inline_hash_func(ZEND_STRS("routesById")), (void**)&z_routes_by_id) == SUCCESS ) {
        Z_ADDREF_PP(z_routes_by_id);
        zend_update_property(ce_pux_mux, return_value, "routesById", sizeof("routesById")-1, *z_routes_by_id TSRMLS_CC);
    }

    if ( zend_hash_quick_find(Z_ARRVAL_P(z_array), "submux", sizeof("submux"), zend_inline_hash_func(ZEND_STRS("submux")), (void**)&z_submux) == SUCCESS ) {
        Z_ADDREF_PP(z_submux);
        zend_update_property(ce_pux_mux, return_value, "submux", sizeof("submux")-1, *z_submux TSRMLS_CC);
    }

    if ( zend_hash_quick_find(Z_ARRVAL_P(z_array), "expand", sizeof("expand"), zend_inline_hash_func(ZEND_STRS("expand")), (void**)&z_expand) == SUCCESS ) {
        zend_update_property(ce_pux_mux, return_value, "expand", sizeof("expand")-1, *z_expand TSRMLS_CC);
    }
}


/**
 * get_mux_function_entry("method", sizeof("method"), zend_inline_hash_func(ZEND_STRS("pattern")));
 */
inline zend_function * get_mux_function_entry(char * method_name, int method_name_len, ulong h) {
    zend_function *fe;
    if ( zend_hash_quick_find( &ce_pux_mux->function_table, method_name, method_name_len, h, (void **) &fe) == SUCCESS ) {
        return fe;
    }
    php_error(E_ERROR, "%s method not found", method_name);
    return NULL;
}

inline zval * call_mux_method(zval * object , char * method_name , int method_name_len, int param_count, zval* arg1, zval* arg2, zval* arg3 TSRMLS_DC)
{
    zend_function *fe;
    if ( zend_hash_find( &ce_pux_mux->function_table, method_name, method_name_len, (void **) &fe) == FAILURE ) {
        php_error(E_ERROR, "%s method not found", method_name);
    }
    // call export method
    zval *z_retval = NULL;
    zend_call_method_with_3_params( &object, ce_pux_mux, &fe, method_name, method_name_len, &z_retval, param_count, arg1, arg2, arg3 TSRMLS_CC );
    return z_retval;
}


PHP_METHOD(Mux, get) {
    zval *z_pattern = NULL, *z_callback = NULL, *z_options = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|a", &z_pattern, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }
    if ( z_options == NULL ) {
        MAKE_STD_ZVAL(z_options);
        array_init_size(z_options, 1);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init_size(z_options, 1);
    }

    // $options['method'] = REQUEST_METHOD_GET;
    add_assoc_long(z_options, "method", REQUEST_METHOD_GET);

    // $this->add($pattern, $callback, $options);
    zval * z_retval = call_mux_method( this_ptr, "add" , sizeof("add"), 3 , z_pattern, z_callback, z_options TSRMLS_CC);
    if ( z_retval ) {
        *return_value = *z_retval;
        zval_copy_ctor(return_value);
    }
    // zval_ptr_dtor(&z_retval);
}





PHP_METHOD(Mux, put) {
    zval *z_pattern = NULL, *z_callback = NULL, *z_options = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|a", &z_pattern, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }
    if ( z_options == NULL ) {
        MAKE_STD_ZVAL(z_options);
        array_init_size(z_options, 1);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init_size(z_options, 1);
    }

    add_assoc_long(z_options, "method", REQUEST_METHOD_PUT);

    // $this->add($pattern, $callback, $options);
    zval * z_retval = call_mux_method( getThis(), "add" , sizeof("add"), 3 , z_pattern, z_callback, z_options TSRMLS_CC);
    if ( z_retval ) {
        *return_value = *z_retval;
        zval_copy_ctor(return_value);
    }
    // zval_ptr_dtor(&z_retval);
}

PHP_METHOD(Mux, delete) {
    zval *z_pattern = NULL, *z_callback = NULL, *z_options = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|a", &z_pattern, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }
    if ( z_options == NULL ) {
        MAKE_STD_ZVAL(z_options);
        array_init_size(z_options, 1);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init_size(z_options, 1);
    }

    add_assoc_long(z_options, "method", REQUEST_METHOD_DELETE);

    // $this->add($pattern, $callback, $options);
    zval * z_retval = call_mux_method( getThis(), "add" , sizeof("add"), 3 , z_pattern, z_callback, z_options TSRMLS_CC);
    if ( z_retval ) {
        *return_value = *z_retval;
        zval_copy_ctor(return_value);
    }
    // zval_ptr_dtor(&z_retval);
}


PHP_METHOD(Mux, post) {
    zval *z_pattern = NULL, *z_callback = NULL, *z_options = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|a", &z_pattern, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }
    if ( z_options == NULL ) {
        MAKE_STD_ZVAL(z_options);
        array_init_size(z_options, 1);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init_size(z_options, 1);
    }

    add_assoc_long(z_options, "method", REQUEST_METHOD_POST);

    // $this->add($pattern, $callback, $options);
    zval * z_retval = call_mux_method( getThis(), "add" , sizeof("add"), 3 , z_pattern, z_callback, z_options TSRMLS_CC);
    if ( z_retval ) {
        *return_value = *z_retval;
        zval_copy_ctor(return_value);
    }
}


PHP_METHOD(Mux, patch) {
    zval *z_pattern = NULL, *z_callback = NULL, *z_options = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|a", &z_pattern, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }
    if ( z_options == NULL ) {
        MAKE_STD_ZVAL(z_options);
        array_init_size(z_options, 1);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init_size(z_options, 1);
    }

    add_assoc_long(z_options, "method", REQUEST_METHOD_PATCH);

    // $this->add($pattern, $callback, $options);
    zval * z_retval = call_mux_method( getThis(), "add" , sizeof("add"), 3 , z_pattern, z_callback, z_options TSRMLS_CC);
    if ( z_retval ) {
        *return_value = *z_retval;
        zval_copy_ctor(return_value);
    }
}

PHP_METHOD(Mux, head) {
    zval *z_pattern = NULL, *z_callback = NULL, *z_options = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|a", &z_pattern, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }
    if ( z_options == NULL ) {
        MAKE_STD_ZVAL(z_options);
        array_init_size(z_options, 1);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init_size(z_options, 1);
    }

    add_assoc_long(z_options, "method", REQUEST_METHOD_HEAD);

    // $this->add($pattern, $callback, $options);
    zval * z_retval = call_mux_method( getThis(), "add" , sizeof("add"), 3 , z_pattern, z_callback, z_options TSRMLS_CC);
    if ( z_retval ) {
        *return_value = *z_retval;
        zval_copy_ctor(return_value);
    }
}

PHP_METHOD(Mux, options) {
    zval *z_pattern = NULL, *z_callback = NULL, *z_options = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|a", &z_pattern, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }
    if ( z_options == NULL ) {
        MAKE_STD_ZVAL(z_options);
        array_init_size(z_options, 1);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init_size(z_options, 1);
    }

    add_assoc_long(z_options, "method", REQUEST_METHOD_OPTIONS);

    // $this->add($pattern, $callback, $options);
    zval * z_retval = call_mux_method( getThis(), "add" , sizeof("add"), 3 , z_pattern, z_callback, z_options TSRMLS_CC);
    if ( z_retval ) {
        *return_value = *z_retval;
        zval_copy_ctor(return_value);
    }
}


PHP_METHOD(Mux, mount) {
    char *pattern;
    int  pattern_len;

    zval *z_mux = NULL;
    zval *z_options = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|a", &pattern, &pattern_len, &z_mux, &z_options) == FAILURE) {
        RETURN_FALSE;
    }
    if ( Z_TYPE_P(z_mux) == IS_NULL ) {
        RETURN_FALSE;
    }

    if ( Z_TYPE_P(z_mux) == IS_OBJECT ) {
        if ( instanceof_function( Z_OBJCE_P(z_mux), ce_pux_controller TSRMLS_CC) ) {
            zval *rv;
            zend_call_method(&z_mux, ce_pux_controller, NULL, "expand", sizeof("expand")-1, &rv, 0, NULL, NULL TSRMLS_CC );
            z_mux = rv;
        }
    }


    if ( z_options == NULL ) {
        MAKE_STD_ZVAL(z_options); // ref = 1
        array_init(z_options);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init(z_options);
        INIT_PZVAL(z_options); // set ref = 1
    }


    zend_class_entry **ce_pattern_compiler = get_pattern_compiler_ce(TSRMLS_C);
    if ( ce_pattern_compiler == NULL ) {
        RETURN_FALSE;
    }

    zval *z_routes;
    zval *z_expand;
    zval *z_mux_routes;

    z_routes = zend_read_property( ce_pux_mux, getThis(), "routes", sizeof("routes")-1, 1 TSRMLS_CC);
    z_expand = zend_read_property( ce_pux_mux, getThis(), "expand", sizeof("expand")-1, 1 TSRMLS_CC);

    // TODO: merge routesById and staticRoutes properties


    if ( Z_BVAL_P(z_expand) ) {
        // fetch routes from $mux
        z_mux_routes = zend_read_property( ce_pux_mux, z_mux, "routes", sizeof("routes")-1, 1 TSRMLS_CC);

        HashPosition route_pointer;
        zval **z_mux_route;

        // iterate mux
        for(zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(z_mux_routes), &route_pointer); 
                zend_hash_get_current_data_ex(Z_ARRVAL_P(z_mux_routes), (void**) &z_mux_route, &route_pointer) == SUCCESS; 
                zend_hash_move_forward_ex(Z_ARRVAL_P(z_mux_routes), &route_pointer)) 
        {
            // zval for new route
            zval *z_new_routes;

            // zval for route item
            zval **z_is_pcre; // route[0]
            zval **z_route_pattern;
            zval **z_route_callback;
            zval **z_route_options;
            zval **z_route_original_pattern; // for PCRE pattern

            if ( zend_hash_index_find( Z_ARRVAL_PP(z_mux_route), 0, (void**) &z_is_pcre) == FAILURE ) {
                zend_hash_move_forward_ex(Z_ARRVAL_P(z_mux_routes), &route_pointer);
                continue;
            }
            if ( zend_hash_index_find( Z_ARRVAL_PP(z_mux_route), 1, (void**) &z_route_pattern) == FAILURE ) {
                zend_hash_move_forward_ex(Z_ARRVAL_P(z_mux_routes), &route_pointer);
                continue;
            }
            if ( zend_hash_index_find( Z_ARRVAL_PP(z_mux_route), 2, (void**) &z_route_callback) == FAILURE ) {
                zend_hash_move_forward_ex(Z_ARRVAL_P(z_mux_routes), &route_pointer);
                continue;
            }
            if ( zend_hash_index_find( Z_ARRVAL_PP(z_mux_route), 3, (void**) &z_route_options) == FAILURE ) {
                zend_hash_move_forward_ex(Z_ARRVAL_P(z_mux_routes), &route_pointer);
                continue;
            }

            MAKE_STD_ZVAL(z_new_routes);
            array_init(z_new_routes);

            if ( Z_BVAL_PP(z_is_pcre) ) {
                // $newPattern = $pattern . $route[3]['pattern'];

                if ( zend_hash_quick_find( Z_ARRVAL_PP(z_route_options), "pattern", sizeof("pattern"), zend_inline_hash_func(ZEND_STRS("pattern")), (void**) &z_route_original_pattern) == FAILURE ) {
                    php_error( E_ERROR, "Can not compile pattern, original pattern not found");
                }

                char *new_pattern = NULL;
                int  new_pattern_len;

                // php_printf("%s (%d)\n", pattern, pattern_len);

                new_pattern_len = pattern_len + Z_STRLEN_PP(z_route_original_pattern);
                new_pattern = (char*) ecalloc( sizeof(char), ( pattern_len +  Z_STRLEN_PP(z_route_original_pattern) ) );

                strncat( new_pattern, pattern , pattern_len );
                strncat( new_pattern, Z_STRVAL_PP(z_route_original_pattern) , Z_STRLEN_PP(z_route_original_pattern) );


                zval *z_new_pattern = NULL;
                MAKE_STD_ZVAL(z_new_pattern);
                ZVAL_STRINGL(z_new_pattern, new_pattern, new_pattern_len, 0);


                // TODO: merge options

                // $routeArgs = PatternCompiler::compile($newPattern, 
                //     array_merge_recursive($route[3], $options) );
                zval *z_compiled_route = compile_route_pattern(z_new_pattern, *z_route_options, ce_pattern_compiler TSRMLS_CC);
                if ( z_compiled_route == NULL || Z_TYPE_P(z_compiled_route) == IS_NULL ) {
                    php_error( E_ERROR, "Cannot compile pattern: %s", new_pattern);
                }


                zval **z_compiled_route_pattern;
                if ( zend_hash_quick_find( Z_ARRVAL_P(z_compiled_route) , "compiled", sizeof("compiled"), zend_inline_hash_func(ZEND_STRS("compiled")), (void**)&z_compiled_route_pattern) == FAILURE ) {
                    php_error( E_ERROR, "compiled pattern not found: %s", new_pattern);
                }
                Z_ADDREF_PP(z_compiled_route_pattern);


                zend_hash_quick_update( Z_ARRVAL_P(z_compiled_route), "pattern", sizeof("pattern"), zend_inline_hash_func(ZEND_STRS("pattern")), &z_new_pattern, sizeof(zval *), NULL);

                Z_ADDREF_PP(z_route_callback);

                // create new route and append to mux->routes
                add_index_bool(z_new_routes, 0 , 1); // pcre flag == false
                add_index_zval(z_new_routes, 1, *z_compiled_route_pattern);
                add_index_zval(z_new_routes, 2, *z_route_callback);
                add_index_zval(z_new_routes, 3, z_compiled_route);
                add_next_index_zval(z_routes, z_new_routes);

            } else {

                //  $this->routes[] = array(
                //      false,
                //      $pattern . $route[1],
                //      $route[2],
                //      $options,
                //  );
                char *new_pattern = NULL;
                int  new_pattern_len;

                new_pattern_len = pattern_len + Z_STRLEN_PP(z_route_pattern);
                new_pattern = (char*) ecalloc( sizeof(char), new_pattern_len );

                strncat( new_pattern, pattern , pattern_len );
                strncat( new_pattern, Z_STRVAL_PP(z_route_pattern) , Z_STRLEN_PP(z_route_pattern) );

                // Merge the mount options with the route options
                zval *z_new_route_options;
                MAKE_STD_ZVAL(z_new_route_options);
                array_init(z_new_route_options);
                php_array_merge(Z_ARRVAL_P(z_new_route_options), Z_ARRVAL_P(z_options), 0 TSRMLS_CC);
                php_array_merge(Z_ARRVAL_P(z_new_route_options), Z_ARRVAL_P(*z_route_options), 0 TSRMLS_CC);

                Z_ADDREF_PP(z_route_callback);

                // make the array: [ pcreFlag, pattern, callback, options ]
                add_index_bool(z_new_routes, 0 , 0); // pcre flag == false
                add_index_stringl(z_new_routes, 1 , new_pattern , new_pattern_len, 0);
                add_index_zval( z_new_routes, 2 , *z_route_callback);
                add_index_zval( z_new_routes, 3 , z_new_route_options);
                add_next_index_zval(z_routes, z_new_routes);
            }
        }
        return;
    }

    zend_function *fe_getid = NULL; // method entry
    zend_function *fe_add   = NULL; // method entry

    if ( zend_hash_quick_find(&ce_pux_mux->function_table, "getid", sizeof("getid"), zend_inline_hash_func(ZEND_STRS("getid")), (void **) &fe_getid) == FAILURE ) {
        php_error(E_ERROR, "Cannot call method Mux::getid()");
        RETURN_FALSE;
    }
    if ( zend_hash_quick_find(&ce_pux_mux->function_table, "add", sizeof("add"), zend_inline_hash_func(ZEND_STRS("add")), (void **) &fe_add) == FAILURE ) {
        php_error(E_ERROR, "Cannot call method Mux::add()");
        RETURN_FALSE;
    }

    // $muxId = $mux->getId();
    // $this->add($pattern, $muxId, $options);
    // $this->submux[ $muxId ] = $mux;
    // zval *z_mux_id = call_mux_method(z_mux, "getid", sizeof("getid") ,  );

    zval *z_mux_id = NULL;
    zend_call_method( &z_mux, ce_pux_mux, &fe_getid, "getid", sizeof("getid")-1, &z_mux_id, 0, NULL, NULL TSRMLS_CC );

    if ( z_mux_id == NULL || Z_TYPE_P(z_mux_id) == IS_NULL ) {
        php_error(E_ERROR, "Mux id is required. got NULL.");
    }

    // create pattern
    zval *z_pattern = NULL;
    MAKE_STD_ZVAL(z_pattern);
    ZVAL_STRINGL(z_pattern, pattern, pattern_len, 1); // duplicate

    zval *z_retval = NULL;
    zend_call_method_with_3_params( &this_ptr, ce_pux_mux, &fe_add, "add", sizeof("add")-1, &z_retval, 3, z_pattern, z_mux_id, z_options TSRMLS_CC);

    zval_ptr_dtor(&z_pattern);



    zval *z_submux_array = zend_read_property( ce_pux_mux, this_ptr , "submux", sizeof("submux") - 1, 1 TSRMLS_CC);
    add_index_zval(z_submux_array, Z_LVAL_P(z_mux_id) , z_mux);
    Z_ADDREF_P(z_mux); // add reference since we add the mux to submux array
}

PHP_METHOD(Mux, getSubMux) {
    long submux_id = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &submux_id ) == FAILURE) {
        RETURN_FALSE;
    }

    zval *z_submux_array;
    zval **z_submux;
    z_submux_array = zend_read_property( Z_OBJCE_P(this_ptr), this_ptr, "submux", sizeof("submux")-1, 1 TSRMLS_CC);

    if ( zend_hash_index_find( Z_ARRVAL_P(z_submux_array),  submux_id , (void**) &z_submux) == SUCCESS ) {
        *return_value = **z_submux;
        zval_copy_ctor(return_value);
        INIT_PZVAL(return_value);
        return;
    }
    RETURN_FALSE;
}


PHP_METHOD(Mux, getRequestMethodConstant) {
    char *req_method        = NULL, *mthit, *mthp;
    long req_method_const   = 0;
    long req_method_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &req_method, &req_method_len) != FAILURE) {
        mthit = mthp = estrndup(req_method, req_method_len);
        while(( *mthit = toupper(*mthit))) mthit++;

        if (strcmp(mthp, "GET") == 0) {
            req_method_const = REQUEST_METHOD_GET;
        } else if (strcmp(mthp, "POST") == 0) {
            req_method_const = REQUEST_METHOD_POST;
        } else if (strcmp(mthp, "PUT") == 0) {
            req_method_const = REQUEST_METHOD_PUT;
        } else if (strcmp(mthp, "DELETE") == 0) {
            req_method_const = REQUEST_METHOD_DELETE;
        } else if (strcmp(mthp, "HEAD") == 0) {
            req_method_const = REQUEST_METHOD_HEAD;
        } else if (strcmp(mthp, "OPTIONS") == 0) {
            req_method_const = REQUEST_METHOD_OPTIONS;
        } else if (strcmp(mthp, "PATCH") == 0) {
            req_method_const = REQUEST_METHOD_PATCH;
        }

        efree(req_method);
    }

    RETURN_LONG(req_method_const);
}

PHP_METHOD(Mux, getRoute) {
    char * route_id = NULL;
    int    route_id_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &route_id, &route_id_len ) == FAILURE) {
        RETURN_FALSE;
    }

    zval *z_routes_by_id = NULL;
    zval **z_route = NULL;
    z_routes_by_id = zend_read_property( ce_pux_mux , this_ptr, "routesById", sizeof("routesById")-1, 1 TSRMLS_CC);

    // php_var_dump(&z_routes_by_id, 1 TSRMLS_CC);
    if ( zend_hash_find( Z_ARRVAL_P(z_routes_by_id) , route_id, route_id_len + 1, (void**) &z_route ) == SUCCESS ) {
        *return_value = **z_route;
        zval_copy_ctor(return_value);
        INIT_PZVAL(return_value);
        return;
    }
    RETURN_NULL();
}

PHP_METHOD(Mux, setRoutes) {
    zval * routes;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &routes ) == FAILURE) {
        RETURN_FALSE;
    }
    Z_ADDREF_P(routes);
    zend_update_property(ce_pux_mux , this_ptr, "routes", sizeof("routes")-1, routes TSRMLS_CC);
    RETURN_TRUE;
}

PHP_METHOD(Mux, getRoutes) {
    zval *z_routes;
    z_routes = zend_read_property(ce_pux_mux, this_ptr, "routes", sizeof("routes")-1, 1 TSRMLS_CC);
    *return_value = *z_routes;
    zval_copy_ctor(return_value);
    INIT_PZVAL(return_value);
}

PHP_METHOD(Mux, export) {
    smart_str buf = {0};
    php_var_export_ex(&this_ptr, 0, &buf TSRMLS_CC);
    smart_str_0 (&buf);
    RETVAL_STRINGL(buf.c, buf.len, 0);
}

PHP_METHOD(Mux, getId) {
    zval *z_id;
    long counter = 0;

    z_id = zend_read_property(ce_pux_mux, getThis(), "id", sizeof("id")-1, 1 TSRMLS_CC);

    if ( z_id != NULL && Z_TYPE_P(z_id) != IS_NULL ) {
        RETURN_LONG( Z_LVAL_P(z_id) );
    }

    zval *rv = NULL;
    zend_call_method( NULL, ce_pux_mux, NULL, "generate_id", sizeof("generate_id")-1, &rv, 0, NULL, NULL TSRMLS_CC );

    if ( rv ) {
        counter = Z_LVAL_P(rv);
        zend_update_property_long(ce_pux_mux, this_ptr, "id" , sizeof("id") - 1, counter TSRMLS_CC);
        zval_ptr_dtor(&rv);
    }
    RETURN_LONG(counter);
}

PHP_METHOD(Mux, length) {
    zval *z_routes;
    z_routes = zend_read_property(ce_pux_mux, this_ptr, "routes", sizeof("routes")-1, 1 TSRMLS_CC);
    long length = zend_hash_num_elements( Z_ARRVAL_P(z_routes) );
    RETURN_LONG(length);
}

PHP_METHOD(Mux, sort) {
    zval *z_routes;
    z_routes = zend_read_property(Z_OBJCE_P(this_ptr) , this_ptr, "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    zval *retval_ptr = NULL;

    zval *z_sort_callback = NULL;
    MAKE_STD_ZVAL(z_sort_callback);
    ZVAL_STRING( z_sort_callback, "pux_sort_routes" , 1 );

    Z_SET_ISREF_P(z_routes);
    zend_call_method( NULL, NULL, NULL, "usort", sizeof("usort")-1, &retval_ptr, 2, 
            z_routes, z_sort_callback TSRMLS_CC );
    zval_ptr_dtor(&z_sort_callback);
    if (retval_ptr) {
        zval_ptr_dtor(&retval_ptr);
    }
}

PHP_METHOD(Mux, compile) {
    char *filename;
    int  filename_len;
    zend_bool sort_before_compile = 1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &filename, &filename_len, &sort_before_compile) == FAILURE) {
        RETURN_FALSE;
    }

    if (sort_before_compile) {
        zval *z_routes;
        z_routes = zend_read_property(ce_pux_mux, this_ptr, "routes", sizeof("routes")-1, 1 TSRMLS_CC);

        Z_SET_ISREF_P(z_routes);
        // duplicated code to sort method
        zval *rv = NULL;
        zval *z_sort_callback = NULL;
        MAKE_STD_ZVAL(z_sort_callback);
        ZVAL_STRING( z_sort_callback, "pux_sort_routes" , 1 );

        zend_call_method( NULL, NULL, NULL, "usort", sizeof("usort")-1, &rv, 2, 
                z_routes, z_sort_callback TSRMLS_CC );
        zval_ptr_dtor(&z_sort_callback); // recycle sort callback zval
        // php_error(E_ERROR,"route sort failed.");
        // zend_update_property(ce_pux_mux, getThis(), "routes", sizeof("routes")-1, z_routes TSRMLS_CC);
    }

    // $code = '<?php return ' . $this->export() . ';';

    // get export method function entry
    zend_function *fe_export;
    if ( zend_hash_quick_find( &Z_OBJCE_P(this_ptr)->function_table, "export", sizeof("export"), zend_inline_hash_func(ZEND_STRS("export")),  (void **) &fe_export) == FAILURE ) {
        php_error(E_ERROR, "export method not found");
    }

    // call export method
    zval *compiled_code = NULL;
    zend_call_method( &this_ptr, Z_OBJCE_P(this_ptr) , &fe_export, "export", sizeof("export")-1, &compiled_code, 0, NULL, NULL TSRMLS_CC );

    if ( compiled_code == NULL || Z_TYPE_P(compiled_code) == IS_NULL ) {
        php_error(E_ERROR, "Cannot compile routes.");
    }


    int  buf_len = Z_STRLEN_P(compiled_code) + strlen("<?php return ;") + 1;
    char *buf = (char* ) ecalloc(buf_len, sizeof(char));
    strncat(buf, "<?php return ", strlen("<?php return ") );
    strncat(buf, Z_STRVAL_P(compiled_code), Z_STRLEN_P(compiled_code));
    strncat(buf, ";", 1);
    *(buf + buf_len - 1) = '\0';


    // memcpytrncat(buf, "\0", 1);
    zval *z_code = NULL;
    zval *z_filename = NULL;
    zval *retval = NULL;
    MAKE_STD_ZVAL(z_code);
    MAKE_STD_ZVAL(z_filename);
    ZVAL_STRING(z_code, buf, 1);
    // CHECK_ZVAL_STRING(z_code);
    ZVAL_STRINGL(z_filename, filename, filename_len, 1);

    zend_call_method( NULL, NULL, NULL, "file_put_contents", sizeof("file_put_contents")-1, &retval, 2, z_filename, z_code TSRMLS_CC );
    zval_ptr_dtor(&z_filename);
    zval_ptr_dtor(&z_code);
    zval_ptr_dtor(&compiled_code);

    if (retval) {
        *return_value = *retval;
        zval_copy_ctor(return_value);
        INIT_PZVAL(return_value);
    }
}

PHP_METHOD(Mux, dispatch) {
    char *path;
    int  path_len;
    zval *z_path;

    zval *z_return_route = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len) == FAILURE) {
        RETURN_FALSE;
    }

    MAKE_STD_ZVAL(z_path);
    ZVAL_STRINGL(z_path, path, path_len, 1);

    zend_function *fe; // method entry
    zend_hash_quick_find( &ce_pux_mux->function_table, "match",    sizeof("match"), zend_inline_hash_func(ZEND_STRS("match")),    (void **) &fe);
    zend_call_method( &this_ptr, ce_pux_mux, &fe, "match", sizeof("match")-1, &z_return_route, 1, z_path, NULL TSRMLS_CC );

    if ( ! z_return_route || Z_TYPE_P(z_return_route) == IS_NULL ) {
        zval_ptr_dtor(&z_path);
        RETURN_NULL();
    }


    // read data from matched route
    zval **z_pcre;
    zval **z_pattern;
    zval **z_callback;
    zval **z_options;

    zend_hash_index_find( Z_ARRVAL_P(z_return_route) , 0 , (void**) &z_pcre );
    zend_hash_index_find( Z_ARRVAL_P(z_return_route) , 1 , (void**) &z_pattern );
    zend_hash_index_find( Z_ARRVAL_P(z_return_route) , 2 , (void**) &z_callback );
    zend_hash_index_find( Z_ARRVAL_P(z_return_route) , 3 , (void**) &z_options );

    zval *z_submux_array = NULL;
    zval **z_submux = NULL;

    zval *z_retval = NULL;

    // dispatch to submux if the callback is an ID.
    if ( Z_TYPE_PP(z_callback) == IS_LONG ) {
        z_submux_array = zend_read_property( ce_pux_mux, this_ptr, "submux", sizeof("submux")-1, 1 TSRMLS_CC);

        if ( z_submux_array == NULL ) {
            zend_throw_exception(ce_pux_exception, "submux property is null", 0 TSRMLS_CC);
            return;
        }

        if ( zend_hash_index_find( Z_ARRVAL_P(z_submux_array),  Z_LVAL_PP(z_callback) , (void**) &z_submux) == FAILURE ) {
            zend_throw_exception(ce_pux_exception, "submux not found", 0 TSRMLS_CC);
            return;
        }


        if ( z_submux == NULL || *z_submux == NULL ) {
            zend_throw_exception(ce_pux_exception, "submux not found", 0 TSRMLS_CC);
            return;
        }

        // php_var_dump(z_submux, 1 TSRMLS_CC);

        //  $matchedString = $route[3]['vars'][0];
        //  return $submux->dispatch(substr($path, strlen($matchedString))
        if ( Z_BVAL_PP(z_pcre) ) {
            zval **z_route_vars = NULL;
            zval **z_route_vars_0 = NULL;
            zval *z_substr;

            if ( zend_hash_quick_find( Z_ARRVAL_PP(z_options) , "vars", sizeof("vars"), zend_inline_hash_func(ZEND_STRS("vars")), (void**) &z_route_vars ) == FAILURE ) {
                php_error(E_ERROR, "require route vars");
                RETURN_FALSE;
            }
            if ( zend_hash_index_find( Z_ARRVAL_PP(z_options) , 0 , (void**) &z_route_vars_0 ) == FAILURE ) {
                php_error(E_ERROR, "require route vars[0]");
                RETURN_FALSE;
            }

            MAKE_STD_ZVAL(z_substr);
            ZVAL_STRING(z_substr, path + Z_STRLEN_PP(z_route_vars_0), 1);

            z_retval = call_mux_method( *z_submux, "dispatch" , sizeof("dispatch"), 1 , z_substr, NULL, NULL TSRMLS_CC);
            zval_ptr_dtor(&z_substr);

            if (z_retval) {
                *return_value = *z_retval;
                zval_copy_ctor(return_value);
                INIT_PZVAL(return_value);

                zval_ptr_dtor(&z_path);
                zval_ptr_dtor(&z_return_route);
                return;
            }

            zval_ptr_dtor(&z_path);
            zval_ptr_dtor(&z_return_route);
            RETURN_FALSE;

        } else {
            zval *z_substr;

            MAKE_STD_ZVAL(z_substr);
            ZVAL_STRING(z_substr, path + Z_STRLEN_PP(z_pattern), 1);

            //     return $submux->dispatch(
            //         substr($path, strlen($route[1]))
            //     );

            z_retval = call_mux_method( *z_submux, "dispatch" , sizeof("dispatch"), 1 , z_substr, NULL, NULL TSRMLS_CC);
            zval_ptr_dtor(&z_substr);

            if ( z_retval ) {
                *return_value = *z_retval;
                zval_copy_ctor(return_value);
                INIT_PZVAL(return_value);

                zval_ptr_dtor(&z_path);
                zval_ptr_dtor(&z_return_route);
                return;
            }

            zval_ptr_dtor(&z_path);
            zval_ptr_dtor(&z_return_route);
            return;
        }
    }

    if ( z_return_route ) {
        *return_value = *z_return_route;
        zval_copy_ctor(return_value);
    }
    zval_ptr_dtor(&z_path);
    zval_ptr_dtor(&z_return_route);
    return;
}

PHP_METHOD(Mux, match) {
    char *path;
    int  path_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len) == FAILURE) {
        RETURN_FALSE;
    }

    zval **z_route_pp = NULL;
    zval *z_route = NULL;
    // XXX: not to match static routes for now because we might have different methods for the same url
    /*
    if ( zend_hash_find( Z_ARRVAL_P( zend_read_property(ce_pux_mux, this_ptr, "staticRoutes", sizeof("staticRoutes") - 1, 1 TSRMLS_CC) ), path, path_len, (void**)&z_route_pp) == SUCCESS ) {
        if ( Z_TYPE_PP(z_route_pp) != IS_NULL ) {
            *return_value = **z_route_pp;
            Z_ADDREF_PP(z_route_pp);
            zval_copy_ctor(return_value);
            return;
        }
    }
    */

    z_route = php_pux_match(zend_read_property(ce_pux_mux , this_ptr , "routes", sizeof("routes")-1, 1 TSRMLS_CC), path, path_len TSRMLS_CC);
    if ( z_route != NULL ) {
        *return_value = *z_route;
        zval_copy_ctor(return_value);
        INIT_PZVAL(return_value);
        return;
    }
    RETURN_NULL();
}

PHP_METHOD(Mux, appendRoute) {
    char *pattern;
    int  pattern_len;
    zval *z_callback;
    zval *z_options;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa|a", &pattern, &pattern_len, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }

    zval *z_routes;
    zval *z_new_routes;

    if ( z_options == NULL ) {
        MAKE_STD_ZVAL(z_options);
        array_init(z_options);
    }
    if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init(z_options);
    }

    z_routes = zend_read_property(Z_OBJCE_P(this_ptr), this_ptr, "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    MAKE_STD_ZVAL(z_new_routes);
    array_init_size(z_new_routes, 4);

    add_index_bool(z_new_routes, 0 , 0); // pcre flag == false
    add_index_stringl( z_new_routes, 1 , pattern , pattern_len, 1);
    add_index_zval( z_new_routes, 2 , z_callback);
    add_index_zval( z_new_routes, 3 , z_options);

    add_next_index_zval(z_routes, z_new_routes);
}


PHP_METHOD(Mux, appendPCRERoute) {
    char *pattern;
    int  pattern_len;
    zval *z_callback;
    zval *z_options;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa|a", &pattern, &pattern_len, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }

    if ( z_options == NULL ) {
        MAKE_STD_ZVAL(z_options);
        array_init(z_options);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init(z_options);
    }

    zval *z_pattern = NULL;
    zval *z_routes;

    MAKE_STD_ZVAL(z_pattern);
    ZVAL_STRINGL(z_pattern, pattern, pattern_len, 1);

    z_routes = zend_read_property(Z_OBJCE_P(this_ptr), getThis(), "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    zend_class_entry **ce_pattern_compiler = get_pattern_compiler_ce(TSRMLS_C);
    if ( ce_pattern_compiler == NULL ) {
        RETURN_FALSE;
    }

    zval *rv = NULL;
    zend_call_method( NULL, *ce_pattern_compiler, NULL, "compile", strlen("compile"), &rv, 1, z_pattern, NULL TSRMLS_CC );

    if ( rv == NULL || Z_TYPE_P(rv) == IS_NULL ) {
        zend_throw_exception(ce_pux_exception, "Can not compile route pattern", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    add_next_index_zval(z_routes, rv);
}


inline void mux_add_route(INTERNAL_FUNCTION_PARAMETERS)
{
    char *pattern;
    int  pattern_len;

    zval *z_callback = NULL;
    zval *z_options = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|a", &pattern, &pattern_len, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }

    // $pcre = strpos($pattern,':') !== false;
    char *is_pcre = find_place_holder(pattern, pattern_len);


    if ( z_options == NULL ) {
        MAKE_STD_ZVAL(z_options); // ref =1 
        array_init(z_options);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        // make it as an array
        array_init(z_options);
        INIT_PZVAL(z_options); // ref = 1
    }

    // Generalize callback variable
    if ( Z_TYPE_P(z_callback) == IS_STRING ) {
        if ( strpos( Z_STRVAL_P(z_callback), ":" ) != -1 ) {
            zval *delim;
            MAKE_STD_ZVAL(delim);
            ZVAL_STRINGL(delim, ":" , 1 , 1);

            zval *rv;
            MAKE_STD_ZVAL(rv);
            array_init(rv);
            php_explode(delim, z_callback, rv, 2);

            *z_callback = *rv;
            zval_copy_ctor(z_callback);
            zval_ptr_dtor(&delim);
        }
    }


    zval * z_routes = zend_read_property(ce_pux_mux, this_ptr, "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    // we're going to append the route into the routes array, so add the ref here
    Z_ADDREF_P(z_callback);


    // PCRE pattern here
    if ( is_pcre ) {
        zval *z_pattern = NULL;
        MAKE_STD_ZVAL(z_pattern);
        ZVAL_STRINGL(z_pattern, pattern, pattern_len, 1);

        zval *z_compiled_route = compile_route_pattern(z_pattern, z_options, NULL TSRMLS_CC);
        if ( z_compiled_route == NULL ) {
            zend_throw_exception(ce_pux_exception, "Unable to compile route pattern.", 0 TSRMLS_CC);
            RETURN_FALSE;
        }

        zval **z_compiled_route_pattern;
        if ( zend_hash_quick_find( Z_ARRVAL_P(z_compiled_route) , "compiled", sizeof("compiled"), zend_inline_hash_func(ZEND_STRS("compiled")), (void**)&z_compiled_route_pattern) == FAILURE ) {
            zend_throw_exception(ce_pux_exception, "Unable to find compiled pattern.", 0 TSRMLS_CC);
            RETURN_FALSE;
        }
        Z_ADDREF_PP(z_compiled_route_pattern);

        zval *z_new_routes;
        MAKE_STD_ZVAL(z_new_routes);
        array_init_size(z_new_routes, 4);

        zval_ptr_dtor(&z_pattern);

        add_index_bool(z_new_routes,0 , 1); // pcre flag == false
        add_index_zval(z_new_routes,1 , *z_compiled_route_pattern);
        add_index_zval(z_new_routes,2 , z_callback);
        add_index_zval(z_new_routes,3 , z_compiled_route);
        add_next_index_zval(z_routes, z_new_routes);

        zval **z_route_id;
        if ( zend_hash_quick_find( Z_ARRVAL_P(z_options) , "id", sizeof("id"), zend_inline_hash_func(ZEND_STRS("id")), (void**)&z_route_id) == SUCCESS ) {
            zval * z_routes_by_id = zend_read_property(ce_pux_mux, this_ptr, "routesById", sizeof("routesById")-1, 1 TSRMLS_CC);
            add_assoc_zval(z_routes_by_id, Z_STRVAL_PP(z_route_id), z_new_routes);
        }

    } else {

        zval *z_new_route;
        MAKE_STD_ZVAL(z_new_route);
        array_init_size(z_new_route, 4);

        Z_ADDREF_P(z_options);

        /* make the array: [ pcreFlag, pattern, callback, options ] */
        add_index_bool(z_new_route, 0 , 0); // pcre flag == false
        add_index_stringl( z_new_route, 1 , pattern, pattern_len , 1 );
        add_index_zval( z_new_route, 2 , z_callback);
        add_index_zval( z_new_route, 3 , z_options);
        add_next_index_zval(z_routes, z_new_route);

        // if there is no option specified in z_options, we can add the route to our static route hash
        if ( zend_hash_num_elements(Z_ARRVAL_P(z_options)) ) {
            zval * z_static_routes = zend_read_property(ce_pux_mux, this_ptr, "staticRoutes", sizeof("staticRoutes")-1, 1 TSRMLS_CC);
            if ( z_static_routes ) {
                add_assoc_zval(z_static_routes, pattern, z_new_route);
            }
        }

        zval **z_route_id;
        if ( zend_hash_quick_find( Z_ARRVAL_P(z_options) , "id", sizeof("id"), zend_inline_hash_func(ZEND_STRS("id")), (void**)&z_route_id) == SUCCESS ) {
            zval * z_routes_by_id = zend_read_property(ce_pux_mux, this_ptr, "routesById", sizeof("routesById")-1, 1 TSRMLS_CC);
            add_assoc_zval(z_routes_by_id, Z_STRVAL_PP(z_route_id), z_new_route);
        }
    }
}


PHP_METHOD(Mux, add) {
    mux_add_route(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

PHP_METHOD(Mux, any) {
    mux_add_route(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

