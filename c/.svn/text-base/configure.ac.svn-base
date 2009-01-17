AC_PREREQ(2.59)

AC_INIT(wp, 0.1)

AM_INIT_AUTOMAKE

AC_PROG_CC

AC_CHECK_LIB(bz2, BZ2_bzBuffToBuffDecompress, [], \
             [AC_MSG_FAILURE(I require libbz2)])
AC_CHECK_LIB(ncurses, initscr, [], \
             [AC_MSG_FAILURE(I require ncurses)])

AM_PROG_CC_C_O

AC_OUTPUT(Makefile)
