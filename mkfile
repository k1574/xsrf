<mkconfig

VERSION = 2.0
INC = $X11INC $GTKINC
LIB = $X11LIB $GTKLIB $GTHREADLIB
CPPFLAGS = -DVERSION=\"$VERSION\" -DWEBEXTDIR=\"$LIBDIR\" \
           -D_DEFAULT_SOURCE -DGCR_API_SUBJECT_TO_CHANGE \
	$INC
CFLAGS = $ADDCFLAGS $INC $CPPFLAGS \
	-fPIC
LDFLAGS = $LIB

SRC = `{ ls *.c }
CSRC = common.c

OBJ = ${SRC:%.c=%.o}
COBJ = ${CSRC:%.c=%.o}

HDR = `{ ls *.h }
TGT = $NAME\

WESRC = lib$NAME-webext.c
WEOBJ = ${WESRC:%.c=%.o}
WECPPFLAGS = $WEBEXTINC
WECFLAGS = -fPIC $WEBEXTCFLAGS
WELDFLAGS = $WELIB
WETGT = lib$NAME.so

all :VQ: $TGT
	echo -n
$TGT : $OBJ
	$LD $LDFLAGS -o $target $prereq
%.o : %.c
	$CC $CFLAGS -c -o $target $prereq
%.c :Q: $HDR
	echo -n
%.h :Q:
	echo -n
webext :VQ: $WETGT
	echo -n
$WETGT : $WEOBJ $COBJ
	$CC -shared -Wl,-soname,$target $LDFLAGS -o $target \
		$WEOBJ $COBJ $WELDFLAGS
install : $TGT $WETGT
	mkdir -p $ROOT/bin $LIBDIR $ROOT/share/man1
	cp -f $TGT $ROOT/bin/
	chmod 0755 $ROOT/bin/$TGT 
	cp -f $WETGT $LIBDIR
	chmod 644 $LIBDIR/$WETGT
	sed s/VERSION/$VERSION/g < $NAME.1 > $ROOT/share/man1/$NAME.1
	chmod 644 $ROOT/share/man1/$NAME.1
uninstall: 
	rm -f $ROOT/share/man/man1/$TGT.1 $ROOT/bin/$TGT \
		$LIBDIR/$WETGT
clean :
	rm -rf $TGT $WETGT *.o 
