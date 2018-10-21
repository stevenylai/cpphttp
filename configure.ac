AC_INIT([CppHTTP], [0.0.1])

AC_DEFUN([AC_DEFINE_SUBST], [
AC_DEFINE([$1], [$2], [$3])
AC_SUBST([$1], ['$2'])
])

AC_CHECK_PROGS([CMAKE], [cmake], [no])
if test CMAKE = no ; then
   AC_MSG_ERROR([Cannot find cmake. Please install it first.])
fi

AC_PROG_CC
AC_PROG_CXX
LT_INIT

AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_SILENT_RULES([yes])

AC_CONFIG_FILES(Makefile packages/Makefile)
#AC_CONFIG_FILES(m4_esyscmd(find src lib -name 'Makefile.am' | sed 's/Makefile\.am$/Makefile/g' | sed ':a;N;$!ba;s/\n/ /g'))
AC_OUTPUT
