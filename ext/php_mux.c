
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
#include "ct_helper.h"
#include "pux_functions.h"
#include "php_mux.h"

zend_class_entry *ce_pux_mux;

const zend_function_entry mux_methods[] = {
  PHP_ME(Mux, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
  // PHP_ME(Mux, __destruct,  NULL, ZEND_ACC_PUBLIC|ZEND_ACC_DTOR) 
  PHP_ME(Mux, getId, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, add, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, compile, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, sort, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, dispatch, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, length, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, mount, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, appendRoute, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, appendPCRERoute, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, matchRoute, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, getRoutes, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, getSubMux, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, export, NULL, ZEND_ACC_PUBLIC)

  PHP_ME(Mux, get, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, post, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, put, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, delete, NULL, ZEND_ACC_PUBLIC)

  PHP_ME(Mux, __set_state, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
  PHP_ME(Mux, generate_id, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
  PHP_FE_END
};

void pux_init_mux(TSRMLS_D) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Pux\\Mux", mux_methods);
    ce_pux_mux = zend_register_internal_class(&ce TSRMLS_CC);
    zend_declare_property_null(ce_pux_mux, "id", strlen("id"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(ce_pux_mux, "routes", strlen("routes"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(ce_pux_mux, "submux", strlen("submux"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_bool(ce_pux_mux, "expandSubMux", strlen("expandSubMux"), 1, ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_long(ce_pux_mux, "id_counter", strlen("id_counter"), 0, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC TSRMLS_CC);
}

zend_class_entry ** get_pattern_compiler_ce(TSRMLS_DC) {
    zend_class_entry **ce_pattern_compiler = NULL;
    if ( zend_lookup_class( "Pux\\PatternCompiler", strlen("Pux\\PatternCompiler") , &ce_pattern_compiler TSRMLS_CC) == FAILURE ) {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Class Pux\\PatternCompiler does not exist.", 0 TSRMLS_CC);
        return NULL;
    }
    if ( ce_pattern_compiler == NULL || *ce_pattern_compiler == NULL ) {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Class Pux\\PatternCompiler does not exist.", 0 TSRMLS_CC);
        return NULL;
    }
    return ce_pattern_compiler;
}

// Returns compiled route zval
zval * compile_route_pattern(zval *z_pattern, zval *z_options, zend_class_entry **ce_pattern_compiler TSRMLS_DC)
{
    // zend_class_entry **ce_pattern_compiler;
    if ( ce_pattern_compiler == NULL ) {
        ce_pattern_compiler = get_pattern_compiler_ce(TSRMLS_CC);
        if ( ce_pattern_compiler == NULL ) {
            return NULL;
        }
    }

    zval *z_compiled_route = NULL; // will be an array
    ALLOC_INIT_ZVAL(z_compiled_route);
    zend_call_method( NULL, *ce_pattern_compiler, NULL, "compile", strlen("compile"), &z_compiled_route, 2, z_pattern, z_options TSRMLS_CC );

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
    return z_compiled_route;
}



PHP_METHOD(Mux, __construct) {
    zval *z_routes = NULL , *z_submux = NULL;

    ALLOC_INIT_ZVAL(z_routes);
    ALLOC_INIT_ZVAL(z_submux);

    array_init(z_routes);
    array_init(z_submux);

    zend_update_property( Z_OBJCE_P(this_ptr), this_ptr, "routes", sizeof("routes")-1, z_routes TSRMLS_CC);
    zend_update_property( Z_OBJCE_P(this_ptr), this_ptr, "submux", sizeof("submux")-1, z_submux TSRMLS_CC);
}

PHP_METHOD(Mux, generate_id) {
    zval * z_counter = NULL;
    long counter = 0;
    z_counter = zend_read_static_property(ce_pux_mux, "id_counter", strlen("id_counter") , 0 TSRMLS_CC);
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

    HashTable *z_array_hash;
    z_array_hash = Z_ARRVAL_P(z_array);

    zval **z_id;
    zval **z_routes;
    zval **z_submux;
    zval **z_expandSubMux;

    if ( zend_hash_find(z_array_hash, "id", sizeof("id"), (void**)&z_id) == FAILURE ) {
        zend_throw_exception(ce_pux_exception, "mux->id load failed.", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    if ( zend_hash_find(z_array_hash, "routes", sizeof("routes"), (void**)&z_routes) == FAILURE ) {
        zend_throw_exception(ce_pux_exception, "mux->routes load failed.", 0 TSRMLS_CC);
        RETURN_FALSE;
    }

    if ( zend_hash_find(z_array_hash, "submux", sizeof("submux"), (void**)&z_submux) == FAILURE ) {
        zend_throw_exception(ce_pux_exception, "mux->submux load failed.", 0 TSRMLS_CC);
        RETURN_FALSE;
    }

    if ( zend_hash_find(z_array_hash, "expandSubMux", sizeof("expandSubMux"), (void**)&z_expandSubMux) == FAILURE ) {
        ALLOC_INIT_ZVAL(*z_expandSubMux);
        ZVAL_BOOL(*z_expandSubMux, 1);
    }

    zval *new_object;
    ALLOC_INIT_ZVAL(new_object);
    object_init_ex(new_object, ce_pux_mux);
    CALL_METHOD(Mux, __construct, new_object, new_object);

    // zend_update_property_long( Z_OBJCE_P(new_object), new_object, "id", sizeof("id")-1, Z_LVAL_PP(z_id) TSRMLS_CC);
    zend_update_property(ce_pux_mux, new_object, "id", sizeof("id")-1, *z_id TSRMLS_CC);
    zend_update_property(ce_pux_mux, new_object, "routes", sizeof("routes")-1, *z_routes TSRMLS_CC);
    zend_update_property(ce_pux_mux, new_object, "submux", sizeof("submux")-1, *z_submux TSRMLS_CC);
    zend_update_property(ce_pux_mux, new_object, "expandSubMux", sizeof("expandSubMux")-1, *z_expandSubMux TSRMLS_CC);
    *return_value = *new_object;
    zval_copy_ctor(return_value);
    zval_ptr_dtor(&z_array);
}


zval * call_mux_method(zval * object , char * method_name , int method_name_len, int param_count, zval* arg1, zval* arg2, zval* arg3 TSRMLS_DC)
{
    zend_function *fe;
    if ( zend_hash_find( &Z_OBJCE_P(object)->function_table, method_name, method_name_len, (void **) &fe) == FAILURE ) {
        php_error(E_ERROR, "%s method not found", method_name);
    }
    // call export method
    zval *z_retval;
    ALLOC_INIT_ZVAL(z_retval);
    zend_call_method_with_3_params( &object, ce_pux_mux, &fe, method_name, method_name_len, &z_retval, param_count, arg1, arg2, arg3 TSRMLS_CC );
    return z_retval;
}


PHP_METHOD(Mux, get) {
    zval *z_pattern = NULL, *z_callback = NULL, *z_options = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|a", &z_pattern, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }
    if ( z_options == NULL ) {
        ALLOC_INIT_ZVAL(z_options);
        array_init(z_options);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init(z_options);
    }

    // $options['method'] = REQ_METHOD_GET;
    add_assoc_long(z_options, "method", REQ_METHOD_GET);

    // $this->add($pattern, $callback, $options);
    zval * z_retval = call_mux_method( this_ptr, "add" , sizeof("add"), 3 , z_pattern, z_callback, z_options TSRMLS_CC);
    *return_value = *z_retval;
    zval_copy_ctor(return_value);
    // zval_ptr_dtor(&z_retval);
}




PHP_METHOD(Mux, post) {
    zval *z_pattern = NULL, *z_callback = NULL, *z_options = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|a", &z_pattern, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }
    if ( z_options == NULL ) {
        ALLOC_INIT_ZVAL(z_options);
        array_init(z_options);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init(z_options);
    }

    add_assoc_long(z_options, "method", REQ_METHOD_POST);

    // $this->add($pattern, $callback, $options);
    zval * z_retval = call_mux_method( getThis(), "add" , sizeof("add"), 3 , z_pattern, z_callback, z_options TSRMLS_CC);
    *return_value = *z_retval;
    zval_copy_ctor(return_value);
    // zval_ptr_dtor(&z_retval);
}

PHP_METHOD(Mux, put) {
    zval *z_pattern = NULL, *z_callback = NULL, *z_options = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|a", &z_pattern, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }
    if ( z_options == NULL ) {
        ALLOC_INIT_ZVAL(z_options);
        array_init(z_options);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init(z_options);
    }

    add_assoc_long(z_options, "method", REQ_METHOD_PUT);

    // $this->add($pattern, $callback, $options);
    zval * z_retval = call_mux_method( getThis(), "add" , sizeof("add"), 3 , z_pattern, z_callback, z_options TSRMLS_CC);
    *return_value = *z_retval;
    zval_copy_ctor(return_value);
    // zval_ptr_dtor(&z_retval);
}

PHP_METHOD(Mux, delete) {
    zval *z_pattern = NULL, *z_callback = NULL, *z_options = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|a", &z_pattern, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }
    if ( z_options == NULL ) {
        ALLOC_INIT_ZVAL(z_options);
        array_init(z_options);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init(z_options);
    }

    add_assoc_long(z_options, "method", REQ_METHOD_DELETE);

    // $this->add($pattern, $callback, $options);
    zval * z_retval = call_mux_method( getThis(), "add" , sizeof("add"), 3 , z_pattern, z_callback, z_options TSRMLS_CC);
    *return_value = *z_retval;
    zval_copy_ctor(return_value);
    // zval_ptr_dtor(&z_retval);
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

    if ( z_options == NULL ) {
        ALLOC_INIT_ZVAL(z_options);
        array_init(z_options);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init(z_options);
    }


    zend_class_entry **ce_pattern_compiler = get_pattern_compiler_ce(TSRMLS_CC);
    if ( ce_pattern_compiler == NULL ) {
        RETURN_FALSE;
    }

    zval *z_routes;
    zval *z_expandSubMux;
    zval *z_mux_routes;
    zend_class_entry *obj_ce;

    obj_ce = Z_OBJCE_P(this_ptr);

    z_routes = zend_read_property( obj_ce, getThis(), "routes", sizeof("routes")-1, 1 TSRMLS_CC);
    z_expandSubMux = zend_read_property( obj_ce, getThis(), "expandSubMux", sizeof("expandSubMux")-1, 1 TSRMLS_CC);


    if ( Z_BVAL_P(z_expandSubMux) ) {
        // fetch routes from $mux
        //
        z_mux_routes = zend_read_property( Z_OBJCE_P(z_mux), z_mux, "routes", sizeof("routes")-1, 1 TSRMLS_CC);

        HashPosition route_pointer;
        HashTable    *mux_routes_hash;
        mux_routes_hash = Z_ARRVAL_P(z_mux_routes);
        zval **z_mux_route;


        // iterate mux
        for(zend_hash_internal_pointer_reset_ex(mux_routes_hash, &route_pointer); 
                zend_hash_get_current_data_ex(mux_routes_hash, (void**) &z_mux_route, &route_pointer) == SUCCESS; 
                zend_hash_move_forward_ex(mux_routes_hash, &route_pointer)) 
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
                continue;
            }
            if ( zend_hash_index_find( Z_ARRVAL_PP(z_mux_route), 1, (void**) &z_route_pattern) == FAILURE ) {
                continue;
            }
            if ( zend_hash_index_find( Z_ARRVAL_PP(z_mux_route), 2, (void**) &z_route_callback) == FAILURE ) {
                continue;
            }
            if ( zend_hash_index_find( Z_ARRVAL_PP(z_mux_route), 3, (void**) &z_route_options) == FAILURE ) {
                continue;
            }

            // Z_ADDREF_P(z_route_callback);
            // Z_ADDREF_P(z_route_options); // reference it so it will not be recycled.
            ALLOC_INIT_ZVAL(z_new_routes);
            array_init(z_new_routes);

            if ( Z_BVAL_PP(z_is_pcre) ) {
                // $newPattern = $pattern . $route[3]['pattern'];

                if ( zend_hash_find( Z_ARRVAL_PP(z_route_options), "pattern", sizeof("pattern"), (void**) &z_route_original_pattern) == FAILURE ) {
                    php_error( E_ERROR, "Can not compile pattern, original pattern not found");
                }

                char new_pattern[120] = { 0 };
                int  new_pattern_len;
                strncat( new_pattern, pattern , pattern_len );
                strncat( new_pattern, Z_STRVAL_PP(z_route_original_pattern) , Z_STRLEN_PP(z_route_original_pattern) );

                new_pattern_len = pattern_len + Z_STRLEN_PP(z_route_original_pattern);

                zval *z_new_pattern = NULL;
                ALLOC_INIT_ZVAL(z_new_pattern);
                ZVAL_STRINGL(z_new_pattern, new_pattern, new_pattern_len, 1);

                // TODO: merge options

                // $routeArgs = PatternCompiler::compile($newPattern, 
                //     array_merge_recursive($route[3], $options) );
                zval *z_compiled_route = compile_route_pattern(z_new_pattern, *z_route_options, ce_pattern_compiler TSRMLS_CC);


                if ( z_compiled_route == NULL || Z_TYPE_P(z_compiled_route) == IS_NULL ) {
                    php_error( E_ERROR, "Can not compile pattern: %s", new_pattern);
                }


                zval **z_compiled_route_pattern;
                if ( zend_hash_find( Z_ARRVAL_P(z_compiled_route) , "compiled", sizeof("compiled"), (void**)&z_compiled_route_pattern) == FAILURE ) {
                    php_error( E_ERROR, "compiled pattern not found: %s", new_pattern);
                }

                zend_hash_update( Z_ARRVAL_P(z_compiled_route), "pattern", sizeof("pattern"), &z_new_pattern, sizeof(zval *), NULL);


                // create new route and append to mux->routes
                add_index_bool(z_new_routes, 0 , 1); // pcre flag == false
                add_index_zval(z_new_routes, 1, *z_compiled_route_pattern);
                add_index_zval(z_new_routes, 2 , *z_route_callback);
                add_index_zval(z_new_routes, 3, z_compiled_route);
                add_next_index_zval(z_routes, z_new_routes);

            } else {


                //  $this->routes[] = array(
                //      false,
                //      $pattern . $route[1],
                //      $route[2],
                //      $options,
                //  );
                char new_pattern[120] = { 0 };
                strncat(new_pattern, pattern, pattern_len);
                strncat(new_pattern, Z_STRVAL_PP(z_route_pattern), Z_STRLEN_PP(z_route_pattern) );

                int new_pattern_len = pattern_len + Z_STRLEN_PP(z_route_pattern);

                /* make the array: [ pcreFlag, pattern, callback, options ] */
                add_index_bool(z_new_routes, 0 , 0); // pcre flag == false
                add_index_stringl(z_new_routes, 1 , new_pattern , new_pattern_len, 1);
                add_index_zval( z_new_routes, 2 , *z_route_callback);
                add_index_zval( z_new_routes, 3 , *z_route_options);
                add_next_index_zval(z_routes, z_new_routes);
            }
        }

    } else {
        zend_function *fe_getid = NULL; // method entry
        zend_function *fe_add   = NULL; // method entry

        if ( zend_hash_find( &Z_OBJCE_P(z_mux)->function_table, "getid", sizeof("getid"), (void **) &fe_getid) == FAILURE ) {
            php_error(E_ERROR, "Cannot call method Mux::getid()");
            RETURN_FALSE;
        }
        if ( zend_hash_find( &Z_OBJCE_P(this_ptr)->function_table, "add", sizeof("add"),    (void **) &fe_add) == FAILURE ) {
            php_error(E_ERROR, "Cannot call method Mux::add()");
            RETURN_FALSE;
        }

        // $muxId = $mux->getId();
        // $this->add($pattern, $muxId, $options);
        // $this->submux[ $muxId ] = $mux;
        // zval *z_mux_id = call_mux_method(z_mux, "getid", sizeof("getid") ,  );

        zval *z_mux_id = NULL;
        ALLOC_INIT_ZVAL(z_mux_id);
        zend_call_method( &z_mux, Z_OBJCE_P(z_mux), &fe_getid, "getid", strlen("getid"), &z_mux_id, 0, NULL, NULL TSRMLS_CC );

        if ( Z_TYPE_P(z_mux_id) == IS_NULL ) {
            php_error(E_ERROR, "Mux id is required. got NULL.");
        }

        // create pattern
        zval *z_pattern = NULL;
        ALLOC_INIT_ZVAL(z_pattern);
        ZVAL_STRINGL(z_pattern, pattern, pattern_len, 1); // duplicate

        zval *z_retval = NULL;
        ALLOC_INIT_ZVAL(z_retval);
        zend_call_method_with_3_params( &this_ptr, ce_pux_mux, &fe_add, "add",
                strlen("add"), &z_retval, 3, z_pattern, z_mux_id, z_options TSRMLS_CC);
        // zend_call_method( &this_obj, Z_OBJCE_P(this_obj), &fe_add, "add", strlen("add"), &z_retval, 2, z_pattern, z_mux_id TSRMLS_CC);
        zval *z_submux_array = zend_read_property( ce_pux_mux, this_ptr , "submux", sizeof("submux") - 1, 1 TSRMLS_CC);
        add_index_zval( z_submux_array, Z_LVAL_P(z_mux_id) , z_mux);

        // release zvals
        // zval_ptr_dtor(&z_retval);
        zval_ptr_dtor(&z_mux_id);
        // zval_ptr_dtor(&z_pattern);
    }
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
        return;
    }
    RETURN_FALSE;
}

PHP_METHOD(Mux, getRoutes) {
    zval *z_routes;
    z_routes = zend_read_property( Z_OBJCE_P(this_ptr) , this_ptr, "routes", sizeof("routes")-1, 1 TSRMLS_CC);
    *return_value = *z_routes;
    zval_copy_ctor(return_value);
}

PHP_METHOD(Mux, export) {

    zval *should_return;
    ALLOC_INIT_ZVAL(should_return);
    ZVAL_BOOL(should_return, 1);


    zval *retval_ptr = NULL;
    ALLOC_INIT_ZVAL(retval_ptr);

    // return var_export($this, true);
    zend_call_method( NULL, NULL, NULL, "var_export", strlen("var_export"), &retval_ptr, 2, this_ptr, should_return TSRMLS_CC );

    *return_value = *retval_ptr;
    zval_copy_ctor(return_value);
    // zval_ptr_dtor(&should_return);
    // zval_ptr_dtor(&retval_ptr);
}

PHP_METHOD(Mux, getId) {
    zval *z_id;
    long counter = 0;

    z_id = zend_read_property( Z_OBJCE_P(this_ptr) , getThis(), "id", sizeof("id")-1, 1 TSRMLS_CC);

    if ( z_id != NULL && Z_TYPE_P(z_id) != IS_NULL ) {
        RETURN_LONG( Z_LVAL_P(z_id) );
    }

    zval *retval_ptr = NULL;
    ALLOC_INIT_ZVAL(retval_ptr);
    zend_call_method( NULL, Z_OBJCE_P(this_ptr) , NULL, "generate_id", strlen("generate_id"), &retval_ptr, 0, NULL, NULL TSRMLS_CC );

    counter = Z_LVAL_P(retval_ptr);
    zend_update_property_long(Z_OBJCE_P(this_ptr), this_ptr, "id" , sizeof("id") - 1, counter TSRMLS_CC);

    zval_ptr_dtor(&retval_ptr);
    RETURN_LONG(counter);
}

PHP_METHOD(Mux, length) {
    zval *z_routes;
    z_routes = zend_read_property(Z_OBJCE_P(this_ptr), this_ptr, "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    long length = 0;
    length = zend_hash_num_elements( Z_ARRVAL_P(z_routes) );

    RETURN_LONG(length);
}

PHP_METHOD(Mux, sort) {
    zval *z_routes;
    z_routes = zend_read_property(Z_OBJCE_P(this_ptr) , this_ptr, "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    zval *retval_ptr = NULL;
    ALLOC_INIT_ZVAL(retval_ptr);

    zval *z_sort_callback = NULL;
    ALLOC_INIT_ZVAL(z_sort_callback);
    ZVAL_STRING( z_sort_callback, "pux_sort_routes" , 1 );

    Z_SET_ISREF_P(z_routes);
    zend_call_method( NULL, NULL, NULL, "usort", strlen("usort"), &retval_ptr, 2, 
            z_routes, z_sort_callback TSRMLS_CC );

    // zval_ptr_dtor(&retval_ptr);
    // zval_ptr_dtor(&z_sort_callback);
}

PHP_METHOD(Mux, compile) {
    char *filename;
    int  filename_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename, &filename_len) == FAILURE) {
        RETURN_FALSE;
    }

    zval *z_routes;
    z_routes = zend_read_property(ce_pux_mux, this_ptr, "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    Z_SET_ISREF_P(z_routes);

    // duplicated code to sort method
    zval *z_retval = NULL;
    ALLOC_INIT_ZVAL(z_retval);

    zval *z_sort_callback = NULL;
    ALLOC_INIT_ZVAL(z_sort_callback);
    ZVAL_STRING( z_sort_callback, "pux_sort_routes" , 1 );

    zend_call_method( NULL, NULL, NULL, "usort", strlen("usort"), &z_retval, 2, 
            z_routes, z_sort_callback TSRMLS_CC );

    if ( Z_BVAL_P(z_retval) == 0 ) {
        php_error(E_ERROR,"route sort failed.");
    }
    zval_ptr_dtor(&z_sort_callback); // recycle sort callback zval
    zval_ptr_dtor(&z_retval); // recycle sort callback zval

    // zend_update_property(ce_pux_mux, getThis(), "routes", sizeof("routes")-1, z_routes TSRMLS_CC);


    // $code = '<?php return ' . $this->export() . ';';

    // get export method function entry
    zend_function *fe_export;
    if ( zend_hash_find( &Z_OBJCE_P(this_ptr)->function_table, "export", sizeof("export"),  (void **) &fe_export) == FAILURE ) {
        php_error(E_ERROR, "export method not found");
    }

    // call export method
    zval *compiled_code;
    ALLOC_INIT_ZVAL(compiled_code);
    zend_call_method( &this_ptr, Z_OBJCE_P(this_ptr) , &fe_export, "export", strlen("export"), &compiled_code, 0, NULL, NULL TSRMLS_CC );


    if ( compiled_code == NULL || Z_TYPE_P(compiled_code) == IS_NULL ) {
        php_error(E_ERROR, "Can not compile routes.");
    }


    int  buf_len = Z_STRLEN_P(compiled_code) + sizeof("<?php return ;") + 3;
    char *buf = (char* ) ecalloc(buf_len, sizeof(char));

    strncat(buf, "<?php return ", sizeof("<?php return ") );
    strncat(buf, Z_STRVAL_P(compiled_code), Z_STRLEN_P(compiled_code));
    strncat(buf, ";", 1);
    // strncat(buf, "\0", 1);

    zval *z_code = NULL;
    zval *z_filename = NULL;
    zval *retval = NULL;
    ALLOC_INIT_ZVAL(z_code);
    ALLOC_INIT_ZVAL(z_filename);
    ALLOC_INIT_ZVAL(retval);
    ZVAL_STRING(z_code, buf, 0);
    ZVAL_STRINGL(z_filename, filename, filename_len, 0);


    zend_call_method( NULL, NULL, NULL, "file_put_contents", strlen("file_put_contents"), &retval, 2, z_filename, z_code TSRMLS_CC );

    *return_value = *retval;
    zval_copy_ctor(return_value);

    zval_ptr_dtor(&compiled_code);
    zval_ptr_dtor(&z_code);
    zval_ptr_dtor(&z_filename);
    zval_ptr_dtor(&retval);
}

PHP_METHOD(Mux, dispatch) {
    char *path;
    int  path_len;
    zval *z_path;

    char  trim_char[2] = { '\\', 0 };
    zval *z_trimed_path;
    zval *z_return_route;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len) == FAILURE) {
        RETURN_FALSE;
    }

    if ( path_len == 0 ) {
        php_error(E_ERROR, "Dispatch path required. empty path given.");
    }

    ALLOC_INIT_ZVAL(z_trimed_path);
    ALLOC_INIT_ZVAL(z_return_route);
    ALLOC_INIT_ZVAL(z_path);
    ZVAL_STRINGL(z_path, path, path_len, 1);

    php_trim(path, path_len, trim_char, 1, z_trimed_path, 2 TSRMLS_CC); // mode 2 == rtrim

    zend_function *fe; // method entry
    zend_hash_find( &Z_OBJCE_P(this_ptr)->function_table, "matchroute",    sizeof("matchroute"),    (void **) &fe);
    zend_call_method( &this_ptr, ce_pux_mux, &fe, "matchroute", strlen("matchroute"), &z_return_route, 1, z_trimed_path, NULL TSRMLS_CC );

    if ( z_return_route == NULL || Z_TYPE_P(z_return_route) == IS_NULL ) {
        // zval_ptr_dtor(&z_trimed_path);
        // zval_ptr_dtor(&z_path);
        RETURN_NULL();
    }


    // read data from matched route
    zval **z_pcre;
    zval **z_pattern;
    zval **z_callback;
    zval **z_options;

    HashTable *hash_return_route = Z_ARRVAL_P(z_return_route);
    zend_hash_index_find( hash_return_route , 0 , (void**) &z_pcre );
    zend_hash_index_find( hash_return_route , 1 , (void**) &z_pattern );
    zend_hash_index_find( hash_return_route , 2 , (void**) &z_callback );
    zend_hash_index_find( hash_return_route , 3 , (void**) &z_options );

    // dispatch to submux if the callback is an ID.
    if ( Z_TYPE_PP(z_callback) == IS_LONG ) {
        zval *z_submux_array = NULL;
        zval **z_submux = NULL;
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
            zval *retval = NULL;
            zval *z_path;
            zval *z_route_vars_0_len;
            zval *z_substr;
            if ( zend_hash_find( Z_ARRVAL_PP(z_options) , "vars", strlen("vars") , (void**) &z_route_vars ) == FAILURE ) {
                php_error(E_ERROR, "require route vars");
                RETURN_FALSE;
            }
            if ( zend_hash_index_find( Z_ARRVAL_PP(z_options) , 0 , (void**) &z_route_vars_0 ) == FAILURE ) {
                php_error(E_ERROR, "require route vars[0]");
                RETURN_FALSE;
            }

            if ( z_route_vars == NULL ) {
                ALLOC_INIT_ZVAL(*z_route_vars);
                array_init(*z_route_vars);
            } else if ( Z_TYPE_PP(z_route_vars) == IS_NULL ) {
                array_init(*z_route_vars); // initilize an empty array
            }

            ALLOC_INIT_ZVAL(z_path);
            ALLOC_INIT_ZVAL(z_route_vars_0_len);
            ALLOC_INIT_ZVAL(z_substr);

            ZVAL_STRINGL(z_path, path ,path_len, 1);
            ZVAL_LONG(z_route_vars_0_len, Z_STRLEN_PP(z_route_vars_0) );

            zend_call_method( NULL, NULL, NULL, "substr", strlen("substr"), &z_substr, 2, z_path, z_route_vars_0_len TSRMLS_CC );

            retval = call_mux_method( *z_submux, "dispatch" , sizeof("dispatch"), 1 , z_substr, NULL, NULL TSRMLS_CC);
            if ( retval == NULL ) {
                RETURN_FALSE;
            }
            *return_value = *retval;
            zval_copy_ctor(return_value);

            zval_ptr_dtor(&retval);
            zval_ptr_dtor(&z_substr);
            zval_ptr_dtor(&z_path);
            zval_ptr_dtor(&z_route_vars_0_len);
            return;

        } else {
            zval *z_path = NULL, *z_pattern_len = NULL, *z_substr = NULL;

            ALLOC_INIT_ZVAL(z_path);
            ZVAL_STRINGL(z_path, path ,path_len, 1);

            ALLOC_INIT_ZVAL(z_pattern_len);
            ZVAL_LONG(z_pattern_len, Z_STRLEN_PP(z_pattern));

            ALLOC_INIT_ZVAL(z_substr);
            zend_call_method( NULL, NULL, NULL, "substr", strlen("substr"), &z_substr, 2, z_path, z_pattern_len TSRMLS_CC );

            //     return $submux->dispatch(
            //         substr($path, strlen($route[1]))
            //     );

            zval * z_retval = call_mux_method( *z_submux, "dispatch" , sizeof("dispatch"), 1 , z_substr, NULL, NULL TSRMLS_CC);
            *return_value = *z_retval;
            zval_copy_ctor(return_value);

            zval_ptr_dtor(&z_path);
            zval_ptr_dtor(&z_pattern_len);
            zval_ptr_dtor(&z_substr);
            return;

        }
    }


    *return_value = *z_return_route;
    zval_copy_ctor(return_value);

    zval_ptr_dtor(&z_path);
    zval_ptr_dtor(&z_trimed_path);
    zval_ptr_dtor(&z_return_route);
    return;
}

PHP_METHOD(Mux, matchRoute) {
    char *path;
    int  path_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len) == FAILURE) {
        RETURN_FALSE;
    }

    zval *z_route;
    zval *z_routes;

    z_routes = zend_read_property(ce_pux_mux , this_ptr , "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    z_route = php_pux_match(z_routes, path, path_len);
    if ( z_route != NULL ) {
        *return_value = *z_route;
        zval_copy_ctor(z_route);
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

    if ( z_options == NULL ) {
        ALLOC_INIT_ZVAL(z_options);
        array_init(z_options);
    }
    if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init(z_options);
    }

    zval *z_routes;
    z_routes = zend_read_property(Z_OBJCE_P(this_ptr), this_ptr, "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    zval *z_new_routes;
    ALLOC_INIT_ZVAL(z_new_routes);
    array_init(z_new_routes);

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
        ALLOC_INIT_ZVAL(z_options);
        array_init(z_options);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init(z_options);
    }

    zval *z_pattern = NULL;
    zval *z_routes;

    ALLOC_INIT_ZVAL(z_pattern);
    ZVAL_STRINGL(z_pattern, pattern, pattern_len, 1);

    z_routes = zend_read_property(Z_OBJCE_P(this_ptr), getThis(), "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    zend_class_entry **ce_pattern_compiler = get_pattern_compiler_ce(TSRMLS_CC);
    if ( ce_pattern_compiler == NULL ) {
        RETURN_FALSE;
    }

    zval *retval_ptr = NULL;
    ALLOC_INIT_ZVAL(retval_ptr);
    zend_call_method( NULL, *ce_pattern_compiler, NULL, "compile", strlen("compile"), &retval_ptr, 1, z_pattern, NULL TSRMLS_CC );

    if ( retval_ptr == NULL || Z_TYPE_P(retval_ptr) == IS_NULL ) {
        zend_throw_exception(ce_pux_exception, "Can not compile route pattern", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    add_next_index_zval(z_routes, retval_ptr);
}



PHP_METHOD(Mux, add) {
    char *pattern;
    int  pattern_len;

    zval *z_callback = NULL;
    zval *z_options = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|a", &pattern, &pattern_len, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }

    // $pcre = strpos($pattern,':') !== false;
    char *found = find_place_holder(pattern, pattern_len);


    zval *z_routes;

    if ( z_options == NULL ) {
        ALLOC_INIT_ZVAL(z_options);
        array_init(z_options);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        // make it as an array
        array_init(z_options);
    }

    z_routes = zend_read_property(ce_pux_mux, this_ptr, "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    // PCRE pattern here
    if ( found ) {
        zval *z_pattern = NULL;
        ALLOC_INIT_ZVAL(z_pattern);
        ZVAL_STRINGL(z_pattern, pattern, pattern_len, 1);

        zval *z_compiled_route = compile_route_pattern(z_pattern, z_options, NULL TSRMLS_CC);
        if ( z_compiled_route == NULL ) {
            zend_throw_exception(ce_pux_exception, "Unable to compile route pattern.", 0 TSRMLS_CC);
            RETURN_FALSE;
        }

        zval **z_compiled_route_pattern;
        if ( zend_hash_find( Z_ARRVAL_P(z_compiled_route) , "compiled", sizeof("compiled"), (void**)&z_compiled_route_pattern) == FAILURE ) {
            zend_throw_exception(ce_pux_exception, "Unable to find compiled pattern.", 0 TSRMLS_CC);
            RETURN_FALSE;
        }

        // Z_ADDREF_P(z_compiled_route);
        // Z_ADDREF_PP(z_compiled_route_pattern);
        Z_ADDREF_P(z_callback);

        zval *z_new_routes;
        ALLOC_INIT_ZVAL(z_new_routes);
        array_init(z_new_routes);

        add_index_bool(z_new_routes, 0 , 1); // pcre flag == false
        add_index_zval(z_new_routes, 1, *z_compiled_route_pattern);
        add_index_zval(z_new_routes, 2 , z_callback);
        add_index_zval(z_new_routes, 3, z_compiled_route);
        add_next_index_zval(z_routes, z_new_routes);
        // zval_ptr_dtor(&z_pattern);

    } else {
        Z_ADDREF_P(z_options); // reference it so it will not be recycled.
        Z_ADDREF_P(z_callback);

        zval *z_new_routes;
        ALLOC_INIT_ZVAL(z_new_routes);
        array_init(z_new_routes);

        /* make the array: [ pcreFlag, pattern, callback, options ] */
        add_index_bool(z_new_routes, 0 , 0); // pcre flag == false
        add_index_stringl( z_new_routes, 1 , pattern, pattern_len , 1 );
        add_index_zval( z_new_routes, 2 , z_callback);
        add_index_zval( z_new_routes, 3 , z_options);
        add_next_index_zval(z_routes, z_new_routes);
    }
}


