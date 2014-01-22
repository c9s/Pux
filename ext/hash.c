
#include "hash.h"

HashTable * zend_hash_clone_persistent(HashTable* src TSRMLS_DC)
{
    zval **tmp;
    return persistent_copy_hashtable(NULL, src, (ht_copy_fun_t) my_copy_zval_ptr, (void*) &tmp, sizeof(zval *) TSRMLS_CC);
}


/**
 * Recursively copy hash and all its value.
 *
 * This replaces zend_hash_copy
 */
HashTable * persistent_copy_hashtable(HashTable *target, HashTable *source, ht_copy_fun_t copy_fn, void *tmp, uint size TSRMLS_DC)
{
    Bucket *curr = NULL, *prev = NULL , *newp = NULL;
    void *new_entry;
    int first = 1;

    assert(source != NULL);

    // allocate persistent memory for target and initialize it.
    if (!target) {
        CHECK(target = pecalloc(1, sizeof(source[0]), 1));
    }
    memcpy(target, source, sizeof(source[0]));
    memset(target->arBuckets, 0, target->nTableSize * sizeof(Bucket*));
    target->pInternalPointer = NULL;
    target->pListHead = NULL;

    // since it's persistent, destructor should be NULL
    target->persistent = 1;
    target->pDestructor = NULL;

    curr = source->pListHead;
    while (curr) {
        // hash index
        int n = curr->h % target->nTableSize;

        // allocate new bucket

// from apc
#ifdef ZEND_ENGINE_2_4
        if (!curr->nKeyLength) {
            CHECK(newp = (Bucket*) pecalloc(1, sizeof(Bucket), 1));
            memcpy(newp, curr, sizeof(Bucket));
        } else if (IS_INTERNED(curr->arKey)) {
            CHECK(newp = (Bucket*) pecalloc(1, sizeof(Bucket), 1));
            memcpy(newp, curr, sizeof(Bucket));
        } else {
            // CHECK((newp = (Bucket*) apc_pmemcpy(curr, sizeof(Bucket) + curr->nKeyLength, pool TSRMLS_CC)));
            // ugly but we need to copy
            CHECK(newp = (Bucket*) pecalloc(1, sizeof(Bucket) + curr->nKeyLength, 1));
            memcpy(newp, curr, sizeof(Bucket) + curr->nKeyLength );
            newp->arKey = (const char*)(newp+1);
        }
#else
        CHECK(newp = (Bucket*) pecalloc(1, (sizeof(Bucket) + curr->nKeyLength - 1), 1));
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
        newp->pData = copy_fn(NULL, curr->pData TSRMLS_CC);
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

