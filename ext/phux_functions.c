
#include "php.h"
#include "string.h"
#include "main/php_main.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "zend_object_handlers.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_string.h"
#include "phux_functions.h"

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

PHP_FUNCTION(phux_sort_routes)
{
    zval *a;
    zval *b;

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", 
                    &a, 
                    &b ) == FAILURE) {
        RETURN_FALSE;
    }

    zval **a_pcre;
    zval **a_pattern;
    zval **a_compiled_pattern;
    zval **a_options;

    zval **b_pcre;
    zval **b_pattern;
    zval **b_compiled_pattern;
    zval **b_options;


    zend_hash_index_find( Z_ARRVAL_P(a) , 0, (void**)&a_pcre);
    zend_hash_index_find( Z_ARRVAL_P(b) , 0, (void**)&b_pcre);

    zend_hash_index_find( Z_ARRVAL_P(a) , 1, (void**)&a_pattern);
    zend_hash_index_find( Z_ARRVAL_P(b) , 1, (void**)&b_pattern);

    zend_hash_index_find( Z_ARRVAL_P(a) , 3, (void**)&a_options);
    zend_hash_index_find( Z_ARRVAL_P(b) , 3, (void**)&b_options);
    

    // return strlen($a[3]['compiled']) > strlen($b[3]['compiled']);
    if ( Z_BVAL_P(a) && Z_BVAL_P(b) ) {
        zend_hash_find( Z_ARRVAL_PP(a_options) , "compiled", strlen("compiled"), (void**)&a_compiled_pattern);
        zend_hash_find( Z_ARRVAL_PP(b_options) , "compiled", strlen("compiled"), (void**)&b_compiled_pattern);

        int a_len = Z_STRLEN_PP(a_compiled_pattern);
        int b_len = Z_STRLEN_PP(b_compiled_pattern);
        if ( a == b ) {
            RETURN_LONG(0);
        } else if ( a > b ) {
            RETURN_LONG(1);
        } else {
            RETURN_LONG(-1);
        }
    }
    else if ( Z_BVAL_P(a) ) {
        RETURN_LONG(1);
    }
    else if ( Z_BVAL_P(b) ) {
        RETURN_LONG(-1);
    }

    int a_len = Z_STRLEN_PP(a_pattern);
    int b_len = Z_STRLEN_PP(b_pattern);

    if ( a_len == b_len ) {
        RETURN_LONG(0);
    }
    else if ( a_len > b_len ) {
        RETURN_LONG(1);
    }
    else {
        RETURN_LONG(-1);
    }
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

