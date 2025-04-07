// ****************************************************************************
//  equations.cc                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//      Implementation of the equations library
//
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

#include "equations.h"

#include "expression.h"
#include "grob.h"
#include "parser.h"
#include "renderer.h"
#include "solve.h"
#include "tag.h"
#include "variables.h"

#include <algorithm>

RECORDER(equations,         16, "Equation objects");
RECORDER(equations_error,   16, "Error on equation objects");


// ============================================================================
//
//   Equation definitions
//
// ============================================================================

static const cstring basic_equations[] =
// ----------------------------------------------------------------------------
//   List of basic equations
// ----------------------------------------------------------------------------
//   clang-format off
{

    // ------------------------------------------------------------------------
    "Columns and Beams", nullptr,
    // ------------------------------------------------------------------------
    //T#*: 21 vars 14 eqns 10 sims 10 secs
    "Elastic Buckling",  "{ "
    "  '(Pcr_kN)=(Ⓒπ²*(E_kPa)*(A_cm²))/sq((K*(L_m))/(r_cm))' "
    "  '(Pcr_kN)=(Ⓒπ²*(E_kPa)*(I_mm^4))/sq(K*(L_m))' "
    "  '(σcr_kPa)=(Pcr_kN)/(A_cm²)' "
    "  '(r_cm)²=(I_mm^4)/(A_cm²)' "
    "}",

    "Eccentric Columns", "{"
    "  '(σmax_kPa)=((P_kN)/(A_cm²))*(1+((ε_cm)*(c_cm))/sq(r_cm)*inv(cos(K/2*((L_m)/(r_cm))*sqrt((P_kN)/((E_kPa)*(A_cm²)))*1_r)))'"
    "  '(r_cm)²=(I_mm^4)/(A_cm²)'"
    "}",

    "Simple Deflection", "{"
    "  '(y_cm)=((P_kN)*((L_m)-(a_m)))*(x_m)/(6*(L_m)*(E_kPa)*(I_mm^4))*(x²+(L-a)²-L²)-((M_N*m)*x)/(E*I)*((c_m)-x²/(6*L)-L/3-c²/(2*L))-((w_N/m)*x)/(24*E*I)*(L³+x²*(x-2*L))'"
    "}",

    "Simple Slope", "{"
    "  '(θ_°)=(1_r)*(((P_N)*((L_m)-(a_m)))/(6*L*(E_kPa)*(I_mm^4))*(3*(x_m)²+(L-a)²-L²)-(M_N*m)/(E*I)*(c-(x²+c²)/(2*L)-L/3)-(w_N/m)/(24*E*I)*(L³+x²*(4*x-6*L)))'"
    "}",

    "Simple Moment", "{"
    "  '(Mx_N*m)=(P_N)*IFTE((x_m)≤(a_m);((L_m)-(a_m))*(x_m);(L-x)*a)/L+(M_N*m)*IFTE(x≤(c_m);x;x-L)/L+((w_N/m)*x*(L-x))/2'"
    "}",

    "Simple Shear", "{"
    "  '(V_N)=((P_N)*((L_m)-(a_m)))/L+(M_N*m)/L+((w_N/m)*(L-2*(x_m)))/2'"
    "}",

    "Cantilever Deflection", "{"
    "  '(y_m)=((P_N)*IFTE((x_m)≤(a_m);x;a)²)/(6*(E_kPa)*(I_mm^4))*IFTE(x≤a;x-3*a;a-3*x)+(M_N*m)*IFTE(x≤(c_m);x²;c*(2*x-c))/(2*E*I)-((w_N/m)*x²)/(24*E*I)*(6*L²-4*L*x+x²)'"
    "}",

    "Cantilever Slope", "{"
    "  '(θ_°)=(1_r)*((P_N)*IFTE((x_m)≤(a_m);x*(x-2*a);-a²)/(2*(E_kPa)*(I_mm^4))+(M_N*m)*IFTE(x≤(c_m);x;c)/(E*I)-((w_N/m)*x)/(6*E*I)*(3*L²-3*L*x+x²))'"
    "}",

    "Cantilever Moment", "{"
    "  '(Mx_N*m)=IFTE((x_m)≤(a_m);1;0)*(P_N)*((x_m)-(a_m))+IFTE(x≤(c_m);1;0)*(M_N*m)-(w_N/m)/2*(L²-2*L*x+x²)'"
    "}",


    "Cantilever Shear", "{"
    "  '(V_N)=IFTE((x_m)≤(a_m);1;0)*(P_N)+(w_N/m)*((L_m)-(x_m))'"
    "}",



    // ------------------------------------------------------------------------
    "Electricity", nullptr,
    // ------------------------------------------------------------------------
    //T#*: 78 vars 67 eqns 32 sims 30 secs

    "Coulomb’s Law & E Field",  "{ "
    "  '(F_N)=1/(4*Ⓒπ*Ⓒε₀*εr)*((q1_C)*(q2_C)/(r_m)²)' "
    "  '(Er_(N/C))=(F_N)/(qtest_C)' "
    "}",

    "E Field Infinite Line",  "{ "
    "  '(Er_(N/C))=1/(2*Ⓒπ*Ⓒε₀*εr)*((λ_(C/m))/(r_m))' "
    "  '(λ_(C/m))=(Q_C)/(L_m)' "
    "}",

    "E Field Finite Line",  "{ "
    "  '(Er_(N/C))=1/(4*Ⓒπ*Ⓒε₀*εr)*((λ_(C/m))/(r_m)*(SIN(θ₁_r)-SIN(θ₂_r)))' "
    "  '(λ_(C/m))=(Q_C)/(L_m)' "
    "}",

    "E Field Infinite Plate",  "{ "
    "  '(Ep_(N/C))=(σ_(μC/cm²))/(2*Ⓒε₀*εr)' "
    "  '(σ_(μC/cm²))=(Q_μC)/(A_(cm²))' "
    "}",

    "Ohm’s Law & Power",  "{ "
    "  '(V_V)=(I_A)*(R_Ω)' "
    "  '(P_W)=(V_V)*(I_A)' "
    "  '(P_W)=(I_A)²*(R_Ω)' "
    "  '(P_W)=(V_V)²/(R_Ω)' "
    "}",

    "Volt Divider",  "{ "
    "  '(V1_V)=(V_V)*((R1_Ω)/((R1_Ω)+(R2_Ω)))' "
    "}",

    "Current Divider",  "{ "
    "  '(I1_A)=(I_A)*((R2_Ω)/((R1_Ω)+(R2_Ω)))' "
    "}",

    "Wire Resistance",  "{ "
    "  '(R_Ω)=(ρ_(Ω*m))*(L_m)/(A_(m²))' "
    "}",

    "Resistivity & Conductivity",  "{ "
    "  '(ρ_(Ω*m))=(ρ0_(Ω*m))*(1+UBASE(αT_K^-1)*(UBASE(T_K)-UBASE(T0_K)))' "
    "  '(σ_(S/m))=1/(ρ_(Ω*m))' "
    "}",

    "Series & Parallel R",  "{ "
    "  '(Rs_Ω)=(R1_Ω)+(R2_Ω)'"
    "  '1/(Rp_Ω)=1/(R1_Ω)+1/(R2_Ω)' "
    "}",

    "Series & Parallel C",  "{ "
    "  '1/(Cs_μF)=1/(C1_μF)+1/(C2_μF)' "
    "  '(Cp_μF)=(C1_μF)+(C2_μF)' "
    "}",

    "Series & Parallel L",  "{ "
    "  '(Ls_mH)=(L1_mH)+(L2_mH)' "
    "  '1/(Lp_mH)=1/(L1_mH)+1/(L2_mH)' "
    "}",

    "Capacitive Energy",  "{ "
    "  '(E_J)=(1/2)*(C_μF)*(V_V)²' "
    "  '(E_J)=(1/2)*(q_μC)*(V_V)' "
    "  '(E_J)=(q_μC)²/(2*(C_μF))' "
    "}",

    "Volumic Density Electric Energy",  "{ "
    "  '(uE_(J/m^3))=(1/2)*Ⓒε₀*εr*(E_(V/m))²' "
    "}",

    "Inductive Energy",  "{ "
    "  '(E_J)=(1/2)*(L_mH)*(I_A)²' "
    "}",

    "RLC Current Delay",  "{ "
    "  'TAN(φs_°)=((XL_Ω)-(XC_Ω))/(R_Ω)' "
    "  'TAN(φp_°)=(1/(XC_Ω)-1/(XL_Ω))*(R_Ω)' "
    "  '(XC_Ω)=1/(((ω_(r/s))/(1_r))*(C_μF))' "
    "  '(XL_Ω)=(ω_(r/s))/(1_r)*(L_mH)' "
    "  '(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "}",

    "DC Capacitor Current",  "{ "
    "  '(I_A)=(C_μF)*((ΔV_V)/(Δt_s))' "
    "  '(ΔV_V)=(Vf_V)-(Vi_V)' "
    "  '(Δt_μs)=(tf_μs)-(ti_μs)' "
    "}",

    "Capacitor Charge",  "{ "
    "  '(q_C)=(C_μF)*(V_V)' "
    "}",

    "DC Inductor Voltage",  "{ "
    "  '(V_V)=-(L_mH)*((ΔIL_A)/(Δt_μs))' "
    "  '(ΔIL_A)=(ILf_A)-(ILi_A)' "
    "  '(Δt_μs)=(tf_μs)-(ti_μs)' "
    "}",

    "RC Transient",  "{ "
    "  '(V_V)=(Vf_V)-((Vf_V)-(Vi_V))*EXP((-(t_ms))/((R_Ω)*(C_F)))' "
    "}",
// Error in the placing of (R_Ω) that should be in the numerator
    "RL Transient",  "{ "
//    "  '(I_A)=1/(R_Ω)*((Vf_V)-((Vf_V)-(Vi_V))*EXP((-(t_μs))/((R_Ω)*(L_mH))))' "
    "  '(I_A)=1/(R_Ω)*((Vf_V)-((Vf_V)-(Vi_V))*EXP((-(t_μs)*(R_Ω))/(L_mH)))' "

    "}",
    // Modif of radian in eqn (4)
    "Resonant Frequency",  "{ "
    "  'Qs=1/(R_Ω)*√((L_mH)/(C_μF))' "
    "  'Qp=(R_Ω)*√((C_μF)/(L_mH))' "
    "  '(ω₀_(r/s))=2*(Ⓒπ_r)*(f0_Hz)' "
    "  '(ω₀_(r/s))=(1_r)/√((L_mH)*(C_μF))' "
    "}",

    "Plate Capacitor",  "{ "
    "  '(C_μF)=Ⓒε₀*εr*(A_(cm²))/(d_cm)' "
    "  '(ΔV_V)=(Ein_(V/m))*(d_cm)' "
    "  '(Ein_(N/C))=(σ_(μC/cm²))/(Ⓒε₀*εr)' "
    "  '(σ_(μC/m²))=(Q_μC)/(A_(cm²))' "
    "}",

    "Cylindrical Capacitor",  "{ "
    "  '(C_μF)=2*Ⓒπ*Ⓒε₀*εr*(L_cm)/(LN((ro_cm)/(ri_cm)))' "
    "  '(ΔV_V)=(Q_μC)*(LN((ro_cm)/(ri_cm)))/(2*Ⓒπ*Ⓒε₀*εr*(L_cm))' "
    "}",

    "Solenoid Inductance",  "{ "
    "  '(L_mH)=Ⓒμ₀*μr*(n_cm^-1)²*(A_(cm²))*(h_cm)' "
    "}",

    "Toroid Inductance",  "{ "
    "  '(L_mH)=Ⓒμ₀*μr*N²*(h_cm)/(2*Ⓒπ)*LN((ro_cm)/(ri_cm))' "
    "}",

    "Sinusoidal Voltage",  "{ "
    "  '(V_V)=(Vmax_V)*SIN((ω_(r/s))*(t_μs)+(φ_°))' "
    "  '(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "}",

    "Sinusoidal Current",  "{ "
    "  '(I_A)=(Imax_A)*SIN((ω_(r/s))*(t_s)+(φ_°))' "
    "  '(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "}",

    // Example of the following in
    // https://en.wikipedia.org/wiki/Drift_velocity#Numerical_example
    "Drift Speed & Current Density",  "{ "
    "  '(vd_(m/s))=(I_A)/((n_(m^-3))*Ⓒqe*(A_(cm²)))' "
    "  '(J_(A/m²))=(vd_(m/s))*(ρ_(C/m^3))' "
    "  '(J_(A/m²))=(σ_(S/m))*(E_(V/m))' "
    "}",

    // Example of the following in
    // https://en.wikipedia.org/wiki/Electron_mobility#Examples
    // In Ge typical values can be μe=500_(cm²/(V*s)) μh=200_(cm²/(V*s)) meeff_kg='0.12*Ⓒme'
    // mheff_kg='0.5*Ⓒme'  ne=1.04e19_(cm^-3)  nh=6.0e18_(m^-3) E=6.0e-9_V/m
    "Electron & Hole Mobilities",  "{ "
    "  '(J_(A/m²))=(Je_(A/m²))+(Jh_(A/m²))' "
    "  '(Je_(A/m²))=Ⓒqe*(ne_(m^-3))*(μe_(cm²/(V*s)))*(E_(V/m))' "
    "  '(Jh_(A/m²))=Ⓒqe*(nh_(m^-3))*(μh_(cm²/(V*s)))*(E_(V/m))' "
    "  '(μe_(cm²/(V*s)))=Ⓒqe*(τc_s)/(meeff_kg)' "
    "  '(μh_(cm²/(V*s)))=Ⓒqe*(τc_s)/(mheff_kg)' "
    "  '(σ_(S/m))=Ⓒqe*((μe_(cm²/(V*s)))*(ne_(m^-3))+(μh_(cm²/(V*s)))*(nh_(m^-3)))' "
    "}",

    // ------------------------------------------------------------------------
    "Fluids", nullptr,
    // ------------------------------------------------------------------------
    //T#*: 30 vars 30 eqns 4 sims 4 secs in Fluids
    "Pressure at Depth",  "{ "
    "  '(P_kPa)=(P0_atm)+(ρ_(kg/m^3))*Ⓒg*(h_m)' "
    "}",

    // WARNING Errors in HP50G_AUR.pdf for eqns 7 & 8
    // ("*" instead of"-") but not in HP50G
    // WARNING doesn't work on the native HP50G
    "Bernoulli Equation",  "{ "
    "  '(ΔP_Pa)/(ρ_(kg/m^3))+((v2_(m/s))²-(v1_(m/s))²)/2+Ⓒg*(Δy_m)' "
    "  '(ΔP_Pa)/(ρ_(kg/m^3))+((v2_(m/s))²)*(1-((A2_(m²))/(A1_(m²)))²)/2+Ⓒg*(Δy_m)' "
    "  '(ΔP_Pa)/(ρ_(kg/m^3))+((v1_(m/s))²)*(((A1_(m²))/(A2_(m²)))²-1)/2+Ⓒg*(Δy_m)' "
    "  '(ΔP_Pa)=(P2_Pa)-(P1_Pa)' "
    "  '(Δy_m)=(y2_m)-(y1_m)' "
    "  '(M_(kg/s))=(ρ_(kg/m^3))*(Q_(m^3/s))' "
    "  '(Q_(m^3/s))=(A2_(m²))*(v2_(m/s))' "
    "  '(Q_(m^3/s))=(A1_(m²))*(v1_(m/s))' "
    "  '(A1_(m²))=(Ⓒπ*(D1_m)²)/4' "
    "  '(A2_(m²))=(Ⓒπ*(D2_m)²)/4' "
    "}",

    // WARNING doesn't work on the native HP50G
    "Flow with Losses",  "{ "
    "  '(M_(kg/s))*((ΔP_Pa)/(ρ_(kg/m^3))+((v2_(m/s))²-(v1_(m/s))²)/2+Ⓒg*(Δy_m)+(hL_((m/s)²)))=(W_W)' "
    "  '(M_(kg/s))*((ΔP_Pa)/(ρ_(kg/m^3))+(((v2_(m/s))²)*(1-((A2_(m²))/(A1_(m²)))²))/2+Ⓒg*(Δy_m)+(hL_((m/s)²)))=(W_W)' "
    "  '(M_(kg/s))*((ΔP_Pa)/(ρ_(kg/m^3))+(((v1_(m/s))²)*(((A1_(m²))/(A2_(m²)))²-1))/2+Ⓒg*(Δy_m)+(hL_((m/s)²)))=(W_W)' "
    "  '(ΔP_Pa)=(P2_Pa)-(P1_Pa)' "
    "  '(Δy_m)=(y2_m)-(y1_m)' "
    "  '(M_(kg/s))=(ρ_(kg/m^3))*(Q_(m^3/s))' "
    "  '(Q_(m^3/s))=(A2_(m²))*(v2_(m/s))' "
    "  '(Q_(m^3/s))=(A1_(m²))*(v1_(m/s))' "
    "  '(A1_(m²))=(Ⓒπ*(D1_m)²)/4' "
    "  '(A2_(m²))=(Ⓒπ*(D2_m)²)/4' "
    "}",

    // WARNING: Missing or undefined parameter "f" in eqn 1
    // WARNING doesn't work on the native HP50G
   // f represents probably the Fanning friction factor which is near 0 with
   // the high RE number of the example the absolute roughness coefficient ϵ_in
   // is defined as a variable in both the HP50G and the manual but here it is
   // absent of all actual eqns
   //24-11-12 I replace the call for FANNING by the corresponding calculation in eqn (2). Error in last eqn
    "Flow In Full Pipes",  "{ "
    "  '(ρ_(kg/m^3))*((Ⓒπ*(D_m)²)/4)*(vavg_(m/s))*((ΔP_Pa)/(ρ_(kg/m^3))+Ⓒg*(Δy_m)+(vavg_(m/s))²*(2*f*((L_m)/(D_m))+ΣK/2))=(W_W)' "
  //"  'f=FANNING((ε_m)/(D_m);Reynolds)' "
    "  'f=IFTE(Reynolds ≤ 2100;16/Reynolds;(4.781-((-2*log(((ϵ_m)/(D_m))/3.7+12/Reynolds)-4.781)²/((-2*log(((ϵ_m)/(D_m))/3.7+2.51*(-2*log(((ϵ_m)/(D_m))/3.7+12/Reynolds))/Reynolds))-2*(-2*log(((ϵ_m)/(D_m))/3.7+12/Reynolds))+4.781)))^-2)' "
    "  '(ΔP_Pa)=(P2_Pa)-(P1_Pa)' "
    "  '(Δy_m)=(y2_m)-(y1_m)' "
    "  '(M_(kg/s))=(ρ_(kg/m^3))*(Q_(m^3/s))' "
    "  '(Q_(m^3/s))=(A_(m²))*(vavg_(m/s))' "
    "  '(A_(m²))=Ⓒπ*(D_m)²/4' "
    "  'Reynolds=(D_m)*(vavg_(m/s))*(ρ_(kg/m^3))/(μ_(kg/(m*s)))' "
    "  '(n_(m²/s))=(μ_(kg/(m*s)))/(ρ_(kg/m^3))' "
    "}",

    // ------------------------------------------------------------------------
    "Forces And Energy", nullptr,
    // ------------------------------------------------------------------------
    //T#*: 37 vars 34 eqs 8 sims 8 secs

    "Linear Mechanics",  "{ "
    "'(F_N)=(m_kg)*(a_(m/s²))' "
    "'(Ki_J)=1/2*(m_kg)*(vi_(m/s))²' "
    "'(Kf_J)=1/2*(m_kg)*(vf_(m/s))²' "
    "'(W_J)=(F_N)*(x_m)' "
    "'(W_J)=(Kf_J)-(Ki_J)' "
    "'(P_W)=(F_N)*(v_(m/s))' "
    "'(Pavg_W)=(W_J)/(t_s)' "
    "'(vf_(m/s))=(vi_(m/s))+(a_(m/s²))*(t_s)' "
    "}",
// Problems of radians units in eqns (2), (3) & (4) and remove radians from eqns (10), (11) & (12)
// And in eqns (10), (11) & (12 change units of N from HZ to rpm)
    "Angular Mechanics",  "{ "
    "'(τ_(N*m))=(I_(kg*m²))*(α_(r/s²))/(1_r)' "
    "'(Ki_J)=1/2*(I_(kg*m²))*((ωi_(r/s))²)/(1_r)²' "
    "'(Kf_J)=1/2*(I_(kg*m²))*((ωf_(r/s))²)/(1_r)²' "
    "'(W_J)=(τ_(N*m))*(θ_r)/(1_r)' "
    "'(W_J)=(Kf_J)-(KI_J)' "
    "'(P_W)=(τ_(N*m))*(ω_(r/s))/(1_r)' "
    "'(Pavg_W)=(W_J)/(t_s)' "
    "'(ωf_(r/s))=(ωi_(r/s))+(α_(r/s²))*(t_s)' "
    "'(at_(m/s²))=(α_(r/s²))/(1_r)*(r_m)' "
    "'(ω_(r/s))=2*(Ⓒπ)*(N_rpm)' "
    "'(ωi_(r/s))=2*(Ⓒπ)*(Ni_rpm)' "
    "'(ωf_(r/s))=2*(Ⓒπ)*(Nf_rpm)' "
    "}",

    "Centripetal Force",  "{ "
    "'(F_N)=(m_kg)*((ω_(r/s))/(1_r))²*(r_m)' "
    "'(ω_(r/s))=(v_(m/s))*(1_r)/(r_m)' "
    "'(ar_(m/s²))=(v_(m/s))²/(r_m)' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(N_Hz)' "
    "}",

    "Hooke’s Law",  "{ "
    "'(F_N)=-(k_(N/m))*(x_m)' "
    "'(W_J)=-1/2*(k_(N/m))*(x_m)²' "
    "}",

    "1D Elastic Collisions",  "{ "
    "'(v1f_(m/s))=(((m1_kg)-(m2_kg))/((m1_kg)+(m2_kg)))*(v1i_(m/s))' "
    "'(v2f_(m/s))=((2*(m1_kg))/((m1_kg)+(m2_kg)))*(v1i_(m/s))' "
    "}",
    // The following sub-section was missing from v8.2
    "Drag Force",  "{ "
    "'(F_N)=Cd*((ρ_kg/m^3)*(v_m/s)²/2)*(A_cm²)' "
    "}",
    // Formulas 3 & 4 corrected: (m1_kg)*(m2_kg) in the numerator instead of (m_kg) alone in the gravitational potential
    "Gravitation Law",  "{ "
    "'(F_N)=ⒸG*((m1_kg)*(m2_kg))/(r_m)²' "
    "'(W_J)=(UGf_J)-(UGi_J)' "
    "'(UGf_J)=-ⒸG*((m1_kg)*(m2_kg))/(rf_m)' "
    "'(UGi_J)=-ⒸG*((m1_kg)*(m2_kg))/(ri_m)' "
    "}",

    "Relativity Mass Energy",  "{ "
    "'(E_J)=(m_kg)*Ⓒc²' "
    "}",


    // ------------------------------------------------------------------------
    "Gases", nullptr,
    // ------------------------------------------------------------------------
    //t#*: 38 vars 21 eqns 8 sims 8 secs
    // Change all occurrences of gmol by mol
    // In eq (1) units of T should be K instead of °C
    "Ideal Gas",  "{ "
    "'(P_atm)*(V_l)=(n_mol)*ⒸR*UBASE(T_K)' "
    "'(m_kg)=(n_mol)*(MW_(g/mol))' "
    "}",

    "Ideal Gas Law Change",  "{ "
    "'((Pf_Pa)*(Vf_l))/UBASE(Tf_K)=((Pi_Pa)*(Vi_l))/UBASE(Ti_K)' "
    "}",
    // Change all occurrences of gmol by mol
    "Isothermal Expansion",  "{ "
    "'(W_J)=(n_mol)*ⒸR*(T_°C)*LN((Vf_l)/(Vi_l))' "
    "'(m_kg)=(n_mol)*(MW_(g/mol))' "
    "}",

    "Polytropic Processes",  "{ "
    "'(Pf_atm)/(Pi_atm)=((Vf_(ft^3))/(Vi_(ft^3)))^(-n)' "
    "'(Tf_°C)/(Ti_°C)=((Pf_atm)/(Pi_atm))^((n-1)/n)' "
    "}",

    "Isentropic Flow",  "{ "
    "'UBASE(T_K)/UBASE(T0_K)=2/(2+(k-1)*M²)' "
    "'(P_kPa)/(P0_kPa)=((T_°C)/(T0_°C))^(k/(k-1))' "
    "'(ρ_(kg/m^3))/(ρ0_(kg/m^3))=(UBASE(T_K)/UBASE(T0_K))^(1/(k-1))' "
    "'(A_(cm²))/(At_(cm²))=(1/M)*(2/(k+1)*(1+(k-1)/2*M²))^((k+1)/(2*(k-1)))' "
    "}",
    // Change all occurrences of gmol by mol : Change °C for K in eqn 1
    "Real Gas Law",  "{ "
    "'(P_atm)*(V_l)=(n_mol)*Z*ⒸR*UBASE(T_K)' "
    "'(m_kg)=(n_mol)*(MW_(g/mol))' "
    //"'Tr=UBASE(T_K)/UBASE(Tc_K)' " These 3 eqns will be substitute in a closed form for Z
    //"'Pr=(P_Pa)/(Pc_Pa)' "
    //"'ρr=0.27*(Pr/(Z*Tr))' "
    //"'Z=1+(0.31506237-1.04670990/Tr-0.57832729/Tr^3)*ρr+(0.53530771-0.61232032/Tr)*ρr²+0.61232032*0.10488813*ρr^5/Tr+0.68157001*ρr²/Tr^3*(1+0.68446549*ρr²)*exp(-0.68446549*ρr²)' "
    // change Ti for T and Pi for P in the following
      "'Z=1+(0.31506237-1.04670990/(UBASE(T_K)/UBASE(Tc_K))-0.57832729/(UBASE(T_K)/UBASE(Tc_K))^3)*(0.27*(((P_Pa)/(Pc_Pa))/(Z*(UBASE(T_K)/UBASE(Tc_K)))))+(0.53530771-0.61232032/(UBASE(T_K)/UBASE(Tc_K)))*(0.27*(((P_Pa)/(Pc_Pa))/(Z*(UBASE(T_K)/UBASE(Tc_K)))))²+0.61232032*0.10488813*(0.27*(((P_Pa)/(Pc_Pa))/(Z*(UBASE(T_K)/UBASE(Tc_K)))))^5/(UBASE(T_K)/UBASE(Tc_K))+0.68157001*(0.27*(((P_Pa)/(Pc_Pa))/(Z*(UBASE(T_K)/UBASE(Tc_K)))))²/(UBASE(T_K)/UBASE(Tc_K))^3*(1+0.68446549*(0.27*(((P_Pa)/(Pc_Pa))/(Z*(UBASE(T_K)/UBASE(Tc_K)))))²)*exp(-0.68446549*(0.27*(((P_Pa)/(Pc_Pa))/(Z*(UBASE(T_K)/UBASE(Tc_K)))))²)' "
    "}",
    // Change all °C for K in eqn 1
    "Real Gas State Change",  "{ "
    "'((Pf_atm)*(Vf_l))/(Zf*UBASE(Tf_K))=((Pi_atm)*(Vi_l))/(Zi*UBASE(Ti_K))' "
    //"'Tri=UBASE(Ti_K)/UBASE(Tc_K)' " These 3 eqns will be substitute in a closed form for Zi
    //"'Pri=(Pi_Pa)/(Pc_Pa)' "
    //"'ρri=0.27*(Pri/(Zi*Tri))' "
    //"'Zi=1+(0.31506237-1.04670990/Tri-0.57832729/Tri^3)*ρri+(0.53530771-0.61232032/Tri)*ρri²+0.61232032*0.10488813*ρri^5/Tri+0.68157001*ρri²/Tri^3*(1+0.68446549*ρri²)*exp(-0.68446549*ρri²)' "
    "'Zi=1+(0.31506237-1.04670990/(UBASE(Ti_K)/UBASE(Tc_K))-0.57832729/(UBASE(Ti_K)/UBASE(Tc_K))^3)*(0.27*(((Pi_Pa)/(Pc_Pa))/(Zi*(UBASE(Ti_K)/UBASE(Tc_K)))))+(0.53530771-0.61232032/(UBASE(Ti_K)/UBASE(Tc_K)))*(0.27*(((Pi_Pa)/(Pc_Pa))/(Zi*(UBASE(Ti_K)/UBASE(Tc_K)))))²+0.61232032*0.10488813*(0.27*(((Pi_Pa)/(Pc_Pa))/(Zi*(UBASE(Ti_K)/UBASE(Tc_K)))))^5/(UBASE(Ti_K)/UBASE(Tc_K))+0.68157001*(0.27*(((Pi_Pa)/(Pc_Pa))/(Zi*(UBASE(Ti_K)/UBASE(Tc_K)))))²/(UBASE(Ti_K)/UBASE(Tc_K))^3*(1+0.68446549*(0.27*(((Pi_Pa)/(Pc_Pa))/(Zi*(UBASE(Ti_K)/UBASE(Tc_K)))))²)*exp(-0.68446549*(0.27*(((Pi_Pa)/(Pc_Pa))/(Zi*(UBASE(Ti_K)/UBASE(Tc_K)))))²)' "
    //"'Trf=UBASE(Tf_K)/UBASE(Tc_K)' " These 3 eqns will be substitute in a closed form for Zf
    //"'Prf=(Pf_Pa)/(Pc_Pa)' "
    //"'ρrf= 0.27*(Prf/(Zf*Trf))' "
    //"'Zf=1+(0.31506237-1.04670990/Trf-0.57832729/Trf^3)*ρrf+(0.53530771-0.61232032/Trf)*ρrf²+0.61232032*0.10488813*ρrf^5/Trf+0.68157001*ρrf²/Trf^3*(1+0.68446549*ρrf²)*exp(-0.68446549*ρrf²)' "
    "'Zf=1+(0.31506237-1.04670990/(UBASE(Tf_K)/UBASE(Tc_K))-0.57832729/(UBASE(Tf_K)/UBASE(Tc_K))^3)*(0.27*(((Pf_Pa)/(Pc_Pa))/(Zf*(UBASE(Tf_K)/UBASE(Tc_K)))))+(0.53530771-0.61232032/(UBASE(Tf_K)/UBASE(Tc_K)))*(0.27*(((Pf_Pa)/(Pc_Pa))/(Zf*(UBASE(Tf_K)/UBASE(Tc_K)))))²+0.61232032*0.10488813*(0.27*(((Pf_Pa)/(Pc_Pa))/(Zf*(UBASE(Tf_K)/UBASE(Tc_K)))))^5/(UBASE(Tf_K)/UBASE(Tc_K))+0.68157001*(0.27*(((Pf_Pa)/(Pc_Pa))/(Zf*(UBASE(Tf_K)/UBASE(Tc_K)))))²/(UBASE(Tf_K)/UBASE(Tc_K))^3*(1+0.68446549*(0.27*(((Pf_Pa)/(Pc_Pa))/(Zf*(UBASE(Tf_K)/UBASE(Tc_K)))))²)*exp(-0.68446549*(0.27*(((Pf_Pa)/(Pc_Pa))/(Zf*(UBASE(Tf_K)/UBASE(Tc_K)))))²)' "
    "}",
    // Change all occurrences of gmol by mol
    "Kinetic Theory",  "{ "
    "'(P_kPa)=((n_mol)*(MW_(g/mol))*(vrms_(m/s))²)/(3*(V_l))' "
    "'(vrms_(m/s))=√((3*ⒸR*(T_°C))/(MW_(g/mol)))' "
    "'(λ_nm)=1/(√(2)*Ⓒπ*((n_mol)*ⒸNA)/(V_l)*(d_nm)²)' "
    "'(m_kg)=(n_mol)*(MW_(g/mol))' "
    "}",

    // ------------------------------------------------------------------------
    "Heat Transfer", nullptr,
    // ------------------------------------------------------------------------
    //T#*: 31 vars 17 eqns 6 sims 6 secs
    "Heat Capacity",  "{ "
    "'(Q_kJ)=(m_kg)*(c_(kJ/(kg*K)))*(UBASE(Tf_°C)-UBASE(Ti_°C))' "
    "'(ΔT_°C)=(Tf_°C)-(Ti_°C)' "
    "}",

    "Thermal Expansion",  "{ "
    "'(δ_cm)=(α_(1/K))*(L_m)*(UBASE(Tf_°C)-UBASE(Ti_°C))' "
    "'(ΔT_°C)=(Tf_°C)-(Ti_°C)' "
    "}",

    "Conduction",  "{ "
    "'(qr_W)=((k_(W/(m*K)))*(A_(m²))/(L_m))*(UBASE(Th_°C)-UBASE(Tc_°C))' "
    "'(ΔT_°C)=(Th_°C)-(Tc_°C)' "
    "}",

    "Convection",  "{ "
    "'(qr_W)=(h_(W/(m²*K)))*(A_(m²))*(UBASE(Th_°C)-UBASE(Tc_°C))' "
    "'(ΔT_°C)=(Th_°C)-(Tc_°C)' "
    "}",

    "Conduction & Convection",  "{ "
    "'(qr_W)=(A_(m²))*(UBASE(Th_°C)-UBASE(Tc_°C))/(1/(h1_(W/(m²*K)))+(L1_cm)/(k1_(W/(m*K)))+(L2_cm)/(k2_(W/(m*K)))+(L3_cm)/(k3_(W/(m*K)))+1/(h3_(W/(m²*K))))' "
    "'(qr_W)=(A_(m²))*((Th_°C)-(Tc_°C))/(1/(h1_(W/(m²*K)))+(L1_cm)/(k1_(W/(m*K)))+(L2_cm)/(k2_(W/(m*K)))+(L3_cm)/(k3_(W/(m*K)))+1/(h3_(W/(m²*K))))' "
    "'(U_(W/(m²*K)))=(qr_W)/((A_(m²))*(UBASE(Th_°C)-UBASE(Tc_°C)))' "
    "'(ΔT_°C)=(Th_°C)-(Tc_°C)' "
    "}",
//
    // WARNING The db48x needs the Black-Body Integral function F0λ(T_K,λ_nm)
    // defined in the HP50G (SP50G_AUR 3-82, 5-31)
    // In eqn 1, change °C for K
    // In eqn 2, change the calls for F0λ to explicit integral where the integration limits are from x1 to x2 with xi,j=Ⓒh*Ⓒc/((λi,j_nm)*Ⓒk*UBASE(T_K))
    "Black Body Radiation",  "{ "
    "'(eb_(W/m²))=Ⓒσ*UBASE(T_K)^4' "
    //"'f=F0λ((λ2_nm);(T_°C))-F0λ((λ1_nm);(T_°C))' "
    "'f=15/Ⓒπ^4*∫(UBASE(Ⓒh*Ⓒc/((λ1_nm)*Ⓒk*UBASE(T_K)));UBASE(Ⓒh*Ⓒc/((λ2_nm)*Ⓒk*(T_K)));X^3/expm1(X);X)' "
    "'(eb12_(W/m²))=f*(eb_(W/m²))' "
    "'(λmax_nm)*UBASE(Tmax_°C)=Ⓒc3' "
    "'(q_W)=(eb_(W/m²))*(A_(cm²))' "
    "}",

    // ------------------------------------------------------------------------
    "Magnetism", nullptr,
    // ------------------------------------------------------------------------
    //T#*: 28 vars 14 eqns 13 sims 10 secs
    // WARNING both HP50G & HP50G_AUR.pdf used a variables rw absent from all
    // equations
    "Straight Wire Infinite",  "{ "
    "'(B_T)=Ⓒμ₀*IFTE((r_m)≤(rw_m);μr*(r_m)*(I_A)/(2*Ⓒπ*(rw_cm)²);(I_A)/(2*Ⓒπ*(r_cm)))' "
    "}",
    // Correction factor 1/4 replace factor 1/2
    "Straight Wire Finite",  "{ "
    "'(B_T)=Ⓒμ₀*IFTE((r_m)≤(rw_m);μr*(r_m)*(I_A)/(4*Ⓒπ*(rw_cm)²);(I_A)/(4*Ⓒπ*(r_cm)))*(COS(θ₁_r)-COS(θ₂_r))' "
    "}",

    "Force Between Wires",  "{ "
    "'(Fba_N)=(Ⓒμ₀*μr*(Ib_A)*(Ia_A)*(L_cm))/(2*Ⓒπ*(d_m))' "
    "}",

    "B Field In Infinite Solenoid",  "{ "
    "'(B_T)=Ⓒμ₀*μr*(I_A)*nl' "
    "}",

    "B Field In Finite Solenoid",  "{ "
    "'(B_T)=(1/2)*Ⓒμ₀*μr*(I_A)*nl*(COS(α2_°)-COS(α1_°))' "
    "}",

    "B Field In Toroid",  "{ "
    "'(B_T)=(Ⓒμ₀*μr*(I_A)*N)/(2*Ⓒπ)*(2/((ro_cm)+(ri_cm)))' "
    "}",

    "Hall Effect",  "{ "
    // Correction (24-10-05): (L_m) needs to be in the denominator and not in the numerator
    "'(VH_V)=((I_A)*(B_T))/((n_(1/m^3))*ABS(q_C)*(L_m))' "
    "}",

    "Cyclotron Motion",  "{ "
    "'(Rc_m)=((m_kg)*(v_(m/s)))/(ABS(q_C)*(B_T))' "
    "'(fc_Hz)=(ABS(q_C)*(B_T))/(2*Ⓒπ*(m_kg))' "
    "'(T_s)=1/(fc_Hz)' "
    "}",

    "Helicoidal Motion",  "{ "
    "'(Rc_m)=((m_kg)*(v_(m/s))*SIN(θ_°))/(ABS(q_C)*(B_T))' "
    "'(T_s)=(2*Ⓒπ*(Rc_m))/((v_(m/s))*ABS(SIN(θ_°)))' "
    "'(Dpitch_m)=(v_(m/s))*COS(θ_°)*(T_s)' "
    "}",

    "Volumic Density Magnetic Energy",  "{ "
    "'(uB_(J/m^3))=(1/(2*Ⓒμ₀*μr))*(B_T)²' "
    "}",

    // ------------------------------------------------------------------------
    "Motion", nullptr,
    // ------------------------------------------------------------------------
    //T#: 38 vars 34 eqns 10 sims 7 secs
    "Linear Motion",  "{ "
    "'(x_m)=(x0_m)+(v0_(m/s))*(t_s)+1/2*(a_(m/s²))*(t_s)²' "
    "'(x_m)=(x0_m)+(v_(m/s))*(t_s)-1/2*(a_(m/s²))*(t_s)²' "
    "'(x_m)=(x0_m)+1/2*((v0_(m/s))+v_(m/s))*(t_s)' "
    "'(v_(m/s))=(v0_(m/s))+(a_(m/s²))*(t_s)' "
    "}",

//Ref.: https://fr.wikipedia.org/wiki/Pesanteur#cite_ref-10:~:text=sur%20les%20plantes.-,Variation%20en%20fonction%20du%20lieu,-%5Bmodifier%20%7C
// Reference: Commissions romandes de mathématique, de physique et de chimie, Formulaires et tables : Mathématiques, Physique, Chimie, Tricorne, 2000, 278
    // Replacing φ by φ_° in eqn (6) AND rearranging units in eqn (6)
    "Object In Free Fall",  "{ "
    "'(y_m)=(y0_m)+(v0_(m/s))*(t_s)-1/2*(gloc_(m/s²))*(t_s)²' "
    "'(y_m)=(y0_m)+(v_(m/s))*(t_s)+1/2*(gloc_(m/s²))*(t_s)²' "
    "'(v_(m/s))=(v0_(m/s))-(gloc_(m/s²))*(t_s)' "
    "'(v_(m/s))²=(v0_(m/s))²-2*(gloc_(m/s²))*((y_m)-(y0_m))' "
    "'gloc_(m/s²)=ⒸG*(Mp_kg)/(r_m)²' "
    //"'gearth_(m/s²)=9.780327*(1+5.3024E-3*(SIN(φ_°))²-5.8E-6*(SIN(φ_°))²-3.086E-7*(h_m))' "
    "'gearth_(m/s²)=(9.780327_(m/s²))*(1+5.3024E-3*(SIN(φ_°))²-5.8E-6*(SIN(φ_°))²-3.086E-7*(h_m)/(1_m))' "
    "}",

    "Projectile Motion",  "{ "
    "'(x_m)=(x0_m)+(v0_(m/s))*COS(θ0_°)*(t_s)' "
    "'(y_m)=(y0_m)+(v0_(m/s))*SIN(θ0_°)*(t_s)-1/2*Ⓒg*(t_s)²' "
    "'(vcx_(m/s))=(v0_(m/s))*COS(θ0_°)' "
    "'(vcy_(m/s))=(v0_(m/s))*SIN(θ0_°)-Ⓒg*(t_s)' "
    "'(R_m)=((v0_(m/s))²)/Ⓒg*SIN(2*(θ0_°))' "
    "'(hmax_m)=((v0_(m/s))²)/(2*Ⓒg)*(SIN(θ0_r))²' "
    "'(tf_s)=(2*(v0_(m/s)))/Ⓒg*SIN(θ0_r)' "
    "}",

    "Angular Motion",  "{ "
// Due to "Inconsistent units" I had to rewrite all angular units of ω & α
//    "'(θ_°)=(θ0_°)+(ω₀_rpm)*(t_s)+1/2*(α_(rpm²))*(t_s)²' "
//    "'(θ_°)=(θ0_°)+(ω_rpm)*(t_s)-1/2*(α_(rpm²))*(t_s)²' "
//    "'(θ_°)=(θ0_°)+1/2*((ω₀_rpm)+(ω_rpm))*(t_s)' "
//    "'(ω_rpm)=(ω₀_rpm)+(α_(rpm²))/(1_turn)*(t_s)' "
    "'(θ_°)=(θ0_°)+(ω₀_r/min)*(t_s)+1/2*(α_r/min²)*(t_s)²' "
    "'(θ_°)=(θ0_°)+(ω_r/min)*(t_s)-1/2*(α_r/min²)*(t_s)²' "
    "'(θ_°)=(θ0_°)+1/2*((ω₀_r/min)+(ω_r/min))*(t_s)' "
    "'(ω_r/min)=(ω₀_r/min)+(α_r/min²)*(t_s)' "

    "}",
    // I rewrote the angular units in eqns (1) & (3)
    "Uniform Circular Motion",  "{ "
    //"'(ω_rpm)=(v_(m/s))*(1_r)/(rc_cm)' "
    "'(ω_r/min)=(v_(m/s))*(1_r)/(rc_cm)' "
    "'(ar_(m/s²))=(v_(m/s))²/(rc_cm)' "
    //"'(ω_rpm)=2*(Ⓒπ_r)*(N_rpm)' "
    "'(ω_r/min)=2*(Ⓒπ_r/turn)*(N_rpm)' "
    "}",
// 2 new eqns added (3) & (4),
    "Terminal Velocity",  "{ "
    "'(vt_(m/s))=√((2*(m_kg)*Ⓒg)/(Cd*(ρ_(kg/m^3))*(Ah_cm²)))' "
    "'v_(m/s)=(vt_(m/s))*TANH((t_s)*Ⓒg/ABS(vt_(m/s)))' "
    "'tfr_s=ATANH(fr)/(Ⓒg/ABS(vt_(m/s)))' "
    "'xfr_ft=(vt_(m/s))*∫(0_s;tfr_s;TANH(t*Ⓒg/ABS(vt_(m/s)));t)' "
    "}",

// New section added with new eqns (1), (3) & (4),
//W=Fb+D <=> Vol*(ρ-ρf)*g=1/2*Cd*Ah*ρf*vt²  => vt=IFTE('ρ<ρf';-1;1)*√(2*Vol/Ah*ABS(ρ/ρf-1)*g/Cd)
    "Buoyancy & Terminal Velocity",  "{ "
    "'(vt_(m/s))=IFTE((ρ_(kg/m^3))<(ρf_(kg/m^3));-1;1)*√(2*(Vol_m^3)/(Ah_m²)*ABS((ρ_(kg/m^3))/(ρf_(kg/m^3))-1)*Ⓒg/Cd)' "
    "'v_(m/s)=(vt_(m/s))*TANH((t_s)*Ⓒg/ABS(vt_(m/s)))' "
    "'tfr_s=ATANH(fr)/(Ⓒg/ABS(vt_(m/s)))' "
    "'xfr_m=(vt_(m/s))*∫(0_s;tfr_s;TANH(t*Ⓒg/ABS(vt_(m/s)));t)' "
    "}",

    "Escape & Orbital Velocity",  "{ "
    "'(ve_(m/s))=√((2*ⒸG*(Mp_kg))/(R_m))' "
    "'(vo_(m/s))=√((ⒸG*(Mp_kg))/(R_m))' "
    "}",


    // ------------------------------------------------------------------------
    "Optics", nullptr,
    // ------------------------------------------------------------------------
    //T#*: 40 vars 38 eqns 11 sims 11 secs

    "Refraction Law",  "{ "
    "'n1*SIN(θ₁_°)=n2*SIN(θ₂_°)' "
    "'n1=Ⓒc/(v1_(m/s))' "
    "'n2=Ⓒc/(v2_(m/s))' "
    "}",

    "Critical Angle",  "{ "
    "'SIN(θc_°)=n1/n2' "
    "'n1=Ⓒc/(v1_(m/s))' "
    "'n2=Ⓒc/(v2_(m/s))' "
    "}",

//Ref.: https://mppolytechnic.ac.in/mp-staff/notes_upload_photo/AS273fiberoptics.pdf
    "Fiber Optic",  "{ "
//    "'SIN(θc_°)=n2/n1' "
	"'nf0=Ⓒc/(vf0_(m/s))' "
	"'nf1=Ⓒc/(vf1_(m/s))' "
	"'nf2=Ⓒc/(vf2_(m/s))' "
	"'SIN(θ0_°)=(√(nf1²-nf2²))/nf0' "
	"'NA=(√(nf1²-nf2²))/nf0'"
    "}",

    "Brewster’s Law",  "{ "
       "'TAN(θB_°)=n2/n1' "
       "'(θB_°)+(θ₂_°)=90_°' "
       "'n1=Ⓒc/(v1_(m/s))' "
       "'n2=Ⓒc/(v2_(m/s))' "
    "}",
// Third equn missing for magnification
    "Spherical Reflection",  "{ "
    "'1/(u_cm)+1/(v_cm)=1/(f_cm)' "
    "'(f_cm)=(r_cm)/2' "
    "'m=-(v_cm)/(u_cm)' "
    "}",

    "Spherical Refraction",  "{ "
    "'n1/(u_cm)+n2/(v_cm)=(n2-n1)/(r_cm)' "
    "'n1=Ⓒc/(v1_(m/s))' "
    "'n2=Ⓒc/(v2_(m/s))' "
    "}",

    "Thin Lens",  "{ "
    "'1/(u_cm)+1/(v_cm)=1/(f_cm)' "
    "'1/(f_cm)=(n-1)*(1/(r1_cm)-1/(r2_cm))' "
    "'m=-(v_cm)/(u_cm)' "
    "'n=Ⓒc/(vn_(m/s))' "
    "}",

    "Rayleigh’s Criterion",  "{ "
    "'SIN(θr_°)=UBASE(1.21966989*(λ_nm)/(d_m))' "
    "'TAN(θr_°)=UBASE((y_m)/(L_m))' "
    "}",
// Error in second eqn last "+" to be replaced by "*"
    "Malus Law",  "{ "
    "'(I_(W/m²))/(I₀_(W/m²))=(COS(θ_°))²' "
    //"'(Ix_(W/m²))/I₀x_(W/m²)=((fx_Hz)/(fx₀_Hz))*(1+Ⓒλc/Ⓒc*((fx₀_Hz)-(fx_Hz)))+(COS(θ_°))²' "
    "'(Ix_(W/m²))/I₀x_(W/m²)=((fx_Hz)/(fx₀_Hz))*(1+Ⓒλc/Ⓒc*((fx₀_Hz)-(fx_Hz)))*(COS(θ_°))²' "
    "'(I₀_(W/m²))=(1/(2*Ⓒμ₀*Ⓒc))*(E₀_(V/m))²' "
    "}",

    "2 Slits Young Interference",  "{ "
    "'(I_(W/m²))/(Imax_(W/m²))=4*(COS(Δφ_°)/2)²' "
    "'(Δφ_r)=(2*(Ⓒπ_r)*(d_μm)*SIN(θ_°))/(λ_nm)' "
    "'TAN(θ_°)=(y_m)/(L_m)' "
    "'(Δyint_m)=(λ_nm)*(L_m)/(d_μm)' "
    "}",
// Impose explicit radians units to Δα_° => Δα_r & Remove radians with *(1_r²)
    "One Slit Diffraction",  "{ "
    //"'(I_(W/m²))/(Imax_(W/m²))=(IFTE(Δα_°;SIN(Δα/2)/(Δα/2);1))²' "
    "'(I_(W/m²))/(Imax_(W/m²))=(1_r²)*(IFTE(Δα_r;SIN((Δα_r)/2)/((Δα_r)/2);1))²' "
    "'(Δα_r)=(2*(Ⓒπ_r)*(a_μm)*SIN(θ_°))/(λ_nm)' "
    "'TAN(θ_°)=(y_m)/(L_m)' "
    "'(Δydiff_m)=2*(λ_nm)*(L_m)/(a_μm)' "
    "}",

    // ------------------------------------------------------------------------
    "Oscillations", nullptr,
    // ------------------------------------------------------------------------
    //T#*: 25 vars 37 equs 7 sims 7 secs
    "Mass‐Spring System",  "{ "
    "'(ω_(r/s))=(1_r)*√((k_(N/m))/(m_kg))' "
    "'(T_s)=2*(Ⓒπ_r)/(ω_(r/s))' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "}",
// Add radians units explicitely to θmax => (θmax_°)
    "Simple Pendulum",  "{ "
    "'(ω_(r/s))=(1_r)*√(Ⓒg/(L_cm))' "
    "'(Treal_s)=2*Ⓒπ*√((L_cm)/Ⓒg)*(Σ(x;0;5;((2·x)!÷((2↑x)·x!)²)²·sin((θmax_°)÷2)↑(2·x)))' "
    "'(T_s)=2*(Ⓒπ_r)/(ω_(r/s))' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "}",

    "Conical Pendulum",  "{ "
    "'(ω_(r/s))=(1_r)*√(Ⓒg/(h_cm))' "
    "'(h_m)=(L_m)*COS(θ_°)' "
    "'(T_s)=2*(Ⓒπ_r)/(ω_(r/s))' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "}",

    "Torsional Pendulum",  "{ "
    "'(ω_(r/s))=(1_r)*√(((G_kPa)*(J_(cm^4)))/((L_cm)*(I_(kg*m²))))' "
    "'(T_s)=2*(Ⓒπ_r)/(ω_(r/s))' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "}",
    // Modif of radian in eqns 2, 3, 4 & 5
    "Simple Harmonic",  "{ "
    "'(x_cm)=(xm_cm)*COS((ω₀_(r/s))*(t_s)+(φ_°))' "
    "'(v_(m/s))=-(ω₀_(r/s))/(1_r)*(xm_m)*SIN((ω₀_(r/s))*(t_s)+(φ_°))' "
    "'(a_(m/s²))=-(ω₀_(r/s))²/(1_r)²*(xm_m)*COS((ω₀_(r/s))*(t_s)+(φ_°))' "
    "'(ω₀_(r/s))²=(1_r)²*((k_(N/m))/(m_kg))' "
    "'(E_J)=(1/2)*(m_kg)*((ω₀_(r/s))/(1_r)*(xm_m))²' "
    "}",
    // Modif of radian in eqns (3) & (6)
    // Add radians to eqn (2); Eliminate radians in eqn (1) & 2 times in eqn (5) & in eqn (6)
    // Ref.: https://scholar.harvard.edu/files/david-morin/files/waves_oscillations.pdf
    "Underdamped Oscillations",  "{ "
    "'(x_m)=(xm_m)*EXP(-(γ_(r/s))*(t_s)/2/(1_r))*COS((ωu_(r/s))*(t_s)+(φ_°))' "
    "'(γ_(r/s))=(1_r)*(b_(kg/s))/(m_kg)' "
    "'(ω₀_(r/s))²=(1_r)²*((k_(N/m))/(m_kg))' "
    "'(ωu_(r/s))²=(ω₀_(r/s))²*(1-((γ_(r/s))/(2*(ω₀_(r/s))))²)' "
    "'(v_(m/s))=(xm_m)/(1_r)*EXP(-(γ_(r/s))*(t_s)/2/(1_r))*(-((γ_(r/s))/2)*COS((ωu_(r/s))*(t_s)+(φ_°))-(ωu_(r/s))*SIN((ωu_(r/s))*(t_s)+(φ_°)))' "
    "'(a_(m²/s))=-((ω₀_(r/s))²/(1_r)²*(x_m)+(γ_(r/s))*(v_m/s)/(1_r))' "
    "'(E_J)=(1/2)*(k_(N/m))*(x_m)²+(1/2)*(m_kg)*(v_(m/s))²' "
    "'Q=(ω₀_(r/s))/(γ_(r/s))' "
    "}",
    // Ref.: https://scholar.harvard.edu/files/david-morin/files/waves_oscillations.pdf
    "Driven Damped Oscillations",  "{ "
    "'(x_m)=(xp_m)*COS((ω_(r/s))*(t_s)+(φ_°))+(xh_m)*EXP(-(γ_(r/s))/(1_r)*(t_s)/2)*COS((ωu_(r/s))*(t_s)+(θ_°))' "
    "'(γ_(r/s))=(1_r)*(b_(kg/s))/(m_kg)' "
    "'(ω₀_(r/s))²=(1_r)²*((k_(N/m))/(m_kg))' "
    "'(ωu_(r/s))²=(ω₀_(r/s))²*(1-((γ_(r/s))/(2*(ω₀_(r/s))))²)' "
    "'(xp_m)²=((1_r)²*((Fd_N)/(m_kg)))²/(((ω₀_(r/s))²-(ω_(r/s))²)²+((γ_(r/s))*(ω_(r/s)))²)' "
    "'TAN(φ_°)=-((γ_(r/s))*(ω_(r/s)))/((ω₀_(r/s))²-(ω_(r/s))²)' "
    // The next 7th eqn doesn't fit on the screen, but it goes on the stack with Eq
    "'(v_(m/s))=-(xp_m)*(ω_(r/s))/(1_r)*SIN((ω_(r/s))*(t_s)+(φ_°))+(xh_m)*EXP(-(γ_(r/s))/(1_r)*(t_s)/2)*(-((γ_(r/s))/(1_r)/2)*COS((ωu_(r/s))*(t_s)+(θ_°))-(ωu_(r/s))/(1_r)*SIN((ωu_(r/s))*(t_s)+(θ_°)))' "
    "'(a_(m²/s))=-(((ω₀_(r/s))/(1_r))²*(x_m)+(γ_(r/s))/(1_r)*(v_m/s))+((Fd_N)/(m_kg))*COS((ω_(r/s))*(t_s))' "
    "'(E_J)=(1/2)*(k_(N/m))*(x_m)²+(1/2)*(m_kg)*(v_(m/s))²' "
    "'Q=(ω₀_(r/s))/(γ_(r/s))' "
    "}",

    // ------------------------------------------------------------------------
    "Plane Geometry", nullptr,
    // ------------------------------------------------------------------------
    //T#*: 21 vars 31 eqns 6 sims 6 secs
    "Circle",  "{ "
    "'(A_(cm²))=Ⓒπ*(r_cm)²' "
    "'(C_cm)=2*Ⓒπ*(r_cm)' "
    "'(I_(cm^4))=1/4*Ⓒπ*(r_cm)^4' "
    "'(J_(cm^4))=1/2*Ⓒπ*(r_cm)^4' "
    "'(Id_(cm^4))=(I_(cm^4))+(A_(cm²))*(d_cm)²' "
    "}",

    "Ellipse",  "{ "
    "'(A_(cm²))=Ⓒπ*(b_cm)*(h_cm)' "
    "'(C_cm)²=(2*Ⓒπ)²*(((b_cm)²+(h_cm)²)/2)' "
    "'(I_(cm^4))=1/4*Ⓒπ*(b_cm)*(h_cm)^3' "
    "'(J_(cm^4))=1/4*Ⓒπ*(b_cm)*(h_cm)*((b_cm)²+(h_cm)²)' "
    "'(Id_(cm^4))=(I_(cm^4))+(A_(cm²))*(d_cm)²' "
    "}",

    "Rectangle",  "{ "
    "'(A_(cm²))=(b_cm)*(h_cm)' "
    "'(P_cm)=2*((b_cm)+(h_cm))' "
    "'(I_(m^4))=1/12*(b_cm)*(h_cm)^3' "
    "'(J_(cm^4))=1/12*(b_cm)*(h_cm)*((b_cm)²+(h_cm)²)' "
    "'(Id_(cm^4))=(I_(cm^4))+(A_(cm²))*(d_cm)²' "
    "}",

    "Regular Polygon",  "{ "
    "'(A_(cm²))=(1/4*n*(L_yd)²)/TAN((180_°)/n)' "
    "'(P_cm)=n*(L_cm)' "
    "'(rs_cm)=((L_cm)/2)/TAN((180_°)/n)' "
    "'(rv_cm)=((L_cm)/2)/SIN((180_°)/n)' "
    "'(θ_°)=(n-2)/n*(180_°)' "
    "'(β_°)=(360_°)/n' "
    "}",

    "Circular Ring",  "{ "
    "'(A_(cm²))=Ⓒπ*((ro_cm)²-(ri_cm)²)' "
    "'(I_(cm^4))=Ⓒπ/4*((ro_cm)^4-(ri_cm)^4)' "
    "'(J_(cm^4))=Ⓒπ/2*((ro_cm)^4-(ri_cm)^4)' "
    "'(Id_(cm^4))=(I_(m^4))+(A_(m²))*(d_cm)²' "
    "}",
// Error correction: change I_(cm^4) for Ix_(cm^4) in eqn (6)
    "Triangle",  "{ "
    "'(A_(in²))=(b_cm)*(h_cm)/2' "
    "'(P_cm)=(b_cm)+√((v_cm)²+(h_cm)²)+√(((b_cm)-(v_cm))²+(h_cm)²)' "
    "'(Ix_(cm^4))=1/36*(b_cm)*(h_cm)^3' "
    "'(Iy_(cm^4))=1/36*(b_cm)*(h_cm)*((b_cm)²+(v_cm)²-(b_cm)*(v_cm))' "
    "'(J_(cm^4))=1/36*(b_cm)*(h_cm)*((b_cm)²+(v_cm)²+(h_cm)²-(b_cm)*(v_cm))' "
    "'(Id_(cm^4))=(Ix_(cm^4))+(A_(in²))*(d_cm)²' "
    "}",

    // ------------------------------------------------------------------------
    "Solid Geometry", nullptr,
    // ------------------------------------------------------------------------
    //T#*: 12 vars 18 eqns 4 sims 4 secs
    "Cone",  "{ "
    "'(V_(cm^3))=(Ⓒπ/3)*(r_cm)²*(h_cm)' "
    "'(A_(cm²))=Ⓒπ*(r_cm)²+Ⓒπ*(r_cm)*√((r_cm)²+(h_cm)²)' "
    "'(Ixx_(kg*m²))=(3/20)*(m_kg)*(r_cm)²+(3/80)*(m_kg)*(h_cm)²' "
    "'(Izz_(kg*m²))=(3/10)*(m_kg)*(r_cm)²' "
    "'(Id_(kg*m²))=(Ixx_(kg*m²))+(m_kg)*(d_cm)²' "
    "}",

    //WARNING mass units in the example of HP50G_AUR (12.2) is lbs...
    // is it lbf or lb (lb probably)
    "Cylinder",  "{ "
    "'(V_(cm^3))=Ⓒπ*(r_cm)²*(h_cm)' "
    "'(A_(cm²))=2*Ⓒπ*(r_cm)²+2*Ⓒπ*(r_cm)*(h_cm)' "
    "'(Ixx_(kg*cm²))=(1/4)*(m_kg)*(r_cm)²+(1/12)*(m_kg)*(h_cm)²' "
    "'(Izz_(kg*cm²))=(1/2)*(m_kg)*(r_cm)²' "
    "'(Id_(kg*m²))=(Ixx_(kg*m²))+(m_kg)*(d_m)²' "
    "}",

    "Parallelepiped",  "{ "
    "'(V_(cm^3))=(b_cm)*(h_cm)*(t_cm)' "
    "'(A_(cm²))=2*((b_cm)*(h_cm)+(b_m)*(t_cm)+(h_cm)*(t_cm))' "
    "'(I_(kg*cm²))=(1/12)*(m_kg)*((h_cm)²+(t_cm)²)' "
    "'(Id_(kg*cm²))=(I_(kg*cm²))+(m_kg)*(d_cm)²' "
    "}",

    "Sphere",  "{ "
    "'(V_(cm^3))=(4/3)*Ⓒπ*(r_cm)^3' "
    "'(A_(cm²))=4*Ⓒπ*(r_cm)²' "
    "'(I_(kg*m²))=(2/5)*(m_kg)*(r_cm)²' "
    "'(Id_(kg*cm²))=(I_(kg*m²))+(m_kg)*(d_cm)²' "
    "}",

    // ------------------------------------------------------------------------
    "Solid State Device", nullptr,
    // ------------------------------------------------------------------------
    //T#*: 54 vars 36 eqns 4 sims 4 secs
    // **WARNING: Required function for the parameter ni_(cm^-3)=SIDENS(T_K)
    // And Here q = Ⓒqe.
    // ERROR in gm formula in HP50G_AUR.pdf: μm=μn (OK)
    // ERROR in 4th formula of HP50G_AUR.pdf αR=>αF
    //24-11-12 Replace the call for SIDENS by its explicit calculation in eqn (2)
    "PN Step Junctions",  "{ "
    "'(Vbi_V)=(Ⓒk*(T_°C))/Ⓒqe*LN((NA_(cm^-3))*(ND_(cm^-3))/((ni_(cm^-3))²))' "
    "'(ni_(cm^-3))=ⓁSiDensity(T_K)' "
    //"'(ni_(cm^-3))=(8.35123e20_cm^-3)*exp(-(7555.17_K)/(T_K))' "
    "'(xd_μ)²=((2*Ⓒεsi*Ⓒε₀)/Ⓒqe*((Vbi_V)-(Va_V))*(1/(NA_(cm^-3))+1/(ND_(cm^-3))))' "
    "'(Cj_(pF/cm²))=(Ⓒεsi*Ⓒε₀)/(xd_μ)' "
    "'(Emax_(V/m))=2*((Vbi_V)-(Va_V))/(xd_μ)' "
    "'(BV_V)=(Ⓒεsi*Ⓒε₀*(E1_(V/m))²)/(2*Ⓒqe)*(1/(NA_(cm^-3))+1/(ND_(cm^-3)))' "
    "'(J_(A/cm²))=(Js_(μA/cm²))*(EXP((Ⓒqe*(Va_V))/(Ⓒk*UBASE(T_K)))-1)' "
    "'(Aj_(cm²))=((W_μ)+2*(ΔW_μ))*((L_μ)+2*(ΔL_μ))+Ⓒπ*((W_μ)+2*(ΔW_μ)+2*(ΔL_μ))*(xj_μ)+2*Ⓒπ*(xj_m)²' "
    "'(I_mA)=(J_(A/cm²))*(Aj_(cm²))' "
    "}",

    // WARNING: In the 9th formula of HP50G it is μn and not μm as described in
    // HP50G_AUR
    //24-11-12 Replace the call for SIDENS by its explicit calculation in eqn (8) not anymore
    //Error correction negative sign missing in the calculation of (φp_V)=-...
    "NMOS Transistor",  "{ "
    "'(We_μ)=(W_μ)-2*(ΔW_μ)' "
    "'(Le_μ)=(L_m)-2*(ΔL_μ)' "
    "'(Cox_(pF/cm²))=(Ⓒεox*Ⓒε₀)/(tox_nm)' "
    "'(IDS_mA)=(Cox_(pF/cm²))*(μn_((cm²)/(V*s)))*((We_μ)/(Le_μ))*(((VGS_V)-(Vt_V))*(VDS_V)-(VDS_V)²/2)*(1+(λ_(1/V))*(VDS_V))' "
    "'(γ_(V^(1/2)))²=((2*Ⓒεsi*Ⓒε₀)*Ⓒqe*(NA_(cm^-3)))/(Cox_(pF/cm²))²' "
    "'(Vt_V)=(Vt0_V)+(γ_(V^(1/2)))*(√(2*ABS(φp_V)-ABS(VBS_V))-√(2*ABS(φp_V)))' "
    //"'(φp_V)=Ⓒk*UBASE(T_K)/Ⓒqe*LN((NA_(cm^-3))/(ni_(cm^-3)))' "
    "'(φp_V)=-Ⓒk*UBASE(T_K)/Ⓒqe*LN((NA_(cm^-3))/(ni_(cm^-3)))' "
    "'(ni_(cm^-3))=ⓁSiDensity(T_K)' "
    //"'(ni_(cm^-3))=(8.35123e20_cm^-3)*exp(-(7555.17_K)/UBASE(T_K))' "
    "'(gds_S)=(IDS_mA)*(λ_(V^-1))' "
    "'(gm_(mA/V))²=((Cox_(pF/cm²))*(μn_((cm²)/(V*s)))*((We_m)/(Le_m))*(1+(λ_(V^-1))*(VDS_V))*2*(IDS_mA))' "
    "'(VDsat_V)=(VGS_V)-(Vt_V)' "
    "}",

    // WARNING HP50G has precedence here for the presence of VBC in 2nd exp of
    // 1st eqn (contrary to HP50G_AUR)
    // WARNING HP50G has precedence here for the presence of αF instead of αR in
    // 3rd eqn (contrary to HP50G_AUR)
    "Bipolar Transistors",  "{ "
    "'(IE_mA)=-(IES_nA)*(exp((Ⓒqe*(VBE_V))/(Ⓒk*UBASE(T_K)))-1)+αR*(IES_nA)*(EXP((Ⓒqe*(VBC_V))/(Ⓒk*UBASE(T_K)))-1)' "
    "'(IC_mA)=-(ICS_nA)*(exp((Ⓒqe*(VBC_V))/(Ⓒk*UBASE(T_K)))-1)+αF*(ICS_nA)*(EXP((Ⓒqe*(VBE_V))/(Ⓒk*UBASE(T_K)))-1)' "
    "'(IS_nA)=αF*(IES_nA)' "
    "'(IS_nA)=αR*(ICS_nA)' "
    "'(IB_nA)+(IE_nA)+(IC_nA)=0_nA' "
    "'(ICO_nA)=(ICS_nA)*(1-αF*αR)' "
    "'(ICEO_nA)=(ICO_nA)/(1-αF)' "
    "'(VCEsat_V)=((Ⓒk*UBASE(T_K)))/Ⓒqe*LN(MAX(((1+(IC_mA)/(IB_mA)*(1-αR)))/(αR*(1-(IC_mA)/(IB_mA)*((1-αF)/αF)));1E-100))' "
    "}",

    //24-11-12 Replace the call for SIDENS by its explicit calculation in eqn (2)
    "JFETs",  "{ "
    "'(Vbi_V)=(Ⓒk*UBASE(T_K))/Ⓒqe*LN((ND_(cm^-3))/(ni_(cm^-3)))' "
    "'(ni_(cm^-3))=ⓁSiDensity(T_K)' "
    //"'(ni_(cm^-3))=(8.35123e20_cm^-3)*exp(-(7555.17_K)/UBASE(T_K))' "
    "'(xdmax_μ)²=((2*Ⓒεsi*Ⓒε₀)/(Ⓒqe*(ND_(cm^-3)))*((Vbi_V)-(VGS_V)+(VDS_V)))' "
    "'(G0_S)=Ⓒqe*(ND_(cm^-3))*(μn_((cm²)/(V*s)))*(((a_μ)*(W_μ))/(L_μ))' "
    "'(ID_mA)=(G0_S)*((VDS_V)-((2/3)*√((2*Ⓒεsi*Ⓒε₀)/(Ⓒqe*(ND_(cm^-3))*(a_μ)²)))*(((Vbi_V)-(VGS_V)+(VDS_V))^(3/2)-((Vbi_V)-(VGS_V))^(3/2)))' "
    "'(VDsat_V)=(Ⓒqe*(ND_(cm^-3))*(a_μ)²)/(2*Ⓒεsi*Ⓒε₀)-((Vbi_V)-(VGS_V))' "
    "'(Vt_V)=(Vbi_V)-(Ⓒqe*(ND_(cm^-3))*(a_μ)²)/(2*Ⓒεsi*Ⓒε₀)' "
    "'(gm_(mA/V))=(G0_S)*(1-√(((2*Ⓒεsi*Ⓒε₀)/(Ⓒqe*(ND_(cm^-3))*(a_μ)²))*((Vbi_V)-(VGS_V))))' "
    "}",

    // ------------------------------------------------------------------------
    "Stress Analysis", nullptr,
    // ------------------------------------------------------------------------
    //T#*: 28 vars 16 eqns 4 sims 4 secs
    //"Inconsistent units" in eqn (2), New trial with UBASE
    "Normal Stress",  "{ "
    "'(σ_atm)=(E_atm)*ε' "
    //"'ε=(δ_cm)/(L_m)' "
    "'ε=UBASE((δ_cm)/(L_m))' "
    "'(σ_Pa)=(P_N)/(A_cm²)' "
    "}",
    // ALL angles should be in radians in eqns 1 & 2, with a correction in eqn 1
    "Shear Stress",  "{ "
    "'(τ_atm)=(G_atm)*(γ_r)/(1_r)' "
    "'(γ_r)=((r_cm)*(φ_r))/(L_m)' "
    "'(τ_atm)=((T_(cm*N))*(r_cm))/(J_(cm^4))' "
    "}",

    "Stress On An Element",  "{ "
    //Error in eqn (3) of HP50g_AUR
    "'(σx1_kPa)=((σx_kPa)+(σy_kPa))/2+((σx_kPa)-(σy_kPa))/2*COS(2*(θ_°))+(τxy_kPa)*SIN(2*(θ_°))' "
    "'(σx1_kPa)+(σy1_kPa)=(σx_kPa)+(σy_kPa)' "
    //"'(τx1y1_kPa)=-(((σx_kPa)-(σy_kPa))/2)*SIN(2*(θ_°))+(τxy_kPa)*(σy_kPa)' "
    "'(τx1y1_kPa)=-(((σx_kPa)-(σy_kPa))/2)*SIN(2*(θ_°))+(τxy_kPa)*COS(2*(θ_°))' "
    "}",

    // WARNING Error in formula 1 of HP50G_AUR square missing HP50G takes
    // precedence
    "Mohr’s Circle",  "{ "
    "'(σ1_atm)=((σx_atm)+(σy_atm))/2+√((((σx_atm)-(σy_atm))²)/2+(τxy_atm)²)' "
    "'(σ1_atm)+(σ2_atm)=(σx_atm)+(σy_atm)' "
    "'(SIN(2*(θp1_°)))²=(τxy_atm)²/((((σx_atm)-(σy_atm))/2)²+(τxy_atm)²)' "
    "'(θp2_°)=(θp1_°)+(90_°)' "
    "'(τmax_atm)=((σ1_atm)-(σ2_atm))/2' "
    "'(θs_°)=(θp1_°)-(45_°)' "
    "'(σavg_atm)=((σx_atm)+(σy_atm))/2' "
    "}",

    // ------------------------------------------------------------------------
    "Waves", nullptr,
    // ------------------------------------------------------------------------
    //T#*: 39 vars 49 eqns 11 sims 9 secs
    "Transverse Waves",  "{ "
    "'(y_cm)=(ym_cm)*SIN((k_(r/cm))*(x_cm)-(ω_(r/s))*(t_s)+(φ_r))' "
    "'(v_(cm/s))=(λ_cm)*(f_Hz)' "
    "'(k_(r/cm))=2*(Ⓒπ_r)/(λ_cm)' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "'(vy_(cm/s))=-(ym_cm)*(ω_(r/s))/(1_r)*COS((k_(r/cm))*(x_cm)-(ω_(r/s))*(t_s)+(φ_r))' "
    "'(ay_(cm/(s²)))=-((ω_(r/s))/(1_r))²*(ym_cm)' "
    "}",

    "Longitudinal Waves",  "{ "
    "'(s_cm)=(sm_cm)*COS((k_(r/cm))*(x_cm)-(ω_(r/s))*(t_s)+(φ_r))' "
    "'(v_(m/s))=(λ_cm)*(f_Hz)' "
    "'(k_(r/cm))=2*(Ⓒπ_r)/(λ_cm)' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "'(vs_(cm/s))=(sm_cm)*(ω_(r/s))/(1_r)*SIN((k_(r/cm))*(x_cm)-(ω_(r/s))*(t_s)+(φ_r))' "
    "'(as_(cm/(s²)))=-((ω_(r/s))/(1_r))²*(s_cm)' "
    "}",
    // Rewrite the last eqn to have explicit LOG in base 10 and with the fact that I₀ has 1_(W/m²) units
    "Sound Waves",  "{ "
    "'(s_cm)=(sm_cm)*SIN((k_(r/cm))*(x_cm)-(ω_(r/s))*(t_s)+(φ_r))' "
    "'(vs_(cm/s))=(sm_cm)*(ω_(r/s))/(1_r)*COS((k_(r/cm))*(x_cm)-(ω_(r/s))*(t_s)+(φ_r))' "
    "'(as_(cm/(s²)))=-((ω_(r/s))/(1_r))²*(s_cm)' "
    "'(Δp_Pa)=-(Δpm_Pa)*COS((k_(r/cm))*(x_cm)-(ω_(r/s))*(t_s)+(φ_r))' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "'(v_(m/s))²=((B_kPa)/(ρ_(kg/(m^3))))' "
    "'(Δpm_Pa)=(ρ_(kg/(m^3)))*(ω_(r/s))/(1_r)*(v_(m/s))*(sm_cm)' "
    "'(I_(W/(m²)))=1/2*(ρ_(kg/(m^3)))*(v_(m/s))*((ω_(r/s))/(1_r))²*(sm_cm)²' "
    "'(I_(W/m²))=(Ps_W)/(4*Ⓒπ*(r_m)²)' "
    //"'(β_dB)=10*LOG((I_(W/(m²)))/(ⒸI₀_(W/(m²))))' "
    "'(β_dB)=(10_dB)*LOG10((I_(W/(m²)))/(ⒸI₀))' "
    "}",
    // Error in  eqn 1 cair => vsair
    "Doppler Effect",  "{ "
    "'(f_Hz)=(f0_Hz)*(((vsair_(m/s))+(vr_(m/s)))/((vsair_(m/s))-(vs_(m/s))))' "
    "'(vsair_(m/s))²=(1.4*((8.314462618153_(J/(mol*K)))/(0.0289645_(kg/mol))*UBASE(Tair_K)))' "
    "}",

    "Mach Number",  "{ "
    "'M=(u_(m/s))/(vsair_(m/s))' "
    "'SIN(θcone_°)=(vsair_(m/s))/(u_(m/s))' "
    "'(vsair_(m/s))²=(1.4*((8.314462618153_(J/(mol*K)))/(0.0289645_(kg/mol))*UBASE(Tair_K)))' "
    "}",

    "String Standing Waves",  "{ "
    "'(y_m)=(ym_m)*SIN((k_(r/m))*(x_m))*COS((ω_(r/s))*(t_s))' "
    "'(v_(m/s))=(λ_m)*(f_Hz)' "
    "'(k_(r/m))=2*(Ⓒπ_r)/(λ_m)' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "'(v_(m/s))²=((Ts_N)/(μ_(kg/m)))' "
    "'(ffixedfixed_Hz)=(ninteger*(v_(m/s)))/(2*(L_m))' "
    "'(ffixedfree_Hz)=(nodd*(v_(m/s)))/(4*(L_m))' "
    "}",

    "Sound Wave Harmonics",  "{ "
    "'(s_m)=(sm_m)*SIN((k_(r/m))*(x_m))*COS((ω_(r/s))*(t_s))' "
    "'(vsair_(m/s))=(λ_m)*(f_Hz)' "
    "'(k_(r/m))=2*(Ⓒπ_r)/(λ_m)' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "'(vsair_(m/s))²=(1.4*((8.314462618153_(J/(mol*K)))/(0.0289645_(kg/mol))*UBASE(Tair_K)))' "
    "'(fopenopen_Hz)=(ninteger*(vsair_(m/s)))/(2*(L_m))' "
    "'(fopenclose_Hz)=(nodd*(vsair_(m/s)))/(4*(L_m))' "
    "}",

    "Beat Acoustics",  "{ "
    "'(s_m)=(sm_m)*COS(2*(Ⓒπ_r)*(favg_Hz)*(t_s))*COS(2*(Ⓒπ_r)*(fbeat_Hz)/2*(t_s))' "
    "'(favg_Hz)=((f1_Hz)+(f2_Hz))/2' "
    "'(fbeat_Hz)=ABS((f1_Hz)-(f2_Hz))' "
    "}",

    "Electromagnetic Waves",  "{ "
    "'(E_(N/C))=(Em_(N/C))*SIN((k_(r/m))*(x_m)-(ω_(r/s))*(t_s)+(φ_r))' "
    "'(E_(N/C))/(B_T)=Ⓒc' "
    "'Ⓒc=(λ_m)*(f_Hz)' "
    "'(k_(r/m))=2*(Ⓒπ_r)/(λ_m)' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "}",

    // ------------------------------------------------------------------------
    "Relativity", nullptr,
    // ------------------------------------------------------------------------
    //T#*: 110 vars 110 eqns 29 sims 17 secs
    //The primed frame is travelling with speed v in the positive x direction
    // ref.: https://en.wikipedia.org/wiki/Special_relativity
    // https://galileo-unbound.blog/2021/06/03/the-transverse-doppler-effect-and-relativistic-time-dilation/
    // For circumnavigation : https://www.mathpages.com/rr/s6-06/6-06.htm
    // for time dilation at height : http://ws680.nist.gov/publication/get_pdf.cfm?pub_id=905055 & Note de cours PhyN03 J. Wilson
    "Lorentz Transformation",  "{ "
    "'(xp_m)=γ*((x_m)-β*Ⓒc*(t_s))' "
    "'Ⓒc*(tp_s)=γ*((Ⓒc*(t_s))-β*(x_m))' "
    "'(yp_m)=(y_m)' "
    "'(zp_m)=(z_m)' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ²=1/(1-β²)' "
    "}",

    "Time Dilation",  "{ "
    "'(Δtp_s)=γ*(Δt_s)' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ²=1/(1-β²)' "
    "}",

    "Space Contraction",  "{ "
    "'(Δxp_m)=(Δx_m)/γ' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ²=1/(1-β²)' "
    "}",

    "Velocity Superposition",  "{ "
    "'(upx_(m/s))=((ux_(m/s))-(v_(m/s)))/(1-((((v_(m/s))*(ux_(m/s))))/Ⓒc²))' "
    "'(upy_(m/s))=((uy_(m/s))/γ)/(1-((v_(m/s))*(ux_(m/s))/Ⓒc²))' "
    "'(upz_(m/s))=((uz_(m/s))/γ)/(1-((v_(m/s))*(ux_(m/s))/Ⓒc²))' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ²=1/(1-β²)' "
    "}",
    //Error in the expression of apx the exponent 3 is misplaced. Other expressions: OK
    "Acceleration Superposition",  "{ "
    //"'(apx_(m/s²))=(ax_(m/s²))/(γ^3*(1-((((v_(m/s))*(ux_(m/s))))/Ⓒc²)^3))' "
      "'(apx_(m/s²))=(ax_(m/s²))/(γ^3*(1-((((v_(m/s))*(ux_(m/s))))/Ⓒc²))^3)' "
    "'(apy_(m/s))=(ay_(m/s))/(γ²*(1-((v_(m/s))*(ux_(m/s))/Ⓒc²))²)+((ax_(m/s))*((v_(m/s))*(uy_(m/s)))/Ⓒc²)/(γ²*(1-((v_(m/s))*(ux_(m/s))/Ⓒc²))^3)' "
    "'(apz_(m/s))=(az_(m/s))/(γ²*(1-((v_(m/s))*(ux_(m/s))/Ⓒc²))²)+((ax_(m/s))*((v_(m/s))*(uz_(m/s)))/Ⓒc²)/(γ²*(1-((v_(m/s))*(ux_(m/s))/Ⓒc²))^3)' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ²=1/(1-β²)' "
    "}",
// Reference: https://farside.ph.utexas.edu/teaching/355/Surveyhtml/node156.html
// Errors in eqns 2, 3, 5 & 6. Now corrected, to be checked. Algebraics: "Inconsistent units"
    "E & B Fields Transformation",  "{ "
    "'(Epx_(N/C))=(Ex_(N/C))' "
    //"'(Epy_(N/C))=γ*((Ey_(N/C))-β*(Bz_T))' "
    //"'(Epz_(N/C))=γ*((Ez_(N/C))+β*(By_T))' "
    "'(Epy_(N/C))=γ*((Ey_(N/C))-v*(Bz_T))' "
    "'(Epz_(N/C))=γ*((Ez_(N/C))+v*(By_T))' "
    "'(Bpx_T)=(Bx_T)'"
    //"'(Bpy_T)=γ*((By_T)+β*(Ez_(N/C)))' "
    //"'(Bpz_T)=γ*((Bz_T)-β*(Ey_(N/C)))' "
    "'(Bpy_T)=γ*((By_T)+β/Ⓒc*(Ez_(N/C)))' "
    "'(Bpz_T)=γ*((Bz_T)-β/Ⓒc*(Ey_(N/C)))' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ²=1/(1-β²)' "
    "'(E_(N/C))²=((Ex_(N/C))²+(Ey_(N/C))²+(Ez_(N/C))²)' "
    "'(Ep_(N/C))²=((Epx_(N/C))²+(Epy_(N/C))²+(Epz_(N/C))²)' "
    "'(B_(T))²=((Bx_(T))²+(By_(T))²+(Bz_(T))²)' "
    "'(Bp_(T))²=((Bpx_(T))²+(Bpy_(T))²+(Bpz_(T))²)' "
    "'EDB=convert((E_(N/C))/(B_(T));1_m/s)' "
    "'EpDBp=convert((Ep_(N/C))/(Bp_(T));1_m/s)' "
    "}",

    "Longitudinal Doppler Effect",  "{ "
    "'(frl_Hz)²=(fs_Hz)²*((1-β)/(1+β))' "
    "'β=(v_(m/s))/Ⓒc' "
    "}",

    "Transverse Doppler Effect",  "{ "
    "'(frt_Hz)=γ*(fs_Hz)' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ²=1/(1-β²)' "
    "}",
// Ref.: chrome-extension://efaidnbmnnnibpcajpcglclefindmkaj/https://www.fisica.net/formulas/The-Cambridge-Handbook-of-Physics-Formulas.pdf
// Name of var fp changed to fpr
    "Light Propagation",  "{ "
    "'(fpr_Hz)/(f_Hz)=γ*(1+β*COS(α_°))' "
    "'COS(θp_°)=(COS(θ_°)-β)/(1-β*COS(θ_°))' "
    "'Pθ=SIN(θ_°)/(2*γ²*(1-β*COS(θ_°))²)' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ²=1/(1-β²)' "
    "}",

    "Energy & Momentum",  "{ "
    "'(ppx_(kg*(m/s)))=γ*((px_(kg*(m/s)))-(v_(m/s))*(E_J)/Ⓒc²)' "
    "'(ppy_(kg*(m/s)))=(py_(kg*(m/s)))' "
    "'(ppz_(kg*(m/s)))=(pz_(kg*(m/s)))' "
    "'(Ep_J)=γ*((E_J)-(v_m/s)*(px_(kg*(m/s))))' "
    "'(p_(kg*(m/s)))=γ*(m0_kg)*(v_(m/s))' "
    "'(E_J)=γ*(E₀_J)' "
    "'(E₀_J)=(m0_kg)*Ⓒc²' "
    "'(E_J)²=(p_(kg*(m/s)))²*Ⓒc²+(m0_kg)²*Ⓒc^4' "
    "'(K_J)=(γ-1)*(E₀_J)' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ²=1/(1-β²)' "
    "}",

    "Ultrarelativistic Cases",  "{ "
    "'(E₀_J)=(m0_kg)*Ⓒc²' "
    "'(E_J)²=(p_(kg*(m/s)))²*Ⓒc²+(m0_kg)²*Ⓒc^4' "
    "'(K_J)=(γ-1)*(E₀_J)' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ²=1/(1-β²)' "
    "'(Δtp_s)=γ*(Δt_s)' "
    "'(Δxp_m)=(Δx_m)/γ' "
    "}",
// Error in eqn (3) r(m) => (r_m)
    "Gravitational Time Dilation",  "{ "
    "'(ΔtpG_s)=γG*(Δt_s)' "
    "'βe=(ve_(m/s))/Ⓒc' "
    "'γG²=1/(1-βe²)' "
    "'(ve_(m/s))²=(2*ⒸG*(M_kg)/(r_m))' "
    "}",
// Rename "Rem" to "Remp" in eqn 2 & 4
    "Gravitational Redshift",  "{ "
    "'z=(λ∞_nm)/(λe_nm)-1' "
    "'((λ∞_nm)/(λe_nm))²=1/(1-((rs_m)/(Remp_m)))' "
    "'((λ1_nm)/(λ2_nm))²=((1-(rs_m)/(R1_m))/(1-(rs_m)/(R2_m)))' "
    "'zNL=ⒸG*(M_kg)/((Ⓒc)²*(Remp_m))' "
    "'(rs_m)=2*ⒸG*(M_kg)/(Ⓒc)²' "
    "}",
// Ref https://mathpages.com/rr/s6-06/6-06.htm; Adding ' at the end of eqn 1; Modifs to euns 5 & 6
    "Circumnavigating Airplanes",  "{ "
    "'(Δt_s)=2*Ⓒπ*(R_km)*COS(φ_°)/(vp_m/s)' "
    "'(ΔτWE_ns)=(ΔτW_ns)-(ΔτE_ns)' "
    "'(ΔτE_ns)=(ΔτpE_ns)-(Δτg_ns)' "
    "'(ΔτW_ns)=(ΔτpW_ns)-(Δτg_ns)' "
    "'(ΔτpE_ns)=(1-(MGu_m)/((R_m)+(hp_m))-((βg+βp)²)/2)*(Δt_s)' "
    "'(ΔτpW_ns)=(1-(MGu_m)/((R_m)+(hp_m))-((βg-βp)²)/2)*(Δt_s)' "
    "'(Δτg_ns)=(1-(MGu_m)/(R_m)-βg²/2)*(Δt_s)' "
    "'(MGu_m)=ⒸG*(M_kg)/Ⓒc²' "
    "'βp=(vp_(m/s))/Ⓒc' "
    "'βg=(vg_(m/s))/Ⓒc' "
    "'(vg_m/s)=(2*Ⓒπ*(R_km))/(Tday_s)*COS(φ_°)' "
    "}",
// Ref: https://mathpages.com/rr/s6-06/6-06.htm
    "Clocks at different heights",  "{ "
    "'γ21=(γv1/γv2)*(γG1/γG2)' "
    "'γv1²=1/(1-((v1_(m/s))/Ⓒc)²)' "
    "'(v1_m/s)=(ω_(r/s))/(1_r)*((R_km)+(h1_m))*COS(φ_°)' "
    "'γv2²=1/(1-((v2_(m/s))/Ⓒc)²)' "
    "'(v2_m/s)=(ω_(r/s))/(1_r)*((R_km)+(h2_m))*COS(φ_°)' "
    "'γG1²=1/(1-(2*(MGu_m))/(R+h1))' "
    "'γG2²=1/(1-(2*(MGu_m))/(R+h2))' "
    "'(MGu_m)=ⒸG*(M_kg)/Ⓒc²' "
    "'(ω_(r/s))=(2*(Ⓒπ_r))/(Tday_s)' "
    "}",
// Error in eqn (2) "√" missing; Scientific notations corrected in eqns 4, 6 & 7
    "B H Schwarzschild Geometry",  "{ "
    "'(rs_m)=2*ⒸG*(M_kg)/(Ⓒc)²' "
    "'(ve_(m/s))²=(2*ⒸG*(M_kg)/(rs_m))' "
    "'(Vs_(m^3))=(4/3)*Ⓒπ*(rs_m)^3' "
    "'Vxsun=(V_(km^3))/(1.412e18_(km^3))' "
    "'rxearth=(r_km)/(6371_km)' "
    "'Mxsun=(M_kg)/(1.9885e30_kg)' "
    "'Mxearth=(M_kg)/(5.972168e24_kg)' "
    "}",
// ERROR: Scientific notations corrected in eqns 7, 8, 9 & 10
    "B H Thermodynamics",  "{ "
    "'UBASE(TH_K)=(Ⓒℏ*(Ⓒc)^3)/(8*Ⓒπ*ⒸG*Ⓒk*(M_kg))' "
    "'(PBH_W)=(Ⓒℏ*(Ⓒc)^6)/(15360*Ⓒπ*(ⒸG)²*(M_kg)²)' "
    "'(SBH_(J/K))=(Ⓒk*(As_(m²))*(Ⓒc)^3)/(4*ⒸG*Ⓒℏ)' "
    "'(As_(m²))=4*Ⓒπ*(rs_m)²' "
    "'(rs_m)=2*ⒸG*(M_kg)/(Ⓒc)²' "
    "'(tev_s)=(5120*Ⓒπ*(ⒸG)²*(M_kg)^3)/(Ⓒℏ*(Ⓒc)^4)' "
    "'Mxsun=(M_kg)/(1.9885e30_kg)' "
    "'MxSagA=(M_kg)/(8.54e36_kg)' "
    "'Mxearth=(M_kg)/(5.972168e24_kg)' "
    "'txyr=(t_s)/(3.15576e7_s)' "
    "}",

    // ------------------------------------------------------------------------
    "Modern Physics", nullptr,
    // ------------------------------------------------------------------------
    //T#*: 43 vars 56 eqns 7 sims 6 secs
    "Planck & Wien Comparison",  "{ "
    // Eqns order change. Modifications to eqns 5, 6, 9, 11 & 12
    "'(fpeak_THz)=UBASE(T_K)*Ⓒc3f' "
    "'(f1_THz)=Ⓒk*4*UBASE(T_K)/Ⓒh' "
    "'(f2_THz)=Ⓒk*9*UBASE(T_K)/Ⓒh' "
    "'FrPl12=15/Ⓒπ^4*∫(UBASE(Ⓒh*(f1_Hz)/(Ⓒk*UBASE(T_K)));UBASE(Ⓒh*(f2_Hz)/(Ⓒk*UBASE(T_K)));x^3/expm1(x);x)' "
    "'FrWn12=15/Ⓒπ^4*∫(UBASE(Ⓒh*(f1_Hz)/(Ⓒk*UBASE(T_K)));UBASE(Ⓒh*(f2_Hz)/(Ⓒk*UBASE(T_K)));x^3/exp(x);x)' "
    "'%rFr12=ABS(FrPl12-FrWn12)/FrPl12*100' "
    "'(f3_Hz)=Ⓒk*0.01*UBASE(T_K)/Ⓒh' "
    "'(f4_Hz)=Ⓒk*2.5*UBASE(T_K)/Ⓒh' "
    "'FrPl34=15/Ⓒπ^4*∫(UBASE(Ⓒh*(f3_Hz)/(Ⓒk*UBASE(T_K)));UBASE(Ⓒh*(f4_Hz)/(Ⓒk*UBASE(T_K)));x^3/expm1(x);x)' "
    "'FrWn34=15/Ⓒπ^4*∫(UBASE(Ⓒh*(f3_Hz)/(Ⓒk*UBASE(T_K)));UBASE(Ⓒh*(f4_Hz)/(Ⓒk*UBASE(T_K)));x^3/exp(x);x)' "
    "'%rFr34=ABS(FrPl34-FrWn34)/FrPl34*100' "
    "'FrPlab=15/Ⓒπ^4*∫(UBASE(Ⓒh*(fa_Hz)/(Ⓒk*UBASE(T_K)));UBASE(Ⓒh*(fb_Hz)/(Ⓒk*UBASE(T_K)));x^3/expm1(x);x)' "
    "'(eb_(W/m²))=Ⓒσ*(T_°K)^4' "
    "'(ebfafb_(W/m²))=Frfafb*(eb_(W/m²))' "
    "'(q_W)=(ebfafb_(W/m²))*(A_(cm²))' "
    "}",

    "Planck & Rayleigh‐Jeans Comparison",  "{ "
    // Eqns order change. Modifications to eqns 6, 9, 11 & 12
    "'(fpeak_THz)=UBASE(T_K)*Ⓒc3f' "
    "'(f1_THz)=Ⓒk*1.7*UBASE(T_K)/Ⓒh' "
    "'(f2_THz)=Ⓒk*3.7*UBASE(T_K)/Ⓒh' "
    "'FrPl12=15/Ⓒπ^4*∫(UBASE(Ⓒh*(f1_Hz)/(Ⓒk*UBASE(T_K)));UBASE(Ⓒh*(f2_Hz)/(Ⓒk*UBASE(T_K)));x^3/expm1(x);x)' "
    "'FrRJ12=15/Ⓒπ^4*∫(UBASE(Ⓒh*(f1_Hz)/(Ⓒk*UBASE(T_K)));UBASE(Ⓒh*(f2_Hz)/(Ⓒk*UBASE(T_K)));x²;x)' "
    "'%rFr12=ABS(FrPl12-FrRJ12)/FrPl12*100' "
    "'(f3_Hz)=Ⓒk*0.001*UBASE(T_K)/Ⓒh' "
    "'(f4_Hz)=Ⓒk*0.05*UBASE(T_K)/Ⓒh' "
    "'FrPl34=15/Ⓒπ^4*∫(UBASE(Ⓒh*(f3_Hz)/(Ⓒk*UBASE(T_K)));UBASE(Ⓒh*(f4_Hz)/(Ⓒk*UBASE(T_K)));x^3/expm1(x);x)' "
    "'FrRJ34=15/Ⓒπ^4*∫(UBASE(Ⓒh*(f3_Hz)/(Ⓒk*UBASE(T_K)));UBASE(Ⓒh*(f4_Hz)/(Ⓒk*UBASE(T_K)));x²;x)' "
    "'%rFr34=ABS(FrPl34-FrRJ34)/FrPl34*100' "
    "'FrPlab=15/Ⓒπ^4*∫(UBASE(Ⓒh*(fa_Hz)/(Ⓒk*UBASE(T_K)));UBASE(Ⓒh*(fb_Hz)/(Ⓒk*UBASE(T_K)));x^3/expm1(x);x)' "
    "'(eb_(W/m²))=Ⓒσ*(T_°K)^4' "
    "'(ebfafb_(W/m²))=Frfafb*(eb_(W/m²))' "
    "'(q_W)=(ebfafb_(W/m²))*(A_(cm²))' "
    "}",

    "Photoelectric Effect",  "{ "
    // All eqns modified INPUT: f_Hz;φ_eV
    "'f_Hz=Ⓒc/(λ_nm)' "
    "'Eph_eV=Ⓒh*(f_Hz)' "
    "'f0_Hz=(φ_eV)/Ⓒh' "
    "'λ0_nm=Ⓒc/(f0_Hz)' "
    "'Kmax_eV=IFTE(f_Hz>f0_Hz;Ⓒh*(f_Hz)-(φ_eV);0)' "
    "'Kmax_eV=Ⓒqe*(Vo_V)' "
    "'(vmax_m/s)²=((2*Kmax_J)/Ⓒme)' "
    "}",
    // All eqns modified INPUT: λp_nm;θ_° OUTPUT: K_J;γ;β;v_m/s;p_(kg*m/s);φ_°
    "Compton Scattering",  "{ "
    "'(λp_nm)-(λ_nm)=Ⓒλc*(1-COS(θ_°))' "
    "'K_eV=Ⓒh*Ⓒc/(λ_nm)-Ⓒh*Ⓒc/(λp_nm)' "
    "'K_eV=((γ-1))*Ⓒme*Ⓒc²' "
    "'γ²=1/(1-β²)' "
    "'β=(v_(m/s))/Ⓒc' "
    "'Eph_eV=Ⓒh*Ⓒc/(λ_nm)' "
    "'Epph_eV=Ⓒh*Ⓒc/(λp_nm)' "
    "'p_(kg*m/s)=γ*Ⓒme*(v_m/s)' "
    "'0=Ⓒh/(λp_nm)*SIN(θ_°)-(p_(kg*m/s))*SIN(φ_°)' "
    "}",

     //Ref to Bragg's law  https://en.wikipedia.org/wiki/Bragg%27s_law 	http://hyperphysics.phy-astr.gsu.edu/hbase/quantum/bragg.html
    "De Broglie Wave",  "{ "
    "'(λ_nm)=Ⓒh/(p_(kg*m/s))' "
    "'(K_eV)=(p_(kg*m/s))²/(2*(m_kg))' "
    "'(p_(kg*m/s))=(m_kg)*(v_(m/s))' "
    "'2*(d_nm)*SIN(θ_°)=n*(λ_nm)' "
    "}",
    // Modified Eph to Eph_eV; adding last eqn
    "Bohr Atomic Model",  "{ "
    "'(Eph_eV)=Ⓒh*(f_Hz)' "
    "'Ⓒh*(f_Hz)=(Enp_eV)-(En_eV)' "
    "'(En_eV)=-(Ⓒme*Ⓒqe^4*Z²)/(8*Ⓒε₀²*Ⓒh²)*(1/n²)' "
    "'(Enp_eV)=-(Ⓒme*Ⓒqe^4*Z²)/(8*Ⓒε₀²*Ⓒh²)*(1/np²)' "
    "'(r_m)=n²/Z*(Ⓒε₀*Ⓒh²)/(Ⓒπ*Ⓒme*Ⓒqe²)' "
    "'(λ_nm)=Ⓒc/(f_Hz)' "
    "}",

    // ------------------------------------------------------------------------
    "Nuclear Physics", nullptr,
    // ------------------------------------------------------------------------
    //T#:* 52 var 47 eqns 14 sims 7 secs
    // We need activity units : 1 Bq = 1 disintegration/s & 1 Ci = 3.7e10 Bq
    "Radioactivity",  "{ "
    "'(Thl_s)=ln(2)/(λ_s^-1)' "
    "'abtot=abparent*abdaughter' "
    "'N0=abtot*(m_kg)*ⒸNA/(MW_(g/mol))' "
    "'N=N0*exp(-(λ_s^-1)*(t_s))' "
    "'(A0_Bq)=(λ_s^-1)*N0' "
    "'(A_Bq)=(A0_Bq)*exp(-(λ_s^-1)*(t_s))' "
    "}",

    // We need the constant for the mass of the neutral H atom
    // mH = 1.007825031898 u
    // The definition of the mass unit u need to be revised:
    // u = 1.6605390689 2e-27 kg
    "Radius & Binding Energy",  "{ "
    "'A=N+Z' "
    "'(R_m)=(1.2e-15_m)*A^(1/3)' "
    "'(V_m^3)=4/3*Ⓒπ*R^3' "
    "'(EB_MeV)=(Z*ⒸmH+N*Ⓒmn-(mX_u))*Ⓒc²' "
    "'(EBse_MeV)=(1_MeV)*(15.75*A-17.8*A^(2/3)-0.711*Z*(Z-1)/A^(1/3)-23.7*(A-2*Z)²/A+11.18/(A^(1/2))*IFTE((Z mod 2=0)and(N mod 2=0);1;IFTE((Z mod 2=1)and(N mod 2=1);-1;0)))' "
    "}",
    //New energy & kinematic eqns
    "α Decay",  "{ "
    "'A=N+Z' "
    "'(Δm_u)=((mX_u)-(mY_u)-ⒸmHe)' "
    "'(Qα_MeV)=(Δm_u)*Ⓒc²' "
    "'(Kα_MeV)=(Qα_MeV)/(ⒸmHe/(mX_u)+1)' "
    "'γ=(Kα_MeV)/((ⒸmHe)*Ⓒc²)+1' "
    "'β²=(1-1/γ²)' "
    "'(AYα)=(AXα)-4' "
    "'(ZYα)=(ZXα)-2' "
    "}",
    //New energy & kinematic eqns
    "β⊖ Decay",  "{ "
    "'A=N+Z' "
    "'(Δm_u)=((mX_u)-(mY_u))' "
    "'(Qβ⊖_MeV)=(Δm_u)*Ⓒc²' "
    "'(Kmax_MeV)=(Qβ⊖_MeV)' "
    "'γmax=(Kmax_MeV)/(Ⓒme*Ⓒc²)+1' "
    "'βmax²=(1-1/γmax²)' "
    "'(AYβ⊖)=(AXβ⊖)' "
    "'(ZYβ⊖)=(ZXβ⊖)+1' "
    "}",
    //New energy & kinematic eqns
    "β⊕ Decay",  "{ "
    "'A=N+Z' "
    "'(Δm_u)=((mX_u)-(mY_u)-2*Ⓒme)' "
    "'(Qβ⊕_MeV)=(Δm_u)*Ⓒc²' "
    "'(Kmax_MeV)=(Qβ⊕_MeV)' "
    "'γmax=(Kmax_MeV)/(Ⓒme*Ⓒc²)+1' "
    "'βmax²=(1-1/γmax²)' "
    "'(AYβ⊕)=(AXβ⊕)' "
    "'(ZYβ⊕)=(ZXβ⊕)-1' "
    "}",

    "General Nuclear Reaction",  "{ "
    "'A=N+Z' "
    "'(Δm_u)=((ma_u)+(mX_u)-(mY_u)-(mb_u))' "
    "'(Q_MeV)=(Δm_u)*Ⓒc²' "
    "'(ΔKtot_MeV)=(Q_MeV)' "
    "'(Aa)+(AX)=(AY)+(Ab)' "
    "'(Za)+(ZX)=(ZY)+(Zb)' "
    "}",

    "Fission Reaction",  "{ "
    "'A=N+Z' "
    "'(Δm_u)=(Ⓒmn+(mX_u)-(mY1_u)-(mY2_u)-nn*Ⓒmn)' "
    "'(Q_MeV)=(Δm_u)*Ⓒc²' "
    "'(ΔKtot_MeV)=(Q_MeV)' "
    "'1+(AX)=(AY1)+(AY2)+nn' "
    "'(ZX)=(ZY1)+(ZY2)' "
    "}",


    "Finance", nullptr,
    "TVMBeg", "'(1+I%Yr/(100*PYr))*(100*PYr)/I%Yr*Pmt*(1-(1+I%Yr/(100*PYr))^(-n))+FV*(1+I%Yr/(100*PYr))^(-n)+PV'",
    "TVMEnd", "'(100*PYr)/I%Yr*Pmt*(1-(1+I%Yr/(100*PYr))^(-n))+FV*(1+I%Yr/(100*PYr))^(-n)+PV'",

    // As of 24-11-12: Total 695 vars, 614 eqns, 163 sims in 18 sections (eqns: 614/315=1.95; vars: 693/397=1.75 )
    // As of 24-12-11: Total 724 vars, 644 eqns, 182 sims in 158 subsections, 18 sections (644/315=2.04; 724/397=1.82)
    // As of 24-12-18: Total 725 vars, 669 eqns, 188 sims in 158 subsections, 18 sections (669/315=2.12; 725/397=1.83)
};
//   clang-format on


static runtime &invalid_equation_error()
// ----------------------------------------------------------------------------
//    Return the error message for invalid equations
// ----------------------------------------------------------------------------
{
    return rt.invalid_equation_error();
}


static symbol_p equation_label(symbol_r sym)
// ----------------------------------------------------------------------------
//   Simplify equations to show then in menu label
// ----------------------------------------------------------------------------
{
    if (sym)
    {
        size_t   len    = 0;
        utf8     source = sym->value(&len);
        if (object_p obj = object::parse(source, len))
            if (expression_p expr = obj->as<expression>())
                if (symbol_p ssym = expr->as_symbol(false))
                    return ssym;
    }
    return sym;
}


static bool show_builtin_equations()
// ----------------------------------------------------------------------------
//   Show the builtin equations
// ----------------------------------------------------------------------------
{
    return Settings.ShowBuiltinEquations();
}


const equation::config equation::equations =
// ----------------------------------------------------------------------------
//  Define the configuration for the equations
// ----------------------------------------------------------------------------
{
    .menu_help      = "",
    .help           = "",
    .prefix         = L'Ⓔ',
    .type           = ID_equation,
    .first_menu     = ID_EquationsMenu00,
    .last_menu      = ID_EquationsMenu99,
    .name           = ID_EquationName,
    .value          = ID_EquationValue,
    .command        = ID_EquationSolver,
    .file           = "config/equations.csv",
    .library        = "library",
    .builtins       = basic_equations,
    .nbuiltins      = sizeof(basic_equations) / sizeof(*basic_equations),
    .error          = invalid_equation_error,
    .label          = equation_label,
    .show_builtins  = show_builtin_equations,
    .stack_prefix   = false,
};



// ============================================================================
//
//   Menu implementation
//
// ============================================================================

PARSE_BODY(equation)
// ----------------------------------------------------------------------------
//    Skip, the actual parsing is done in the symbol parser
// ----------------------------------------------------------------------------
{
    return do_parsing(equations, p);
}


EVAL_BODY(equation)
// ----------------------------------------------------------------------------
//   Equations always evaluate to their value
// ----------------------------------------------------------------------------
{
    object_g value = o->value();
    return rt.push(+value) ? OK : ERROR;
}


RENDER_BODY(equation)
// ----------------------------------------------------------------------------
//   Render the equation into the given buffer
// ----------------------------------------------------------------------------
{
    equation_g eq = o;
    do_rendering(equations, o, r);
    if (!r.editing() && Settings.ShowEquationBody())
    {
        if (object_g obj = eq->value())
        {
            r.put(':');
            obj->render(r);
        }
    }
    return r.size();
}


GRAPH_BODY(equation)
// ----------------------------------------------------------------------------
//   Render "normally"
// ----------------------------------------------------------------------------
{
    equation_g eq = o;
    if (Settings.ShowEquationBody())
    {
        if (object_g val = eq->value())
        {
            size_t namelen = 0;
            utf8 name = eq->name(&namelen);
            if (symbol_g namesym = symbol::make(name, namelen))
            {
                if (grob_g valg = val->graph(g))
                {
                    coord vv = g.voffset;
                    g.voffset = 0;
                    if (grob_g nameg = object::do_graph(+namesym, g))
                    {
                        coord nv = g.voffset;
                        g.voffset = 0;
                        if (grob_g r = expression::infix(g,
                                                         nv, nameg,
                                                         0, ":",
                                                         vv, valg))
                            return r;
                    }
                }
            }
        }
    }
    return object::do_graph(+eq, g);
}


HELP_BODY(equation)
// ----------------------------------------------------------------------------
//   Help topic for equations
// ----------------------------------------------------------------------------
{
    return o->do_instance_help(equation::equations);
}


MENU_BODY(equation_menu)
// ----------------------------------------------------------------------------
//   Build a equations menu
// ----------------------------------------------------------------------------
{
    return o->do_submenu(equation::equations, mi);
}


HELP_BODY(equation_menu)
// ----------------------------------------------------------------------------
//   Show the help for the given equation menu
// ----------------------------------------------------------------------------
{
    return o->do_menu_help(equation::equations, o);
}


MENU_BODY(EquationsMenu)
// ----------------------------------------------------------------------------
//   The equations menu is dynamically populated
// ----------------------------------------------------------------------------
{
    return equation::do_collection_menu(equation::equations, mi);
}


utf8 equation_menu::name(id type, size_t &len)
// ----------------------------------------------------------------------------
//   Return the name for a menu entry
// ----------------------------------------------------------------------------
{
    return do_name(equation::equations, type, len);
}


COMMAND_BODY(EquationName)
// ----------------------------------------------------------------------------
//   Put the name of a equation on the stack
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (constant_p cst = equation::do_key(equation::equations, key))
        if (equation_p eq = cst->as<equation>())
            if (rt.push(eq))
                return OK;
    if (!rt.error())
        rt.type_error();
    return ERROR;
}


INSERT_BODY(EquationName)
// ----------------------------------------------------------------------------
//   Put the name of a equation in the editor
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    return ui.insert_softkey(key, " Ⓔ", " ", false);
}


HELP_BODY(EquationName)
// ----------------------------------------------------------------------------
//   Put the help for a given equation name
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    rt.command(o);
    if (constant_p cst = equation::do_key(equation::equations, key))
        if (equation_p eq = cst->as<equation>())
            return eq->help();
    return utf8("Equations");
}


COMMAND_BODY(EquationValue)
// ----------------------------------------------------------------------------
//   Put the value of a equation on the stack
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (constant_p cst = equation::do_key(equation::equations, key))
        if (equation_p eq = cst->as<equation>())
            if (object_p value = eq->value())
                if (rt.push(value))
                    return OK;
    if (!rt.error())
        rt.type_error();
    return ERROR;
}


INSERT_BODY(EquationValue)
// ----------------------------------------------------------------------------
//   Insert the value of a equation
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (object_p cstobj = equation::do_key(equation::equations, key))
        if (equation_p eq = cstobj->as<equation>())
            if (object_p value = eq->value())
                return ui.insert_object(value, " ", " ");
    return ERROR;
}


HELP_BODY(EquationValue)
// ----------------------------------------------------------------------------
//   Put the help for a given equation value
// ----------------------------------------------------------------------------
{
    return EquationName::do_help(nullptr);
}



COMMAND_BODY(EquationSolver)
// ----------------------------------------------------------------------------
//   Solve for a given equation
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (constant_p cst = equation::do_key(equation::equations, key))
        if (equation_p eq = cst->as<equation>())
            if (directory::store_here(static_object(ID_Equation), eq))
                if (const SolvingMenu *smenu =
                    object::static_object(ID_SolvingMenu)->as<SolvingMenu>())
                    return smenu->object::evaluate();
    if (!rt.error())
        rt.type_error();
    return ERROR;
}


INSERT_BODY(EquationSolver)
// ----------------------------------------------------------------------------
//   Insert the code in a program to solve a library equation
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (constant_p cst = equation::do_key(equation::equations, key))
        if (equation_p eq = cst->as<equation>())
            if (rt.push(eq))
                return ui.insert_object(eq, " ", " StEQ SolvingMenu ");
    return ERROR;
}


HELP_BODY(EquationSolver)
// ----------------------------------------------------------------------------
//   Put the help for a given equation value
// ----------------------------------------------------------------------------
{
    return EquationName::do_help(nullptr);
}


COMMAND_BODY(LibEq)
// ----------------------------------------------------------------------------
//   Evaluate a library equation
// ----------------------------------------------------------------------------
{
    return equation::lookup_command(equation::equations, false);
}



// ============================================================================
//
//    Assignment operations
//
// ============================================================================

PARSE_BODY(assignment)
// ----------------------------------------------------------------------------
//    Try to parse this as an assignment
// ----------------------------------------------------------------------------
{
    // Assignments can only be parsed in non-algebraic mode
    if (p.precedence)
        return SKIP;

    // Check if we have a name
    algebraic_g name = p.out ? p.out->as_extended_algebraic() : nullptr;
    if (!name)
        return SKIP;

    size_t max = p.length;
    if (!max)
        return SKIP;

    // First character must be compatible with a unit
    size_t  offs  = 0;
    size_t  noffs = 0;
    unicode cp    = p.separator;
    if (cp != '=')
        return SKIP;

    // Parse the body
    offs = utf8_next(p.source, offs, max);
    size_t   vsz   = max - offs;
    object_p vobj  = parse(p.source + offs, vsz);
    if (!vobj)
        return ERROR;
    algebraic_g value = vobj->as_extended_algebraic();
    if (!value)
    {
        rt.type_error();
        return ERROR;
    }
    offs += vsz;

    if (name->type() != ID_symbol)
    {
        rt.invalid_name_error().source(p.source + noffs);
        return ERROR;
    }

    p.out    = assignment::make(name, value);
    p.length = offs;
    return p.out ? OK : ERROR;
}


RENDER_BODY(assignment)
// ----------------------------------------------------------------------------
//   Do not emit quotes around assignment objects
// ----------------------------------------------------------------------------
{
    algebraic_g name  = o->name();
    algebraic_g value = o->value();
    if (r.expression())
    {
        // Inside an expression, use the old HP syntax
        value->render(r);
        r.put(unicode(L'▶'));
        name->render(r);
    }
    else
    {
        name->render(r);
        r.put('=');
        value->render(r);
    }
    return r.size();
}


GRAPH_BODY(assignment)
// ----------------------------------------------------------------------------
//   Render assignments graphically
// ----------------------------------------------------------------------------
{
    algebraic_g name  = o->name();
    algebraic_g value = o->value();
    grob_g      ng    = name->graph(g);
    coord       nv    = g.voffset;
    grob_g      vg    = value->graph(g);
    coord       vv    = g.voffset;
    if (g.expression)
        ng = expression::infix(g, vv, vg, 0, "▶", nv, ng);
    else
        ng = expression::infix(g, nv, ng, 0, "=", vv, vg);
    return ng;
}


EVAL_BODY(assignment)
// ----------------------------------------------------------------------------
//   Evaluate the value, assign it to the name, and return evaluated value
// ----------------------------------------------------------------------------
{
    assignment_g asn = o;

    symbol_g name = asn->name()->as<symbol>();
    if (!name)
    {
        rt.invalid_name_error();
        return ERROR;
    }

    algebraic_g value = asn->value();
    algebraic_p evalue = value->evaluate();
    if (!evalue)
        return ERROR;
    if (evalue != value)
    {
        value = evalue;
        if (Settings.PushEvaluatedAssignment())
            asn = assignment::make(+name, evalue);
    }

    if (!directory::store_here(name, value))
        return ERROR;

    if (!rt.push(+asn))
        return ERROR;
    return OK;
}


HELP_BODY(assignment)
// ----------------------------------------------------------------------------
//   Help topic for assignments
// ----------------------------------------------------------------------------
{
    return utf8("Assignments");
}
