
#include "php.h"
#include "string.h"
#include "main/php_main.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "zend_object_handlers.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_string.h"
#include "php_functions.h"

/*
 * pux_match(array $routes, string $path);
 */
PHP_FUNCTION(pux_match)
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
    z_route = php_pux_match(z_routes, path, path_len TSRMLS_CC);
    if ( z_route != NULL ) {
        *return_value = *z_route;
        zval_copy_ctor(return_value);
        // zval_ptr_dtor(&z_route);
        return;
    }
    RETURN_NULL();
}

PHP_FUNCTION(pux_sort_routes)
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
    if ( Z_BVAL_PP(a_pcre) && Z_BVAL_PP(b_pcre) ) {
        zend_hash_find( Z_ARRVAL_PP(a_options) , "compiled", strlen("compiled"), (void**)&a_compiled_pattern);
        zend_hash_find( Z_ARRVAL_PP(b_options) , "compiled", strlen("compiled"), (void**)&b_compiled_pattern);

        int a_len = Z_STRLEN_PP(a_compiled_pattern);
        int b_len = Z_STRLEN_PP(b_compiled_pattern);
        if ( a_len == b_len ) {
            RETURN_LONG(0);
        } else if ( a_len > b_len ) {
            RETURN_LONG(-1);
        } else {
            RETURN_LONG(1);
        }
    }
    else if ( Z_BVAL_PP(a_pcre) ) {
        RETURN_LONG(-1);
    }
    else if ( Z_BVAL_PP(b_pcre) ) {
        RETURN_LONG(1);
    }

    int a_len = Z_STRLEN_PP(a_pattern);
    int b_len = Z_STRLEN_PP(b_pattern);

    if ( a_len == b_len ) {
        RETURN_LONG(0);
    }
    else if ( a_len > b_len ) {
        RETURN_LONG(-1);
    }
    else {
        RETURN_LONG(1);
    }
}


int validate_request_method(zval **z_route_options_pp, int current_request_method)
{
    zval **z_route_method = NULL;
    if ( zend_hash_find( Z_ARRVAL_PP(z_route_options_pp) , "method", sizeof("method"), (void**) &z_route_method ) == SUCCESS ) {
        if ( Z_TYPE_PP(z_route_method) == IS_LONG && Z_LVAL_PP(z_route_method) != current_request_method ) {
            return 0;
        }
    }
    return 1;
}

int validate_https(zval **z_route_options_pp, int https) {
    zval **z_route_secure = NULL;
    if ( zend_hash_find( Z_ARRVAL_PP(z_route_options_pp) , "secure", sizeof("secure"), (void**) &z_route_secure ) == SUCCESS ) {
        // check HTTPS flag
        if ( https && ! Z_BVAL_PP(z_route_secure) ) {
            return 0;
        }
    }
    return 1;
}

int validate_domain(zval **z_route_options_pp, zval * http_host) {
    zval **z_route_domain = NULL;
    if ( zend_hash_find( Z_ARRVAL_PP(z_route_options_pp) , "domain", sizeof("domain"), (void**) &z_route_domain ) == SUCCESS ) {
        // check HTTP_HOST from $_SERVER
        if ( strncmp(Z_STRVAL_PP(z_route_domain), Z_STRVAL_P(http_host), Z_STRLEN_PP(z_route_domain) ) != 0 ) {
            return 0;
        }
    }
    return 1;
}

// 
// int zend_hash_has_key( )
//
zval * php_pux_match(zval *z_routes, char *path, int path_len TSRMLS_DC) {

    int current_request_method;
    int current_https;
    zval * current_http_host;

    current_request_method = get_current_request_method_const(TSRMLS_CC);
    current_https          = get_current_https(TSRMLS_CC);
    current_http_host      = get_current_http_host(TSRMLS_CC);

    HashPosition z_routes_pointer;
    HashTable    *z_routes_hash;

    z_routes_hash = Z_ARRVAL_P(z_routes);

    // for iterating routes
    zval **z_route_pp;

    zval **z_is_pcre_pp; // route[0]
    zval **z_pattern_pp; // route[1]
    // callback @ route[2]
    zval **z_route_options_pp; // route[3]

    pcre_cache_entry *pce;              /* Compiled regular expression */

    for(zend_hash_internal_pointer_reset_ex(z_routes_hash, &z_routes_pointer); 
            zend_hash_get_current_data_ex(z_routes_hash, (void**) &z_route_pp, &z_routes_pointer) == SUCCESS; 
            zend_hash_move_forward_ex(z_routes_hash, &z_routes_pointer)) 
    {
        if ( zend_hash_index_find( Z_ARRVAL_PP(z_route_pp), 0, (void**) &z_is_pcre_pp) == FAILURE ) {
            continue;
        }
        if ( zend_hash_index_find( Z_ARRVAL_PP(z_route_pp), 1, (void**) &z_pattern_pp) == FAILURE ) {
            continue;
        }
        if ( zend_hash_index_find( Z_ARRVAL_PP(z_route_pp), 3, (void**) &z_route_options_pp) == FAILURE ) {
            continue;
        }

        if ( Z_BVAL_PP(z_is_pcre_pp) ) {
            /* Compile regex or get it from cache. */
            if ((pce = pcre_get_compiled_regex_cache(Z_STRVAL_PP(z_pattern_pp), Z_STRLEN_PP(z_pattern_pp) TSRMLS_CC)) == NULL) {
                zend_throw_exception(zend_exception_get_default(TSRMLS_C), "PCRE pattern compile failed.", 0 TSRMLS_CC);
                return NULL;
            }

            zval *z_subpats = NULL; /* Array for subpatterns */
            zval *pcre_ret = NULL;
            ALLOC_INIT_ZVAL(z_subpats);
            ALLOC_INIT_ZVAL(pcre_ret);
            php_pcre_match_impl(pce, path, path_len, pcre_ret, z_subpats, 0, 0, 0, 0 TSRMLS_CC);

            // is matched ?
            if ( ! Z_BVAL_P(pcre_ret) ) {
                zval_ptr_dtor(&pcre_ret);
                zval_ptr_dtor(&z_subpats);
                continue;
            }

            // check conditions
            if ( 0 == validate_request_method( z_route_options_pp, current_request_method ) ) {
                continue;
            }
            if ( 0 == validate_https( z_route_options_pp, current_https ) ) {
                continue;
            }
            if ( 0 == validate_domain( z_route_options_pp, current_http_host ) ) {
                continue;
            }


            if ( z_subpats == NULL ) {
                ALLOC_INIT_ZVAL(z_subpats);
                array_init(z_subpats);
            } else if ( Z_TYPE_P(z_subpats) == IS_NULL ) {
                array_init(z_subpats);
            }

            zval_ptr_dtor(&pcre_ret);
            Z_ADDREF_P(z_subpats);
            add_assoc_zval(*z_route_options_pp , "vars" , z_subpats);
            return *z_route_pp;
            // Apply "default" value to "vars"
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
            if ( zend_hash_find(z_route_options_pp, "default", sizeof("default"), (void**) &z_route_default ) == FAILURE ) {
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
        } else {
            // normal string comparison
            // pattern-prefix match
            if ( strncmp(Z_STRVAL_PP( z_pattern_pp ), path, Z_STRLEN_PP( z_pattern_pp )) == 0 ) {

                // check conditions
                if ( 0 == validate_request_method( z_route_options_pp, current_request_method ) ) {
                    continue;
                }
                if ( 0 == validate_https( z_route_options_pp, current_https ) ) {
                    continue;
                }
                if ( 0 == validate_domain( z_route_options_pp, current_http_host ) ) {
                    continue;
                }
                return *z_route_pp;
            }
        }
    }
    return NULL;
}

zval * fetch_server_var( char *key , int key_len ) {
    zval **z_server_hash;
    zval **rv;
    if (zend_hash_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER"), (void **) &z_server_hash) == SUCCESS &&
        Z_TYPE_PP(z_server_hash) == IS_ARRAY &&
        zend_hash_find(Z_ARRVAL_PP(z_server_hash), key, key_len, (void **) &rv) == SUCCESS
    ) {
        return *rv;
    }
    return NULL;
}

zval * get_current_remote_addr(TSRMLS_D) {
    // REMOTE_ADDR
    return fetch_server_var( "REMOTE_ADDR", sizeof("REMOTE_ADDR") );
}

zval * get_current_http_host(TSRMLS_D) {
    return fetch_server_var( "HTTP_HOST", sizeof("HTTP_HOST") );
}

zval * get_current_request_uri(TSRMLS_D) {
    return fetch_server_var( "REQUEST_URI", sizeof("REQUEST_URI") );
}

int get_current_https(TSRMLS_D) {
    zval **z_server_hash;
    zval *https = fetch_server_var( "HTTPS", sizeof("HTTPS") );
    if ( https && Z_BVAL_P(https) ) {
        return 1;
    }
    return 0;
}

zval * get_current_request_method(TSRMLS_D) {
    return fetch_server_var( "REQUEST_METHOD", sizeof("REQUEST_METHOD") );
}

/**
 * get request method type in constant value.
 */
int get_current_request_method_const(TSRMLS_D) {
    char *c_request_method;
    zval *z_request_method = get_current_request_method();
    if ( z_request_method ) {
        c_request_method = Z_STRVAL_P(z_request_method);
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

