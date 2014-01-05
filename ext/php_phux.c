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
  PHP_ME(Mux, mount, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, appendRoute, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, appendPCRERoute, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, matchRoute, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, getRoutes, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, export, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Mux, __set_state, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
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



/* {{{ zend_call_method
 Only returns the returned zval if retval_ptr != NULL */
ZEND_API zval* zend_call_method_with_3_params(zval **object_pp, zend_class_entry *obj_ce, zend_function **fn_proxy, const char *function_name, int function_name_len, zval **retval_ptr_ptr, int param_count, zval* arg1, zval* arg2, zval* arg3 TSRMLS_DC)
{
	int result;
	zend_fcall_info fci;
	zval z_fname;
	zval *retval;
	HashTable *function_table;

	zval **params[3];

	params[0] = &arg1;
	params[1] = &arg2;
	params[2] = &arg3;

	fci.size = sizeof(fci);
	/*fci.function_table = NULL; will be read form zend_class_entry of object if needed */
	fci.object_ptr = object_pp ? *object_pp : NULL;
	fci.function_name = &z_fname;
	fci.retval_ptr_ptr = retval_ptr_ptr ? retval_ptr_ptr : &retval;
	fci.param_count = param_count;
	fci.params = params;
	fci.no_separation = 1;
	fci.symbol_table = NULL;

	if (!fn_proxy && !obj_ce) {
		/* no interest in caching and no information already present that is
		 * needed later inside zend_call_function. */
		ZVAL_STRINGL(&z_fname, function_name, function_name_len, 0);
		fci.function_table = !object_pp ? EG(function_table) : NULL;
		result = zend_call_function(&fci, NULL TSRMLS_CC);
	} else {
		zend_fcall_info_cache fcic;

		fcic.initialized = 1;
		if (!obj_ce) {
			obj_ce = object_pp ? Z_OBJCE_PP(object_pp) : NULL;
		}
		if (obj_ce) {
			function_table = &obj_ce->function_table;
		} else {
			function_table = EG(function_table);
		}
		if (!fn_proxy || !*fn_proxy) {
			if (zend_hash_find(function_table, function_name, function_name_len+1, (void **) &fcic.function_handler) == FAILURE) {
				/* error at c-level */
				zend_error(E_CORE_ERROR, "Couldn't find implementation for method %s%s%s", obj_ce ? obj_ce->name : "", obj_ce ? "::" : "", function_name);
			}
			if (fn_proxy) {
				*fn_proxy = fcic.function_handler;
			}
		} else {
			fcic.function_handler = *fn_proxy;
		}
		fcic.calling_scope = obj_ce;
		if (object_pp) {
			fcic.called_scope = Z_OBJCE_PP(object_pp);
		} else if (obj_ce &&
		           !(EG(called_scope) &&
		             instanceof_function(EG(called_scope), obj_ce TSRMLS_CC))) {
			fcic.called_scope = obj_ce;
		} else {
			fcic.called_scope = EG(called_scope);
		}
		fcic.object_ptr = object_pp ? *object_pp : NULL;
		result = zend_call_function(&fci, &fcic TSRMLS_CC);
	}
	if (result == FAILURE) {
		/* error at c-level */
		if (!obj_ce) {
			obj_ce = object_pp ? Z_OBJCE_PP(object_pp) : NULL;
		}
		if (!EG(exception)) {
			zend_error(E_CORE_ERROR, "Couldn't execute method %s%s%s", obj_ce ? obj_ce->name : "", obj_ce ? "::" : "", function_name);
		}
	}
	if (!retval_ptr_ptr) {
		if (retval) {
			zval_ptr_dtor(&retval);
		}
		return NULL;
	}
	return *retval_ptr_ptr;
}
/* }}} */



void phux_init_mux(TSRMLS_D) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Phux\\MuxNew", mux_methods);
    phux_ce_mux = zend_register_internal_class(&ce TSRMLS_CC);
    zend_declare_property_null(phux_ce_mux, "id", strlen("id"), ZEND_ACC_PUBLIC TSRMLS_CC);
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

    zend_hash_find(z_array_hash, "id", sizeof("id"), (void**)&z_id);
    zend_hash_find(z_array_hash, "routes", sizeof("routes"), (void**)&z_routes);
    zend_hash_find(z_array_hash, "subMux", sizeof("subMux"), (void**)&z_submux);
    zend_hash_find(z_array_hash, "expandSubMux", sizeof("expandSubMux"), (void**)&z_expandSubMux);


    zval *new_object;
    ALLOC_INIT_ZVAL(new_object);
    object_init_ex(new_object, phux_ce_mux);
    CALL_METHOD(Mux, __construct, new_object, new_object);

    zend_update_property_long(phux_ce_mux, new_object, "id", sizeof("id")-1, Z_LVAL_PP(z_id) TSRMLS_CC);
    zend_update_property(phux_ce_mux, new_object, "routes", sizeof("routes")-1, *z_routes TSRMLS_CC);
    zend_update_property(phux_ce_mux, new_object, "subMux", sizeof("subMux")-1, *z_submux TSRMLS_CC);
    zend_update_property(phux_ce_mux, new_object, "expandSubMux", sizeof("expandSubMux")-1, *z_expandSubMux TSRMLS_CC);

    *return_value = *new_object;
    zval_copy_ctor(return_value);
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
        MAKE_STD_ZVAL(z_options);
        array_init(z_options);
    }
    if ( Z_TYPE_P(z_options) == IS_NULL ) {
        array_init(z_options);
    }


    zval *z_expandSubMux = zend_read_property(phux_ce_mux, getThis(), "expandSubMux", sizeof("expandSubMux")-1, 1 TSRMLS_CC);


    if ( Z_BVAL_P(z_expandSubMux) ) {
        // fetch routes from $mux
        //
        zval *z_mux_routes = zend_read_property(phux_ce_mux, z_mux, "routes", sizeof("routes")-1, 1 TSRMLS_CC);

        HashPosition route_pointer;
        HashTable    *routes_array;
        routes_array = Z_ARRVAL_P(z_mux_routes);
        zval **z_route;

        // iterate mux
        for(zend_hash_internal_pointer_reset_ex(routes_array, &route_pointer); 
                zend_hash_get_current_data_ex(routes_array, (void**) &z_route, &route_pointer) == SUCCESS; 
                zend_hash_move_forward_ex(routes_array, &route_pointer)) 
        {
            // read z_route
            HashTable *route_item_hash;
            route_item_hash = Z_ARRVAL_PP(z_route);


            zval **z_is_pcre; // route[0]
            zval **z_route_pattern;
            zval **z_route_callback;
            zval **z_route_options;
            zval **z_route_original_pattern; // for PCRE pattern

            if ( zend_hash_index_find( route_item_hash, 0, (void**) &z_is_pcre) == FAILURE ) {
                continue;
            }
            if ( zend_hash_index_find( route_item_hash, 1, (void**) &z_route_pattern) == FAILURE ) {
                continue;
            }
            if ( zend_hash_index_find( route_item_hash, 2, (void**) &z_route_callback) == FAILURE ) {
                continue;
            }
            if ( zend_hash_index_find( route_item_hash, 3, (void**) &z_route_options) == FAILURE ) {
                continue;
            }

            // Z_ADDREF_P(z_route_callback);
            // Z_ADDREF_P(z_route_options); // reference it so it will not be recycled.

            zval *z_new_routes;
            MAKE_STD_ZVAL(z_new_routes);
            array_init(z_new_routes);

            if ( Z_BVAL_PP(z_is_pcre) ) {
                // $newPattern = $pattern . $route[3]['pattern'];

                if ( zend_hash_find( Z_ARRVAL_PP(z_route_options), "pattern", sizeof("pattern"), (void**) &z_route_original_pattern) == FAILURE ) {
                    continue;
                }

                char new_pattern[120] = { 0 };
                int  new_pattern_len;
                strncat( new_pattern, pattern , pattern_len );
                strncat( new_pattern, Z_STRVAL_PP(z_route_original_pattern) , Z_STRLEN_PP(z_route_original_pattern) );

                new_pattern_len = pattern_len + Z_STRLEN_PP(z_route_original_pattern);

                zval *z_new_pattern = NULL;
                MAKE_STD_ZVAL(z_new_pattern);
                ZVAL_STRINGL(z_new_pattern, new_pattern, new_pattern_len, 1);


                // $routeArgs = RouteCompiler::compile($newPattern, 
                //     array_merge_recursive($route[3], $options) );

                zend_class_entry **ce_route_compiler = NULL;
                if ( zend_lookup_class( "Phux\\RouteCompiler", strlen("Phux\\RouteCompiler") , &ce_route_compiler TSRMLS_CC) == FAILURE ) {
                    zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Class Phux\\RouteCompiler does not exist.", 0 TSRMLS_CC);
                }

                // TODO: merge options
                zval *z_compiled_route = NULL;
                ALLOC_INIT_ZVAL(z_compiled_route);
                zend_call_method( NULL, *ce_route_compiler, NULL, "compile", strlen("compile"), &z_compiled_route, 2, z_new_pattern, *z_route_options TSRMLS_CC );


                zval **z_compiled_route_pattern;
                zend_hash_find( Z_ARRVAL_P(z_compiled_route) , "compiled", sizeof("compiled"), (void**)&z_compiled_route_pattern);

                // create new route and append to mux->routes
                add_index_bool(z_new_routes, 0 , 1); // pcre flag == false
                add_index_zval(z_new_routes, 1, *z_compiled_route_pattern);
                add_index_zval(z_new_routes, 2 , *z_route_callback);
                add_index_zval(z_new_routes, 3, *z_route_options);
                add_next_index_zval(z_mux_routes, z_new_routes);

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
                add_next_index_zval(z_mux_routes, z_new_routes);
            }
        }
    } else {
        zval *this_obj = getThis();

        zend_function *fe_getid = NULL; // method entry
        zend_function *fe_add   = NULL; // method entry

        if ( zend_hash_find( &Z_OBJCE_P(z_mux)->function_table, "getid", sizeof("getid"), (void **) &fe_getid) == FAILURE ) {
            zend_error(E_ERROR, "Cannot call method Mux::getid()");
            RETURN_FALSE;
        }
        if ( zend_hash_find( &Z_OBJCE_P(this_obj)->function_table, "add", sizeof("add"),    (void **) &fe_add) == FAILURE ) {
            zend_error(E_ERROR, "Cannot call method Mux::add()");
            RETURN_FALSE;
        }

        // $muxId = $mux->getId();
        // $this->add($pattern, $muxId, $options);
        // $this->subMux[ $muxId ] = $mux;
        zval *z_mux_id = NULL;
        ALLOC_INIT_ZVAL(z_mux_id);
        zend_call_method( &z_mux, Z_OBJCE_P(z_mux), &fe_getid, "getid", strlen("getid"), &z_mux_id, 0, NULL, NULL TSRMLS_CC );

        if ( Z_TYPE_P(z_mux_id) == IS_NULL ) {
            zend_error(E_ERROR, "Mux id is required. got NULL.");
        }

        // create pattern
        zval *z_pattern = NULL;
        MAKE_STD_ZVAL(z_pattern);
        ZVAL_STRINGL(z_pattern, pattern, pattern_len, 1); // duplicate

        zval *z_retval = NULL;
        ALLOC_INIT_ZVAL(z_retval);
        zend_call_method_with_3_params( &this_obj, Z_OBJCE_P(this_obj), &fe_add, "add",
                strlen("add"), &z_retval, 3, z_pattern, z_mux_id, z_options TSRMLS_CC);
        // zend_call_method( &this_obj, Z_OBJCE_P(this_obj), &fe_add, "add", strlen("add"), &z_retval, 2, z_pattern, z_mux_id TSRMLS_CC);
        zval *z_submux_array = zend_read_property( phux_ce_mux, getThis(), "subMux", sizeof("subMux") - 1, 1 TSRMLS_CC);
        add_index_zval( z_submux_array, Z_LVAL_P(z_mux_id) , z_mux);
    }
}

PHP_METHOD(Mux, getRoutes) {
    zval *z_routes;
    z_routes = zend_read_property(phux_ce_mux, getThis(), "routes", sizeof("routes")-1, 1 TSRMLS_CC);
    *return_value = *z_routes;
    zval_copy_ctor(return_value);
}

PHP_METHOD(Mux, export) {
    zval *this_object = getThis();

    zval *should_return;
    MAKE_STD_ZVAL(should_return);
    ZVAL_BOOL(should_return, 1);


    zval *retval_ptr = NULL;
    ALLOC_INIT_ZVAL(retval_ptr);

    // return var_export($this, true);
    zend_call_method( NULL, NULL, NULL, "var_export", strlen("var_export"), &retval_ptr, 2, this_object, should_return TSRMLS_CC );

    *return_value = *retval_ptr;
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

    // XXX: sort and write to file.
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


    zval **z_callback;
    zend_hash_index_find( Z_ARRVAL_P(z_return_route) , 2 , (void**) &z_callback );
    if ( Z_TYPE_PP(z_callback) == IS_LONG ) {
        zval **z_submux;
        zval *z_submux_array;
        z_submux_array = zend_read_property(phux_ce_mux, getThis(), "subMux", sizeof("subMux")-1, 1 TSRMLS_CC);
        zend_hash_index_find( Z_ARRVAL_P(z_submux_array),  Z_LVAL_PP(z_callback) , (void**) &z_submux);
        // TODO: dispatch to submux
    }

    *return_value = *z_return_route;
    zval_copy_ctor(return_value);
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

    if ( z_options == NULL ) {
        MAKE_STD_ZVAL(z_options);
        array_init(z_options);
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

    if ( z_options == NULL ) {
        MAKE_STD_ZVAL(z_options);
        array_init(z_options);
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
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|a", &pattern, &pattern_len, &z_callback, &z_options) == FAILURE) {
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
    }
    if ( Z_TYPE_P(z_options) == IS_NULL ) {
        // make it as an array
        array_init(z_options);
    }

    z_routes = zend_read_property(phux_ce_mux, getThis(), "routes", sizeof("routes")-1, 1 TSRMLS_CC);

    Z_ADDREF_P(z_callback);
    Z_ADDREF_P(z_options); // reference it so it will not be recycled.

    // PCRE pattern here
    if ( found ) {
        zval *z_route_compiler_class = NULL;
        MAKE_STD_ZVAL(z_route_compiler_class);
        ZVAL_STRING(z_route_compiler_class, "Phux\\RouteCompiler", 1);

        if ( zend_lookup_class( Z_STRVAL_P(z_route_compiler_class), Z_STRLEN_P(z_route_compiler_class) , &ce TSRMLS_CC) == FAILURE ) {
            zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Class Phux\\RouteCompiler does not exist.", 0 TSRMLS_CC);
        }

        zval *z_compiled_route = NULL;
        ALLOC_INIT_ZVAL(z_compiled_route);
        zend_call_method( NULL, *ce, NULL, "compile", strlen("compile"), &z_compiled_route, 1, z_pattern, NULL TSRMLS_CC );

        if ( z_compiled_route == NULL || Z_TYPE_P(z_compiled_route) == IS_NULL ) {
            zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Can not compile route pattern", 0 TSRMLS_CC);
        }

        zval **z_compiled_route_pattern;
        zend_hash_find( Z_ARRVAL_P(z_compiled_route) , "compiled", sizeof("compiled"), (void**)&z_compiled_route_pattern);

        zval *z_new_routes;
        MAKE_STD_ZVAL(z_new_routes);
        array_init(z_new_routes);

        add_index_bool(z_new_routes, 0 , 1); // pcre flag == false
        add_index_zval(z_new_routes, 1, *z_compiled_route_pattern);
        add_index_zval( z_new_routes, 2 , z_callback);
        add_index_zval(z_new_routes, 3, z_options);
        add_next_index_zval(z_routes, z_new_routes);

    } else {

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

