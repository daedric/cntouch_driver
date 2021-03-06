# This file is NOT licensed under the GPLv3, which is the license for the rest
# of YouCompleteMe.
#
# Here's the license text for this file:
#
# This is free and unencumbered software released into the public domain.
#
# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means.
#
# In jurisdictions that recognize copyright laws, the author or authors
# of this software dedicate any and all copyright interest in the
# software to the public domain. We make this dedication for the benefit
# of the public at large and to the detriment of our heirs and
# successors. We intend this dedication to be an overt act of
# relinquishment in perpetuity of all present and future rights to this
# software under copyright law.
#
# THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# For more information, please refer to <http://unlicense.org/>

import os
import ycm_core

# These are the compilation flags that will be used in case there's no
# compilation database set (by default, one is not set).
# CHANGE THIS LIST OF FLAGS. YES, THIS IS THE DROID YOU HAVE BEEN LOOKING FOR.
flags = [
'-Wall',
'-Wextra',
'-I',
'/lib/modules/3.13.0-32-generic/build/include',
'-I',
'.',
'-std=c99'


'-nostdinc',
'-isystem',
'/usr/lib/gcc/x86_64-linux-gnu/4.8/include',
'-I',
'/usr/src/linux-headers-3.13.0-32-generic/arch/x86/include',
'-I',
'arch/x86/include/generated',
'-I',
'include',
'-I',
'/usr/src/linux-headers-3.13.0-32-generic/arch/x86/include/uapi',
'-I',
'arch/x86/include/generated/uapi',
'-I',
'/usr/src/linux-headers-3.13.0-32-generic/include/uapi',
'-I',
'include/generated/uapi',
'-include',
'/usr/src/linux-headers-3.13.0-32-generic/include/linux/kconfig.h',
'-I',
'ubuntu/include',
'-D__KERNEL__',
'-Wall',
'-Wundef',
'-Wstrict-prototypes',
'-Wno-trigraphs',
'-fno-strict-aliasing',
'-fno-common',
'-Werror-implicit-function-declaration',
'-Wno-format-security',
'-fno-delete-null-pointer-checks',
'-O2',
'-m64',
'-mno-mmx',
'-mno-sse',
'-mpreferred-stack-boundary=3',
'-mtune=generic',
'-mno-red-zone',
'-mcmodel=kernel',
'-funit-at-a-time',
'-maccumulate-outgoing-args',
'-fstack-protector',
'-DCONFIG_X86_X32_ABI',
'-DCONFIG_AS_CFI=1',
'-DCONFIG_AS_CFI_SIGNAL_FRAME=1',
'-DCONFIG_AS_CFI_SECTIONS=1',
'-DCONFIG_AS_FXSAVEQ=1',
'-DCONFIG_AS_AVX=1',
'-DCONFIG_AS_AVX2=1',
'-pipe',
'-Wno-sign-compare',
'-fno-asynchronous-unwind-tables',
'-mno-sse',
'-mno-mmx',
'-mno-sse2',
'-mno-3dnow',
'-mno-avx',
'-Wframe-larger-than=1024',
'-Wno-unused-but-set-variable',
'-fno-omit-frame-pointer',
'-fno-optimize-sibling-calls',
'-fno-var-tracking-assignments',
'-pg',
'-mfentry',
'-DCC_USING_FENTRY',
'-Wdeclaration-after-statement',
'-Wno-pointer-sign',
'-fno-strict-overflow',
'-fconserve-stack',
'-Werror=implicit-int',
'-Werror=strict-prototypes',
'-DCC_HAVE_ASM_GOTO',
'-O2',
'-I/home/daedric/usb-driver/../include',
'-DMODULE',
'-D"KBUILD_STR(s)=#s"',
'-D"KBUILD_BASENAME=KBUILD_STR(cntouch)"',
'-D"KBUILD_MODNAME=KBUILD_STR(cntouch)"',

]

# Set this to the absolute path to the folder (NOT the file!) containing the
# compile_commands.json file to use that instead of 'flags'. See here for
# more details: http://clang.llvm.org/docs/JSONCompilationDatabase.html
#
# Most projects will NOT need to set this to anything; you can just change the
# 'flags' list of compilation flags. Notice that YCM itself uses that approach.
def DirectoryOfThisScript():
  return os.path.dirname( os.path.abspath( __file__ ) )

compilation_database_folder = os.path.join(DirectoryOfThisScript(), 'build/')

if os.path.exists(compilation_database_folder):
  database = ycm_core.CompilationDatabase( compilation_database_folder )
else:
  database = None

CPP_EXTS = ['cpp', 'c', 'cc', 'cxx', 'C']

def IsHeader(path):
  name, ext = os.path.splitext(path);
  return ext[1:] in ['h', 'hpp', 'hh', 'hxx', 'H']

def IsSource(path):
  name, ext = os.path.splitext(path);
  return ext[1:] in CPP_EXTS

def GetSource(path):
  name, ext = os.path.splitext(path);

  if 'include' in name:
    name = name.replace('include', 'src')

  for ext in CPP_EXTS:
    filename = name + '.' + ext
    if os.path.exists(filename):
      return filename

  filename = os.path.basename(path)
  filename, ext = os.path.splitext(filename)

  possibilities = set([filename + '.' + ext for ext in CPP_EXTS])
  for root, dirs, files in os.walk('.'):
    files = set(files)
    intersection = files & possibilities
    for file in intersection:
      file = os.path.join(root, file)
      if os.path.exists(file):
        return file

  return None

def BestEffort(filename):
  file_dir = os.path.dirname(filename)
  for root, dirs, files in os.walk(file_dir):
    for f in files:
      f = os.path.join(root, f)
      if IsHeader(f):
        source = GetSource(f)
        if source:
          return source
      elif IsSource(f):
        return f

def MakeRelativePathsInFlagsAbsolute( flags, working_directory ):
  if not working_directory:
    return list( flags )
  new_flags = []
  make_next_absolute = False
  path_flags = [ '-isystem', '-I', '-iquote', '--sysroot=' ]
  for flag in flags:
    new_flag = flag

    if make_next_absolute:
      make_next_absolute = False
      if not flag.startswith( '/' ) and not flag.startswith('.'):
        new_flag = os.path.join( '/usr/src/linux-headers-3.13.0-32-generic', flag )

    for path_flag in path_flags:
      if flag == path_flag:
        make_next_absolute = True
        break

      if flag.startswith( path_flag ):
        path = flag[ len( path_flag ): ]
        new_flag = path_flag + os.path.join( working_directory, path )
        break

    if new_flag:
      new_flags.append( new_flag )
  return new_flags


def FlagsForFile( filename ):
  initial_filename = filename
  final_flags = None
  if database:
    if IsHeader(filename):
      source = GetSource(filename) or BestEffort(filename)
      if source:
        filename = os.path.abspath(source)

    compilation_info = database.GetCompilationInfoForFile( filename )
    final_flags = MakeRelativePathsInFlagsAbsolute(
        compilation_info.compiler_flags_,
        compilation_info.compiler_working_dir_ )

  if not final_flags:
    relative_to = DirectoryOfThisScript()
    final_flags = MakeRelativePathsInFlagsAbsolute( flags, relative_to )

  return {
      'flags': final_flags,
      'do_cache': True
      }
