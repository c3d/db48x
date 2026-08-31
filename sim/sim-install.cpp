// ****************************************************************************
//  sim-install.cpp                                               DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Copy installed simulator resources, normalizing help file line endings
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2026 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************
//   This file is part of DB48X.
//
//   DB48X is free software: you can redistribute it and/or modify
//   it under the terms outlined in the LICENSE.txt file
//
//   DB48X is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// ****************************************************************************

#include "sim-install.h"

#include <QFile>


bool sim_install_help_file_uses_lf(const QString &relativePath)
// ----------------------------------------------------------------------------
//   Help markdown and index files use byte offsets; require LF on all platforms
// ----------------------------------------------------------------------------
{
    if (!relativePath.startsWith("help/"))
        return false;
    return relativePath.endsWith(".md", Qt::CaseInsensitive)
        || relativePath.endsWith(".idx", Qt::CaseInsensitive);
}


bool sim_install_copy_file(const QString &from,
                           const QString &to,
                           const QString &relativePath,
                           QFileDevice::Permissions perms)
// ----------------------------------------------------------------------------
//   Copy one installed file, stripping CR bytes from help files if needed
// ----------------------------------------------------------------------------
{
    if (sim_install_help_file_uses_lf(relativePath))
    {
        QFile in(from);
        if (!in.open(QIODevice::ReadOnly))
            return false;
        QByteArray data = in.readAll();
        in.close();
        data.replace("\r", "");

        QFile out(to);
        if (!out.open(QIODevice::WriteOnly))
            return false;
        if (out.write(data) != data.size())
            return false;
        out.close();
        return QFile::setPermissions(to, perms);
    }

    if (!QFile::copy(from, to))
        return false;
    return QFile::setPermissions(to, perms);
}
