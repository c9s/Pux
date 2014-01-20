
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
#include "php_functions.h"
#include "php_expandable_mux.h"

#define CHECK(p) { if ((p) == NULL) return NULL; }


void my_zval_copy_ctor_persistent_func(zval *zvalue ZEND_FILE_LINE_DC);


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
                zend_hash_copy(tmp_ht, original_ht, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
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


void my_zval_copy_ctor_persistent_func(zval *zvalue ZEND_FILE_LINE_DC)
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
                zvalue->value.str.val = (char *) pestrndup(zvalue->value.str.val, zvalue->value.str.len, 1);
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


inline int persistent_store(char *key, int key_len, int list_type, void * val TSRMLS_DC)
{
    zend_rsrc_list_entry new_le;
    zend_rsrc_list_entry *le;
    if ( zend_hash_find(&EG(persistent_list), key, key_len + 1, (void**) &le) == SUCCESS ) {
        // if the key exists, delete it.
        zend_hash_del(&EG(persistent_list), key, key_len + 1);
    }
    new_le.type = list_type;
    new_le.ptr = val;
    return zend_hash_update(&EG(persistent_list), key, key_len + 1, &new_le, sizeof(zend_rsrc_list_entry), NULL);
}

inline void * persistent_fetch(char *key, int key_len TSRMLS_DC)
{
    zend_rsrc_list_entry *le;
    if ( zend_hash_find(&EG(persistent_list), key, key_len + 1, (void**) &le) == SUCCESS ) {
        return le->ptr;
    }
    return NULL;
}


inline void * pux_persistent_fetch(char *ns, char *key TSRMLS_DC)
{
    char *newkey;
    int   newkey_len;
    void *ptr;
    newkey_len = spprintf(&newkey, 0, "pux_%s_%s", ns, key);
    ptr = persistent_fetch(newkey, newkey_len TSRMLS_CC);
    efree(newkey);
    return ptr;
}

/*
 * Store persistent value with pux namespace.
 */
inline int pux_persistent_store(char *ns, char *key, void * val TSRMLS_DC) 
{
    char *newkey;
    int   newkey_len;
    int   status;
    newkey_len = spprintf(&newkey, 0, "pux_%s_%s", ns, key);
    status = persistent_store(newkey, newkey_len, le_mux_hash_persist, val TSRMLS_CC);
    efree(newkey);
    return status;
}

HashTable * zend_hash_clone_persistent(HashTable* src TSRMLS_DC)
{
    zval *tmp;
    HashTable* dst;
    dst = pecalloc(1, sizeof(HashTable), 1);
    zend_hash_init(dst, 5, NULL, NULL, 1);
    zend_hash_copy(dst, src, (copy_ctor_func_t) my_zval_copy_ctor_persistent_func, &tmp, sizeof(zval*) );
    return dst;
}


int _pux_store_mux(char *name, zval * mux TSRMLS_DC) 
{
    zend_rsrc_list_entry new_le, *le;

    // variables for copying mux object properties
    char        *id_key, *expand_key;
    int status, id_key_len, expand_key_len;

    id_key_len = spprintf(&id_key, 0, "mux_id_%s", name);
    expand_key_len = spprintf(&expand_key, 0, "mux_expand_%s", name);

    // make the hash table persistent
    zval *prop, *tmp;
    HashTable *routes_dst, *static_routes_dst; 

    prop = zend_read_property(Z_OBJCE_P(mux), mux, "routes", sizeof("routes")-1, 1 TSRMLS_CC);
    routes_dst = zend_hash_clone_persistent( Z_ARRVAL_P(prop) TSRMLS_CC);
    pux_persistent_store( name, "routes", (void*) routes_dst TSRMLS_CC);

    prop = zend_read_property(Z_OBJCE_P(mux), mux, "staticRoutes", sizeof("staticRoutes")-1, 1 TSRMLS_CC);
    static_routes_dst = zend_hash_clone_persistent( Z_ARRVAL_P(prop)  TSRMLS_CC);

    pux_persistent_store(name, "static_routes", (void *) static_routes_dst TSRMLS_CC) ;
    
    // copy ID
    prop = zend_read_property(Z_OBJCE_P(mux), mux, "id", sizeof("id")-1, 1 TSRMLS_CC);
    tmp = pemalloc(sizeof(zval), 1);
    INIT_PZVAL(tmp);
    Z_TYPE_P(tmp) = IS_LONG;
    Z_LVAL_P(tmp) = Z_LVAL_P(prop);
    pux_persistent_store( name, "id", (void*) tmp TSRMLS_CC);


    // We cannot copy un-expandable mux object because we don't support recursively copy for Mux object.
    prop = zend_read_property(Z_OBJCE_P(mux), mux, "expand", sizeof("expand")-1, 1 TSRMLS_CC);
    if ( ! Z_BVAL_P(prop) ) {
        php_error(E_ERROR, "We cannot copy un-expandable mux object because we don't support recursively copy for Mux object.");
    }
    efree(id_key);
    efree(expand_key);
    return SUCCESS;
}

zval * _pux_fetch_mux(char *name TSRMLS_DC)
{
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

    zval *z_id, *z_routes, *z_static_routes, *tmp;
    z_id = (zval*) pux_persistent_fetch(name, "id" TSRMLS_CC);
    MAKE_STD_ZVAL(z_routes);
    MAKE_STD_ZVAL(z_static_routes);

    array_init(z_routes);
    array_init(z_static_routes);

    Z_ADDREF_P(z_routes);
    Z_ADDREF_P(z_static_routes);

    zend_hash_copy( Z_ARRVAL_P(z_routes)        , routes_hash        , (copy_ctor_func_t) my_zval_copy_ctor_func , (void*) &tmp , sizeof(zval *));
    zend_hash_copy( Z_ARRVAL_P(z_static_routes) , static_routes_hash , (copy_ctor_func_t) my_zval_copy_ctor_func , (void*) &tmp , sizeof(zval *));

    // create new object and return to userspace.
    zval *new_mux = NULL;
    ALLOC_INIT_ZVAL(new_mux);
    object_init_ex(new_mux, ce_pux_mux);
    CALL_METHOD(Mux, __construct, new_mux, new_mux);

    Z_SET_REFCOUNT_P(new_mux, 1);

    if ( z_id ) {
        Z_ADDREF_P(z_id);
        zend_update_property_long(ce_pux_mux, new_mux, "id" , sizeof("id")-1, Z_LVAL_P(z_id) TSRMLS_CC);
    }
    // persistent mux should always be expanded. (no recursive structure)
    zend_update_property_bool(ce_pux_mux , new_mux , "expand"       , sizeof("expand")-1       , 1  TSRMLS_CC);
    zend_update_property(ce_pux_mux      , new_mux , "routes"       , sizeof("routes")-1       , z_routes TSRMLS_CC);
    zend_update_property(ce_pux_mux      , new_mux , "staticRoutes" , sizeof("staticRoutes")-1 , z_static_routes TSRMLS_CC);
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
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", 
                    &name, &name_len ,
                    &mux ) == FAILURE) {
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
        // zend_print_zval_r(retval, 0 TSRMLS_CC);
        if ( _pux_store_mux(ns, mux TSRMLS_CC) == FAILURE ) {
            php_error(E_ERROR, "Can not store Mux object from %s", filename);
        }
    }

    /*
    *return_value = *mux;
    zval_copy_ctor(return_value);

    // php_debug_zval_dump( &mux, 1 TSRMLS_CC);
    // php_var_dump(&mux, 1);
    return;
    */

    ALLOC_INIT_ZVAL(z_path);
    ZVAL_STRINGL(z_path, path ,path_len, 1); // no copy

    // do dispatch
    route = call_mux_method(mux, "dispatch" , sizeof("dispatch"), 1 , z_path, NULL, NULL TSRMLS_CC);
    zval_ptr_dtor(&z_path);

    if ( route ) {
        Z_ADDREF_P(route);
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
        zend_hash_quick_find( Z_ARRVAL_PP(a_options) , "compiled", strlen("compiled"), zend_inline_hash_func(ZEND_STRS("compiled")), (void**)&a_compiled_pattern);
        zend_hash_quick_find( Z_ARRVAL_PP(b_options) , "compiled", strlen("compiled"), zend_inline_hash_func(ZEND_STRS("compiled")), (void**)&b_compiled_pattern);

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


inline int validate_request_method(zval **z_route_options_pp, int current_request_method TSRMLS_DC)
{
    zval **z_route_method = NULL;
    if ( zend_hash_quick_find( Z_ARRVAL_PP(z_route_options_pp) , "method", sizeof("method"),  zend_inline_hash_func(ZEND_STRS("method")), (void**) &z_route_method ) == SUCCESS ) {
        if ( Z_TYPE_PP(z_route_method) == IS_LONG && Z_LVAL_PP(z_route_method) != current_request_method ) {
            return 0;
        }
    }
    return 1;
}

inline int validate_https(zval **z_route_options_pp, int https TSRMLS_DC) 
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

inline int validate_domain(zval **z_route_options_pp, zval * http_host TSRMLS_DC) 
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

// 
// int zend_hash_has_key( )
//
inline zval * php_pux_match(zval *z_routes, char *path, int path_len TSRMLS_DC) {
    int current_request_method;
    int current_https;
    zval * current_http_host;

    zval **server_vars_hash = fetch_server_vars_hash(TSRMLS_C);
    current_request_method = get_current_request_method_const(server_vars_hash TSRMLS_CC);
    current_https          = get_current_https(server_vars_hash TSRMLS_CC);
    current_http_host      = get_current_http_host(server_vars_hash TSRMLS_CC);

    HashPosition z_routes_pointer;

    // for iterating routes
    zval **z_route_pp;

    zval **z_is_pcre_pp; // route[0]
    zval **z_pattern_pp; // route[1]
    // callback @ route[2]
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
                if ( zend_hash_has_more_elements(Z_ARRVAL_PP(z_route_options_pp)) == SUCCESS ) {
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
            if ( strncmp(Z_STRVAL_PP( z_pattern_pp ), path, Z_STRLEN_PP( z_pattern_pp )) == 0 ) {
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

inline zval ** fetch_server_vars_hash(TSRMLS_D) {
    zval **z_server_hash = NULL;
    if ( zend_hash_quick_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER"), zend_inline_hash_func(ZEND_STRS("_SERVER")), (void **) &z_server_hash) == SUCCESS ) {
        return z_server_hash;
    }
    return NULL;
}

inline zval * fetch_server_var(zval ** z_server_hash, char *key , int key_len TSRMLS_DC) {
    zval **rv;
    if ( zend_hash_find(Z_ARRVAL_PP(z_server_hash), key, key_len, (void **) &rv) == SUCCESS ) {
        return *rv;
    }
    return NULL;
}

inline zval * get_current_remote_addr(zval ** server_vars_hash TSRMLS_DC) {
    // REMOTE_ADDR
    return fetch_server_var(server_vars_hash, "REMOTE_ADDR", sizeof("REMOTE_ADDR") TSRMLS_CC);
}

inline zval * get_current_http_host(zval ** server_vars_hash TSRMLS_DC) {
    return fetch_server_var(server_vars_hash, "HTTP_HOST", sizeof("HTTP_HOST") TSRMLS_CC);
}

inline zval * get_current_request_uri(zval ** server_vars_hash TSRMLS_DC) {
    return fetch_server_var(server_vars_hash, "REQUEST_URI", sizeof("REQUEST_URI") TSRMLS_CC);
}

inline int get_current_https(zval ** server_vars_hash TSRMLS_DC) {
    zval *https = fetch_server_var(server_vars_hash, "HTTPS", sizeof("HTTPS") TSRMLS_CC);
    if ( https && Z_BVAL_P(https) ) {
        return 1;
    }
    return 0;
}

inline zval * get_current_request_method(zval ** server_vars_hash TSRMLS_DC) {
    return fetch_server_var(server_vars_hash, "REQUEST_METHOD", sizeof("REQUEST_METHOD") TSRMLS_CC);
}

/**
 * get request method type in constant value.
 */
inline int get_current_request_method_const(zval **server_vars_hash TSRMLS_DC) {
    char *c_request_method;
    zval *z_request_method = get_current_request_method(server_vars_hash TSRMLS_CC);
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

