# $Id: Makefile 769 2007-01-22 09:09:43Z bastian $
#
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=perlvdb.so
LIBS:=$(shell perl -MExtUtils::Embed -e ldopts)
PERL_DEFS:=$(shell perl -MExtUtils::Embed -e ccopts)
DEFS+=$(PERL_DEFS)



DEFS+=-DOPENSER_MOD_INTERFACE

SERLIBPATH=../../lib
SER_LIBS+=$(SERLIBPATH)/srdb1/srdb1
include ../../Makefile.modules
