#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "string.h"
#include "php_phux.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_string.h"

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

    zval *z_subpats = NULL; /* Array for subpatterns */

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "as", 
                    &routes, 
                    &path, &path_len ) == FAILURE) {
        RETURN_FALSE;
    }

    if( z_subpats == NULL ) {
        ALLOC_INIT_ZVAL( z_subpats );
    }

    HashPosition route_pointer;
    HashTable    *routes_array;

    routes_array = Z_ARRVAL_P(routes);

    // for iterating routes
    zval **z_route;

    HashTable *route_item_hash;

    zval **z_is_pcre;
    zval **z_pattern;

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

        if ( Z_BVAL_PP(z_is_pcre) ) {
            // do pcre_match comparision

        } else {
            // normal string comparison
            char *pattern = Z_STRVAL_PP( z_pattern );
            int   pattern_len = Z_STRLEN_PP( z_pattern );

            // pattern-prefix match
            if ( strncmp(pattern, path, pattern_len) == 0 ) {
                *return_value = **z_route;
                // zval_copy_ctor(return_value);
                return;
            }
        }
    }
    RETURN_FALSE;
}

