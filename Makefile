#ident %W% %E% %Q%
###########################################################################
SRCROOT=	.
DIRNAME=	SRCROOT
RULESDIR=	RULES
include		$(SRCROOT)/$(RULESDIR)/rules.top
###########################################################################

#include		Targetdirs.$(M_ARCH)

DIRS=	lib star man

###########################################################################
include		$(SRCROOT)/$(RULESDIR)/rules.dir
###########################################################################