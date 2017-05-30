#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import urllib
import urllib2
import tempfile
import zipfile
import glob
import subprocess
import getopt
import sys
import shutil
import os
import string

FIRST_RELEASE_ID=3084382
VERSIONS = [
  ('develop', 'develop branch (development)', 'develop'),
  ('master', 'branch master (stable)', 'stable')
]

current_dir = os.path.dirname(__file__)
output_dir = getopt.getopt(sys.argv[1:], 'o:')[0][0][1]
github_releases = json.load(urllib2.urlopen('https://api.github.com/repos/marvinroger/homie-esp8266/releases'))

def generate_docs(tag_name, description, destination_folder):
  print('Generating docs for ' + tag_name + ' (' + description  + ') at /' + destination_folder + '...')
  zip_url = 'https://github.com/marvinroger/homie-esp8266/archive/' + tag_name + '.zip'
  zip_path = tempfile.mkstemp()[1]
  urllib.urlretrieve(zip_url, zip_path)

  zip_file = zipfile.ZipFile(zip_path, 'r')
  unzip_path = tempfile.mkdtemp()
  zip_file.extractall(unzip_path)
  src_path = glob.glob(unzip_path + '/*')[0]

  if not os.path.isfile(src_path + '/mkdocs.yml'): shutil.copy(current_dir + '/mkdocs.default.yml', src_path + '/mkdocs.yml')

  subprocess.call(['docker', 'run', '--rm', '-it', '-p', '8000:8000', '-v', src_path + ':/docs', 'squidfunk/mkdocs-material', 'build'])
  shutil.copytree(src_path + '/site', output_dir + '/' + destination_folder)
  print('Done.')

# Generate docs for branches

for version in VERSIONS:
  generate_docs(version[0], version[1], version[2])

# Generate docs for releases

for release in github_releases:
  if (release['id'] < FIRST_RELEASE_ID): continue

  tag_name = release['tag_name']
  version = tag_name[1:]
  description = 'release ' + version

  VERSIONS.append((tag_name, description, version))
  generate_docs(tag_name, description, version)

# Generate index

versions_html = '<ul>'
for version in VERSIONS:
  versions_html += '<li><a href="' + version[2] + '"># ' + version[0] + ' <span class="description">' + version[1] + '</span></a></li>'
versions_html += '</ul>'

docs_index_template_file = open(current_dir + '/docs_index_template.html')
docs_index_template_html = docs_index_template_file.read()
docs_index_template = string.Template(docs_index_template_html)
docs_index = docs_index_template.substitute(versions_html=versions_html)

docs_index_file = open(output_dir + '/index.html', 'w')
docs_index_file.write(docs_index)
docs_index_file.close()
