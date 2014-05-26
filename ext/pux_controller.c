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

static char * translate_method_name_to_path(const char *method_name, int * path_len);

static void zend_parse_action_annotations(zend_class_entry *ce, zval *retval, int parent TSRMLS_DC);

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


static char * translate_method_name_to_path(const char *method_name, int * path_len)
{
    char *p = strstr(method_name, "Action");
    if ( p == NULL ) {
        return NULL;
    }

    if ( strncmp(method_name, "indexAction", sizeof("indexAction")-1 ) == 0 ) {
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
            new_path_len++;
        }
        c++;
    }
    *path_len = new_path_len;
    return new_path;
}


static int zend_parse_method_annotations(zend_function *mptr, zval * z_indexed_annotations TSRMLS_DC)
{
    zval *z_comment;
    zval *z_file;
    zval *z_line_start;

    MAKE_STD_ZVAL(z_comment);
    ZVAL_STRING(z_comment, mptr->op_array.doc_comment, 1);

    MAKE_STD_ZVAL(z_file);
    ZVAL_STRING(z_file, mptr->op_array.filename, 1);

    MAKE_STD_ZVAL(z_line_start);
    ZVAL_LONG(z_line_start, mptr->op_array.line_start);

    // This isfor the end line number
    /*
    zval *z_line_end;
    MAKE_STD_ZVAL(z_line_end);
    ZVAL_LONG(z_line_start, mptr->op_array.line_end);
    */


    /**
     * This variable is used for phannot_parse_annotations function to return
     * annotation results.
     *
     * if there no annotation, we pass an empty array.
     */
    zval *z_method_annotations;
    MAKE_STD_ZVAL(z_method_annotations);
    array_init(z_method_annotations);


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
            * the actuall structure in PHP:
            *
            [
                {
                        "type"=> int(300)
                        "name"=> "Route"
                        "arguments"=> [
                        {
                            "expr"=> {
                                "type" => int(303)
                                "value" => string(7) "/delete"
                            }
                        }
                        ]
                        "file" => string(48) "...."
                        "line" => int(54)
                    },
                    .....
            ]
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
    // zval_ptr_dtor(&z_file);

    // return SUCCESS if there are annotations
    return zend_hash_num_elements( Z_ARRVAL_P(z_indexed_annotations) );
}







static void zend_parse_action_annotations(zend_class_entry *ce, zval *retval, int parent TSRMLS_DC) {
    HashTable *func_table;
    zend_function *mptr; // prepare zend_function mptr for iterating hash 
    HashPosition pos;

    zend_class_entry *parent_ce;

    const char *fn;
    size_t fn_len;
    int p;


    // we parse the annotations from top level parent class,
    // so in the child controller class, these child class methods can inherit
    // the annotations from parent.
    if (ce->parent) {
        parent_ce = ce->parent;
        zend_parse_action_annotations(parent_ce, retval, 1 TSRMLS_CC);
    }

    func_table = &ce->function_table;
    zend_hash_internal_pointer_reset_ex(func_table, &pos);

    while (zend_hash_get_current_data_ex(func_table, (void **) &mptr, &pos) == SUCCESS) {

        if ( mptr->type != ZEND_USER_FUNCTION ) {
            zend_hash_move_forward_ex(func_table, &pos);
            continue;
        }




        // the function name
        fn     = mptr->common.function_name;
        fn_len = strlen(mptr->common.function_name);
        p      = strpos(fn, "Action");

        if ( p == -1 || (size_t)p != (fn_len - (sizeof("Action")-1) )  ) {
            zend_hash_move_forward_ex(func_table, &pos);
            continue;
        }

        // append the structure [method name, annotations]
        zval *new_item;
        zval *z_indexed_annotations;
        zval *z_route_meta;

        MAKE_STD_ZVAL(new_item);
        array_init(new_item);

        // simplified annotation information is saved to this variable.
        MAKE_STD_ZVAL(z_indexed_annotations);
        array_init(z_indexed_annotations);

        MAKE_STD_ZVAL(z_route_meta);
        array_init(z_route_meta);

        // Create the route meta info
        // we put the attributes in lower-case letter key to avoid the name
        // collision of the annotation attribute names.
        //
        // perhaps we should put them in the third assoc array.
        add_assoc_stringl(z_route_meta, "class", ce->name, ce->name_length, 1);
        if ( parent ) {
            add_assoc_bool(z_route_meta, "is_parent", 1);
        }



        int found_annotations = 0;

        // if we found there is a doc comment block,
        // use annotation parser to parse the annotations
        //
        // method comment with "/** */" are doc_comment, others are not.
        if ( mptr->op_array.doc_comment ) {
            found_annotations = zend_parse_method_annotations(mptr, z_indexed_annotations TSRMLS_CC);
        }
        // php_printf("%s %d %s comment: %s\n", fn, found_annotations, " annotation", mptr->op_array.doc_comment);


        if ( found_annotations == 0 ) {
            // lookup parent parent annotations
            // php_printf("==> lookup parent parent annotations\n");
            zval **method_meta;
            zval **method_ann;
            // php_var_dump(&retval, 0 TSRMLS_CC);

            if (zend_hash_find(Z_ARRVAL_P(retval), fn, fn_len + 1, (void**)&method_meta) == SUCCESS) {
                // php_printf("==> found parent method declaration\n");
                // php_var_dump(method_meta, 0 TSRMLS_CC);


                if ( zend_hash_index_find( Z_ARRVAL_PP(method_meta), 0, (void**)&method_ann) == SUCCESS ) {
                    // php_printf("==> found parent method annotations\n");
                    // php_var_dump(method_ann, 0 TSRMLS_CC);
                    MAKE_COPY_ZVAL(method_ann, z_indexed_annotations);
                }
            }
        }

        Z_ADDREF_P(z_indexed_annotations);
        Z_ADDREF_P(z_route_meta);
        Z_ADDREF_P(new_item);
        add_next_index_zval(new_item, z_indexed_annotations);
        add_next_index_zval(new_item, z_route_meta);

        add_assoc_zval_ex(retval, fn, fn_len + 1, new_item); // use null-terminated string for the key

        // add_next_index_zval(retval, new_item);
        zend_hash_move_forward_ex(func_table, &pos);
    }

    // php_var_dump(&retval, 0 TSRMLS_CC);
}



PHP_METHOD(Controller, __construct) {
}



/**
 * Returns [ method, annotations ]
 *
 * This method returns a map of path => route info 
 *
 * [
 *      [
 *          'pageAction', 
 *          [
 *              'Route' => '/update',
 *              'Method' => 'GET',
 *          ],
 *          [ 'class' => ..., 'is_parent' => true ]
 *      ], ....
 * ]
 *
 *
 */
PHP_METHOD(Controller, getActionMethods)
{
    // Get function table hash from the current object.
    zend_class_entry *ce = Z_OBJCE_P(this_ptr);

    array_init(return_value);

    // looping in the parent class function table if we have one.
    // so we can override with our current class later.
    zend_parse_action_annotations(ce, return_value, 0 TSRMLS_CC);
}



// return path => path pairs
// structure: [[ path, method name ], [ ... ], [ ... ], ... ]
PHP_METHOD(Controller, getActionRoutes)
{
    zend_function *fe;
    if ( zend_hash_quick_find( &ce_pux_controller->function_table, "getactionmethods", sizeof("getactionmethods"), zend_inline_hash_func(ZEND_STRS("getactionmethods")), (void **) &fe) == FAILURE ) {
        php_error(E_ERROR, "getActionMethods method not found");
    }

    // call the getActionMethods to return the array of paths, 
    //    [ [path, [ annotations...] ], [ path2, [ annotations...] ],..... ]
    //
    zval *rv = NULL;
    zend_call_method_with_0_params( &this_ptr, ce_pux_controller, &fe, "getactionmethods", &rv );

    HashTable *func_list = Z_ARRVAL_P(rv);
    HashPosition pos;
    zval **z_method_name;
    zval **z_annotations;
    zval **z_doc_method;
    zval **z_doc_uri;
    zval **item;

    array_init(return_value);

    for(zend_hash_internal_pointer_reset_ex( Z_ARRVAL_P(rv) , &pos); 
            zend_hash_get_current_data_ex(Z_ARRVAL_P(rv), (void**) &item, &pos) == SUCCESS; 
            zend_hash_move_forward_ex(Z_ARRVAL_P(rv), &pos)) 
    {
        char * method_name = NULL;
        uint   method_name_len; // this method_name_len contains \0 charactar
        ulong  index;

        // use zend_hash_get_current_key to get method name from the return-value array, well it should be string key, not ulong key
        if (zend_hash_get_current_key_ex( Z_ARRVAL_P(rv), &method_name, &method_name_len, &index, 0, &pos) != HASH_KEY_IS_STRING) {
            zend_hash_move_forward_ex(Z_ARRVAL_P(rv), &pos);
            continue;
        }

        // zend_hash_index_find(Z_ARRVAL_PP(item), 0, (void**)&z_method_name);

        char *path              = NULL;
        int   path_len = 0;


        zval *z_route_options;
        MAKE_STD_ZVAL(z_route_options);
        array_init(z_route_options);

        if ( zend_hash_index_find(Z_ARRVAL_PP(item), 0, (void**)&z_annotations) == SUCCESS ) {
            if (zend_hash_find(Z_ARRVAL_PP(z_annotations), "Route", sizeof("Route"), (void**)&z_doc_uri) == SUCCESS) {
                path = estrndup(Z_STRVAL_PP(z_doc_uri), Z_STRLEN_PP(z_doc_uri));
                path_len = Z_STRLEN_PP(z_doc_uri);
            }
            if (zend_hash_find(Z_ARRVAL_PP(z_annotations), "Method", sizeof("Method"), (void**)&z_doc_method) == SUCCESS) {
                Z_ADDREF_PP(z_doc_method);

                // string_to_method_const
                add_assoc_long(z_route_options, "method", method_str_to_method_const(Z_STRVAL_PP(z_doc_method)) );
                // add_assoc_zval(z_route_options, "method", *z_doc_method);
            }
        }

        if (!path) {
            path = translate_method_name_to_path(method_name, &path_len);
        }

        // return structure [ path, method name, http method ]
        zval * new_item;
        MAKE_STD_ZVAL(new_item);
        array_init_size(new_item, 3);
        add_next_index_stringl(new_item, path, path_len, 1);
        add_next_index_stringl(new_item, method_name, method_name_len - 1, 1); // duplicate it

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
            zend_hash_move_forward_ex(Z_ARRVAL_P(path_array), &pointer);
            continue;
        }
        if ( zend_hash_index_find(Z_ARRVAL_PP(path_entry), 1, (void**) &z_method) == FAILURE ) {
            zend_hash_move_forward_ex(Z_ARRVAL_P(path_array), &pointer);
            continue;
        }

        zend_hash_index_find(Z_ARRVAL_PP(path_entry), 2, (void**) &z_options);

        zval *z_callback;
        MAKE_STD_ZVAL(z_callback);
        array_init_size(z_callback, 2);

        Z_ADDREF_P(z_callback);
        Z_ADDREF_PP(z_method);

        add_next_index_stringl(z_callback, class_name, class_name_len, 1);
        add_next_index_zval(z_callback, *z_method);

        zval *rv = NULL;
        zend_call_method_with_3_params(&new_mux, ce_pux_mux, NULL, "add", sizeof("add")-1, &rv, 3, *z_path, z_callback, *z_options TSRMLS_CC);
    }

    zval *rv = NULL;
    zend_call_method_with_0_params(&new_mux, ce_pux_mux, NULL, "sort", &rv);

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



