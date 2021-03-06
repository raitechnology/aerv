# defines a directory for build, for example, RH6_x86_64
lsb_dist     := $(shell if [ -x /usr/bin/lsb_release ] ; then lsb_release -is ; else echo Linux ; fi)
lsb_dist_ver := $(shell if [ -x /usr/bin/lsb_release ] ; then lsb_release -rs | sed 's/[.].*//' ; else uname -r | sed 's/[-].*//' ; fi)
uname_m      := $(shell uname -m)

short_dist_lc := $(patsubst CentOS,rh,$(patsubst RedHatEnterprise,rh,\
                   $(patsubst RedHat,rh,\
                     $(patsubst Fedora,fc,$(patsubst Ubuntu,ub,\
                       $(patsubst Debian,deb,$(patsubst SUSE,ss,$(lsb_dist))))))))
short_dist    := $(shell echo $(short_dist_lc) | tr a-z A-Z)
pwd           := $(shell pwd)
rpm_os        := $(short_dist_lc)$(lsb_dist_ver).$(uname_m)

# this is where the targets are compiled
build_dir ?= $(short_dist)$(lsb_dist_ver)_$(uname_m)$(port_extra)
bind      := $(build_dir)/bin
libd      := $(build_dir)/lib64
objd      := $(build_dir)/obj
dependd   := $(build_dir)/dep

# use 'make port_extra=-g' for debug build
ifeq (-g,$(findstring -g,$(port_extra)))
  DEBUG = true
endif

CC          ?= gcc
CXX         ?= g++
cc          := $(CC)
cpp         := $(CXX)
# if not linking libstdc++
ifdef NO_STL
cppflags    := -std=c++11 -fno-rtti -fno-exceptions
cpplink     := $(CC)
else
cppflags    := -std=c++11
cpplink     := $(CXX)
endif
arch_cflags := -mavx -maes -fno-omit-frame-pointer
#gcc_wflags  := -Wall -Wextra -Werror
gcc_wflags  := -Wall -Wextra
fpicflags   := -fPIC
soflag      := -shared

ifdef DEBUG
default_cflags := -ggdb
else
default_cflags := -ggdb -O3 -Ofast
endif
# rpmbuild uses RPM_OPT_FLAGS
CFLAGS := $(default_cflags)
#RPM_OPT_FLAGS ?= $(default_cflags)
#CFLAGS ?= $(RPM_OPT_FLAGS)
cflags := $(gcc_wflags) $(CFLAGS) $(arch_cflags)

# where to find the raids/xyz.h files
INCLUDES    ?= -Iinclude
includes    := $(INCLUDES)
DEFINES     ?=
defines     := $(DEFINES)
cpp_lnk     :=
sock_lib    :=
math_lib    := -lm
thread_lib  := -pthread -lrt

# test submodules exist (they don't exist for dist_rpm, dist_dpkg targets)
have_aekv_submodule   := $(shell if [ -f ./aekv/GNUmakefile ]; then echo yes; else echo no; fi )
have_sassrv_submodule := $(shell if [ -f ./sassrv/GNUmakefile ]; then echo yes; else echo no; fi )

lnk_lib     :=
dlnk_lib    :=
lnk_dep     :=
dlnk_dep    :=

ifeq (yes,$(have_sassrv_submodule))
sassrv_lib     := sassrv/$(libd)/libsassrv.a sassrv/raikv/$(libd)/libraikv.a sassrv/raimd/$(libd)/libraimd.a sassrv/raimd/libdecnumber/$(libd)/libdecnumber.a
sassrv_dll     := sassrv/$(libd)/libsassrv.so
sassrv_include := -Isassrv/include -Isassrv/raikv/include -Isassrv/raimd/include
lnk_lib        += $(sassrv_lib)
lnk_dep        += $(sassrv_lib)
dlnk_lib       += -Lsassrv/$(libd) -lsassrv -Lsassrv/raikv/$(libd) -lraikv -Lsassrv/raimd/$(libd) -lraimd -Lsassrv/raimd/libdecnumber/$(libd) -ldecnumber
dlnk_dep       += $(sassrv_dll)
rpath1         = ,-rpath,$(pwd)/sassrv/$(libd)
else
lnk_lib        += -lsassrv -lraikv -lraimd -ldecnumber
dlnk_lib       += -lsassrv -lraikv -lraimd -ldecnumber
endif

# if building submodules, reference them rather than the libs installed
ifeq (yes,$(have_aekv_submodule))
aekv_lib      := aekv/$(libd)/libaekv.a aekv/aeron/$(libd)/libaeron_static.a
aekv_dll      := aekv/$(libd)/libaekv.so aekv/aeron/$(libd)/libaeron.so
aekv_include  := -Iaekv/include
lnk_lib       += aekv/$(libd)/libaekv.a -rdynamic aekv/aeron/$(libd)/libaeron_static.a
lnk_dep       += $(aekv_lib)
dlnk_lib      += -Laekv/aeron/$(libd) -laeron
dlnk_dep      += $(aekv_dll)
rpath5         = ,-rpath,$(pwd)/aekv/$(libd),-rpath,$(pwd)/aekv/aeron/$(libd)
aeron_client_lib := aekv/aeron/$(libd)/libaeron_client.a
aeron_client_dll := aekv/aeron/$(libd)/libaeron_client_shared.so
cpp_lnk_lib      += -rdynamic $(aeron_client_lib)
cpp_lnk_dep      += $(aeron_client_lib)
cpp_dlnk_lib     += -Laekv/aeron/$(libd) -laeron_client_shared
cpp_dlnk_dep     += $(aeron_client_dll)
else
lnk_lib      += -laekv -rdynamic -laeron_static -laeron_client
dlnk_lib     += -laekv -laeron -laeron_client_shared
endif

rpath       := -Wl,-rpath,$(pwd)/$(libd)$(rpath1)$(rpath2)$(rpath3)$(rpath4)$(rpath5)$(rpath6)$(rpath7)
dlnk_lib    += -lpcre2-8 -lcrypto -ldl
malloc_lib  :=
lnk_lib     += -lpcre2-8 -ldl

includes += $(sassrv_include) $(aekv_include)

.PHONY: everything
everything: $(aekv_lib) $(sassrv_lib) all

clean_subs :=
dlnk_dll_depend :=
dlnk_lib_depend :=

# build submodules if have them
ifeq (yes,$(have_aekv_submodule))
$(aekv_lib) $(aekv_dll):
	$(MAKE) -C aekv
.PHONY: clean_aekv
clean_aekv:
	$(MAKE) -C aekv clean
clean_subs += clean_aekv
endif
ifeq (yes,$(have_sassrv_submodule))
$(sassrv_lib) $(sassrv_dll):
	$(MAKE) -C sassrv
.PHONY: clean_sassrv
clean_sassrv:
	$(MAKE) -C sassrv clean
clean_subs += clean_sassrv
endif

# copr/fedora build (with version env vars)
# copr uses this to generate a source rpm with the srpm target
-include .copr/Makefile

# debian build (debuild)
# target for building installable deb: dist_dpkg
-include deb/Makefile

# targets filled in below
all_exes    :=
all_libs    :=
all_dlls    :=
all_depends :=
gen_files   :=

server_defines    := -DAERV_VER=$(ver_build)
aerv_server_files := server
aerv_server_objs  := $(addprefix $(objd)/, $(addsuffix .o, $(aerv_server_files)))
aerv_server_deps  := $(addprefix $(dependd)/, $(addsuffix .d, $(aerv_server_files)))
aerv_server_libs  := $(aekv_lib)
aerv_server_lnk   := $(lnk_lib)

$(bind)/aerv_server: $(aerv_server_objs) $(aerv_server_libs) $(lnk_dep)

all_exes    += $(bind)/aerv_server
all_depends += $(aerv_server_deps)

all_dirs := $(bind) $(libd) $(objd) $(dependd)

# the default targets
.PHONY: all
all: $(all_libs) $(all_dlls) $(all_exes)

.PHONY: dnf_depend
dnf_depend:
	sudo dnf -y install make gcc-c++ git redhat-lsb openssl-devel pcre2-devel chrpath liblzf-devel zlib-devel libbsd-devel

.PHONY: yum_depend
yum_depend:
	sudo yum -y install make gcc-c++ git redhat-lsb openssl-devel pcre2-devel chrpath liblzf-devel zlib-devel libbsd-devel

.PHONY: deb_depend
deb_depend:
	sudo apt-get install -y install make g++ gcc devscripts libpcre2-dev chrpath git lsb-release libssl-dev

# create directories
$(dependd):
	@mkdir -p $(all_dirs)

# remove target bins, objs, depends
.PHONY: clean
clean: $(clean_subs)
	rm -r -f $(bind) $(libd) $(objd) $(dependd)
	if [ "$(build_dir)" != "." ] ; then rmdir $(build_dir) ; fi

.PHONY: clean_dist
clean_dist:
	rm -rf dpkgbuild rpmbuild

.PHONY: clean_all
clean_all: clean clean_dist

# force a remake of depend using 'make -B depend'
.PHONY: depend
depend: $(dependd)/depend.make

$(dependd)/depend.make: $(dependd) $(all_depends)
	@echo "# depend file" > $(dependd)/depend.make
	@cat $(all_depends) >> $(dependd)/depend.make

.PHONY: dist_bins
dist_bins: $(all_libs) $(all_dlls) $(bind)/aerv_server
	chrpath -d $(bind)/aerv_server

.PHONY: dist_rpm
dist_rpm: srpm
	( cd rpmbuild && rpmbuild --define "-topdir `pwd`" -ba SPECS/aeronmd.spec )

# dependencies made by 'make depend'
-include $(dependd)/depend.make

ifeq ($(DESTDIR),)
# 'sudo make install' puts things in /usr/local/lib, /usr/local/include
install_prefix = /usr/local
else
# debuild uses DESTDIR to put things into debian/aeronmd/usr
install_prefix = $(DESTDIR)/usr
endif

install: dist_bins
	install -d $(install_prefix)/lib $(install_prefix)/bin

$(objd)/%.o: src/%.cpp
	$(cpp) $(cflags) $(cppflags) $(includes) $(defines) $($(notdir $*)_includes) $($(notdir $*)_defines) -c $< -o $@

$(objd)/%.o: src/%.c
	$(cc) $(cflags) $(includes) $(defines) $($(notdir $*)_includes) $($(notdir $*)_defines) -c $< -o $@

$(objd)/%.fpic.o: src/%.cpp
	$(cpp) $(cflags) $(fpicflags) $(cppflags) $(includes) $(defines) $($(notdir $*)_includes) $($(notdir $*)_defines) -c $< -o $@

$(objd)/%.fpic.o: src/%.c
	$(cc) $(cflags) $(fpicflags) $(includes) $(defines) $($(notdir $*)_includes) $($(notdir $*)_defines) -c $< -o $@

$(objd)/%.o: test/%.cpp
	$(cpp) $(cflags) $(cppflags) $(includes) $(defines) $($(notdir $*)_includes) $($(notdir $*)_defines) -c $< -o $@

$(objd)/%.o: test/%.c
	$(cc) $(cflags) $(includes) $(defines) $($(notdir $*)_includes) $($(notdir $*)_defines) -c $< -o $@

$(libd)/%.a:
	ar rc $@ $($(*)_objs)

$(libd)/%.so:
	$(cpplink) $(soflag) $(rpath) $(cflags) -o $@.$($(*)_spec) -Wl,-soname=$(@F).$($(*)_ver) $($(*)_dbjs) $($(*)_dlnk) $(cpp_dll_lnk) $(sock_lib) $(math_lib) $(thread_lib) $(malloc_lib) $(dynlink_lib) && \
	cd $(libd) && ln -f -s $(@F).$($(*)_spec) $(@F).$($(*)_ver) && ln -f -s $(@F).$($(*)_ver) $(@F)

$(bind)/%:
	$(cpplink) $(cflags) $(rpath) -o $@ $($(*)_objs) -L$(libd) $($(*)_lnk) $(cpp_lnk) $(sock_lib) $(math_lib) $(thread_lib) $(malloc_lib) $(dynlink_lib)

$(bind)/%.static:
	$(cpplink) $(cflags) -o $@ $($(*)_objs) $($(*)_static_lnk) $(sock_lib) $(math_lib) $(thread_lib) $(malloc_lib) $(dynlink_lib)

$(dependd)/%.d: src/%.cpp
	$(cpp) $(arch_cflags) $(defines) $(includes) $($(notdir $*)_includes) $($(notdir $*)_defines) -MM $< -MT $(objd)/$(*).o -MF $@

$(dependd)/%.d: src/%.c
	$(cc) $(arch_cflags) $(defines) $(includes) $($(notdir $*)_includes) $($(notdir $*)_defines) -MM $< -MT $(objd)/$(*).o -MF $@

$(dependd)/%.fpic.d: src/%.cpp
	$(cpp) $(arch_cflags) $(defines) $(includes) $($(notdir $*)_includes) $($(notdir $*)_defines) -MM $< -MT $(objd)/$(*).fpic.o -MF $@

$(dependd)/%.fpic.d: src/%.c
	$(cc) $(arch_cflags) $(defines) $(includes) $($(notdir $*)_includes) $($(notdir $*)_defines) -MM $< -MT $(objd)/$(*).fpic.o -MF $@

$(dependd)/%.d: test/%.cpp
	$(cpp) $(arch_cflags) $(defines) $(includes) $($(notdir $*)_includes) $($(notdir $*)_defines) -MM $< -MT $(objd)/$(*).o -MF $@

$(dependd)/%.d: test/%.c
	$(cc) $(arch_cflags) $(defines) $(includes) $($(notdir $*)_includes) $($(notdir $*)_defines) -MM $< -MT $(objd)/$(*).o -MF $@

