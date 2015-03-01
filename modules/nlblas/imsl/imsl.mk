#imsl.mk
# dunlsf/dbconf from IMSL FORTRAN library with dependencies
# and exclusion of BLAS code.

dunlsf_obj = \
	dunlsf.o du4lsf.o \
	du2lsf.o \
	du3lsf.o \
	du5lsf.o df2jac.o du6lsf.o du8lsf.o du10sf.o du7lsf.o du9lsf.o \
	iset.o dl2rrr.o \
	du11sf.o du12sf.o du13sf.o \
	idmax.o

dbconf_obj = \
	dbconf.o \
	db2onf.o \
	i1kqu.o db3onf.o \
	du4inf.o du5inf.o dfdgrd.o du6inf.o db6onf.o du9inf.o \
	dcrgrg.o db7onf.o db8onf.o du4iah.o du10nf.o db4onf.o \
	dcdgrd.o dtrnrr.o db5onf.o \
	du19nf.o du18nf.o dadd.o du6iah.o du13nf.o du14nf.o

imsl_shared = \
	dmach.o dcsfrg.o e1pop.o e1usr.o imach.o dset.o du11nf.o e1std.o \
	n1rcd.o e1psh.o dvcal.o e1sti.o iwkin.o n1rty.o i1kgt.o e1mes.o \
	\
	m1ve.o c1tic.o c1tci.o i1erif.o e1init.o i1krl.o i1kst.o e1ucs.o \
	m1vech.o i1dx.o e1pos.o n1rgb.o umach.o e1prt.o e1inpl.o e1stl.o \
	\
	icase.o i1cstr.o i1x.o s1anum.o \
	\
	iachar.o

imsl_objects = \
	$(dbconf_obj:%=imsl/%) \
	$(dunlsf_obj:%=imsl/%) \
	$(imsl_shared:%=imsl/%)

