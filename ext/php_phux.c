#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "string.h"
#include "php_phux.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_string.h"
#include "zend_exceptions.h"

#define ZEND_HASH_FETCH(hash,key,ret) \
    zend_hash_find(hash, key, sizeof(key), (void**)&ret) == SUCCESS

// #define DEBUG 1

static const zend_function_entry phux_functions[] = {
    PHP_FE(phux_dispatch, NULL)
    PHP_FE_END
};

zend_module_entry phux_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_PHUX_EXTNAME,
    phux_functions,
    NULL,
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

#define REQ_METHOD_GET 1
#define REQ_METHOD_POST 2
#define REQ_METHOD_PUT 3
#define REQ_METHOD_DELETE 4


/**
 * get request method type in constant value.
 */
int get_current_request_method() {
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
        php_strtolower(c_request_method ,c_request_method_len);
        if ( strncmp("get", c_request_method , sizeof("get") ) == 0 ) {
            return REQ_METHOD_GET;
        } else if ( strncmp("post", c_request_method , sizeof("post") ) == 0 ) {
            return REQ_METHOD_POST;
        } else if ( strcmp("put" , c_request_method ) == 0 ) {
            return REQ_METHOD_PUT;
        } else if ( strcmp("delete", c_request_method ) == 0 ) {
            return REQ_METHOD_DELETE;
        }
    }
    return 0;
}

// int zend_hash_has_key( )


/*
 * phux_dispatch( $this->routes, $path );
 */
PHP_FUNCTION(phux_dispatch)
{
    zval *routes;
    char *path;
    int  path_len;


    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "as", 
                    &routes, 
                    &path, &path_len ) == FAILURE) {
        RETURN_FALSE;
    }

    zval *z_subpats = NULL; /* Array for subpatterns */
    ALLOC_INIT_ZVAL( z_subpats );

    HashPosition route_pointer;
    HashTable    *routes_array;

    routes_array = Z_ARRVAL_P(routes);

    // for iterating routes
    zval **z_route;

    HashTable *route_item_hash;

    zval **z_is_pcre; // route[0]
    zval **z_pattern; // route[1]
    // callback @ route[2]
    zval **z_route_options; // route[3]

    char *pattern;
    int  pattern_len;

    for(zend_hash_internal_pointer_reset_ex(routes_array, &route_pointer); 
            zend_hash_get_current_data_ex(routes_array, (void**) &z_route, &route_pointer) == SUCCESS; 
            zend_hash_move_forward_ex(routes_array, &route_pointer)) 
    {
        // read z_route
        route_item_hash = Z_ARRVAL_PP(z_route);

        if ( zend_hash_index_find( Z_ARRVAL_PP(z_route), 0, (void**) &z_is_pcre) == FAILURE ) {
            continue;
        }
        if ( zend_hash_index_find( Z_ARRVAL_PP(z_route), 1, (void**) &z_pattern) == FAILURE ) {
            continue;
        }
        if ( zend_hash_index_find( Z_ARRVAL_PP(z_route), 3, (void**) &z_route_options) == FAILURE ) {
            continue;
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
                RETURN_FALSE;
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
            add_assoc_zval( *z_route_options , "vars" , z_subpats );

            *return_value = **z_route;
            zval_copy_ctor(return_value);
            return;

            /*
            }
            */
        } else {
            // normal string comparison
            pattern = Z_STRVAL_PP( z_pattern );
            pattern_len = Z_STRLEN_PP( z_pattern );

            // pattern-prefix match
            if ( strncmp(pattern, path, pattern_len) == 0 ) {
                *return_value = **z_route;
                zval_copy_ctor(return_value);
                return;
            }
        }
    }
    RETURN_FALSE;
}

