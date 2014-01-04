
PHP_ARG_ENABLE(phux,
    [Whether to enable the "phux" extension],
    [  --enable-phux      Enable "phux" extension support])

if test $PHP_ROLLER != "no"; then
    PHP_REQUIRE_CXX()
    PHP_SUBST(ROLLER_SHARED_LIBADD)
    PHP_ADD_LIBRARY(stdc++, 1, ROLLER_SHARED_LIBADD)
    PHP_NEW_EXTENSION(phux, php_phux.c, $ext_shared)
fi
