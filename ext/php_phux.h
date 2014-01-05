#ifndef PHP_PHUX_H
#define PHP_PHUX_H 1

#define PHP_PHUX_VERSION "1.0.0"
#define PHP_PHUX_EXTNAME "phux"

#define ZEND_HASH_FETCH(hash,key,ret) \
    zend_hash_find(hash, key, sizeof(key), (void**)&ret) == SUCCESS

#define PUSH_PARAM(arg) zend_vm_stack_push(arg TSRMLS_CC)
#define POP_PARAM() (void)zend_vm_stack_pop(TSRMLS_C)
#define PUSH_EO_PARAM()
#define POP_EO_PARAM()
 
#define CALL_METHOD_BASE(classname, name) zim_##classname##_##name
 
#define CALL_METHOD_HELPER(classname, name, retval, thisptr, num, param) \
  PUSH_PARAM(param); PUSH_PARAM((void*)num);                            \
  PUSH_EO_PARAM();                                                      \
  CALL_METHOD_BASE(classname, name)(num, retval, NULL, thisptr, 0 TSRMLS_CC); \
  POP_EO_PARAM();                       \
  POP_PARAM(); POP_PARAM();
 
#define CALL_METHOD(classname, name, retval, thisptr)                  \
  CALL_METHOD_BASE(classname, name)(0, retval, NULL, thisptr, 0 TSRMLS_CC);
 
#define CALL_METHOD1(classname, name, retval, thisptr, param1)         \
  CALL_METHOD_HELPER(classname, name, retval, thisptr, 1, param1);
 
#define CALL_METHOD2(classname, name, retval, thisptr, param1, param2) \
  PUSH_PARAM(param1);                                                   \
  CALL_METHOD_HELPER(classname, name, retval, thisptr, 2, param2);     \
  POP_PARAM();
 
#define CALL_METHOD3(classname, name, retval, thisptr, param1, param2, param3) \
  PUSH_PARAM(param1); PUSH_PARAM(param2);                               \
  CALL_METHOD_HELPER(classname, name, retval, thisptr, 3, param3);     \
  POP_PARAM(); POP_PARAM();


void phux_init_mux(TSRMLS_D);
 
PHP_METHOD(Mux, __construct);
PHP_METHOD(Mux, getId);
PHP_METHOD(Mux, add);
PHP_METHOD(Mux, length);
PHP_METHOD(Mux, compile);
PHP_METHOD(Mux, sort);
PHP_METHOD(Mux, appendRoute);
PHP_METHOD(Mux, appendPCRERoute);
PHP_METHOD(Mux, getRoutes);
PHP_METHOD(Mux, matchRoute);
PHP_METHOD(Mux, dispatch);
PHP_METHOD(Mux, getSubMux);
PHP_METHOD(Mux, export);
PHP_METHOD(Mux, mount);

PHP_METHOD(Mux, get);
PHP_METHOD(Mux, post);
PHP_METHOD(Mux, put);
PHP_METHOD(Mux, delete);

PHP_METHOD(Mux, __set_state);

// static method
PHP_METHOD(Mux, generate_id);

PHP_MINIT_FUNCTION(phux);

zval * php_phux_match(zval *z_routes, char *path, int path_len TSRMLS_DC);

extern zend_module_entry phux_module_entry;
#define phpext_phux_ptr &phux_module_entry

#endif
