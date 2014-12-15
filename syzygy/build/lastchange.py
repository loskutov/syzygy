#!/usr/bin/env python
# Copyright 2014 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
lastchange.py -- Chromium revision fetching utility.
"""

import re
import optparse
import os
import subprocess
import sys

class VersionInfo(object):
  def __init__(self, url, revision):
    self.url = url
    self.revision = revision


def RunGitCommand(directory, command):
  """
  Launches git subcommand.

  Errors are swallowed.

  Returns:
    A process object or None.
  """
  command = ['git'] + command
  # Force shell usage under cygwin. This is a workaround for
  # mysterious loss of cwd while invoking cygwin's git.
  # We can't just pass shell=True to Popen, as under win32 this will
  # cause CMD to be used, while we explicitly want a cygwin shell.
  if sys.platform == 'cygwin':
    command = ['sh', '-c', ' '.join(command)]
  try:
    proc = subprocess.Popen(command,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                            cwd=directory,
                            shell=(sys.platform=='win32'))
    return proc
  except OSError:
    return None


def FetchGitRevision(directory):
  """
  Fetch the Git hash for a given directory.

  Errors are swallowed.

  Returns:
    A VersionInfo object or None on error.
  """
  proc = RunGitCommand(directory, ['rev-parse', 'HEAD'])
  if proc:
    output = proc.communicate()[0].strip()
    if proc.returncode == 0 and output:
      return VersionInfo('git', output)
  return None


def WriteIfChanged(file_name, contents):
  """
  Writes the specified contents to the specified file_name
  iff the contents are different than the current contents.
  """
  try:
    old_contents = open(file_name, 'r').read()
  except EnvironmentError:
    pass
  else:
    if contents == old_contents:
      return
    os.unlink(file_name)
  open(file_name, 'w').write(contents)


def main(argv=None):
  if argv is None:
    argv = sys.argv

  parser = optparse.OptionParser(usage="lastchange.py [options]")
  parser.add_option("-o", "--output", metavar="FILE",
                    help="write last change to FILE")
  parser.add_option("--revision-only", action='store_true',
                    help="just print the revision number")
  parser.add_option("-s", "--source-dir", metavar="DIR",
                    help="use repository in the given directory")
  opts, args = parser.parse_args(argv[1:])

  out_file = opts.output

  while len(args) and out_file is None:
    if out_file is None:
      out_file = args.pop(0)
  if args:
    sys.stderr.write('Unexpected arguments: %r\n\n' % args)
    parser.print_help()
    sys.exit(2)

  if opts.source_dir:
    src_dir = opts.source_dir
  else:
    src_dir = os.path.dirname(os.path.abspath(__file__))

  version_info = FetchGitRevision(src_dir)

  if version_info.revision == None:
    version_info.revision = '0'

  if opts.revision_only:
    print version_info.revision
  else:
    contents = """# This file was automatically generated by lastchange.py.
LASTCHANGE=%s
LASTCHANGE_FULL=%s
""" % (version_info.revision[:7], version_info.revision)
    if out_file:
      WriteIfChanged(out_file, contents)
    else:
      sys.stdout.write(contents)

  return 0


if __name__ == '__main__':
  sys.exit(main())
