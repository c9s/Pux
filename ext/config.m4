
PHP_ARG_ENABLE(pux,
    [Whether to enable the "pux" extension],
    [  --enable-pux      Enable "pux" extension support])

if test $PHP_PHUX != "no"; then
    PHP_REQUIRE_CXX()
    PHP_SUBST(PHUX_SHARED_LIBADD)
    PHP_ADD_LIBRARY(stdc++, 1, PHUX_SHARED_LIBADD)
    PHP_NEW_EXTENSION(pux, php_pux.c ct_helper.c pux_functions.c, $ext_shared)
fi
