# -*- coding: utf-8 -*-

# Copyright (c) 2020
#      Christian CAMIER <christian.c@promethee.serrvices>
# 
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
# 
# 
# #@ "CCA.XML.py"
#       -- XML file interpretation toolkit
# 
# Author : Christian CAMIER
# 
# Rev : 1.0 (06/23/2008)
# 
# Change log
# ===========
# 06/23/2008 -- 1.0 -- Christian CAMIER
#         Creation
"""
CCA Libraries -- XML file interpretation toolkit
"""

import xml.sax

XMLMandatory = object()

class XMLError(Exception):
    """
    XML error exception class
    """
    pass

class __XMLTagMeta(type):
    """
    Metaclass for XMLTag classes.
    Foreach XMLTag subclass compute the following class attributes:
    - AttributesList:
      List of the allowed attributes for the XML Element
    - AttributesMandatory:
      List of the mandatories attributes
    - AttributesDefault:
      Dictionary of the defaults values for the optionnal
      attributes
    - ChildrenList:
      List of allowed childrens XML elements
    - ChildrenMandatory:
      List for the mandatory childrens XML elements
    """
    def __new__(mcs, name, bases, dictionary):
        klass = type.__new__(mcs, name, bases, dictionary)
        klass.AttributesList      = list()
        klass.AttributesMandatory = list()
        klass.AttributesDefault   = dict()
        klass.ChildrenList        = list()
        klass.ChildrenMandatory   = list()

        for a, v in klass.AttributesDict.items():
            klass.AttributesList.append(a)
            if v == XMLMandatory:
                klass.AttributesMandatory.append(a)
            else:
                klass.AttributesDefault[a] = str(v)

        for c, m in klass.ChildrenDict.items():
            klass.ChildrenList.append(c)
            if m:
                klass.ChildrenMandatory.append(c)
        return klass

class XMLTag(object, metaclass = __XMLTagMeta):
    """
    XMLTag is a virtual class, it cannot be intanciated directly.

    Each tag, in a specific context of the XML document has it's own
    XMLTag derivated class.

    Each XMLTag derivated class 

    class XMLTagSample(XMLTag):
        IsUnique            = False|True
        ElementTag          = <tagname>
        AttributesDict      = { <attributename>:<defaultvalue>* }
        ChildrenDict        = { child_class:<is_mandatory>* }

        def finalize(self, data):
            finalization code
            ...
            return
    With :
        <attributename> :: string
        <defaultvalue>  :: string|XMLMandatory
        <tagname>       :: string
        <is_mandatory>  :: False|True
        child_class     :: A class derivated from XMLTag
    """

    initialized    = False
    IsUnique       = False
    ElementTag     = ''
    AttributesDict = { }
    ChildrenDict   = { }

    parent   = property (fget = lambda self: self.get_parent(),   doc = '')
    text     = property (fget = lambda self: self.get_text(),     doc = '')
    children = property (fget = lambda self: self.get_children(), doc = '')

    def __init__(self, parent = None):
        if self.__class__ == XMLTag:
            raise RuntimeError("{0:s} is virtuel".format(self.__class__.__name__))
        self.__parent = parent
        self.__text = ''
        self.__attributesValues  = {}
        self.__attributesControl = {}
        for attribute in self.AttributesList:
            self.__attributesControl[attribute] = False
        self.__childNodes   = []
        self.__childControl = {}
        for child in self.ChildrenMandatory:
            self.__childControl[child] = False
        self.__childUnique  = []
        return

    def __del__(self):
        """
        Cleanup object
        """
        del self.__text
        del self.__attributesValues
        del self.__attributesControl
        del self.__childNodes
        del self.__childControl
        del self.__childUnique
        return

    def _validate_attribute(self, key, raiseerror = True):
        if key in self.AttributesList:
            return True
        if raiseerror:
            raise XMLError('Unknown attribut "{:s}" for tag "{:s}"'.format(key, self.ElementTag))
        return False

    def validate(self):
        """
        validate the object content
        """
        attr_ok  = True
        attr_mis = ''
        attr_sep = ''
        chld_ok  = True
        chld_mis = ''
        chld_sep = ''
        err_text = ''
        for a in self.AttributesMandatory:
            if not self.__attributesControl[a]:
                attr_ok   = False
                attr_mis += "{:s}- {:s}".format(attr_sep, a)
                attr_sep  = ',\n'
        for c in self.ChildrenMandatory:
            if not self.__childControl[c]:
                chld_ok   = False
                chld_mis += "{:s}- {:s}".format(chld_sep, c.ElementTag)
                chld_sep = ',\n'
        if attr_ok and chld_ok:
            return
        if not attr_ok:
            err_text += 'The following attributes are missing :\n{:s}\n'.format(attr_mis)
        if not chld_ok:
            err_text += 'The following tags are missing :\n{:s}\n'.format(chld_mis)
        raise XMLError('Error on tag {:s}'.format(self.ElementTag, err_text))

    def visitor(self, tag):
        """
        Visitor for child creation
        Find the class associated to "tag" and return the instanciated object
        """
        for C in self.ChildrenList:
            if C.ElementTag == tag:
                if C.IsUnique:
                    if C in self.__childUnique:
                        raise XMLError('Sub tag {:s} must be unique for tag {:s}'.format(C.ElementTag, self.ElementTag ))
                    self.__childUnique.append(C)
                nobj = C(self)
                self.__childNodes.append(nobj)
                if C in self.ChildrenMandatory:
                    self.__childControl[C] = True
                return nobj
        raise XMLError("Unespected sub marker {0:s} for marker {1:s}".format(tag, self.ElementTag))

    def add_text(self, text):
        """
        Add text to the object
        """
        self.__text += text

    def get_children(self):
        """
        Return list of the object children
        """
        return self.__children

    def get_parent(self):
        """
        Return the parent object
        """
        return self.__parent

    def get_text(self):
        """
        Return associated text
        """
        return str(self.__text)

    def set_attribute(self, key, value, overwrite = True):
        """
        Set object attribute
        """
        self._validate_attribute(key)
        if not overwrite and self.__attributesControl[key]:
            raise XMLError('Marker {:s}: Attribut {:s} already set'.format(self.ElementTag, key))
        self.__attributesValues[key]  = value
        self.__attributesControl[key] = True
        return

    def get_attribute(self, key):
        """
        Get object attribute
        """
        self._validate_attribute(key)
        if self.__attributesControl[key]:
            return str(self.__attributesValues[key])
        return None

    def start(self, data):
        """
        Start object
        Method called when entering in a new object
        data is a reference to user data passed in the XML.parse method.
        """
        return

    def finalize(self, data):
        """
        Finalize object.
        data is a reference to user data passed in the XML.parse method.
        """
        return

class __SAXParser(xml.sax.ContentHandler):
    """
    SAX Parser handler class
    """
    def __init__(self, root, data = None):
        #super(xml.sax.ContentHandler, self).__init__()
        self.__rootElement = root
        self.__currElement = root
        self.__handlerData = data
        return

    def startElement(self, tag, attributes):
        newObject = self.__currElement.visitor(tag)
        for attribute, value in attributes.items():
            newObject.set_attribute(attribute, value)
        newObject.previous = self.__currElement
        self.__currElement = newObject
        newObject.start(self.__handlerData)
        return

    def endElement(self, tag):
        self.__currElement.validate()
        self.__currElement.finalize(self.__handlerData)
        self.__currElement = self.__currElement.parent
        return

    def characters(self, text):
        self.__currElement.add_text(text)
        return

def parse(filename, rootclass, data = None):
    """
    Parse XML file named "filename" according rules given by rootclass.
    The "rootclass" argument is the class (derivated from XMLTag) of
    the root element. The root element is instanciated by the fonction.
    The "data" argument will be passed as "data" argument when finalize
    method of XMLTag instances is invoked.
    Return the intanciated root element. If an error occurs, an XMLError
    exception will be raised.
    """
    if not issubclass(rootclass, XMLTag):
        raise XMLError("The given root class is not an XMLTag subclass")
    root    = rootclass()
    handler = __SAXParser(root, data)
    parser  = xml.sax.make_parser()
    parser.setContentHandler(handler)
    try:
        filedesc = None
        filedesc = open(filename, 'r')
        parser.parse(filedesc)
    except IOError as e:
        raise XMLError('IOError: ' + str(e))
    except xml.sax.SAXException as e:
        raise XMLError('SAXException: ' + str(e))
    finally:
        if filedesc is not None:
            filedesc.close()
        del parser
    return root

__all__ = [ 'XMLError', 'XMLMandatory', 'XMLTag', 'parse' ]

if __name__ == "__main__":
    help(__name__)
    exit(1)
