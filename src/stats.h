#ifndef STATS_H
#define STATS_H
// ****************************************************************************
//  stats.h                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Statistics
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2023 Christophe de Dinechin <christophe@dinechin.org>
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

#include "algebraic.h"
#include "array.h"
#include "command.h"
#include "list.h"
#include "symbol.h"
#include "target.h"


struct StatsParameters : command
// ----------------------------------------------------------------------------
//   A replication of the ΣParameters / ΣPAR variable
// ----------------------------------------------------------------------------
{
    StatsParameters(id type = ID_StatsParameters) : command(type) {}

    struct Access
    {
        Access();
        ~Access();

        object::id      model;
        size_t          xcol;
        size_t          ycol;
        algebraic_g     intercept;
        algebraic_g     slope;

        static object_p name();

        bool            parse(list_p list);
        bool            parse(object_p n = name());

        bool            write(object_p n = name()) const;

        operator bool() const   { return intercept && slope; }
    };
};


struct StatsData : command
// ----------------------------------------------------------------------------
//   Helper to access the ΣData / ΣDAT variable
// ----------------------------------------------------------------------------
{
    StatsData(id type = ID_StatsData) : command(type) {}

    struct Access
    {
        Access();
        ~Access();

        array_g         data;
        array_g         original_data;
        size_t          columns;
        size_t          rows;

        static object_p name();

        bool            parse(array_p a);
        bool            parse(object_p n = name());

        bool            write(object_p n = name()) const;

        operator bool() const   { return data; }
    };
};


struct StatsAccess : StatsParameters::Access, StatsData::Access
// ----------------------------------------------------------------------------
//   Access to stats for processing operations
// ----------------------------------------------------------------------------
{
    StatsAccess() : StatsParameters::Access(), StatsData::Access() {}
    ~StatsAccess() {}

    typedef algebraic_p (*sum_fn)(algebraic_r s, algebraic_r x);
    typedef algebraic_p (*sxy_fn)(algebraic_r s, algebraic_r x, algebraic_r y);
    algebraic_p         total(sum_fn op) const;
    algebraic_p         total(sxy_fn op, algebraic_r arg) const;
    algebraic_p         sum(sum_fn op, uint xcol) const;
    algebraic_p         sum(sxy_fn op, uint xcol, uint ycol) const;
    algebraic_p         fit_transform(algebraic_r x, uint scol) const;

    algebraic_p         num_rows() const;
    algebraic_p         sum_x() const;
    algebraic_p         sum_y() const;
    algebraic_p         sum_xy() const;
    algebraic_p         sum_x2() const;
    algebraic_p         sum_y2() const;

    algebraic_p         total() const;
    algebraic_p         min() const;
    algebraic_p         max() const;
    algebraic_p         average() const;
    algebraic_p         median() const;
    algebraic_p         variance() const;
    algebraic_p         standard_deviation() const;
    algebraic_p         correlation() const;
    algebraic_p         covariance(bool pop) const;
    algebraic_p         covariance() const;
    algebraic_p         population_variance() const;
    algebraic_p         population_standard_deviation() const;
    algebraic_p         population_covariance() const;
    algebraic_p         regression_formula() const;
    algebraic_p         regression_formula_inverse() const;
    algebraic_p         predict(bool predx) const;
    algebraic_p         predict_x() const;
    algebraic_p         predict_y() const;

    algebraic_p         intercept_value() const         { return intercept; }
    algebraic_p         slope_value() const             { return slope; }
    bool                linear_regression();

    typedef algebraic_p (StatsAccess::*eval_fn)() const;
    static object::result evaluate(eval_fn op, bool two_columns);

    operator bool() const
    {
        if (data && intercept && slope)
            return true;
        rt.invalid_stats_data_error();
        return false;
    }

    bool two_columns() const
    {
        if (xcol == 0 || ycol == 0 || xcol > columns || ycol > columns)
        {
            rt.invalid_stats_parameters_error();
            return false;
        }
        return true;
    }
};


COMMAND_DECLARE(AddData,1);
COMMAND_DECLARE(RemoveData,0);
COMMAND_DECLARE(RecallData,0);
COMMAND_DECLARE(StoreData,1);
COMMAND_DECLARE(ClearData,0);
COMMAND_DECLARE(DataSize,0);
COMMAND_DECLARE(Average,0);
COMMAND_DECLARE(Median,0);
COMMAND_DECLARE(MinData,0);
COMMAND_DECLARE(MaxData,0);
COMMAND_DECLARE(SumOfX,0);
COMMAND_DECLARE(SumOfY,0);
COMMAND_DECLARE(SumOfXY,0);
COMMAND_DECLARE(SumOfXSquares,0);
COMMAND_DECLARE(SumOfYSquares,0);
COMMAND_DECLARE(Variance,0);
COMMAND_DECLARE(Correlation,0);
COMMAND_DECLARE(Covariance,0);
COMMAND_DECLARE(StandardDeviation,0);
COMMAND_DECLARE(PopulationVariance,0);
COMMAND_DECLARE(PopulationStandardDeviation,0);
COMMAND_DECLARE(PopulationCovariance,0);
COMMAND_DECLARE(FrequencyBins,3);
COMMAND_DECLARE(DataTotal,0);
COMMAND_DECLARE(IndependentColumn,1);
COMMAND_DECLARE(DependentColumn,1);
COMMAND_DECLARE(DataColumns,2);
COMMAND_DECLARE(Intercept,0);
COMMAND_DECLARE(Slope,0);
COMMAND_DECLARE(LinearRegression,0);
COMMAND_DECLARE(RegressionFormula,0);
COMMAND_DECLARE(PredictX,1);
COMMAND_DECLARE(PredictY,1);
COMMAND_DECLARE(BestFit,0);
COMMAND_DECLARE(LinearFit,0);
COMMAND_DECLARE(ExponentialFit,0);
COMMAND_DECLARE(PowerFit,0);
COMMAND_DECLARE(LogarithmicFit,0);

algebraic_p random_number();
algebraic_p random_number(algebraic_r min, algebraic_r max);

COMMAND_DECLARE(RandomNumber, 0);
COMMAND_DECLARE(RandomSeed, 1);
COMMAND_DECLARE(Random, 2);

#endif // STATS_H
