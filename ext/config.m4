
PHP_ARG_ENABLE(pux,
    [Whether to enable the "pux" extension],
    [  --enable-pux      Enable "pux" extension support])

AC_MSG_CHECKING(if PUX should be built in debug mode)
AC_ARG_ENABLE(pux-debug,
[  --enable-pux-debug     Enable PUX debugging],
[
  PHP_PUX_DEBUG=$enableval
], 
[
  PHP_PUX_DEBUG=no
])
AC_MSG_RESULT($PHP_PUX_DEBUG)


if test $PHP_PUX != "no"; then
    # PHP_REQUIRE_CXX()

    if test "$PHP_PUX_DEBUG" != "no"; then
        AC_DEFINE(PUX_DEBUG, 1, [ ])
        CFLAGS="$CFLAGS -O0 -g3 -ggdb -fprofile-arcs"
    fi

    PHP_SUBST(PUX_SHARED_LIBADD)
    PHP_ADD_LIBRARY(stdc++, 1, PUX_SHARED_LIBADD)
    PHP_ADD_INCLUDE(/opt/local/include)
    PHP_NEW_EXTENSION(pux, php_pux.c \
        pux_mux.c \
        php_expandable_mux.c \
        pux_controller.c \
        ct_helper.c \
        pux_functions.c \
        annotation/parser.c \
        annotation/scanner.c \
        hash.c \
        , $ext_shared)

fi
