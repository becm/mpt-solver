# Makefile: MPT solver modules and library
.PHONY : modules_% examples_% lib_% examples examples_all
all : lib_shared modules_shared
test : examples_test
install : modules_install
examples_test : lib_devel modules_shared
clear : lib_clear modules_clear examples_clear
clean : lib_clean modules_clean examples_clean
lib_% :; ${MAKE} -C mptsolver $(@:lib_%=%)
modules_% :; ${MAKE} -C modules $(@:modules_%=%)
examples_% :; ${MAKE} -C examples $(@:examples_%=%)
