#include "string.h"
#include "ctype.h"
#include "php.h"
#include "main/php_main.h"
#include "Zend/zend_API.h"
#include "Zend/zend_variables.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "Zend/zend_interfaces.h"
#include "zend_object_handlers.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_var.h"

#include "ct_helper.h"
#include "php_pux.h"
#include "pux_mux.h"
#include "pux_functions.h"
#include "php_expandable_mux.h"
#include "pux_controller.h"

#include "annotation/scanner.h"
#include "annotation/annot.h"

zend_class_entry *ce_pux_controller;

const zend_function_entry controller_methods[] = {
  PHP_ME(Controller, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
  PHP_ME(Controller, expand, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Controller, getActionMethods, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Controller, getActionRoutes, NULL, ZEND_ACC_PUBLIC)

  PHP_ME(Controller, before, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Controller, after, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(Controller, toJson, NULL, ZEND_ACC_PUBLIC)

  // PHP_ME(Controller, __destruct,  NULL, ZEND_ACC_PUBLIC|ZEND_ACC_DTOR) 
  PHP_FE_END
};

zend_bool phannot_fetch_argument_value(zval **arg, zval** value TSRMLS_DC) {
    zval **expr;
    if (zend_hash_find(Z_ARRVAL_PP(arg), "expr", sizeof("expr"), (void**)&expr) == FAILURE ) {
        return FAILURE;
    }
    return zend_hash_find(Z_ARRVAL_PP(expr), "value", sizeof("value"), (void**) value);
}

zend_bool phannot_fetch_argument_type(zval **arg, zval **type TSRMLS_DC) {
    zval **expr;
    if (zend_hash_find(Z_ARRVAL_PP(arg), "expr", sizeof("expr"), (void**)&expr) == FAILURE ) {
        return FAILURE;
    }
    return zend_hash_find(Z_ARRVAL_PP(expr), "type", sizeof("type"), (void**)type);
}

int strpos(const char *haystack, char *needle)
{
   char *p = strstr(haystack, needle);
   if (p)
      return p - haystack;
   return -1;
}




void pux_init_controller(TSRMLS_D) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Pux\\Controller", controller_methods);
    ce_pux_controller = zend_register_internal_class(&ce TSRMLS_CC);
    zend_class_implements(ce_pux_controller TSRMLS_CC, 1, ce_pux_expandable_mux);
}

PHP_METHOD(Controller, __construct) {
}


/**
 * Returns [ method, annotations ]
 */
PHP_METHOD(Controller, getActionMethods)
{
    // get function table hash
    HashTable *function_table = &Z_OBJCE_P(this_ptr)->function_table;
    HashPosition pos;

    array_init(return_value);

    zend_function *mptr;
    zend_hash_internal_pointer_reset_ex(function_table, &pos);

    const char *fn;
    size_t fn_len;
    int p;

    while (zend_hash_get_current_data_ex(function_table, (void **) &mptr, &pos) == SUCCESS) {
        fn     = mptr->common.function_name;
        fn_len = strlen(mptr->common.function_name);
        p      = strpos(fn, "Action");

        if ( p == -1 || (size_t)p != (fn_len - strlen("Action"))  ) {
            zend_hash_move_forward_ex(function_table, &pos);
            continue;
        }

        // append the structure [method name, annotations]
        zval *new_item;
        zval *z_method_annotations;
        zval *z_indexed_annotations;

        zval *z_comment;
        zval *z_file;
        zval *z_line_start;

        MAKE_STD_ZVAL(new_item);
        array_init(new_item);
        add_next_index_stringl(new_item, fn, fn_len, 1);

        /* 
         * @var 
         *
         * if there no annotation, we pass an empty array.
         *
         * The method annotation structure
         *
         */
        MAKE_STD_ZVAL(z_method_annotations);
        array_init(z_method_annotations);

        // simplified annotation information is saved to this variable.
        MAKE_STD_ZVAL(z_indexed_annotations);
        array_init(z_indexed_annotations);

        if ( mptr->type == ZEND_USER_FUNCTION && mptr->op_array.doc_comment ) {
            ALLOC_ZVAL(z_comment);
            ZVAL_STRING(z_comment, mptr->op_array.doc_comment, 1);

            ALLOC_ZVAL(z_file);
            ZVAL_STRING(z_file, mptr->op_array.filename, 1);

            ALLOC_ZVAL(z_line_start);
            ZVAL_LONG(z_line_start, mptr->op_array.line_start);

            /*
            zval *z_line_end;
            ALLOC_ZVAL(z_line_end);
            ZVAL_LONG(z_line_start, mptr->op_array.line_end);
            */
            zval **z_ann = NULL, **z_ann_name = NULL, **z_ann_arguments = NULL, **z_ann_argument = NULL, **z_ann_argument_value = NULL;

            // TODO: make phannot_parse_annotations reads comment variable in char* type, so we don't need to create extra zval(s)
            if (phannot_parse_annotations(z_method_annotations, z_comment, z_file, z_line_start TSRMLS_CC) == SUCCESS) {

                /*
                 * z_method_annotations is an indexed array, which contains a structure like this:
                 *
                 *    [ 
                 *      { name => ... , type => ... , arguments => ... , file => , line =>  },
                 *      { name => ... , type => ... , arguments => ... , file => , line =>  },
                 *      { name => ... , type => ... , arguments => ... , file => , line =>  },
                 *    ]
                 *
                 * the actuall structure:
                 *
                 *    array(2) {
                 *        [0]=>
                 *        array(5) {
                 *            ["type"]=>
                 *            int(300)
                 *            ["name"]=>
                 *            string(5) "Route"
                 *            ["arguments"]=>
                 *            array(1) {
                 *            [0]=>
                 *            array(1) {
                 *                ["expr"]=>
                 *                array(2) {
                 *                ["type"]=>
                 *                int(303)
                 *                ["value"]=>
                 *                string(7) "/delete"
                 *                }
                 *            }
                 *            }
                 *            ["file"]=> string(48) "...."
                 *            ["line"]=> int(54)
                 *        },
                 *        .....
                 *    }
                 */
                HashPosition annp;
                for (
                    zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(z_method_annotations), &annp);
                    zend_hash_get_current_data_ex(Z_ARRVAL_P(z_method_annotations), (void**)&z_ann, &annp) == SUCCESS; 
                    zend_hash_move_forward_ex(Z_ARRVAL_P(z_method_annotations), &annp)
                ) {
                    if (zend_hash_find(Z_ARRVAL_P(*z_ann), "name", sizeof("name"), (void**)&z_ann_name) == FAILURE ) {
                        continue;
                    }
                    if (zend_hash_find(Z_ARRVAL_P(*z_ann), "arguments", sizeof("arguments"), (void**)&z_ann_arguments) == FAILURE ) {
                        continue;
                    }

                    // We currenly only support "@Method()" and "@Route"
                    if (    strncmp( Z_STRVAL_PP(z_ann_name), "Method",  sizeof("Method") - 1 ) != 0 
                         && strncmp( Z_STRVAL_PP(z_ann_name), "Route",   sizeof("Route") - 1 ) != 0 ) 
                    {
                        continue;
                    }

                    // read the first argument (we only support for one argument currently, and should support complex syntax later.)
                    if ( zend_hash_index_find(Z_ARRVAL_PP(z_ann_arguments), 0, (void**) &z_ann_argument) == SUCCESS ) {
                        if ( phannot_fetch_argument_value(z_ann_argument, (zval**) &z_ann_argument_value TSRMLS_CC) == SUCCESS ) {
                            Z_ADDREF_PP(z_ann_argument_value);
                            add_assoc_zval(z_indexed_annotations, Z_STRVAL_PP(z_ann_name), *z_ann_argument_value);
                        }
                    }
                }
            }
        }

        Z_ADDREF_P(z_indexed_annotations);
        add_next_index_zval(new_item, z_indexed_annotations);
        add_next_index_zval(return_value, new_item);

        zend_hash_move_forward_ex(function_table, &pos);
    }
    return;
}


char * translate_method_name_to_path(const char *method_name)
{
    char *p = strstr(method_name, "Action");
    if ( p == NULL ) {
        return NULL;
    }

    if ( strncmp(method_name, "indexAction", strlen("indexAction") ) == 0 ) {
        // returns empty string as its path
        return strndup("",sizeof("")-1);
    }
    char * new_path;

    // XXX: this might overflow..
    new_path = ecalloc( 128 , sizeof(char) );

    int    len = p - method_name;
    char * c = (char*) method_name;
    int    x = 0;
    new_path[x++] = '/';
    int    new_path_len = 1;
    while( len-- ) {
        if ( isupper(*c) ) {
            new_path[x++] = '/';
            new_path[x++] = tolower(*c);
            new_path_len += 2;
        } else {
            new_path[x++] = *c;
            new_path_len ++;
        }
        c++;
    }
    return new_path;
}

// return path => path pairs
// structure: [[ path, method name ], [ ... ], [ ... ], ... ]
PHP_METHOD(Controller, getActionRoutes)
{
    zend_function *fe;
    if ( zend_hash_quick_find( &ce_pux_controller->function_table, "getactionmethods", sizeof("getactionmethods"), zend_inline_hash_func(ZEND_STRS("getactionmethods")), (void **) &fe) == FAILURE ) {
        php_error(E_ERROR, "getActionMethods method not found");
    }
    // call export method
    zval *rv = NULL;
    zend_call_method_with_0_params( &this_ptr, ce_pux_controller, &fe, "getactionmethods", &rv );

    HashTable *func_list = Z_ARRVAL_P(rv);
    HashPosition pointer;
    zval **z_method_name;
    zval **z_annotations;
    zval **z_doc_method;
    zval **z_doc_uri;
    zval **item;

    array_init(return_value);

    for(zend_hash_internal_pointer_reset_ex(func_list, &pointer); 
            zend_hash_get_current_data_ex(func_list, (void**) &item, &pointer) == SUCCESS; 
            zend_hash_move_forward_ex(func_list, &pointer)) 
    {

        zend_hash_index_find(Z_ARRVAL_PP(item), 0, (void**)&z_method_name);

        const char *method_name = Z_STRVAL_PP(z_method_name);
        int method_name_len     = Z_STRLEN_PP(z_method_name);
        char *path              = NULL;

        zval *z_route_options;
        MAKE_STD_ZVAL(z_route_options);
        array_init(z_route_options);

        if ( zend_hash_index_find(Z_ARRVAL_PP(item), 1, (void**)&z_annotations) == SUCCESS ) {
            if (zend_hash_find(Z_ARRVAL_PP(z_annotations), "Route", sizeof("Route"), (void**)&z_doc_uri) == SUCCESS) {
                path = estrndup(Z_STRVAL_PP(z_doc_uri), Z_STRLEN_PP(z_doc_uri));
            }
            if (zend_hash_find(Z_ARRVAL_PP(z_annotations), "Method", sizeof("Method"), (void**)&z_doc_method) == SUCCESS) {
                Z_ADDREF_P(z_doc_method);
                add_assoc_zval(z_route_options, "method", *z_doc_method);
            }
        }

        if (!path) {
            path = translate_method_name_to_path(method_name);
        }

        // return structure [ path, method name, http method ]
        zval * new_item;
        MAKE_STD_ZVAL(new_item);
        array_init_size(new_item, 3);
        add_next_index_string(new_item, path, 1);
        add_next_index_stringl(new_item, method_name, method_name_len, 0);

        Z_ADDREF_P(z_route_options);
        add_next_index_zval(new_item, z_route_options);

        // append to the result array
        add_next_index_zval(return_value, new_item);
    }

    return;
}

PHP_METHOD(Controller, expand)
{
    zval *new_mux;
    MAKE_STD_ZVAL(new_mux);
    object_init_ex(new_mux, ce_pux_mux);
    CALL_METHOD(Mux, __construct, new_mux, new_mux);

    Z_ADDREF_P(new_mux); // add reference


    zval *path_array = NULL;
    zend_call_method_with_0_params( &this_ptr, ce_pux_controller, NULL, "getactionroutes", &path_array );

    if ( Z_TYPE_P(path_array) != IS_ARRAY ) {
        php_error(E_ERROR, "getActionRoutes does not return an array.");
        RETURN_FALSE;
    }

    const char *class_name = NULL;
    zend_uint   class_name_len;
    int   dup = zend_get_object_classname(this_ptr, &class_name, &class_name_len TSRMLS_CC);

    HashPosition pointer;
    zval **path_entry = NULL;
    for (
        zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(path_array), &pointer); 
        zend_hash_get_current_data_ex(Z_ARRVAL_P(path_array), (void**) &path_entry, &pointer) == SUCCESS; 
        zend_hash_move_forward_ex(Z_ARRVAL_P(path_array), &pointer)
    ) {
        zval **z_path;
        zval **z_method;
        zval **z_options = NULL;


        if ( zend_hash_index_find(Z_ARRVAL_PP(path_entry), 0, (void**) &z_path) == FAILURE ) {
            continue;
        }
        if ( zend_hash_index_find(Z_ARRVAL_PP(path_entry), 1, (void**) &z_method) == FAILURE ) {
            continue;
        }

        zend_hash_index_find(Z_ARRVAL_PP(path_entry), 2, (void**) &z_options);

        zval *z_callback;
        MAKE_STD_ZVAL(z_callback);
        array_init_size(z_callback, 2);

        Z_ADDREF_P(z_callback);
        Z_ADDREF_P(z_method);

        add_next_index_stringl(z_callback, class_name, class_name_len, 1);
        add_next_index_zval(z_callback, *z_method);

        zval *rv = NULL;
        zend_call_method_with_3_params(&new_mux, ce_pux_mux, NULL, "add", strlen("add"), &rv, 3, *z_path, z_callback, *z_options TSRMLS_CC);
    }

    zval *rv = NULL;
    zend_call_method_with_0_params(&new_mux, ce_pux_mux, NULL, "sort", &rv );

    *return_value = *new_mux;
    zval_copy_ctor(return_value);
}

PHP_METHOD(Controller, before) {

}

PHP_METHOD(Controller, after) {

}

PHP_METHOD(Controller, toJson)
{
    zval *z_data;
    long options = 0;
    long depth = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|ll", &z_data, &options, &depth) == FAILURE) {
        RETURN_FALSE;
    }

    zval *z_options;
    zval *z_depth;

    MAKE_STD_ZVAL(z_options);
    MAKE_STD_ZVAL(z_depth);

    ZVAL_LONG(z_depth, 0);
    ZVAL_LONG(z_options, 0);

    

    zval *rv = NULL;
    // zend_call_method_with_3_params(NULL, NULL, NULL, "json_encode", sizeof("json_encode"), &rv, 3, z_data, z_options, z_depth TSRMLS_CC );
    zend_call_method_with_2_params(NULL, NULL, NULL, "json_encode", &rv, z_data, z_options);

    *return_value = *rv;
    zval_copy_ctor(return_value);
}



