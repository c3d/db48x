#ifndef EQUATIONS_H
#define EQUATIONS_H
// ****************************************************************************
//  equations.h                                                   DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Representation of equations from the equations library
//    This is defined by the file `config/equations.csv'
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2024 Christophe de Dinechin <christophe@dinechin.org>
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

#include "complex.h"
#include "constants.h"
#include "expression.h"

#include <string.h>

// Global eq wildcards for rewrite patterns (defined in expression.cc)
namespace eq_wildcards {

extern const eq_symbol<'a'> a;
extern const eq_symbol<'b'> b;
extern const eq_symbol<'c'> c;
extern const eq_symbol<'d'> d;
extern const eq_symbol<'e'> e;
extern const eq_symbol<'f'> f;
extern const eq_symbol<'i'> i;
extern const eq_symbol<'j'> j;
extern const eq_symbol<'k'> k;
extern const eq_symbol<'l'> l;
extern const eq_symbol<'m'> m;
extern const eq_symbol<'n'> n;
extern const eq_symbol<'o'> o;
extern const eq_symbol<'p'> p;
extern const eq_symbol<'q'> q;
extern const eq_symbol<'r'> r;
extern const eq_symbol<'s'> s;
extern const eq_symbol<'t'> t;
extern const eq_symbol<'u'> u;
extern const eq_symbol<'v'> v;
extern const eq_symbol<'w'> w;
extern const eq_symbol<'x'> x;
extern const eq_symbol<'y'> y;
extern const eq_symbol<'z'> z;

extern const eq_symbol<'A'> A;
extern const eq_symbol<'B'> B;
extern const eq_symbol<'C'> C;
extern const eq_symbol<'D'> D;
extern const eq_symbol<'E'> E;
extern const eq_symbol<'F'> F;
extern const eq_symbol<'I'> I;
extern const eq_symbol<'J'> J;
extern const eq_symbol<'K'> K;
extern const eq_symbol<'L'> L;
extern const eq_symbol<'M'> M;
extern const eq_symbol<'N'> N;
extern const eq_symbol<'O'> O;
extern const eq_symbol<'P'> P;
extern const eq_symbol<'Q'> Q;
extern const eq_symbol<'R'> R;
extern const eq_symbol<'S'> S;
extern const eq_symbol<'T'> T;
extern const eq_symbol<'U'> U;
extern const eq_symbol<'V'> V;
extern const eq_symbol<'W'> W;
extern const eq_symbol<'X'> X;
extern const eq_symbol<'Y'> Y;
extern const eq_symbol<'Z'> Z;

extern const eq_integer<0> k0;
extern const eq_neg_integer<-1> kn1;
extern const eq_integer<1> k1;
extern const eq_integer<2> k2;
extern const eq_integer<3> k3;
extern const eq_integer<4> k4;
extern const eq_integer<5> k5;
extern const eq_integer<7> k7;
extern const eq_integer<10> k10;
extern const eq_always always;

extern const eq_pi pi;
extern const eq_e euler;

extern const eq_symbol<'#'> intk;
extern const eq_symbol<'+'> natk;
extern const eq_symbol<'-'> signk;
extern const eq_symbol<'@'> kpi;
extern const eq_symbol<'!'> ki;
extern const eq_symbol<'='> indep;

} // namespace eq_wildcards

GCP(equation);

struct equation : constant
// ----------------------------------------------------------------------------
//  An equation stored in `config/equations.csv` file
// ----------------------------------------------------------------------------
{
    equation(id type, uint index): constant(type, index) {}

    static equation_p make(uint index)
    {
        return rt.make<equation>(ID_equation, index);
    }

    static equation_p make(id type, uint index)
    {
        return rt.make<equation>(type, index);
    }

    static equation_p lookup(utf8 name, size_t len, bool error)
    {
        return equation_p(do_lookup(equations, name, len, error));
    }

    static equation_p lookup(cstring name, bool error = true)
    {
        return lookup(utf8(name), strlen(name), error);
    }

    uint        index() const
    {
        byte_p p = payload();
        return leb128<uint>(p);
    }

    utf8        name(size_t *size = nullptr) const
    {
        return do_name(equations, size);
    }
    object_p value() const
    {
        if (object_p obj = do_value(equations))
            if (id ty = obj->type())
                if (ty == ID_equation || ty == ID_expression ||
                    ty == ID_list || ty == ID_array)
                    return obj;
        return nullptr;
    }

    static const config equations;
    OBJECT_DECL(equation);
    PARSE_DECL(equation);
    EVAL_DECL(equation);
    RENDER_DECL(equation);
    GRAPH_DECL(equation);
    HELP_DECL(equation);
};


struct equation_menu : constant_menu
// ----------------------------------------------------------------------------
//   A equation menu is like a standard menu, but with equations
// ----------------------------------------------------------------------------
{
    equation_menu(id type) : constant_menu(type) { }
    static utf8 name(id type, size_t &len);
    MENU_DECL(equation_menu);
    HELP_DECL(equation_menu);
};


struct equation_menu_name : object
// ----------------------------------------------------------------------------
//    A menu entry that inserts a constant name
// ----------------------------------------------------------------------------
{
    equation_menu_name(id type) : object(type) {}
    OBJECT_DECL(equation_menu_name);
    EVAL_DECL(equation_menu_name);
    INSERT_DECL(equation_menu_name);
    HELP_DECL(equation_menu_name);
};


struct equation_menu_value : object
// ----------------------------------------------------------------------------
//    A menu entry that inserts a constant value
// ----------------------------------------------------------------------------
{
    equation_menu_value(id type) : object(type) {}
    OBJECT_DECL(equation_menu_value);
    EVAL_DECL(equation_menu_value);
    INSERT_DECL(equation_menu_value);
    HELP_DECL(equation_menu_value);
};


struct equation_menu_solver : object
// ----------------------------------------------------------------------------
//    A menu entry that builds a solver for the equation
// ----------------------------------------------------------------------------
{
    equation_menu_solver(id type) : object(type) {}
    OBJECT_DECL(equation_menu_solver);
    EVAL_DECL(equation_menu_solver);
    INSERT_DECL(equation_menu_solver);
    HELP_DECL(equation_menu_solver);
};


#define ID(i)
#define EQUATION_MENU(EquationMenu)     struct EquationMenu : equation_menu {};
#include "ids.tbl"

COMMAND_DECLARE(LibEq, 1);


GCP(assignment);

struct assignment : complex
// ----------------------------------------------------------------------------
//   An assignment is an operation in the form `A=B` that performs a Store
// ----------------------------------------------------------------------------
{
    assignment(id type, algebraic_r name, algebraic_r value):
        complex(type, name, value) {}

    static assignment_p make(algebraic_g name, algebraic_g value,
                             id ty = ID_assignment)
    {
        if (!name|| !value)
            return nullptr;
        while (assignment_p asn = value->as<assignment>())
            value = asn->value();
        return rt.make<assignment>(ty, name, value);
    }

    algebraic_p name()  const   { return x(); }
    algebraic_p value() const   { return y(); }

public:
    OBJECT_DECL(assignment);
    EVAL_DECL(assignment);
    PARSE_DECL(assignment);
    RENDER_DECL(assignment);
    GRAPH_DECL(assignment);
    HELP_DECL(assignment);
};

#endif // EQUATIONS_H
