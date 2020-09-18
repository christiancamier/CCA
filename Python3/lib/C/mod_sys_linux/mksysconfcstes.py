#!/usr/bin/env python3

import re

splitter = re.compile('\s+').split
issyscnf = re.compile('^#define\s_SC_').match
ispthcnf = re.compile('^#define\s_PC_').match
iscnfcst = lambda l: issyscnf(l) or ispthcnf(l)

with open("/usr/include/bits/confname.h", "r", encoding = 'utf-8') as fdi:
    with open("toto", "w", encoding = 'utf-8') as fdo:
        for line in fdi:
            if iscnfcst(line):
                name = splitter(line)[1]
                fdo.write("""#if defined({0:s})\nif(-1 == PyModule_AddIntConstant(module, "{1:s}", {0:s}))\nreturn 0;\n#endif\n""".format(name, name[1:]))
