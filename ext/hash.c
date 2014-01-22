
#include "hash.h"

HashTable * zend_hash_clone_persistent(HashTable* src TSRMLS_DC)
{
    zval **tmp;
    return my_copy_hashtable(NULL, src, (ht_copy_fun_t) my_copy_zval_ptr, (void*) &tmp, sizeof(zval *), 1 TSRMLS_CC);
}

HashTable * zend_hash_clone(HashTable* src TSRMLS_DC)
{
    zval **tmp;
    return my_copy_hashtable(NULL, src, (ht_copy_fun_t) my_copy_zval_ptr, (void*) &tmp, sizeof(zval *), 0 TSRMLS_CC);
}


/**
 * Recursively copy hash and all its value.
 *
 * This replaces zend_hash_copy
 */
HashTable * my_copy_hashtable(HashTable *target, HashTable *source, ht_copy_fun_t copy_fn, void *tmp, uint size, int persistent TSRMLS_DC)
{
    Bucket *curr = NULL, *prev = NULL , *newp = NULL;
    void *new_entry;
    int first = 1;

    assert(source != NULL);

    // allocate persistent memory for target and initialize it.
    if (!target) {
        target = pemalloc(sizeof(source[0]), persistent);
    }
    memcpy(target, source, sizeof(source[0]));
    target->arBuckets = pemalloc(target->nTableSize * sizeof(Bucket*), persistent);

    memset(target->arBuckets, 0, target->nTableSize * sizeof(Bucket*));
    target->pInternalPointer = NULL;
    target->pListHead = NULL;

    // since it's persistent, destructor should be NULL
    target->persistent = persistent;

    if (! target->persistent) {
        target->pDestructor = ZVAL_PTR_DTOR;
    }

    curr = source->pListHead;
    while (curr) {
        // hash index
        int n = curr->h % target->nTableSize;

        // allocate new bucket
// from apc
#ifdef ZEND_ENGINE_2_4
        if (!curr->nKeyLength) {
            newp = (Bucket*) pemalloc(sizeof(Bucket), persistent);
            memcpy(newp, curr, sizeof(Bucket));
        } else if (IS_INTERNED(curr->arKey)) {
            newp = (Bucket*) pemalloc(sizeof(Bucket), persistent);
            memcpy(newp, curr, sizeof(Bucket));
        } else {
            // ugly but we need to copy
            newp = (Bucket*) pemalloc(sizeof(Bucket) + curr->nKeyLength, persistent);
            memcpy(newp, curr, sizeof(Bucket) + curr->nKeyLength );
            newp->arKey = (const char*)(newp+1);
        }
#else
        newp = (Bucket*) pecalloc(1, (sizeof(Bucket) + curr->nKeyLength - 1), persistent);
        memcpy(newp, curr, sizeof(Bucket) + curr->nKeyLength - 1);
#endif


        /* insert 'newp' into the linked list at its hashed index */
        if (target->arBuckets[n]) {
            newp->pNext = target->arBuckets[n];
            newp->pLast = NULL;
            newp->pNext->pLast = newp;
        } else {
            newp->pNext = newp->pLast = NULL;
        }
        target->arBuckets[n] = newp;

        // now we copy the bucket data using our 'copy_fn'
        newp->pData = copy_fn(NULL, curr->pData, persistent TSRMLS_CC);
        memcpy(&newp->pDataPtr, newp->pData, sizeof(void*));

        /* insert 'newp' into the table-thread linked list */
        newp->pListLast = prev;
        newp->pListNext = NULL;

        if (prev) {
            prev->pListNext = newp;
        }
        if (first) {
            target->pListHead = newp;
            first = 0;
        }
        prev = newp;

        curr = curr->pListNext;
    }

    target->pListTail = newp;
    zend_hash_internal_pointer_reset(target);

    // return the newly allocated memory
    return target;
}

