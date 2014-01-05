
PHP_ARG_ENABLE(phux,
    [Whether to enable the "phux" extension],
    [  --enable-phux      Enable "phux" extension support])

if test $PHP_PHUX != "no"; then
    PHP_REQUIRE_CXX()
    PHP_SUBST(PHUX_SHARED_LIBADD)
    PHP_ADD_LIBRARY(stdc++, 1, PHUX_SHARED_LIBADD)
    PHP_NEW_EXTENSION(phux, php_phux.c ct_helper.c phux_functions.c, $ext_shared)
fi
