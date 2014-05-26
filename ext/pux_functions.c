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
#include "zend_interfaces.h"
#include "zend_object_handlers.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_string.h"
#include "php_pux.h"
#include "pux_functions.h"
#include "pux_persistent.h"
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

    case IS_ARRAY:
    case IS_CONSTANT_ARRAY:
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
        case IS_ARRAY:
        case IS_CONSTANT_ARRAY: {
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

        case IS_ARRAY:
        case IS_CONSTANT_ARRAY: {
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

int _pux_store_mux(char *name, zval * mux TSRMLS_DC) 
{
    zend_rsrc_list_entry new_le, *le;

    // variables for copying mux object properties
    char        *id_key, *expand_key;
    int status, id_key_len, expand_key_len;

    id_key_len = spprintf(&id_key, 0, "mux_id_%s", name);
    expand_key_len = spprintf(&expand_key, 0, "mux_expand_%s", name);


    // Z_ADDREF_P(mux);

    // make the hash table persistent
    zval *prop, *tmp;
    HashTable *routes, *static_routes; 

    prop = zend_read_property(ce_pux_mux, mux, "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    routes = zend_hash_clone_persistent( Z_ARRVAL_P(prop) TSRMLS_CC);
    if ( ! routes ) {
        php_error(E_ERROR, "Can not clone HashTable");
        return FAILURE;
    }
    pux_persistent_store( name, "routes", le_mux_hash_table, (void*) routes TSRMLS_CC);
    return SUCCESS;


    prop = zend_read_property(ce_pux_mux, mux, "staticRoutes", sizeof("staticRoutes")-1, 1 TSRMLS_CC);
    static_routes = zend_hash_clone_persistent( Z_ARRVAL_P(prop)  TSRMLS_CC);
    if ( ! static_routes ) {
        php_error(E_ERROR, "Can not clone HashTable");
        return FAILURE;
    }
    pux_persistent_store(name, "static_routes", le_mux_hash_table, (void *) static_routes TSRMLS_CC) ;

    
    // copy ID
    /*
    prop = zend_read_property(ce_pux_mux, mux, "id", sizeof("id")-1, 1 TSRMLS_CC);
    tmp = pemalloc(sizeof(zval), 1);
    INIT_ZVAL(tmp);
    Z_TYPE_P(tmp) = IS_LONG;
    Z_LVAL_P(tmp) = Z_LVAL_P(prop);
    Z_SET_REFCOUNT_P(tmp, 1);
    pux_persistent_store( name, "id", (void*) tmp TSRMLS_CC);
    */

    // We cannot copy un-expandable mux object because we don't support recursively copy for Mux object.
    prop = zend_read_property(ce_pux_mux, mux, "expand", sizeof("expand")-1, 1 TSRMLS_CC);
    if ( ! Z_BVAL_P(prop) ) {
        php_error(E_ERROR, "We cannot copy un-expandable mux object because we don't support recursively copy for Mux object.");
    }
    efree(id_key);
    efree(expand_key);
    return SUCCESS;
}


/**
 * Fetch mux related properties (hash tables) from EG(persistent_list) hash table and 
 * rebless a new Mux object with these properties.
 *
 * @return Mux object
 */
zval * _pux_fetch_mux(char *name TSRMLS_DC)
{
    zval *z_id, *z_routes, *z_static_routes, *z_submux, *z_routes_by_id, *tmp;
    HashTable *routes_hash;
    HashTable *static_routes_hash;

    // fetch related hash to this mux object.
    routes_hash = (HashTable*) pux_persistent_fetch(name, "routes" TSRMLS_CC);
    if ( ! routes_hash  ) {
        return NULL;
    }

    static_routes_hash = (HashTable*) pux_persistent_fetch(name, "static_routes" TSRMLS_CC);
    if ( ! static_routes_hash ) {
        return NULL;
    }

    z_id = (zval*) pux_persistent_fetch(name, "id" TSRMLS_CC);
    MAKE_STD_ZVAL(z_routes);
    MAKE_STD_ZVAL(z_static_routes);
    MAKE_STD_ZVAL(z_routes_by_id);
    MAKE_STD_ZVAL(z_submux);

    Z_TYPE_P(z_routes) = IS_ARRAY;
    Z_TYPE_P(z_static_routes) = IS_ARRAY;
    array_init(z_routes_by_id);
    array_init(z_submux);

    // we need to clone hash table deeply because when Mux object returned to userspace, it will be freed.
    Z_ARRVAL_P(z_routes)        = zend_hash_clone(routes_hash TSRMLS_CC);
    Z_ARRVAL_P(z_static_routes) = zend_hash_clone(static_routes_hash TSRMLS_CC);

    // create new object and return to userspace.
    zval *new_mux = NULL;
    ALLOC_INIT_ZVAL(new_mux);
    object_init_ex(new_mux, ce_pux_mux);

    // We don't need __construct because we assign the property by ourself.
    // CALL_METHOD(Mux, __construct, new_mux, new_mux);
    Z_SET_REFCOUNT_P(new_mux, 1);



    if ( z_id ) {
        Z_ADDREF_P(z_id);
        zend_update_property_long(ce_pux_mux, new_mux, "id" , sizeof("id")-1, Z_LVAL_P(z_id) TSRMLS_CC);
    }
    // persistent mux should always be expanded. (no recursive structure)
    zend_update_property_bool(ce_pux_mux , new_mux , "expand"       , sizeof("expand")-1       , 1  TSRMLS_CC);
    zend_update_property(ce_pux_mux      , new_mux , "routes"       , sizeof("routes")-1       , z_routes TSRMLS_CC);
    zend_update_property(ce_pux_mux      , new_mux , "staticRoutes" , sizeof("staticRoutes")-1 , z_static_routes TSRMLS_CC);

    zend_update_property(ce_pux_mux, new_mux, "routesById", sizeof("routesById")-1, z_routes_by_id TSRMLS_CC);
    zend_update_property(ce_pux_mux, new_mux, "submux", sizeof("submux")-1, z_submux TSRMLS_CC);
    return new_mux;
}


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

PHP_FUNCTION(pux_store_mux)
{
    zval *mux;
    char *name;
    int  name_len;

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &name, &name_len, &mux ) == FAILURE) {
        RETURN_FALSE;
    }
    if ( _pux_store_mux(name, mux TSRMLS_CC) == SUCCESS ) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}


PHP_FUNCTION(pux_persistent_dispatch)
{
    char *ns, *filename, *path;
    int  ns_len, filename_len, path_len;
    zval *mux = NULL;
    zval *route = NULL;
    zval *z_path = NULL;

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &ns, &ns_len, &filename, &filename_len, &path, &path_len) == FAILURE) {
        RETURN_FALSE;
    }

    mux = _pux_fetch_mux(ns TSRMLS_CC);
    if ( mux == NULL ) {
        ALLOC_INIT_ZVAL(mux);
        if ( mux_loader(filename, mux TSRMLS_CC) == FAILURE ) {
            php_error(E_ERROR, "Can not load Mux object from %s", filename);
        }

        // TODO: compile mux and sort routes
        if ( _pux_store_mux(ns, mux TSRMLS_CC) == FAILURE ) {
            php_error(E_ERROR, "Can not store Mux object from %s", filename);
        }
    }

    ALLOC_INIT_ZVAL(z_path);
    ZVAL_STRINGL(z_path, path ,path_len, 1); // no copy

    // XXX: pass return_value to the method call, so we don't need to copy
    route = call_mux_method(mux, "dispatch" , sizeof("dispatch"), 1 , z_path, NULL, NULL TSRMLS_CC);
    zval_ptr_dtor(&z_path);

    if ( route ) {
        *return_value = *route;
        zval_copy_ctor(return_value);
        return;
    }
    // route not found
    RETURN_FALSE;
}



PHP_FUNCTION(pux_delete_mux)
{
    char *name, *persistent_key;
    int  name_len, persistent_key_len;
    zend_rsrc_list_entry *le;

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", 
                    &name, &name_len) == FAILURE) {
        RETURN_FALSE;
    }

    persistent_key_len = spprintf(&persistent_key, 0, "mux_%s", name);

    if ( zend_hash_find(&EG(persistent_list), persistent_key, persistent_key_len + 1, (void**) &le) == SUCCESS ) {
        zval_ptr_dtor((zval**) &le->ptr);
        zend_hash_del(&EG(persistent_list), persistent_key, persistent_key_len + 1);
        efree(persistent_key);
        RETURN_TRUE;
    }
    efree(persistent_key);
    RETURN_FALSE;
}

PHP_FUNCTION(pux_fetch_mux)
{
    char *name;
    int  name_len;
    zval * mux;

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", 
                    &name, &name_len) == FAILURE) {
        RETURN_FALSE;
    }

    mux = _pux_fetch_mux(name TSRMLS_CC);
    if ( mux ) {
        *return_value = *mux;
        zval_copy_ctor(return_value);
        return;
    }
    RETURN_FALSE;
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

