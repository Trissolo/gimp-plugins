#!/usr/bin/perl -w

use lib '../../tools/pdbgen';

require 'util.pl';

*write_file = \&Gimp::CodeGen::util::write_file;
*FILE_EXT   = \$Gimp::CodeGen::util::FILE_EXT;

$outmk = "Makefile.am$FILE_EXT";
$outignore = ".cvsignore$FILE_EXT";
open MK, "> $outmk";
open IGNORE, "> $outignore";

require 'plugin-defs.pl';

$bins = ""; $opts = "";

foreach (sort keys %plugins) {
    $bins .= "\t";
    if (exists $plugins{$_}->{optional}) {
	$bins .= "\$(\U$_\E)";
	$opts .= "\t$_ \\\n";
    }
    else {
	$bins .= $_;
    }
    $bins .= " \\\n";
}

$extra = "";
foreach (@extra) { $extra .= "\t$_\t\\\n" }
if ($extra) {
    $extra =~ s/\t\\\n$//s;
    $extra = "\t\\\n$extra";
}

foreach ($bins, $opts) { s/ \\\n$//s }

print MK <<EOT;
## This file is autogenerated by mkgen.pl and plugin-defs.pl

if OS_WIN32
mwindows = -mwindows
endif

AM_LDFLAGS = \$(mwindows)

libexecdir = \$(gimpplugindir)/plug-ins

EXTRA_DIST = \\
	mkgen.pl	\\
	plugin-defs.pl$extra

INCLUDES = \\
	-I\$(top_srcdir)	\\
	\@GTK_CFLAGS\@	\\
	\@X_CFLAGS\@	\\
	\@EXIF_CFLAGS\@	\\
	\@SVG_CFLAGS\@	\\
	-I\$(includedir)

libexec_PROGRAMS = \\
$bins

EXTRA_PROGRAMS = \\
$opts

install-\%: \%
	\@\$(NORMAL_INSTALL)
	\$(mkinstalldirs) \$(DESTDIRS)\$(libexecdir)
	\@if test -f \$<; then \\
	  echo " \$(LIBTOOL)  --mode=install \$(INSTALL_PROGRAM) \$< \$(DESTDIR)\$(libexecdir)/`echo \$<|sed 's/\$(EXEEXT)\$\$//'|sed '\$(transform)'|sed 's/\$\$/\$(EXEEXT)/'`"; \\
	  \$(LIBTOOL)  --mode=install \$(INSTALL_PROGRAM) \$< \$(DESTDIR)\$(libexecdir)/`echo \$<|sed 's/\$(EXEEXT)\$\$//'|sed '\$(transform)'|sed 's/\$\$/\$(EXEEXT)/'`; \\
	else :; fi
EOT

print IGNORE <<EOT;
Makefile
Makefile.in
.deps
.libs
EOT

foreach (sort keys %plugins) {
    my $libgimp = "";

    $libgimp .= "\$(top_builddir)/libgimp/libgimp-\$(LT_RELEASE).la";
    if (exists $plugins{$_}->{ui}) {
        $libgimp =~ s/gimp-/gimpui-/;
        $libgimp .= "\t\t\\\n\t\$(top_builddir)/libgimpwidgets/libgimpwidgets-\$(LT_RELEASE).la";
	$libgimp .= "\t\\\n\t\$(top_builddir)/libgimp/libgimp-\$(LT_RELEASE).la";
        $libgimp .= "\t\t\\\n\t\$(top_builddir)/libgimpcolor/libgimpcolor-\$(LT_RELEASE).la";
        $libgimp .= "\t\\\n\t\$(top_builddir)/libgimpbase/libgimpbase-\$(LT_RELEASE).la";
    } else {
        $libgimp .= "\t\t\\\n\t\$(top_builddir)/libgimpcolor/libgimpcolor-\$(LT_RELEASE).la";
        $libgimp .= "\t\\\n\t\$(top_builddir)/libgimpbase/libgimpbase-\$(LT_RELEASE).la";
    }

    my $optlib = "";
    if (exists $plugins{$_}->{optional}) {
	my $name = exists $plugins{$_}->{libopt} ? $plugins{$_}->{libopt} : $_;
	$optlib = "\n\t\$(LIB\U$name\E)\t\t\t\t\t\t\t\\";
    }

    my $deplib = "\@INTLLIBS\@";
    if (exists $plugins{$_}->{libdep}) {
	my @lib = split(/:/, $plugins{$_}->{libdep});
	foreach $lib (@lib) {
	    $deplib = "\@\U$lib\E_LIBS\@\t\t\t\t\t\t\t\\\n\t$deplib";
	}
    }

    if (exists $plugins{$_}->{libsupp}) {
	my @lib = split(/:/, $plugins{$_}->{libsupp});
	foreach $lib (@lib) {
	    $libgimp = "\$(top_builddir)/plug-ins/$lib/lib$lib.a\t\\\n\t$libgimp";
	}
    }

    print MK <<EOT;

${_}_SOURCES = \\
	$_.c

${_}_LDADD = \\
	$libgimp	\\$optlib
	$deplib
EOT

    print IGNORE "$_\n";
}

close MK;
close IGNORE;

&write_file($outmk);
&write_file($outignore);
