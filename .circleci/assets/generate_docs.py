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
DOCS_PATH = 'docs'
DOCS_BRANCHES = [
  { 'tag': 'develop-v3', 'description': 'V3 develop branch (development)', 'path': 'develop-v3' },
  { 'tag': 'develop', 'description': 'develop branch (development)', 'path': 'develop' },
  { 'tag': 'master', 'description': 'master branch (stable)', 'path': 'stable' }
]
CONFIGURATORS_PATH = 'configurators'
CONFIGURATORS_VERSIONS = [
  { 'title': 'v2', 'description': 'For Homie v2.x.x', 'path': 'v2', 'url': 'https://github.com/marvinroger/homie-esp8266-setup/raw/gh-pages/ui_bundle.html' },
  { 'title': 'v1', 'description': 'For Homie v1.x.x', 'path': 'v1', 'file': '/configurator_v1.html' }
]

current_dir = os.path.dirname(__file__)
output_dir = getopt.getopt(sys.argv[1:], 'o:')[0][0][1]
github_releases = json.load(urllib2.urlopen('https://api.github.com/repos/homieiot/homie-esp8266/releases'))

def generate_docs(data):
  print('Generating docs for ' + data['tag'] + ' (' + data['description'] + ') at /' + data['path'] + '...')
  zip_url = 'https://github.com/homieiot/homie-esp8266/archive/' + data['tag'] + '.zip'
  zip_path = tempfile.mkstemp()[1]
  urllib.urlretrieve(zip_url, zip_path)

  zip_file = zipfile.ZipFile(zip_path, 'r')
  unzip_path = tempfile.mkdtemp()
  zip_file.extractall(unzip_path)
  src_path = glob.glob(unzip_path + '/*')[0]

  if not os.path.isfile(src_path + '/mkdocs.yml'): shutil.copy(current_dir + '/mkdocs.default.yml', src_path + '/mkdocs.yml')

  subprocess.call(['mkdocs', 'build'], cwd=src_path)
  shutil.copytree(src_path + '/site', output_dir + '/' + DOCS_PATH + '/' + data['path'])
  print('Done.')

def generate_configurators(data):
  print('Generating configurator for ' + data['title'] + ' (' + data['description']  + ') at /' + data['path'] + '...')
  file_path = None
  if 'file' in data:
    file_path = current_dir + data['file']
  else: # url
    file_path = tempfile.mkstemp()[1]
    urllib.urlretrieve(data['url'], file_path)

  prefix_output = output_dir + '/' + CONFIGURATORS_PATH + '/' + data['path']
  try:
    os.makedirs(prefix_output)
  except:
    pass

  shutil.copy(file_path, prefix_output + '/index.html')

  print('Done.')

shutil.rmtree(output_dir, ignore_errors=True)

# Generate docs

generated_docs = []

# Generate docs for branches

for branch in DOCS_BRANCHES:
  generated_docs.append(branch)
  generate_docs(branch)

# Generate docs for releases

for release in github_releases:
  if (release['id'] < FIRST_RELEASE_ID): continue

  tag_name = release['tag_name']
  version = tag_name[1:]
  description = 'release ' + version

  data = {
    'tag': tag_name,
    'description': description,
    'path': version
  }

  generated_docs.append(data)
  generate_docs(data)

# Generate documentation html

documentation_html = '<ul>'
for documentation_data in generated_docs:
  documentation_html += '<li><a href="' + DOCS_PATH + '/' + documentation_data['path'] + '"># ' + documentation_data['tag'] + ' <span class="description">' + documentation_data['description'] + '</span></a></li>'
documentation_html += '</ul>'

# Generate configurators

generated_configurators = []

for version in CONFIGURATORS_VERSIONS:
  generated_configurators.append(version)
  generate_configurators(version)

# Generate configurators html

configurators_html = '<ul>'
for configurator_data in generated_configurators:
  configurators_html += '<li><a href="' + CONFIGURATORS_PATH + '/' + configurator_data['path'] + '"># ' + configurator_data['title'] + ' <span class="description">' + configurator_data['description'] + '</span></a></li>'
configurators_html += '</ul>'

# Generate index

docs_index_template_file = open(current_dir + '/docs_index_template.html')
docs_index_template_html = docs_index_template_file.read()
docs_index_template = string.Template(docs_index_template_html)
docs_index = docs_index_template.substitute(documentation_html=documentation_html, configurators_html=configurators_html)

docs_index_file = open(output_dir + '/index.html', 'w')
docs_index_file.write(docs_index)
docs_index_file.close()
