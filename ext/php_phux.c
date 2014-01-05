#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "string.h"
#include "main/php_main.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "zend_object_handlers.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_string.h"

#include "php_phux.h"

#define ZEND_HASH_FETCH(hash,key,ret) \
    zend_hash_find(hash, key, sizeof(key), (void**)&ret) == SUCCESS

// #define DEBUG 1

zend_class_entry *phux_ce_mux;
zend_class_entry *phux_ce_exception;


const zend_function_entry mux_methods[] = {
  PHP_ME(Mux, __construct, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, getId, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, add, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, compile, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, dispatch, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, length, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, appendRoute, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, appendPCRERoute, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, matchRoute, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, getRoutes, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, generate_id, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
  PHP_FE_END
};

static const zend_function_entry phux_functions[] = {
    PHP_FE(phux_match, NULL)
    PHP_FE_END
};


void phux_init_exception(TSRMLS_D) {
  zend_class_entry e;
  INIT_CLASS_ENTRY(e, "PhuxException", NULL);
  phux_ce_exception = zend_register_internal_class_ex(&e, (zend_class_entry*)zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
}

zend_module_entry phux_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_PHUX_EXTNAME,
    phux_functions,
    PHP_MINIT(phux),
    NULL,
    NULL,
    NULL,
    NULL,
    PHP_PHUX_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_PHUX
ZEND_GET_MODULE(phux)
#endif



void phux_init_mux(TSRMLS_D) {
  zend_class_entry ce;
 
  INIT_CLASS_ENTRY(ce, "Phux\\MuxNew", mux_methods);
  phux_ce_mux = zend_register_internal_class(&ce TSRMLS_CC);
 
  /* fields */
  zend_declare_property_null(phux_ce_mux, "id", strlen("id"), ZEND_ACC_PUBLIC TSRMLS_CC);

  /*
  zval *z_routes;
  MAKE_STD_ZVAL(z_routes);
  array_init(z_routes);
  zend_declare_property_ex(phux_ce_mux, "routes", strlen("routes"), z_routes, ZEND_ACC_PUBLIC, NULL, 0 TSRMLS_CC);
  */
  zend_declare_property_null(phux_ce_mux, "routes", strlen("routes"), ZEND_ACC_PUBLIC TSRMLS_CC);
  zend_declare_property_null(phux_ce_mux, "subMux", strlen("subMux"), ZEND_ACC_PUBLIC TSRMLS_CC);
  zend_declare_property_bool(phux_ce_mux, "expandSubMux", strlen("expandSubMux"), 1, ZEND_ACC_PUBLIC TSRMLS_CC);

  zend_declare_property_long(phux_ce_mux, "id_counter", strlen("id_counter"), 0, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC TSRMLS_CC);
}

PHP_METHOD(Mux, __construct) {
    zval *z_routes = NULL , *z_submux = NULL;
    z_routes = zend_read_property(phux_ce_mux, getThis(), "routes", sizeof("routes")-1, 1 TSRMLS_CC);
    z_submux = zend_read_property(phux_ce_mux, getThis(), "subMux", sizeof("subMux")-1, 1 TSRMLS_CC);
    array_init(z_routes);
    array_init(z_submux);
}

PHP_METHOD(Mux, generate_id) {
    zval * z_counter = NULL;
    long counter = 0;
    z_counter = zend_read_static_property(phux_ce_mux, "id_counter", strlen("id_counter") , 0 TSRMLS_CC);
    if ( z_counter != NULL ) {
        counter = Z_LVAL_P(z_counter);
    }
    counter++;
    Z_LVAL_P(z_counter) = counter;
    RETURN_LONG(counter);
}

PHP_METHOD(Mux, getRoutes) {
    zval *z_routes;

    z_routes = zend_read_property(phux_ce_mux, getThis(), "routes", sizeof("routes")-1, 1 TSRMLS_CC);
    *return_value = *z_routes;
    zval_copy_ctor(return_value);
}

PHP_METHOD(Mux, getId) {
    zval *z_id;
    zval *z_counter;
    long counter = 0;

    z_id = zend_read_property(phux_ce_mux, getThis(), "id", sizeof("id")-1, 1 TSRMLS_CC);

    if ( z_id != NULL && Z_TYPE_P(z_id) != IS_NULL ) {
        RETURN_LONG( Z_LVAL_P(z_id) );
    }

    zval *retval_ptr = NULL;
    ALLOC_INIT_ZVAL(retval_ptr);
    zend_call_method( NULL, phux_ce_mux, NULL, "generate_id", strlen("generate_id"), &retval_ptr, 0, NULL, NULL TSRMLS_CC );

    counter = Z_LVAL_P(retval_ptr);
    zend_update_property_long(phux_ce_mux, getThis(), "id" , sizeof("id") - 1, counter TSRMLS_CC);
    RETURN_LONG(counter);
}

PHP_METHOD(Mux, length) {
    zval *z_routes;
    z_routes = zend_read_property(phux_ce_mux, getThis(), "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    long length = 0;
    length = zend_hash_num_elements( Z_ARRVAL_P(z_routes) );

    RETURN_LONG(length);
}

PHP_METHOD(Mux, compile) {
    char *filename;
    int  filename_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename, &filename_len) == FAILURE) {
        RETURN_FALSE;
    }

    zend_class_entry **ce = NULL;
    zval *z_routes;

    z_routes = zend_read_property(phux_ce_mux, getThis(), "routes", sizeof("routes")-1, 1 TSRMLS_CC);


    /*
    if ( zend_lookup_class( Z_STRVAL_P(z_route_compiler_class), Z_STRLEN_P(z_route_compiler_class) , &ce TSRMLS_CC) == FAILURE ) {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Class Phux\\RouteCompiler does not exist.", 0 TSRMLS_CC);
    }
    */

    zval *retval_ptr = NULL;
    ALLOC_INIT_ZVAL(retval_ptr);

    zval *z_sort_callback = NULL;
    MAKE_STD_ZVAL(z_sort_callback);
    array_init(z_sort_callback);
    add_index_stringl( z_sort_callback , 0 , "Phux\\Mux" , strlen("Phux\\Mux") , 1);
    add_index_stringl( z_sort_callback , 1 , "sort_routes" , strlen("sort_routes") , 1);

    Z_SET_ISREF_P(z_routes);
    zend_call_method( NULL, NULL, NULL, "usort", strlen("usort"), &retval_ptr, 2, 
            z_routes, z_sort_callback TSRMLS_CC );

}

PHP_METHOD(Mux, dispatch) {
    char *path;
    int  path_len;
    zval *z_path;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z_path) == FAILURE) {
        RETURN_FALSE;
    }

    char  trim_char[2] = { '\\', 0 };


    zval *z_trimed_path;
    zval *z_return_route;

    ALLOC_INIT_ZVAL(z_trimed_path);
    ALLOC_INIT_ZVAL(z_return_route);


    path = Z_STRVAL_P(z_path);
    path_len = Z_STRLEN_P(z_path);
    php_trim(path, path_len, trim_char, 1, z_trimed_path, 2 TSRMLS_CC); // mode 2 == rtrim

    zval * this_object = getThis();


    zend_function *fe; // method entry
    zend_hash_find( &Z_OBJCE_P(this_object)->function_table, "matchroute",    sizeof("matchroute"),    (void **) &fe);
    zend_call_method( &this_object, Z_OBJCE_P(this_object), &fe, "matchroute", strlen("matchroute"), &z_return_route, 1, z_trimed_path, NULL TSRMLS_CC );


    if ( Z_TYPE_P(z_return_route) == IS_NULL ) {
        RETURN_NULL();
    }

    *return_value = *z_return_route;
    zval_copy_ctor(return_value);
    return;

    /*
    zval *z_route;
    zval *z_routes;
    z_routes = zend_read_property(phux_ce_mux, getThis(), "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    z_route = php_phux_match(z_routes, path, path_len);
    if ( z_route != NULL ) {
        *return_value = *z_route;
        zval_copy_ctor(z_route);
        return;
    }
    RETURN_NULL();
    */
}

PHP_METHOD(Mux, matchRoute) {
    char *path;
    int  path_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len) == FAILURE) {
        RETURN_FALSE;
    }

    zval *z_route;
    zval *z_routes;

    z_routes = zend_read_property(phux_ce_mux, getThis(), "routes", sizeof("routes")-1, 1 TSRMLS_CC);


    z_route = php_phux_match(z_routes, path, path_len);
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
    zend_class_entry **ce = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa|a", &pattern, &pattern_len, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }

    if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init(z_options);
    }

    zval *z_routes;
    z_routes = zend_read_property(phux_ce_mux, getThis(), "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    zval *z_new_routes;
    MAKE_STD_ZVAL(z_new_routes);
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
    zend_class_entry **ce = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa|a", &pattern, &pattern_len, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }

    if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init(z_options);
    }

    zval *z_pattern = NULL;
    MAKE_STD_ZVAL(z_pattern);
    ZVAL_STRINGL(z_pattern, pattern, pattern_len, 1);

    zval *z_routes;
    z_routes = zend_read_property(phux_ce_mux, getThis(), "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    zval *z_route_compiler_class = NULL;
    MAKE_STD_ZVAL(z_route_compiler_class);
    ZVAL_STRING(z_route_compiler_class, "Phux\\RouteCompiler", 1);

    if ( zend_lookup_class( Z_STRVAL_P(z_route_compiler_class), Z_STRLEN_P(z_route_compiler_class) , &ce TSRMLS_CC) == FAILURE ) {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Class Phux\\RouteCompiler does not exist.", 0 TSRMLS_CC);
    }

    zval *retval_ptr = NULL;
    ALLOC_INIT_ZVAL(retval_ptr);
    zend_call_method( NULL, *ce, NULL, "compile", strlen("compile"), &retval_ptr, 1, z_pattern, NULL TSRMLS_CC );

    if ( retval_ptr == NULL || Z_TYPE_P(retval_ptr) == IS_NULL ) {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Can not compile route pattern", 0 TSRMLS_CC);
    }
    add_next_index_zval(z_routes, retval_ptr);
}


PHP_METHOD(Mux, add) {
    char *pattern;
    int  pattern_len;

    zval *z_callback = NULL;
    zval *z_options = NULL;
    zend_class_entry **ce = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa|a", &pattern, &pattern_len, &z_callback, &z_options) == FAILURE) {
        RETURN_FALSE;
    }

    // $pcre = strpos($pattern,':') !== false;
    char  needle_char[2] = { ':', 0 };
    char *found = NULL;
    found = php_memnstr(pattern,
                        needle_char,
                        1,
                        pattern + pattern_len);

    zval *z_pattern = NULL;
    MAKE_STD_ZVAL(z_pattern);
    ZVAL_STRINGL(z_pattern, pattern, pattern_len, 1);

    zval *z_routes;

    if ( z_options == NULL ) {
        MAKE_STD_ZVAL(z_options);
        array_init(z_options);
    } else if ( Z_TYPE_P(z_options) == IS_NULL ) {
        // make it as an array
        array_init(z_options);
    }

    z_routes = zend_read_property(phux_ce_mux, getThis(), "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    // PCRE pattern here
    if ( found ) {
        zval *z_route_compiler_class = NULL;
        MAKE_STD_ZVAL(z_route_compiler_class);
        ZVAL_STRING(z_route_compiler_class, "Phux\\RouteCompiler", 1);

        if ( zend_lookup_class( Z_STRVAL_P(z_route_compiler_class), Z_STRLEN_P(z_route_compiler_class) , &ce TSRMLS_CC) == FAILURE ) {
            zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Class Phux\\RouteCompiler does not exist.", 0 TSRMLS_CC);
        }

        zval *retval_ptr = NULL;
        ALLOC_INIT_ZVAL(retval_ptr);
        zend_call_method( NULL, *ce, NULL, "compile", strlen("compile"), &retval_ptr, 1, z_pattern, NULL TSRMLS_CC );

        if ( retval_ptr == NULL || Z_TYPE_P(retval_ptr) == IS_NULL ) {
            zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Can not compile route pattern", 0 TSRMLS_CC);
        }
        add_next_index_zval(z_routes, retval_ptr);
    } else {
        Z_ADDREF_P(z_callback);
        Z_ADDREF_P(z_options); // reference it so it will not be recycled.

        zval *z_new_routes;
        MAKE_STD_ZVAL(z_new_routes);
        array_init(z_new_routes);


        /* make the array: [ pcreFlag, pattern, callback, options ] */
        add_index_bool(z_new_routes, 0 , 0); // pcre flag == false
        add_index_stringl( z_new_routes, 1 , pattern , pattern_len, 1);
        add_index_zval( z_new_routes, 2 , z_callback);
        add_index_zval( z_new_routes, 3 , z_options);
        add_next_index_zval(z_routes, z_new_routes);
    }
}



/**
 * get request method type in constant value.
 */
int get_current_request_method(TSRMLS_D) {
    char *c_request_method;
    int  c_request_method_len;
    zval **z_server_hash;
    zval **z_request_method;


    if (zend_hash_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER"), (void **) &z_server_hash) == SUCCESS &&
        Z_TYPE_PP(z_server_hash) == IS_ARRAY &&
        zend_hash_find(Z_ARRVAL_PP(z_server_hash), "REQUEST_METHOD", sizeof("REQUEST_METHOD"), (void **) &z_request_method) == SUCCESS
    ) {
        c_request_method = Z_STRVAL_PP(z_request_method);
        c_request_method_len = Z_STRLEN_PP(z_request_method);
        if ( strncmp("GET", c_request_method , sizeof("GET") ) == 0 ) {
            return REQ_METHOD_GET;
        } else if ( strncmp("POST", c_request_method , sizeof("POST") ) == 0 ) {
            return REQ_METHOD_POST;
        } else if ( strncmp("PUT" , c_request_method , sizeof("PUT") ) == 0 ) {
            return REQ_METHOD_PUT;
        } else if ( strncmp("DELETE", c_request_method, sizeof("DELETE")  ) == 0 ) {
            return REQ_METHOD_DELETE;
        }
    }
    return 0;
}

// int zend_hash_has_key( )

zval * php_phux_match(zval *z_routes, char *path, int path_len TSRMLS_DC) {

    zval *z_subpats = NULL; /* Array for subpatterns */
    int current_request_method;

    current_request_method = get_current_request_method(TSRMLS_CC);

    HashPosition route_pointer;
    HashTable    *routes_array;

    ALLOC_INIT_ZVAL( z_subpats );
    routes_array = Z_ARRVAL_P(z_routes);

    // for iterating routes
    zval **z_route;

    HashTable *route_item_hash;

    zval **z_is_pcre; // route[0]
    zval **z_pattern; // route[1]
    // callback @ route[2]
    zval **z_route_options; // route[3]
    HashTable * z_route_options_hash;

    zval **z_route_method;

    char *pattern;
    int  pattern_len;

    for(zend_hash_internal_pointer_reset_ex(routes_array, &route_pointer); 
            zend_hash_get_current_data_ex(routes_array, (void**) &z_route, &route_pointer) == SUCCESS; 
            zend_hash_move_forward_ex(routes_array, &route_pointer)) 
    {
        // read z_route
        route_item_hash = Z_ARRVAL_PP(z_route);

        if ( zend_hash_index_find( route_item_hash, 0, (void**) &z_is_pcre) == FAILURE ) {
            continue;
        }
        if ( zend_hash_index_find( route_item_hash, 1, (void**) &z_pattern) == FAILURE ) {
            continue;
        }
        if ( zend_hash_index_find( route_item_hash, 3, (void**) &z_route_options) == FAILURE ) {
            continue;
        }

        z_route_options_hash = Z_ARRVAL_PP(z_route_options);
        if ( zend_hash_find( z_route_options_hash , "method", sizeof("method"), (void**) &z_route_method ) == SUCCESS ) {
            if ( Z_TYPE_PP(z_route_method) == IS_LONG && Z_LVAL_PP(z_route_method) != current_request_method ) {
                continue;
            }
        }


        if ( Z_BVAL_PP(z_is_pcre) ) {
            // do pcre_match comparision

            /* parameters */
            pcre_cache_entry *pce;              /* Compiled regular expression */

            pattern = Z_STRVAL_PP(z_pattern);
            pattern_len = Z_STRLEN_PP(z_pattern);

            /* Compile regex or get it from cache. */
            if ((pce = pcre_get_compiled_regex_cache(pattern, pattern_len TSRMLS_CC)) == NULL) {
                zend_throw_exception(zend_exception_get_default(TSRMLS_C), "PCRE pattern compile failed.", 0 TSRMLS_CC);
            }

            zval *pcre_ret;
            ALLOC_INIT_ZVAL(pcre_ret);
            php_pcre_match_impl(pce, path, path_len, pcre_ret, z_subpats, 0, 0, 0, 0 TSRMLS_CC);

            if ( ! Z_BVAL_P(pcre_ret) ) {
                continue;
            }

            HashTable *subpats_hash = NULL;
            subpats_hash = Z_ARRVAL_P(z_subpats);

            if ( z_subpats == NULL ) {
                ALLOC_INIT_ZVAL(z_subpats);
                array_init(z_subpats);
            }

            // apply "default" value to "vars"
            /*
                foreach( $route['variables'] as $k ) {
                    if( isset($regs[$k]) ) {
                        $route['vars'][ $k ] = $regs[$k];
                    } else {
                        $route['vars'][ $k ] = $route['default'][ $k ];
                    }
                }
            */
            /*
            zval **z_route_default;
            zval **z_route_subpat_val;
            if ( zend_hash_find(z_route_options, "default", sizeof("default"), (void**) &z_route_default ) == FAILURE ) {
                HashPosition  default_pointer;
                HashTable    *default_hash;

                variables_hash = Z_ARRVAL_PP(z_variables);

                // foreach variables as var, check if url contains variable or we should apply default value
                for(zend_hash_internal_pointer_reset_ex(variables_hash, &variables_pointer); 
                        zend_hash_get_current_data_ex(variables_hash, (void**) &z_var_name, &variables_pointer) == SUCCESS; 
                        zend_hash_move_forward_ex(variables_hash, &variables_pointer)) 
                {
                }
                // if ( zend_hash_find(z_route_default, "default", sizeof("default"), (void**) &z_route_default ) == FAILURE ) {
            }
            */
            add_assoc_zval(*z_route_options , "vars" , z_subpats );
            zval_copy_ctor(*z_route_options);
            return *z_route;
        } else {
            // normal string comparison
            pattern = Z_STRVAL_PP( z_pattern );
            pattern_len = Z_STRLEN_PP( z_pattern );

            // pattern-prefix match
            if ( strncmp(pattern, path, pattern_len) == 0 ) {
                return *z_route;
            }
        }
    }
    return NULL;
}




/*
 * phux_match( $routes, $path );
 */
PHP_FUNCTION(phux_match)
{
    zval *z_routes;
    char *path;
    int  path_len;

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "as", 
                    &z_routes, 
                    &path, &path_len ) == FAILURE) {
        RETURN_FALSE;
    }

    zval *z_route;
    z_route = php_phux_match(z_routes, path, path_len);
    if ( z_route != NULL ) {
        *return_value = *z_route;
        zval_copy_ctor(z_route);
        return;
    }
    RETURN_NULL();
}

PHP_MINIT_FUNCTION(phux) {
  phux_init_mux(TSRMLS_C);
  return SUCCESS;
}

