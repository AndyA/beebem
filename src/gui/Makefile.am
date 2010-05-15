CXX = @CXX@ -Wall -DDATA_DIR=\"$(pkgdatadir)\"
CC = @CC@ -Wall -DDATA_DIR=\"$(pkgdatadir)\"

# INCLUDES = -I@top_srcdir@/src -I@top_srcdir@/src/gui
INCLUDES = -I@top_srcdir@/src

noinst_LIBRARIES = libeg.a
libeg_a_SOURCES = gui.h \
                sdl.c sdl.h \
                functions.c functions.h \
                \
		types.h log.h line.h \
		log.c \
		\
                messagebox.c messagebox.h messagebox_private.h \
                window.c window.h window_private.h \
                widget.c widget.h widget_private.h \
                widget_shared.c widget_shared.h \
                \
                label.c label.h label_private.h \
                button.c button.h button_private.h \
                box.c box.h box_private.h \
                tickbox.c tickbox.h tickbox_private.h \
                togglebutton.c togglebutton.h togglebutton_private.h \
                radiobutton.c radiobutton.h radiobutton_private.h \
                radiogroup.c radiogroup.h radiogroup_private.h \
                tabgroup.c tabgroup.h tabgroup_private.h \
                tabpage.c tabpage.h tabpage_private.h \
                slidebar.c slidebar.h slidebar_private.h \
                progressbar.c progressbar.h progressbar_private.h

