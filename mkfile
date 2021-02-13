<mkconfig
MKSHELL = rc
SRC = `{ ls *.c }
CSRC = common.c
OBJ = ${SRC:%.c=%.o}
COBJ = ${CSRC:%.c=%.o}
HDR = config.h
TGT = $NAME
WESRC = lib$NAME-webext.c
WEOBJ = ${WESRC:%.c=%.o}
WETGT = lib$NAME.so
all :VQ: $TGT
	echo -n
$TGT : $OBJ
	$LD -o $target  $LDFLAGS $prereq
%.o : %.c
	$CC  -c -o $target $CFLAGS $prereq
%.c :Q: $HDR
	touch $target
%.h :Q:
	touch $target
webext :VQ: $WETGT
	echo -n
$WETGT : $WEOBJ $COBJ
	$CC $SOCFLAGS $SONAME,$target  -o $target \
		$WEOBJ $COBJ \
		$WELDFLAGS
install :V: $TGT
	mkdir -p $EXEDIR $MANDIR/1
	cp -f $TGT $SCRIPT $EXEDIR/
	chmod 0755 $EXEDIR/^($TGT $SCRIPT)
	cp man $MANDIR/1/$NAME
install_webext :V: $WETGT
	mkdir -p $LIBDIR
	cp -f $WETGT $LIBDIR/
	chmod 644 $LIBDIR/$WETGT
uninstall :V: 
	rm -f $ $EXEDIR/$TGT $LIBDIR/$WETGT
clean :V:
	rm -rf $TGT $WETGT *.o 
