SUBDIRS = src 
ACLOCAL_AMFLAGS = -I m4macros

dist_pkgdata_DATA = 3gauss.ncmap 3saw.ncmap bw.ncmap default.ncmap detail.ncmap hotres.ncmap

EXTRA_DIST =
EXTRA_DIST += RELEASE_NOTES regenerate
EXTRA_DIST += install-appdef Ncview-appdefaults
EXTRA_DIST += data/ncview.1.sed data/SciPlot.1
EXTRA_DIST += save/README.sgi save/README.solaris
EXTRA_DIST += save/README_LOCAL save/hotres.Rmap save/x_interface.c
EXTRA_DIST += README.ncview ChangeLog.ncview

CLEANFILES =
BUILT_SOURCES =

CLEANFILES += $(BUILT_SOURCES)

man1_MANS = $(PACKAGE).1
BUILT_SOURCES += $(man1_MANS)

if TRANSFORM_PACKAGE
APPDEFAULTS = $(CAPITAL_PACKAGE_NAME)-appdefaults
BUILT_SOURCES += $(APPDEFAULTS)

$(APPDEFAULTS): Ncview-appdefaults
	sed -e '/\(^Ncview[*]foreground:\).*$$/s//\1 $(COLOR_FOREGROUND)/' \
		-e '/\(^Ncview[*]background:\).*$$/s//\1 $(COLOR_BACKGROUND)/' \
		-e '/PlotXY_canvas[*]background: /s/[^ ]* *$$/$(COLOR_PLOT_BACKGROUND)/' \
		-e 's/^Ncview\([*]\)/$(CAPITAL_PACKAGE_NAME)\1/' \
		$< > $@
endif

$(PACKAGE).1: data/ncview.1.sed
	sed 's!NCVIEW_LIB_DIR!$(pkgdatadir)!g' $< > $@
