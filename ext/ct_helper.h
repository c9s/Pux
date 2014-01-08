#ifndef PHP_CT_HELPER_H
#define PHP_CT_HELPER_H 1

ZEND_API zval* zend_call_method_with_3_params(zval **object_pp, zend_class_entry *obj_ce, zend_function **fn_proxy, const char *function_name, int function_name_len, zval **retval_ptr_ptr, int param_count, zval* arg1, zval* arg2, zval* arg3 TSRMLS_DC);

char * find_place_holder(char *pattern, int pattern_len);

#endif
