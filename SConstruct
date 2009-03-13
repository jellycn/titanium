#!/usr/bin/env python

# common SConscripts
import os, re, sys, inspect, os.path as path
import subprocess
from sets import Set

from kroll import BuildConfig
build = BuildConfig(
	PRODUCT_VERSION = '0.3',
	INSTALL_PREFIX = '/usr/local',
	PRODUCT_NAME = 'Titanium',
	GLOBAL_NS_VARNAME = 'Titanium',
	CONFIG_FILENAME = 'tiapp.xml',
	BUILD_DIR = path.abspath('build'),
	THIRD_PARTY_DIR = path.join(path.abspath('kroll'), 'thirdparty'),
	BOOT_RUNTIME_FLAG = '--runtime',
	BOOT_HOME_FLAG = '--start',
	BOOT_UPDATESITE_ENVNAME = 'TI_UPDATESITE',
	BOOT_UPDATESITE_URL = 'http://updatesite.titaniumapp.com'
)

build.titanium_source_dir = path.abspath('.')
build.kroll_source_dir = path.abspath('kroll')
build.kroll_third_party = build.third_party
build.kroll_include_dir = path.join(build.dir, 'include')
build.titanium_support_dir = path.join(build.titanium_source_dir, 'support', build.os)

# This should only be used for accessing various
# scripts in the kroll build directory. All resources
# should instead be built to build.dir
build.kroll_build_dir = path.join(build.kroll_source_dir, 'build')

build.env.Append(CPPPATH=[
	build.titanium_source_dir,
	build.kroll_source_dir,
	build.kroll_include_dir
])

build.env.Append(LIBPATH=[build.dir])

# debug build flags
if ARGUMENTS.get('debug', 0):
	build.env.Append(CPPDEFINES = ('DEBUG', 1))
	debug = 1
	if not build.is_win32():
		build.env.Append(CCFLAGS = ['-g'])  # debug
	else:
		build.env.Append(CCFLAGS = ['/Z7','/GR'])  # max debug, C++ RTTI
else:
	build.env.Append(CPPDEFINES = ('NDEBUG', 1 ))
	debug = 0
	if not build.is_win32():
		build.env.Append(CCFLAGS = ['-O9']) # max optimizations
	else:
		build.env.Append(CCFLAGS = ['/GR']) # C++ RTTI

# turn on special debug printouts for reference counting
if ARGUMENTS.get('debug_refcount', 0) == 1:
	build.env.Append(CPPDEFINES = ('DEBUG_REFCOUNT', 1))


if build.is_win32():
	execfile('kroll/site_scons/win32.py')
	build.env.Append(CCFLAGS=['/EHsc'])
	if build.debug:
		build.env.Append(CPPDEFINES=('WIN32_CONSOLE', 1))
	build.env.Append(LINKFLAGS=['/DEBUG', '/PDB:${TARGET}.pdb'])


# dynamically pull in all kroll and titanium modules
modules_to_exclude = Set(['foo','foojs','foopy','foorb'])
possible_dirs = Glob(path.join(build.kroll_source_dir, 'modules','*')) + Glob(path.join(build.titanium_source_dir,'modules','*'))
module_dirs = []
for dir in possible_dirs:
	dirname = str(dir)
	pname = path.basename(dirname)
	if path.exists(dirname) and path.isdir(dirname) and pname not in modules_to_exclude:
		module_dirs.append(dirname)
build.modules = {}
for dir in module_dirs:
	 build.modules[path.basename(dir.lower().replace('.',''))] = dir
	
Export('build')

# Linux can package and build at the same time now
if not(ARGUMENTS.get('package',0)) or build.is_linux():

	## Kroll *must not be required* for installation
	SConscript('installation/SConscript')

	SConscript('kroll/SConscript', exports='debug')

	# Kroll library is now built (hopefully)
	if not build.is_linux():
		build.env.Append(LIBS=['kroll']) 
	SConscript('modules/SConscript')

if ARGUMENTS.get('package',0):
	print "building packaging ..."
	SConscript('installation/runtime/SConscript')

if ARGUMENTS.get('testapp',0):
	print "building packaging ..."
	SConscript('apps/testapp/SConscript')

if ARGUMENTS.get('testsuite',0):
	print 'running testsuite...'
	SConscript('apps/apivalidator/SConscript')
