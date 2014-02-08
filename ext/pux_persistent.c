#include "php.h"
#include "php_pux.h"
#include "pux_persistent.h"
#include "php_expandable_mux.h"

inline int persistent_store(char *key, int key_len, int list_type, void * val TSRMLS_DC)
{
    zend_rsrc_list_entry new_le;
    zend_rsrc_list_entry *le;

    // store it if it's not in persistent_list
    if ( zend_hash_find(&EG(persistent_list), key, key_len + 1, (void**) &le) == SUCCESS ) {
        zend_hash_del(&EG(persistent_list), key, key_len + 1);
    }
    new_le.type = list_type;
    new_le.ptr = val;
    return zend_hash_update(&EG(persistent_list), key, key_len + 1, (void *) &new_le, sizeof(zend_rsrc_list_entry), NULL);
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
inline int pux_persistent_store(char *ns, char *key, int list_type, void * val TSRMLS_DC) 
{
    char *newkey;
    int   newkey_len;
    int   status;
    newkey_len = spprintf(&newkey, 0, "pux_%s_%s", ns, key);
    status = persistent_store(newkey, newkey_len, list_type, val TSRMLS_CC);
    efree(newkey);
    return status;
}

