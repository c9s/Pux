/*
vim:fdm=marker:et:sw=4:ts=4:sts=4:
*/

#include "php.h"
#include "string.h"
#include "pcre.h"
#include "main/php_main.h"
#include "Zend/zend_compile.h"
#include "Zend/zend_alloc.h"
#include "Zend/zend_operators.h"
#include "Zend/zend_API.h"
#include "zend_exceptions.h"
#include "Zend/zend_extensions.h"
#include "zend_interfaces.h"
#include "zend_object_handlers.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_string.h"
#include "php_pux.h"
#include "pux_functions.h"
#include "php_expandable_mux.h"
#include "hash.h"


/**
 * new_dst = ht_copy_fun_t(NULL, src);
 *
 * Can be our zval copy function
 */
/* {{{ my_copy_zval_ptr */
zval** my_copy_zval_ptr(zval** dst, const zval** src, int persistent TSRMLS_DC)
{
    zval* dst_new;
    assert(src != NULL);
    if (!dst) {
        dst = (zval**) pemalloc(sizeof(zval*), persistent);
    }
    CHECK(dst[0] = (zval*) pemalloc(sizeof(zval), persistent));
    CHECK(dst_new = my_copy_zval(*dst, *src, persistent TSRMLS_CC));
    if (dst_new != *dst) {
        *dst = dst_new;
    }
    return dst;
}
/* }}} */




/* {{{ my_copy_zval */
zval* my_copy_zval(zval* dst, const zval* src, int persistent TSRMLS_DC)
{
    zval **tmp;
    assert(dst != NULL);
    assert(src != NULL);
    memcpy(dst, src, sizeof(zval));

    /* deep copies are refcount(1), but moved up for recursive 
     * arrays,  which end up being add_ref'd during its copy. */
    Z_SET_REFCOUNT_P(dst, 1);
    Z_UNSET_ISREF_P(dst);

    switch (src->type & IS_CONSTANT_TYPE_MASK) {
    case IS_RESOURCE:
        php_error(E_ERROR, "Cannot copy resource");
        break;
    case IS_BOOL:
    case IS_LONG:
    case IS_DOUBLE:
    case IS_NULL:
        break;

    case IS_CONSTANT:
    case IS_STRING:
        if (src->value.str.val) {
            dst->value.str.val = pestrndup(src->value.str.val, src->value.str.len, persistent);
        }
        break;

#if ZEND_EXTENSION_API_NO < PHP_5_5_X_API_NO
    case IS_CONSTANT_ARRAY:
#endif
    case IS_ARRAY:
        dst->value.ht = my_copy_hashtable(NULL, src->value.ht, (ht_copy_fun_t) my_copy_zval_ptr, (void*) &tmp, sizeof(zval *), persistent TSRMLS_CC);
        break;

    // XXX: we don't serialize object.
    case IS_OBJECT:
        php_error(E_ERROR, "Cannot copy Object.");
        break;
#ifdef ZEND_ENGINE_2_4
    case IS_CALLABLE:
        php_error(E_ERROR, "Cannot copy Callable.");
        // XXX: we don't serialize callbable object.
        break;
#endif
    default:
        assert(0);
    }
    return dst;
}
/* }}} */

/* my_zval_copy_ctor_func {{{*/
void my_zval_copy_ctor_func(zval *zvalue ZEND_FILE_LINE_DC)
{
    switch (Z_TYPE_P(zvalue) & IS_CONSTANT_TYPE_MASK) {
        case IS_RESOURCE: {
                TSRMLS_FETCH();

                zend_list_addref(zvalue->value.lval);
            }
            break;
        case IS_BOOL:
        case IS_LONG:
        case IS_NULL:
            break;
        case IS_CONSTANT:
        case IS_STRING:
            CHECK_ZVAL_STRING_REL(zvalue);
            if (!IS_INTERNED(zvalue->value.str.val)) {
                zvalue->value.str.val = (char *) estrndup_rel(zvalue->value.str.val, zvalue->value.str.len);
            }
            break;
#if ZEND_EXTENSION_API_NO < PHP_5_5_X_API_NO
        case IS_CONSTANT_ARRAY:
#endif
        case IS_ARRAY: {
                zval *tmp;
                HashTable *original_ht = zvalue->value.ht;
                HashTable *tmp_ht = NULL;
                TSRMLS_FETCH();

                if (zvalue->value.ht == &EG(symbol_table)) {
                    return; /* do nothing */
                }
                ALLOC_HASHTABLE_REL(tmp_ht);
                zend_hash_init(tmp_ht, zend_hash_num_elements(original_ht), NULL, ZVAL_PTR_DTOR, 0);
                zend_hash_copy(tmp_ht, original_ht, (copy_ctor_func_t) my_zval_copy_ctor_func, (void *) &tmp, sizeof(zval *));
                zvalue->value.ht = tmp_ht;
            }
            break;
        case IS_OBJECT:
            {
                TSRMLS_FETCH();
                Z_OBJ_HT_P(zvalue)->add_ref(zvalue TSRMLS_CC);
            }
            break;
    }
    Z_ADDREF_P(zvalue);
}
/*}}}*/

// my_zval_copy_ctor_persistent_func {{{
void my_zval_copy_ctor_persistent_func(zval *zvalue ZEND_FILE_LINE_DC)
{
    /*
    zval *orig_zvalue;
    orig_zvalue = zvalue;
    zvalue = pemalloc(sizeof(zval), 1);
    *zvalue = *zvalue;
    zval_copy_ctor(zvalue);
    Z_SET_REFCOUNT_P(zvalue, 1);
    Z_UNSET_ISREF_P(zvalue);
    // MAKE_COPY_ZVAL(&new_zvalue, zvalue);
    SEPARATE_ZVAL(&zvalue);
    */

    switch (Z_TYPE_P(zvalue) & IS_CONSTANT_TYPE_MASK) {
        case IS_RESOURCE:
        case IS_BOOL:
        case IS_LONG:
        case IS_DOUBLE:
        case IS_NULL:
            break;

        case IS_CONSTANT:
        case IS_STRING:
            CHECK_ZVAL_STRING_REL(zvalue);
            zvalue->value.str.val = (char *) pestrndup(zvalue->value.str.val, zvalue->value.str.len, 1);
            break;


#if ZEND_EXTENSION_API_NO < PHP_5_5_X_API_NO
        case IS_CONSTANT_ARRAY:
#endif
        case IS_ARRAY: {
                zval *tmp;
                HashTable *original_ht = zvalue->value.ht;
                HashTable *tmp_ht = NULL;
                TSRMLS_FETCH();
                if (zvalue->value.ht == &EG(symbol_table)) {
                    return; /* do nothing */
                }
                tmp_ht = pemalloc(sizeof(HashTable), 1);
                zend_hash_init(tmp_ht, zend_hash_num_elements(original_ht), NULL, ZVAL_PTR_DTOR, 1);
                zend_hash_copy(tmp_ht, original_ht, (copy_ctor_func_t) my_zval_copy_ctor_persistent_func, (void *) &tmp, sizeof(zval *));
                zvalue->value.ht = tmp_ht;
            }
            break;
        case IS_OBJECT:
            {
                TSRMLS_FETCH();
                Z_OBJ_HT_P(zvalue)->add_ref(zvalue TSRMLS_CC);
            }
            break;
    }
    Z_SET_REFCOUNT_P(zvalue, 1);
}
// }}}

int mux_loader(char *path, zval *result TSRMLS_DC) 
{
    zend_file_handle file_handle;
    zend_op_array   *op_array;
    char realpath[MAXPATHLEN];

    if (!VCWD_REALPATH(path, realpath)) {
        return FAILURE;
    }

    file_handle.filename = path;
    file_handle.free_filename = 0;
    file_handle.type = ZEND_HANDLE_FILENAME;
    file_handle.opened_path = NULL;
    file_handle.handle.fp = NULL;

    op_array = zend_compile_file(&file_handle, ZEND_INCLUDE TSRMLS_CC);

    if (op_array && file_handle.handle.stream.handle) {
        int dummy = 1;

        if (!file_handle.opened_path) {
            file_handle.opened_path = path;
        }

        // the key of hash is null-terminated string
        zend_hash_add(&EG(included_files), file_handle.opened_path, strlen(file_handle.opened_path)+1, (void *)&dummy, sizeof(int), NULL);
    }
    zend_destroy_file_handle(&file_handle TSRMLS_CC);

    if (op_array) {
        zval *local_retval_ptr = NULL;

        PUX_STORE_EG_ENVIRON();
        EG(return_value_ptr_ptr) = &local_retval_ptr;
        EG(active_op_array)      = op_array;

#if ((PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)) || (PHP_MAJOR_VERSION > 5)
        if (!EG(active_symbol_table)) {
            zend_rebuild_symbol_table(TSRMLS_C);
        }
#endif
        zend_execute(op_array TSRMLS_CC);

        destroy_op_array(op_array TSRMLS_CC);
        efree(op_array);

        if (!EG(exception)) {
            if (local_retval_ptr) {
                if ( result ) {
                    COPY_PZVAL_TO_ZVAL(*result, local_retval_ptr);
                } else {
                    zval_ptr_dtor(EG(return_value_ptr_ptr));
                }
            }
        }
        PUX_RESTORE_EG_ENVIRON();
        return SUCCESS;
    }

    return FAILURE;
}


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

    zval **a_pcre = NULL;
    zval **a_pattern = NULL;
    zval **a_compiled_pattern = NULL;
    zval **a_options = NULL;

    zval **b_pcre = NULL;
    zval **b_pattern = NULL;
    zval **b_compiled_pattern = NULL;
    zval **b_options = NULL;


    if (zend_hash_index_find( Z_ARRVAL_P(a) , 0, (void**)&a_pcre) == FAILURE
        || zend_hash_index_find( Z_ARRVAL_P(b) , 0, (void**)&b_pcre) == FAILURE 
        || zend_hash_index_find( Z_ARRVAL_P(a) , 1, (void**)&a_pattern) == FAILURE 
        || zend_hash_index_find( Z_ARRVAL_P(b) , 1, (void**)&b_pattern) == FAILURE 
        || zend_hash_index_find( Z_ARRVAL_P(a) , 3, (void**)&a_options) == FAILURE 
        || zend_hash_index_find( Z_ARRVAL_P(b) , 3, (void**)&b_options) == FAILURE 
        )
    {
        RETURN_LONG(0);
    }

    // return strlen($a[3]['compiled']) > strlen($b[3]['compiled']);
    if ( Z_BVAL_PP(a_pcre) && Z_BVAL_PP(b_pcre) ) {
        zend_hash_quick_find( Z_ARRVAL_PP(a_options) , "compiled", sizeof("compiled"), zend_inline_hash_func(ZEND_STRS("compiled")), (void**)&a_compiled_pattern);
        zend_hash_quick_find( Z_ARRVAL_PP(b_options) , "compiled", sizeof("compiled"), zend_inline_hash_func(ZEND_STRS("compiled")), (void**)&b_compiled_pattern);

        int a_len = Z_STRLEN_PP(a_compiled_pattern);
        int b_len = Z_STRLEN_PP(b_compiled_pattern);
        // php_printf("%lu vs %lu\n", a_len, b_len);
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

// 
// int zend_hash_has_key( )
//
zval * php_pux_match(zval *z_routes, char *path, int path_len TSRMLS_DC) {
    int current_request_method = 0;
    int current_https = 0;
    zval * current_http_host = NULL;

    HashTable *server_vars_hash = fetch_server_vars_hash(TSRMLS_C);
    if ( server_vars_hash ) {
        current_request_method = get_current_request_method_const(server_vars_hash TSRMLS_CC);
        current_https          = get_current_https(server_vars_hash TSRMLS_CC);
        current_http_host      = get_current_http_host(server_vars_hash TSRMLS_CC);
    }

    HashPosition z_routes_pointer;

    // for iterating routes
    zval **z_route_pp;

    zval **z_is_pcre_pp; // route[0]
    zval **z_pattern_pp; // route[1]
    zval **z_callback_pp; // callback @ route[2]
    zval **z_route_options_pp; // route[3]

    pcre_cache_entry *pce;              /* Compiled regular expression */

    zval *pcre_ret = NULL;
    zval *pcre_subpats = NULL; /* Array for subpatterns */

    HashTable * z_routes_hash = Z_ARRVAL_P(z_routes);

    ALLOC_INIT_ZVAL(pcre_ret); // this is required.
    ALLOC_INIT_ZVAL(pcre_subpats); // also required

    for(zend_hash_internal_pointer_reset_ex(z_routes_hash, &z_routes_pointer); 
            zend_hash_get_current_data_ex(z_routes_hash, (void**) &z_route_pp, &z_routes_pointer) == SUCCESS; 
            zend_hash_move_forward_ex(z_routes_hash, &z_routes_pointer)) 
    {
        zend_hash_index_find( Z_ARRVAL_PP(z_route_pp), 0, (void**) &z_is_pcre_pp);
        zend_hash_index_find( Z_ARRVAL_PP(z_route_pp), 1, (void**) &z_pattern_pp);

        if ( Z_BVAL_PP(z_is_pcre_pp) ) {
            /* Compile regex or get it from cache. */
            if ((pce = pcre_get_compiled_regex_cache(Z_STRVAL_PP(z_pattern_pp), Z_STRLEN_PP(z_pattern_pp) TSRMLS_CC)) == NULL) {
                zend_throw_exception(zend_exception_get_default(TSRMLS_C), "PCRE pattern compile failed.", 0 TSRMLS_CC);
                return NULL;
            }

            php_pcre_match_impl(pce, path, path_len, pcre_ret, pcre_subpats, 0, 0, 0, 0 TSRMLS_CC);

            // not matched ?
            if ( ! Z_BVAL_P(pcre_ret) ) {
                continue;
            }

            // tell garbage collector to collect it, we need to use pcre_subpats later.

            // check conditions only when route option is provided
            if ( zend_hash_index_find( Z_ARRVAL_PP(z_route_pp), 3, (void**) &z_route_options_pp) == SUCCESS ) {
                if ( zend_hash_num_elements(Z_ARRVAL_PP(z_route_options_pp)) ) {
                    if ( 0 == validate_request_method( z_route_options_pp, current_request_method TSRMLS_CC) ) {
                        continue;
                    }
                    if ( 0 == validate_https( z_route_options_pp, current_https TSRMLS_CC) ) {
                        continue;
                    }
                    if ( 0 == validate_domain( z_route_options_pp, current_http_host TSRMLS_CC) ) {
                        continue;
                    }
                }
            }

            if ( Z_TYPE_P(pcre_subpats) == IS_NULL ) {
                array_init(pcre_subpats);
            }

            Z_ADDREF_P(pcre_subpats);
            add_assoc_zval(*z_route_options_pp , "vars" , pcre_subpats);
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
            zend_hash_index_find( Z_ARRVAL_PP(z_route_pp), 2, (void**) &z_callback_pp);
            if ( ( Z_TYPE_PP(z_callback_pp) == IS_LONG && strncmp(Z_STRVAL_PP( z_pattern_pp ), path, Z_STRLEN_PP(z_pattern_pp) ) == 0 )
                 || strcmp(Z_STRVAL_PP( z_pattern_pp ), path ) == 0 ) 
            {

                // check conditions
                if ( zend_hash_index_find( Z_ARRVAL_PP(z_route_pp), 3, (void**) &z_route_options_pp) == SUCCESS ) {
                    if ( zend_hash_num_elements(Z_ARRVAL_PP(z_route_options_pp)) ) {
                        if ( 0 == validate_request_method( z_route_options_pp, current_request_method TSRMLS_CC) ) {
                            continue;
                        }
                        if ( 0 == validate_https( z_route_options_pp, current_https TSRMLS_CC) ) {
                            continue;
                        }
                        if ( 0 == validate_domain( z_route_options_pp, current_http_host TSRMLS_CC) ) {
                            continue;
                        }
                    }
                }
                // we didn't use the pcre variables
                zval_ptr_dtor(&pcre_subpats);
                zval_ptr_dtor(&pcre_ret);
                return *z_route_pp;
            }
        }
    }
    zval_ptr_dtor(&pcre_subpats);
    zval_ptr_dtor(&pcre_ret);
    return NULL;
}

HashTable * fetch_server_vars_hash(TSRMLS_D) {
    zval **z_server_hash = NULL;
    if ( zend_hash_quick_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER"), zend_inline_hash_func(ZEND_STRS("_SERVER")), (void **) &z_server_hash) == SUCCESS ) {
        return Z_ARRVAL_PP(z_server_hash);
    }
    return NULL;
}

zval * fetch_server_var(HashTable * server_hash, char *key , int key_len TSRMLS_DC) {
    zval **rv;
    if ( zend_hash_find(server_hash, key, key_len, (void **) &rv) == SUCCESS ) {
        return *rv;
    }
    return NULL;
}

zval * get_current_remote_addr(HashTable * server_vars_hash TSRMLS_DC) {
    // REMOTE_ADDR
    return fetch_server_var(server_vars_hash, "REMOTE_ADDR", sizeof("REMOTE_ADDR") TSRMLS_CC);
}

zval * get_current_http_host(HashTable * server_vars_hash TSRMLS_DC) {
    return fetch_server_var(server_vars_hash, "HTTP_HOST", sizeof("HTTP_HOST") TSRMLS_CC);
}

zval * get_current_request_uri(HashTable * server_vars_hash TSRMLS_DC) {
    return fetch_server_var(server_vars_hash, "REQUEST_URI", sizeof("REQUEST_URI") TSRMLS_CC);
}

int get_current_https(HashTable * server_vars_hash TSRMLS_DC) {
    zval *https = fetch_server_var(server_vars_hash, "HTTPS", sizeof("HTTPS") TSRMLS_CC);
    if ( https && Z_BVAL_P(https) ) {
        return 1;
    }
    return 0;
}

zval * get_current_request_method(HashTable * server_vars_hash TSRMLS_DC) {
    return fetch_server_var(server_vars_hash, "REQUEST_METHOD", sizeof("REQUEST_METHOD") TSRMLS_CC);
}


int method_str_to_method_const(char * c_request_method ) {
    if ( strncmp("GET", c_request_method , sizeof("GET") ) == 0 ) {
        return REQUEST_METHOD_GET;
    } else if ( strncmp("POST", c_request_method , sizeof("POST") ) == 0 ) {
        return REQUEST_METHOD_POST;
    } else if ( strncmp("PUT" , c_request_method , sizeof("PUT") ) == 0 ) {
        return REQUEST_METHOD_PUT;
    } else if ( strncmp("DELETE", c_request_method, sizeof("DELETE")  ) == 0 ) {
        return REQUEST_METHOD_DELETE;
    } else if ( strncmp("HEAD", c_request_method, sizeof("HEAD")  ) == 0 ) {
        return REQUEST_METHOD_HEAD;
    } else if ( strncmp("PATCH", c_request_method, sizeof("PATCH")  ) == 0 ) {
        return REQUEST_METHOD_HEAD;
    } else if ( strncmp("OPTIONS", c_request_method, sizeof("OPTIONS")  ) == 0 ) {
        return REQUEST_METHOD_OPTIONS;
    }
    return 0;
}

/*
 * get request method type in constant value. {{{
 */
int get_current_request_method_const(HashTable * server_vars_hash TSRMLS_DC) {
    char *c_request_method;
    zval *z_request_method = get_current_request_method(server_vars_hash TSRMLS_CC);
    if ( z_request_method ) {
        c_request_method = Z_STRVAL_P(z_request_method);
        return method_str_to_method_const(c_request_method);
    }
    return 0;
}
// }}}

int validate_https(zval **z_route_options_pp, int https TSRMLS_DC) 
{
    zval **z_route_secure = NULL;
    if ( zend_hash_quick_find( Z_ARRVAL_PP(z_route_options_pp) , "secure", sizeof("secure"), zend_inline_hash_func(ZEND_STRS("secure")), (void**) &z_route_secure ) == SUCCESS ) {
        // check HTTPS flag
        if ( https && ! Z_BVAL_PP(z_route_secure) ) {
            return 0;
        }
    }
    return 1;
}

int validate_domain(zval **z_route_options_pp, zval * http_host TSRMLS_DC) 
{
    zval **z_route_domain = NULL;
    if ( zend_hash_quick_find( Z_ARRVAL_PP(z_route_options_pp) , "domain", sizeof("domain"), zend_inline_hash_func(ZEND_STRS("domain")), (void**) &z_route_domain ) == SUCCESS ) {
        // check HTTP_HOST from $_SERVER
        if ( strncmp(Z_STRVAL_PP(z_route_domain), Z_STRVAL_P(http_host), Z_STRLEN_PP(z_route_domain) ) != 0 ) {
            return 0;
        }
    }
    return 1;
}

int validate_request_method(zval **z_route_options_pp, int current_request_method TSRMLS_DC)
{
    zval **z_route_method = NULL;
    if ( zend_hash_quick_find( Z_ARRVAL_PP(z_route_options_pp) , "method", sizeof("method"),  zend_inline_hash_func(ZEND_STRS("method")), (void**) &z_route_method ) == SUCCESS ) {
        if ( Z_TYPE_PP(z_route_method) == IS_LONG && Z_LVAL_PP(z_route_method) != current_request_method ) {
            return 0;
        }
    }
    return 1;
}

