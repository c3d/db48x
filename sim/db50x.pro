#******************************************************************************
# db50x.pro                                                       DB48X project
#******************************************************************************
#
#  File Description:
#
#
#
#
#
#
#
#
#
#
#******************************************************************************
#  (C) 2024 Christophe de Dinechin <christophe@dinechin.org>
#  This software is licensed under the terms outlined in LICENSE.txt
#******************************************************************************
#  This file is part of DB48X.
#
#  DB48X is free software: you can redistribute it and/or modify
#  it under the terms outlined in the LICENSE.txt file
#
#  DB48X is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#******************************************************************************

include(db48x.pro)

DEFINES-=HELPFILE_NAME=\\\"help/db48x.md\\\"
DEFINES-=HELPINDEX_NAME=\\\"help/db48x.idx\\\"
DEFINES+=HELPFILE_NAME=\\\"help/db50x.md\\\"
DEFINES+=HELPINDEX_NAME=\\\"help/db50x.idx\\\"
INCLUDEPATH -= ../src/dm42
INCLUDEPATH += ../src/dm32

OBJECTS_DIR=db50x-build

ICON = db50x.icns
