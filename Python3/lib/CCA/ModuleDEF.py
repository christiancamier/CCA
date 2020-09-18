#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ----------------------------------------------------------------------
# Project: EKPYRO
# ----------------------------------------------------------------------
# Description :
#   Short description of the file content
# ----------------------------------------------------------------------
# Changelog (in reverse chronological order)
#
# Version : x.y.z
# Author  : Author Name
# Date    : mm/dd/yyyy
# Changes :
#    Changes
# ----------------------------------------------------------------------

"""
Documentation
"""

__version__ = 'x.y.z'

import fnmatch
import os

from distutils.core      import setup, Extension
from distutils.ccompiler import CCompiler

from .XML import *

class XMLTagAuthor(XMLTag):
     IsUnique       = True
     ElementTag     = "author"
     AttributesDict = {
          'name':  XMLMandatory,
          'email': XMLMandatory,
     }
     def finalize(self, data):
          data.author_name  = self.get_attribute('name')
          data.author_email = self.get_attribute('email')
          return

class XMLTagCompArg(XMLTag):
     IsUnique        = False
     ElementTag      = "argument"
     AttributesDict = {
          'arg': XMLMandatory,
     }
     def finalize(self, data):
          data.module_cu.comp_args.append(self.get_attribute('arg'))
          return

class XMLTagCompCompile(XMLTag):
     IsUnique       = True
     ElementTag     = "compile"
     ChildrenDict   = {
          XMLTagCompArg:      True,
     }

class XMLTagIncDir(XMLTag):
     IsUnique        = False
     ElementTag      = "directory"
     AttributesDict = {
          'path': XMLMandatory,
     }
     def finalize(self, data):
          data.module_cu.incdirs.append(self.get_attribute('path'))
          return

class XMLTagCompInc(XMLTag):
     IsUnique       = True
     ElementTag     = "include"
     ChildrenDict   = {
          XMLTagIncDir:      True,
     }

class XMLTagLibDir(XMLTag):
     IsUnique        = False
     ElementTag      = "directory"
     AttributesDict = {
          'path': XMLMandatory,
     }
     def finalize(self, data):
          data.module_cu.libdirs.append(self.get_attribute('path'))
          return

class XMLTagLibLib(XMLTag):
     IsUnique        = False
     ElementTag      = "library"
     AttributesDict = { 'name': XMLMandatory, }
     def finalize(self, data):
          data.module_cu.liblibs.append(self.get_attribute('name'))
          return

class XMLTagCompLibs(XMLTag):
     IsUnique       = True
     ElementTag     = "libraries"
     ChildrenDict   = {
          XMLTagLibDir:      False,
          XMLTagLibLib:      True,
     }

class XMLTagLinkArg(XMLTag):
     IsUnique        = False
     ElementTag      = "argument"
     AttributesDict = {
          'arg': XMLMandatory,
     }
     def finalize(self, data):
          data.module_cu.link_args.append(self.get_attribute('arg'))
          return

class XMLTagCompLink(XMLTag):
     IsUnique       = True
     ElementTag     = "link"
     ChildrenDict   = {
          XMLTagLinkArg:      True,
     }

class XMLTagMacDef(XMLTag):
     IsUnique       = False
     ElementTag     = "define"
     AttributesDict = {
          'name': XMLMandatory,
          'value': None
     }
     def finalize(self, data):
          data.module_cu.macdefine.append((self.get_attribute('name'), self.get_attribute('value'),))
          return

class XMLTagMacUnd(XMLTag):
     IsUnique       = False
     ElementTag     = "undefine"
     AttributesDict = {
          'name': XMLMandatory,
     }
     def finalize(self, data):
          data.module_cu.macundef.append(self.get_attribute('name'))
          return

class XMLTagCompMacros(XMLTag):
     IsUnique       = True
     ElementTag     = "macros"
     ChildrenDict   = {
          XMLTagMacDef:      False,
          XMLTagMacUnd:      False,
     }

class XMLTagCompilation(XMLTag):
     IsUnique       = True
     ElementTag     = "compilation"
     ChildrenDict   = {
          XMLTagCompInc:     False,
          XMLTagCompLibs:    False,
          XMLTagCompMacros:  False,
          XMLTagCompCompile: False,
          XMLTagCompLink:    False,
     }

class XMLTagDescription(XMLTag):
     IsUnique       = True
     ElementTag     = "description"
     AttributesDict = {
          'short': XMLMandatory,
     }
     def finalize(self, data):
          data.description_l = self.get_text()
          data.description_s = self.get_attribute('short')
          return

class XMLTagSite(XMLTag):
     IsUnique       = True
     ElementTag     = "site"
     AttributesDict = {
          'url': XMLMandatory,
     }
     def finalize(self, data):
          data.site = self.get_attribute('url')
          return

class XMLTagSrcFile(XMLTag):
     IsUnique       = False
     ElementTag     = "file"
     AttributesDict = {
          'name': XMLMandatory,
     }

     @staticmethod
     def __dirname(f):
          return( lambda x: x and x or '.')(os.path.dirname(f))

     def finalize(self, data):
          pattern = self.get_attribute('name')
          sources = list(f for f in os.listdir('.') if fnmatch.fnmatch(f, pattern))
          if not sources:
               raise XMLError("{0:s}: No surch file or directory".format(pattern))
          for source in sources:
               if source not in data.module_cu.sources_s:
                    data.module_cu.sources_l.append(source)
                    data.module_cu.sources_s.add   (source)
          return

class XMLTagSources(XMLTag):
     IsUnique       = True
     ElementTag     = "sources"
     ChildrenDict   = {
          XMLTagSrcFile:    True,
     }

class XMLTagModule(XMLTag):
     IsUnique       = False
     ElementTag     = "module"
     ChildrenDict   = {
          XMLTagCompilation: False,
          XMLTagSources:     False,
     }
     AttributesDict = {
          'name':     XMLMandatory,
          'language': None,
     }
     def start(self, data):
          data.new_module(self.get_attribute('name'), self.get_attribute('language'))
          return
     def finalize(self, data):
          data.end_module()
          return

class XMLTagPackage(XMLTag):
     IsUnique       = False
     ElementTag     = "package"
     ChildrenDict   = {
          XMLTagAuthor:      True,
          XMLTagDescription: True,
          XMLTagModule:      True,
          XMLTagSite:        False,
     }
     AttributesDict = {
          'name': XMLMandatory,
          'version': '1.0'
     }
     def finalize(self, data):
          data.pkgname = self.get_attribute('name')
          data.pkgvers = self.get_attribute('version')
          return

class XMLDocumentRoot(XMLTag):
     IsUnique = False;
     ElementTag   = '*root'
     ChildrenDict = { XMLTagPackage: True }

class Module(object):
     def __init__(self, definition, name, language):
          self.comp_args     = list()
          self.definition    = definition
          self.incdirs       = list()
          self.language      = language
          self.libdirs       = list()
          self.liblibs       = list()
          self.link_args     = list()
          self.macdefine     = [ ('MODULENAME', name, ) ]
          self.macundef      = list()
          self.name          = name
          self.sources_l     = list()
          self.sources_s     = set()
          return

     def __str__(self):
          #print(list((x, str(y)) for x, y in self.__dict__.items()))
          return """  Module: {name:s}
   Comp args:        {comp_args:s}
   IncDirs:          {incdirs:s}
   Language:         {language:s}
   LibDirs:          {libdirs:s}
   LibLibs:          {liblibs:s}
   Link Args:        {link_args:s}
   Defines:          {macdefine:s}
   Undefines:        {macundef:s}
   Sources:          {sources_l:s}
""".format(**dict((x, str(y)) for x, y in self.__dict__.items() if x != 'definition'))

     def extension(self):
          kwa = {
               'name':    self.name,
               'sources': self.sources_l,
          }
          if self.comp_args: kwa['extra_compile_args'] = self.comp_args
          if self.incdirs:   kwa['include_dirs'      ] = self.incdirs
          if self.language:  kwa['language'          ] = self.language
          if self.libdirs:   kwa['library_dirs'      ] = self.libdirs
          if self.liblibs:   kwa['libraries'         ] = self.liblibs
          if self.link_args: kwa['extra_link_args'   ] = self.link_args
          if self.macdefine: kwa['define_macros'     ] = self.macdefine
          if self.macundef:  kwa['undef_macros'      ] = self.macundef
          return Extension(**kwa)

     def finalize(self):
          return

     def finalize0(self):
          def is_source(self, source):
               if not os.path.isfile(os.path.join(self.definition.srcdir, source)):
                    return False
               bse, ext = os.path.splitext(source)
               lan = CCompiler.language_map.get(ext)
               if lan is None:
                    return False
               if self.language is None or (self.language is not None and lan == self.language):
                    return True
               return False
          print("FINALIZE")
          if not self.sources_l:
               self.sources_l = list(os.path.join(self.definition.srcdir, f) for f in os.listdir(self.definition.srcdir) if is_source(self, f))

class Definition(object):
     def __init__(self, filename):
          srcdir = os.path.dirname(filename)
          srcdir = srcdir or '.'
          self.author_name   = None
          self.author_email  = None
          self.description_l = None
          self.description_s = None
          self.maint_name    = None # Futur use
          self.maint_email   = None # Futur use
          self.modules_l     = list()
          self.modules_s     = set()
          self.module_cu     = None
          self.pkgname       = None
          self.pkgvers       = None
          self.site          = None
          self.srcdir        = srcdir
          return

     def __str__(self):
          return """Package: {pkgname:s} vers {pkgvers:s}\n
 Author name:      {author_name:s}
 Author email:     {author_email:s}
 Short Desciption: {description_s:s}
 Long  Desciption: {description_l:s}
 Site:             {site:s}
{modules:s}\n""".format(
     author_name   = self.author_name,
     author_email  = self.author_email,
     description_l = self.description_l,
     description_s = self.description_s,
     pkgname       = self.pkgname,
     pkgvers       = self.pkgvers,
     site          = self.site,
     modules = "\n".join(str(m) for m in self.modules_l))

     @classmethod
     def read_configuration(klass, filename):
          definition = klass(filename)
          parse(filename, XMLDocumentRoot, definition)
          return definition

     def new_module(self, name, language):
          if name in self.modules_s:
               raise XMLError("Duplicate module '{0:s}'".format(name))
          new = Module(self, name, language)
          self.modules_s.add(name)
          self.modules_l.append(new)
          self.module_cu = new
          return

     def end_module(self):
          self.module_cu.finalize()
          self.module_cu = None
          return

     def setup(self):
          kwa = {
               'author':           self.author_name,
               'author_email':     self.author_email,
               'description':      self.description_s,
               'ext_modules':      list(m.extension() for m in self.modules_l),
               'long_description': self.description_l,
               'name':             self.pkgname,
               'url':              self.site,
               'version':          self.pkgvers,
               }
          if self.maint_name:  kwa['maintainer'      ] = self.maint_name
          if self.maint_email: kwa['maintainer_email'] = self.maint_email
          return setup(**kwa)

if '__main__' == __name__:
     # Place here elementary test if possible
     raise RuntimeError("I'm a python module")
