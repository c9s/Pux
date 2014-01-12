
PHP_ARG_ENABLE(pux,
    [Whether to enable the "pux" extension],
    [  --enable-pux      Enable "pux" extension support])

if test $PHP_PUX != "no"; then
    PHP_REQUIRE_CXX()
    PHP_SUBST(PUX_SHARED_LIBADD)
    PHP_ADD_LIBRARY(stdc++, 1, PUX_SHARED_LIBADD)
    PHP_NEW_EXTENSION(pux, php_pux.c php_mux.c php_expandable_mux.c php_controller.c ct_helper.c php_functions.c, $ext_shared)
fi
