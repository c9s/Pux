#ifndef PUX_PERSISTENT_H
#define PUX_PERSISTENT_H 1

extern inline int persistent_store(char *key, int key_len, int list_type, void * val TSRMLS_DC);
extern inline int pux_persistent_store(char *ns, char *key, int list_type, void * val TSRMLS_DC) ;

extern inline void * persistent_fetch(char *key, int key_len TSRMLS_DC);
extern inline void * pux_persistent_fetch(char *ns, char *key TSRMLS_DC);

#endif
