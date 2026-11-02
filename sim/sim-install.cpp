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

#include <algorithm>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QString>


struct help_idx_entry
{
    uint        offset;
    QByteArray  text;
};


static bool help_idx_entry_less(const help_idx_entry &a, const help_idx_entry &b)
// ----------------------------------------------------------------------------
//   Sort help index entries like `sort -k2 -t:` in the Makefile
// ----------------------------------------------------------------------------
{
    return a.text < b.text;
}


static QString sim_install_normalized_path(const QString &path)
// ----------------------------------------------------------------------------
//   Normalize a relative install path to forward slashes
// ----------------------------------------------------------------------------
{
    return QDir::fromNativeSeparators(path);
}


static bool sim_install_is_help_markdown(const QString &relativePath)
// ----------------------------------------------------------------------------
//   Check if a relative install path is a help markdown file
// ----------------------------------------------------------------------------
{
    QString path = sim_install_normalized_path(relativePath);
    if (!path.startsWith("help/"))
        return false;
    return path.endsWith(".md", Qt::CaseInsensitive);
}


static bool sim_install_is_help_index(const QString &relativePath)
// ----------------------------------------------------------------------------
//   Check if a relative install path is a help index file
// ----------------------------------------------------------------------------
{
    QString path = sim_install_normalized_path(relativePath);
    if (!path.startsWith("help/"))
        return false;
    return path.endsWith(".idx", Qt::CaseInsensitive);
}


static bool sim_install_write_file(const QString &path, const QByteArray &data,
                                   unsigned perms)
// ----------------------------------------------------------------------------
//   Write a complete file atomically
// ----------------------------------------------------------------------------
{
    QString tmp = path + ".part";
    QFile::remove(tmp);

    QFile out(tmp);
    if (!out.open(QIODevice::WriteOnly))
        return false;
    if (out.write(data) != data.size())
        return false;
    out.close();

    QFile::remove(path);
    if (!QFile::rename(tmp, path))
        return false;
    return QFile::setPermissions(path, QFileDevice::Permissions(perms));
}


static bool sim_install_regenerate_help_index(const QString &mdPath,
                                              const QString &idxPath)
// ----------------------------------------------------------------------------
//   Rebuild one help index from normalized markdown (see Makefile rule)
// ----------------------------------------------------------------------------
{
    QFile md(mdPath);
    if (!md.open(QIODevice::ReadOnly))
        return false;

    QByteArray content = md.readAll();
    md.close();
    content.replace("\r", "");

    QList<help_idx_entry> entries;
    for (uint pos = 0; pos < uint(content.size());)
    {
        uint lineEnd = pos;
        while (lineEnd < uint(content.size()) && content[lineEnd] != '\n')
            lineEnd++;

        uint lineLen = lineEnd - pos;
        char first   = lineLen ? content[pos] : 0;
        if (first == '#')
        {
            entries.append({ pos, content.mid(pos, lineLen) });
        }
        else if (lineLen >= 3 && content[pos] == '*' && content[pos + 1] == ' '
                 && content[pos + 2] == '`')
        {
            QByteArray line = content.mid(pos, lineLen);
            int        end  = line.indexOf('`', 2);
            if (end > 0)
                line = line.left(end + 1);
            entries.append({ pos, line });
        }

        pos = lineEnd + 1;
    }

    std::sort(entries.begin(), entries.end(), help_idx_entry_less);

    QByteArray idx;
    for (const help_idx_entry &entry : entries)
    {
        idx += QByteArray::number(entry.offset);
        idx += ':';
        idx += entry.text;
        idx += '\n';
    }

    return sim_install_write_file(idxPath, idx,
                                  QFileDevice::ReadOwner
                                      | QFileDevice::WriteOwner
                                      | QFileDevice::ReadUser);
}


bool sim_install_copy_file(const QString &from,
                           const QString &to,
                           const QString &relativePath,
                           unsigned     perms)
// ----------------------------------------------------------------------------
//   Copy one installed file, normalizing help markdown to LF line endings
// ----------------------------------------------------------------------------
{
    if (sim_install_is_help_index(relativePath))
        return true;

    if (sim_install_is_help_markdown(relativePath))
    {
        QFile in(from);
        if (!in.open(QIODevice::ReadOnly))
            return false;
        QByteArray data = in.readAll();
        in.close();
        data.replace("\r", "");
        return sim_install_write_file(to, data, perms);
    }

    if (!QFile::copy(from, to))
        return false;
    return QFile::setPermissions(to, QFileDevice::Permissions(perms));
}


void sim_install_regenerate_help_indices(const QString &destRoot)
// ----------------------------------------------------------------------------
//   Rebuild all help/*.idx files from installed help/*.md files
// ----------------------------------------------------------------------------
{
    QDir help(destRoot + "/help");
    if (!help.exists())
        return;

    const QStringList mdFiles = help.entryList({"*.md"}, QDir::Files);
    for (const QString &mdName : mdFiles)
    {
        QString mdPath  = help.filePath(mdName);
        QString idxName = mdName.left(mdName.size() - 3) + ".idx";
        QString idxPath = help.filePath(idxName);
        sim_install_regenerate_help_index(mdPath, idxPath);
    }
}


QString sim_install_data_dir()
// ----------------------------------------------------------------------------
//   Return the writable application data directory (external on Android)
// ----------------------------------------------------------------------------
{
#ifdef ANDROID
    // Prefer Context.getExternalFilesDir(null) (.../Android/data/<pkg>/files)
    // so files are reachable over USB on all Android versions. Unlike
    // GenericDataLocation (which returns /storage/emulated/0 on Android <= 10
    // and fails without runtime storage permissions), standardLocations for
    // AppDataLocation always includes getExternalFilesDir() at index 1.
    const QStringList dirs =
        QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    for (int i = 1; i < dirs.size(); ++i)
    {
        const QString &candidate = dirs.at(i);
        if (!candidate.isEmpty() && QDir().mkpath(candidate) &&
            QFileInfo(candidate).isWritable())
            return candidate;
    }
#endif // ANDROID
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}


bool sim_install_needed(const QString &destRoot,
                        const QString &wantedVersion,
                        const QString &helpFileName)
// ----------------------------------------------------------------------------
//   Return true if bundled resources must be (re-)extracted into destRoot
// ----------------------------------------------------------------------------
{
    if (!QFileInfo::exists(destRoot + "/" + helpFileName) ||
        !QFileInfo::exists(destRoot + "/config/library.csv"))
        return true;

    QFile stamp(destRoot + "/.assets-version");
    if (!stamp.open(QIODevice::ReadOnly))
        return true;

    QString found = QString::fromUtf8(stamp.readAll()).trimmed();
    stamp.close();
    if (found.isEmpty())
        return true;

    return !wantedVersion.isEmpty() && found != wantedVersion;
}


void sim_install_write_stamp(const QString &destRoot,
                             const QString &wantedVersion)
// ----------------------------------------------------------------------------
//   Record the extracted resource version stamp in destRoot
// ----------------------------------------------------------------------------
{
    QFile stamp(destRoot + "/.assets-version");
    if (stamp.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QString value = wantedVersion.isEmpty()
                      ? QStringLiteral("installed")
                      : wantedVersion;
        stamp.write(value.toUtf8());
        stamp.close();
    }
}
