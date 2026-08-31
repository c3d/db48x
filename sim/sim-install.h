#ifndef SIM_INSTALL_H
#define SIM_INSTALL_H
// ****************************************************************************
//  sim-install.h                                                 DB48X project
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

class QString;

bool sim_install_copy_file(const QString &from,
                           const QString &to,
                           const QString &relativePath,
                           unsigned     perms);
void sim_install_regenerate_help_indices(const QString &destRoot);

#endif // SIM_INSTALL_H
