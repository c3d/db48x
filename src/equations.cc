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
    // 14 eqns
    "Elastic Buckling",  "{ "
    "  '(Pcr_kN)=(Ⓒπ²*(E_kPa)*(A_cm^2))/sq((K*(L_m))/(r_cm))' "
    "  '(Pcr_kN)=(Ⓒπ²*(E_kPa)*(I_mm^4))/sq(K*(L_m))' "
    "  '(σcr_kPa)=(Pcr_kN)/(A_cm^2)' "
    "  '(r_cm)=sqrt((I_mm^4)/(A_cm^2))' "
    "}",

    "Eccentric Columns", "{"
    "  '(σmax_kPa)=((P_kN)/(A_cm^2))*(1+((ε_cm)*(c_cm))/sq(r_cm)*inv(cos(K/2*((L_m)/(r_cm))*sqrt((P_kN)/((E_kPa)*(A_cm^2)))*1_r)))'"
    "  '(r_cm)=sqrt((I_mm^4)/(A_cm^2))'"
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
    // 67 eqns

    "Coulomb’s Law & E Field",  "{ "
    "  '(F_N)=1/(4*Ⓒπ*Ⓒε0*εr)*((q1_C)*(q2_C)/(r_m)^2)' "
    "  '(Er_(N/C))=(F_N)/(qtest_C)' "
    "}",

    "E Field Infinite Line",  "{ "
    "  '(Er_(N/C))=1/(2*Ⓒπ*Ⓒε0*εr)*((λ_(C/m))/(r_m))' "
    "  '(λ_(C/m))=(Q_C)/(L_m)' "
    "}",

    "E Field Finite Line",  "{ "
    "  '(Er_(N/C))=1/(4*Ⓒπ*Ⓒε0*εr)*((λ_(C/m))/(r_m)*(SIN(θ1_r)-SIN(θ2_r)))' "
    "  '(λ_(C/m))=(Q_C)/(L_m)' "
    "}",

    "E Field Infinite Plate",  "{ "
    "  '(Ep_(N/C))=(σ_(μC/cm^2))/(2*Ⓒε0*εr)' "
    "  '(σ_(μC/cm^2))=(Q_μC)/(A_(cm^2))' "
    "}",

    "Ohm’s Law & Power",  "{ "
    "  '(V_V)=(I_A)*(R_Ω)' "
    "  '(P_W)=(V_V)*(I_A)' "
    "  '(P_W)=(I_A)^2*(R_Ω)' "
    "  '(P_W)=(V_V)^2/(R_Ω)' "
    "}",

    "Volt Divider",  "{ "
    "  '(V1_V)=(V_V)*((R1_Ω)/((R1_Ω)+(R2_Ω)))' "
    "}",

    "Current Divider",  "{ "
    "  '(I1_A)=(I_A)*((R2_Ω)/((R1_Ω)+(R2_Ω)))' "
    "}",

    "Wire Resistance",  "{ "
    "  '(R_Ω)=(ρ_(Ω*m))*(L_m)/(A_(m^2))' "
    "}",

    "Resistivity & Conductivity",  "{ "
    "  '(ρ_(Ω*m))=(ρ0_(Ω*m))*(1+(αT_K^-1)*((T_K)-(T0_K)))' "
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
    "  '(E_J)=(1/2)*(C_μF)*(V_V)^2' "
    "  '(E_J)=(1/2)*(q_μC)*(V_V)' "
    "  '(E_J)=(q_μC)^2/(2*(C_μF))' "
    "}",

    "Volumic Density Electric Energy",  "{ "
    "  '(uE_(J/m^3))=(1/2)*Ⓒε0*εr*(E_(V/m))^2' "
    "}",

    "Inductive Energy",  "{ "
    "  '(E_J)=(1/2)*(L_mH)*(I_A)^2' "
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
    "  '(V_V)=-(L_mH)*((ΔI_A)/(Δt_μs))' "
    "  '(ΔI_A)=(If_A)-(Ii_A)' "
    "  '(Δt_μs)=(tf_μs)-(ti_μs)' "
    "}",

    "RC Transient",  "{ "
    "  '(V_V)=(Vf_V)-((Vf_V)-(Vi_V))*EXP((-(t_ms))/((R_Ω)*(C_F)))' "
    "}",

    "RL Transient",  "{ "
    "  '(I_A)=1/(R_Ω)*((Vf_V)-((Vf_V)-(Vi_V))*EXP((-(t_μs))/((R_Ω)*(L_mH))))' "
    "}",

    "Resonant Frequency",  "{ "
    "  'Qs=1/(R_Ω)*√((L_mH)/(C_μF))' "
    "  'Qp=(R_Ω)*√((C_μF)/(L_mH))' "
    "  '(ω0_(r/s))=2*(Ⓒπ_r)*(f0_Hz)' "
    "  '(ω0_(r/s))=1/√((L_mH)*(C_μF))' "
    "}",

    "Plate Capacitor",  "{ "
    "  '(C_μF)=Ⓒε0*εr*(A_(cm^2))/(d_cm)' "
    "  '(ΔV_V)=(Ein_(V/m))*(d_cm)' "
    "  '(Ein_(N/C))=(σ_(μC/cm^2))/(Ⓒε0*εr)' "
    "  '(σ_(μC/m^2))=(Q_μC)/(A_(cm^2))' "
    "}",

    "Cylindrical Capacitor",  "{ "
    "  '(C_μF)=2*Ⓒπ*Ⓒε0*εr*(L_cm)/(LN((ro_cm)/(ri_cm)))' "
    "  '(ΔV_V)=(Q_μC)*(LN((ro_cm)/(ri_cm)))/(2*Ⓒπ*Ⓒε0*εr*(L_cm))' "
    "}",

    "Solenoid Inductance",  "{ "
    "  '(L_mH)=Ⓒμ0*μr*(n_cm^-1)^2*(A_(cm^2))*(h_cm)' "
    "}",

    "Toroid Inductance",  "{ "
    "  '(L_mH)=Ⓒμ0*μr*N^2*(h_cm)/(2*Ⓒπ)*LN((ro_cm)/(ri_cm))' "
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
    "  '(vd_(m/s))=(I_A)/((n_(m^-3))*Ⓒqe*(A_(cm^2)))' "
    "  '(J_(A/m^2))=(vd_(m/s))*(ρ_(C/m^3))' "
    "  '(J_(A/m^2))=(σ_(S/m))*(E_(V/m))' "
    "}",

    // Example of the following in
    // https://en.wikipedia.org/wiki/Electron_mobility#Examples
    "Electron & Hole Mobilities",  "{ "
    "  '(J_(A/m^2))=(Je_(A/m^2))+(Jh_(A/m^2))' "
    "  '(Je_(A/m^2))=Ⓒqe*(ne_(m^-3))*(μe_(cm^2/(V*s)))*(E_(V/m))' "
    "  '(Jh_(A/m^2))=Ⓒqe*(nh_(m^-3))*(μh_(cm^2/(V*s)))*(E_(V/m))' "
    "  '(μe_(cm^2/(V*s)))=Ⓒqe*(τc_s)/(meeff_kg)' "
    "  '(μh_(cm^2/(V*s)))=Ⓒqe*(τc_s)/(mheff_kg)' "
    "  '(σ_(S/m))=Ⓒqe*((μe_(cm^2/(V*s)))*(ne_(m^-3))+(μh_(cm^2/(V*s)))*(nh_(m^-3)))' "
    "}",

    // ------------------------------------------------------------------------
    "Fluids", nullptr,
    // ------------------------------------------------------------------------
    // 29 eqns in Fluids
    // The first is the only equ that does work in FLUIDS
    "Pressure at Depth",  "{ "
    "  '(P_kPa)=(P0_atm)+(ρ_(kg/m^3))*Ⓒg*(h_m)' "
    "}",

    // WARNING Errors in HP50G_AUR.pdf for eqns 7 & 8
    // ("*" instead of"-") but not in HP50G
    // WARNING doesn't work on the native HP50G
    "Bernoulli Equation",  "{ "
    "  '(ΔP_Pa)/(ρ_(kg/m^3))+((v2_(m/s))^2-(v1_(m/s))^2)/2+Ⓒg*(Δy_m)=0' "
    "  '(ΔP_Pa)/(ρ_(kg/m^3))+((v2_(m/s))^2)*(1-((A2_(m^2))/(A1_(m^2)))^2)/2+Ⓒg*(Δy_m)=0' "
    "  '(ΔP_Pa)/(ρ_(kg/m^3))+((v1_(m/s))^2)*(((A1_(m^2))/(A2_(m^2)))^2-1)/2+Ⓒg*(Δy_m)=0' "
    "  '(ΔP_Pa)=(P2_Pa)-(P1_Pa)' "
    "  '(Δy_m)=(y2_m)-(y1_m)' "
    "  '(M_(kg/s))=(ρ_(kg/m^3))*(Q_(m^3/s))' "
    "  '(Q_(m^3/s))=(A2_(m^2))*(v2_(m/s))' "
    "  '(Q_(m^3/s))=(A1_(m^2))*(v1_(m/s))' "
    "  '(A1_(m^2))=(Ⓒπ*(D1_m)^2)/4' "
    "  '(A2_(m^2))=(Ⓒπ*(D2_m)^2)/4' "
    "}",

    // WARNING doesn't work on the native HP50G
    "Flow with Losses",  "{ "
    "  '(M_(kg/s))*((ΔP_Pa)/(ρ_(kg/m^3))+((v2_(m/s))^2-(v1_(m/s))^2)/2+Ⓒg*(Δy_m)+(hL_((m/s)^2)))=(W_W)' "
    "  '(M_(kg/s))*((ΔP_Pa)/(ρ_(kg/m^3))+(((v2_(m/s))^2)*(1-((A2_(m^2))/(A1_(m^2)))^2))/2+Ⓒg*(Δy_m)+(hL_((m/s)^2)))=(W_W)' "
    "  '(M_(kg/s))*((ΔP_Pa)/(ρ_(kg/m^3))+(((v1_(m/s))^2)*(((A1_(m^2))/(A2_(m^2)))^2-1))/2+Ⓒg*(Δy_m)+(hL_((m/s)^2)))=(W_W)' "
    "  '(ΔP_Pa)=(P2_Pa)-(P1_Pa)' "
    "  '(Δy_m)=(y2_m)-(y1_m)' "
    "  '(M_(kg/s))=(ρ_(kg/m^3))*(Q_(m^3/s))' "
    "  '(Q_(m^3/s))=(A2_(m^2))*(v2_(m/s))' "
    "  '(Q_(m^3/s))=(A1_(m^2))*(v1_(m/s))' "
    "  '(A1_(m^2))=(Ⓒπ*(D1_m)^2)/4' "
    "  '(A2_(m^2))=(Ⓒπ*(D2_m)^2)/4' "
    "}",

    // WARNING: Missing or undefined parameter "f" in eqn 1
    // WARNING doesn't work on the native HP50G
    // f represents probably the Fanning friction factor which is near 0 with
   // the high RE number of the example the absolute roughness coefficient ϵ_in
   // is defined as a variable in both the HP50G and the manual but here it is
   // absent of all actual eqns
    "Flow In Full Pipes",  "{ "
    "  '(ρ_(kg/m^3))*((Ⓒπ*(D_m)^2)/4)*(vavg_(m/s))*((ΔP_Pa)/(ρ_(kg/m^3))+Ⓒg*(Δy_m)+(vavg_(m/s))^2*(2*f*((L_m)/(D_m))+ΣK/2))=(W_W)' "
    "  'f=FANNING((ε_m)/(D_m);Reynolds)' "
    "  '(ΔP_Pa)=(P2_Pa)-(P1_Pa)' "
    "  '(Δy_m)=(y2_m)-(y1_m)' "
    "  '(M_(kg/s))=(ρ_(kg/m^3))*(Q_(m^3/s))' "
    "  '(Q_(m^3/s))=(A_(m^2))*(vavg_(m/s))' "
    "  '(A_(m^2))=Ⓒπ*(D_m)^2/4' "
    "  'Reynolds=(D_m)*(vavg_(m/s))*(ρ_(kg/m^3))*(μ_(kg/(m*s)))' "
    "  '(n_(m^2/s))=(μ_(kg/(m*s)))/(ρ_(kg/m^3))' "
    "}",

    // ------------------------------------------------------------------------
    "Forces And Energy", nullptr,
    // ------------------------------------------------------------------------
    // 33 eqs

    "Linear Mechanics",  "{ "
    "'(F_N)=(m_kg)*(a_(m/s^2))' "
    "'(Ki_J)=1/2*(m_kg)*(vi_(m/s))^2' "
    "'(Kf_J)=1/2*(m_kg)*(vf_(m/s))^2' "
    "'(W_J)=(F_N)*(x_m)' "
    "'(W_J)=(Kf_J)-(Ki_J)' "
    "'(P_W)=(F_N)*(v_(m/s))' "
    "'(Pavg_W)=(W_J)/(t_s)' "
    "'(vf_(m/s))=(vi_(m/s))+(a_(m/s^2))*(t_s)' "
    "}",

    "Angular Mechanics",  "{ "
    "'(τ_(N*m))=(I_(kg*m^2))*(α_(r/s^2))' "
    "'(Ki_J)=1/2*(I_(kg*m^2))*(ωi_(r/s))^2' "
    "'(Kf_J)=1/2*(I_(kg*m^2))*(ωf_(r/s))^2' "
    "'(W_J)=(τ_(N*m))*(θ_°)' "
    "'(W_J)=(Kf_J)-(KI_J)' "
    "'(P_W)=(τ_(N*m))*(ω_(r/s))/(1_r)' "
    "'(Pavg_W)=(W_J)/(t_s)' "
    "'(ωf_(r/s))=(ωi_(r/s))+(α_(r/s^2))*(t_s)' "
    "'(at_(m/s^2))=(α_(r/s^2))/(1_r)*(r_m)' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(N_Hz)' "
    "'(ωi_(r/s))=2*(Ⓒπ_r)*(Ni_Hz)' "
    "'(ωf_(r/s))=2*(Ⓒπ_r)*(Nf_Hz)' "
    "}",

    "Centripetal Force",  "{ "
    "'(F_N)=(m_kg)*((ω_(r/s))/(1_r))^2*(r_m)' "
    "'(ω_(r/s))=(v_(m/s))*(1_r)/(r_m)' "
    "'(ar_(m/s^2))=(v_(m/s))^2/(r_m)' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(N_Hz)' "
    "}",

    "Hooke’s Law",  "{ "
    "'(F_N)=-(k_(N/m))*(x_m)' "
    "'(W_J)=-1/2*(k_(N/m))*(x_m)^2' "
    "}",

    "1D Elastic Collisions",  "{ "
    "'(v1f_(m/s))=(((m1_kg)-(m2_kg))/((m1_kg)+(m2_kg)))*(v1i_(m/s))' "
    "'(v2f_(m/s))=((2*(m1_kg))/((m1_kg)+(m2_kg)))*(v1i_(m/s))' "
    "}",

    "Gravitation Law",  "{ "
    "'(F_N)=ⒸG*((m1_kg)*(m2_kg))/(r_m)^2' "
    "'(W_J)=(UGf_J)-(UGi_J)' "
    "'(UGf_J)=-ⒸG*((m_kg))/(rf_m)' "
    "'(UGi_J)=-ⒸG*((m_kg))/(ri_m)' "
    "}",

    "Relativity Mass Energy",  "{ "
    "'(E_J)=(m_kg)*Ⓒc^2' "
    "}",


    // ------------------------------------------------------------------------
    "Gases", nullptr,
    // ------------------------------------------------------------------------
    // 18 eqns
    "Ideal Gas",  "{ "
    "'(P_atm)*(V_l)=(n_gmol)*ⒸR*(T_°C)' "
    "'(m_kg)=(n_gmol)*(MW_(g/gmol))' "
    "}",

    "Ideal Gas Law Change",  "{ "
    "'((Pf_Pa)*(Vf_l))/(Tf_K)=((Pi_Pa)*(Vi_l))/(Ti_K)' "
    "}",

    "Isothermal Expansion",  "{ "
    "'(W_J)=(n_gmol)*ⒸR*(T_°C)*LN((Vf_l)/(Vi_l))' "
    "'(m_kg)=(n_gmol)*(MW_(g/gmol))' "
    "}",

    "Polytropic Processes",  "{ "
    "'(Pf_atm)/(Pi_atm)=((Vf_(ft^3))/(Vi_(ft^3)))^(-n)' "
    "'(Tf_°F)/(Ti_°F)=((Pf_atm)/(Pi_atm))^((n-1)/n)' "
    "}",

    "Isentropic Flow",  "{ "
    "'(T_K)/(T0_K)=2/(2+(k-1)*M^2)' "
    "'(P_kPa)/(P0_kPa)=((T_°C)/(T0_°C))^(k/(k-1))' "
    "'(ρ_(kg/m^3))/(ρ0_(kg/m^3))=((T_K)/(T0_K))^(1/(k-1))' "
    "'(A_(cm^2))/(At_(cm^2))=(1/M)*(2/(k+1)*(1+(k-1)/2*M^2))^((k+1)/(2*(k-1)))' "
    "}",

    "Real Gas Law",  "{ "
    "'(P_atm)*(V_l)=(n_gmol)*Z*ⒸR*(T_°C)' "
    "'(m_kg)=(n_gmol)*(MW_(g/gmol))' "
    "'Tr=(T_K)/(Tc_K)' "
    "'Pr=(P_Pa)/(Pc_Pa)' "
    "'ρr=0.27*(Pr/(Z*Tr))' "
    "'Z=1+(0.31506237-1.04670990/Tr-0.57832729/Tr^3)*ρr+(0.53530771-0.61232032/Tr)*ρr^2+0.61232032*0.10488813*ρr^5/Tr+0.68157001*ρr^2/Tr^3*(1+0.68446549*ρr^2)*exp(-0.68446549*ρr^2)' "
    "}",

    "Real Gas State Change",  "{ "
    "'((Pf_atm)*(Vf_l))/(Zf*(Tf_°C))=((Pi_atm)*(Vi_l))/(Zi*(Ti_°C))' "
    "'Tri=(Ti_K)/(Tc_K)' "
    "'Pri=(Pi_Pa)/(Pc_Pa)' "
    "'ρri=0.27*(Pri/(Zi*Tri))' "
    "'Zi=1+(0.31506237-1.04670990/Tri-0.57832729/Tri^3)*ρri+(0.53530771-0.61232032/Tri)*ρri^2+0.61232032*0.10488813*ρri^5/Tri+0.68157001*ρri^2/Tri^3*(1+0.68446549*ρri^2)*exp(-0.68446549*ρri^2)' "
    "'Trf=(Tf_K)/(Tc_K)' "
    "'Prf=(Pf_Pa)/(Pc_Pa)' "
    "'ρrf= 0.27*(Prf/(Zf*Trf))' "
    "'Zf=1+(0.31506237-1.04670990/Trf-0.57832729/Trf^3)*ρrf+(0.53530771-0.61232032/Trf)*ρrf^2+0.61232032*0.10488813*ρrf^5/Trf+0.68157001*ρrf^2/Trf^3*(1+0.68446549*ρrf^2)*exp(-0.68446549*ρrf^2)' "
    "}",

    "Kinetic Theory",  "{ "
    "'(P_kPa)=((n_gmol)*(MW_(g/gmol))*(vrms_(m/s))^2)/(3*(V_l))' "
    "'(vrms_(m/s))=√((3*ⒸR*(T_°C))/(MW_(g/gmol)))' "
    "'(λ_nm)=1/(√(2)*Ⓒπ*((n_gmol)*ⒸNA)/(V_l)*(d_nm)^2)' "
    "'(m_kg)=(n_gmol)*(MW_(g/gmol))' "
    "}",

    // ------------------------------------------------------------------------
    "Heat Transfer", nullptr,
    // ------------------------------------------------------------------------
    // 17 eqns
    "Heat Capacity",  "{ "
    "'(Q_kJ)=(m_kg)*(c_(kJ/(kg*K)))*(ΔT_°C)' "
    "'(Q_kJ)=(m_kg)*(c_(kJ/(kg*K)))*((Tf_°C)-(Ti_°C))' "
    "}",

    "Thermal Expansion",  "{ "
    "'(δ_cm)=(α_(1/°C))*(L_m)*(ΔT_°C)' "
    "'(δ_cm)=(α_(1/°C))*(L_m)*((Tf_°C)-(Ti_°C))' "
    "}",

    "Conduction",  "{ "
    "'(qr_W)=((k_(W/(m*K)))*(A_(m^2))/(L_m))*(ΔT_°C)' "
    "'(qr_W)=((k_(W/(m*K)))*(A_(m^2))/(L_m))*((Th_°C)-(Tc_°C))' "
    "}",

    "Convection",  "{ "
    "'(qr_W)=(h_(W/(m^2*K)))*(A_(m^2))*(ΔT_°C)' "
    "'(qr_W)=(h_(W/(m^2*K)))*(A_(m^2))*((Th_°C)-(Tc_°C))' "
    "}",

    "Conduction & Convection",  "{ "
    "'(qr_W)=(A_(m^2))*(ΔT_°C)/(1/(h1_(W/(m^2*K)))+(L1_cm)/(k1_(W/(m*K)))+(L2_cm)/(k2_(W/(m*K)))+(L3_cm)/(k3_(W/(m*K)))+1/(h3_(W/(m^2*K))))' "
    "'(qr_W)=(A_(m^2))*((Th_°C)-(Tc_°C))/(1/(h1_(W/(m^2*K)))+(L1_cm)/(k1_(W/(m*K)))+(L2_cm)/(k2_(W/(m*K)))+(L3_cm)/(k3_(W/(m*K)))+1/(h3_(W/(m^2*K))))' "
    "'(U_(W/(m^2*K)))=(qr_W)/((A_(m^2))*(ΔT_°C))' "
    "'(U_(W/(m^2*K)))=(qr_W)/((A_(m^2))*((Th_°C)-(Tc_°C)))' "
    "}",

    // WARNING The db48x needs the Black-Body Integral function F0λ(T_K,λ_nm)
    // defined in the HP50G (SP50G_AUR 3-82, 5-31)
    "Black Body Radiation",  "{ "
    "'(eb_(W/m^2))=Ⓒσ*(T_°C)^4' "
    "'f=F0λ((λ2_nm);(T_°C))-F0λ((λ1_nm);(T_°C))' "
    "'(eb12_(W/m^2))=f*(eb_(W/m^2))' "
    "'(λmax_nm)*(Tmax_°C)=Ⓒc3' "
    "'(q_W)=(eb_(W/m^2))*(A_(cm^2))' "
    "}",

    // ------------------------------------------------------------------------
    "Magnetism", nullptr,
    // ------------------------------------------------------------------------
    // 14 eqns
    // WARNING both HP50G & HP50G_AUR.pdf used a variables rw absent from all
    // equations
    "Straight Wire Infinite",  "{ "
    "'(B_T)=Ⓒμ0*IFTE((r_m)≤(rw_m);μr*(r_m)*(I_A)/(2*Ⓒπ*(rw_cm)^2);(I_A)/(2*Ⓒπ*(r_cm)))' "
    "}",

    "Straight Wire Finite",  "{ "
    "'(B_T)=Ⓒμ0*IFTE((r_m)≤(rw_m);μr*(r_m)*(I_A)/(2*Ⓒπ*(rw_cm)^2);(I_A)/(2*Ⓒπ*(r_cm)))*(COS(θ1_r)-COS(θ2_r))' "
    "}",

    "Force Between Wires",  "{ "
    "'(Fba_N)=(Ⓒμ0*μr*(Ib_A)*(Ia_A)*(L_cm))/(2*Ⓒπ*(d_m))' "
    "}",

    "B Field In Infinite Solenoid",  "{ "
    "'(B_T)=Ⓒμ0*μr*(I_A)*nl' "
    "}",

    "B Field In Finite Solenoid",  "{ "
    "'(B_T)=(1/2)*Ⓒμ0*μr*(I_A)*nl*(COS(α2_°)-COS(α1_°))' "
    "}",

    "B Field In Toroid",  "{ "
    "'(B_T)=(Ⓒμ0*μr*(I_A)*N)/(2*Ⓒπ)*(2/((ro_cm)+(ri_cm)))' "
    "}",

    "Hall Effect",  "{ "
    "'(VH_V)=((I_A)*(B_T))/((n_(1/m^3))*ABS(q_C))*(L_m)' "
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
    "'(uB_(J/m^3))=(1/(2*Ⓒμ0*μr))*(B_T)^2' "
    "}",

    // ------------------------------------------------------------------------
    "Motion", nullptr,
    // ------------------------------------------------------------------------
    // 22 eqns
    "Linear Motion",  "{ "
    "'(x_m)=(x0_m)+(v0_(m/s))*(t_s)+1/2*(a_(m/s^2))*(t_s)^2' "
    "'(x_m)=(x0_m)+(v_(m/s))*(t_s)-1/2*(a_(m/s^2))*(t_s)^2' "
    "'(x_m)=(x0_m)+1/2*((v0_(m/s))+v_(m/s))*(t_s)' "
    "'(v_(m/s))=(v0_(m/s))+(a_(m/s^2))*(t_s)' "
    "}",

//Ref.: https://fr.wikipedia.org/wiki/Pesanteur#cite_ref-10:~:text=sur%20les%20plantes.-,Variation%20en%20fonction%20du%20lieu,-%5Bmodifier%20%7C
    "Object In Free Fall",  "{ "
    "'(y_m)=(y0_m)+(v0_(m/s))*(t_s)-1/2*(gloc_(m/s^2))*(t_s)^2' "
    "'(y_m)=(y0_m)+(v_(m/s))*(t_s)+1/2*(gloc_(m/s^2))*(t_s)^2' "
    "'(v_(m/s))=(v0_(m/s))-(gloc_(m/s^2))*(t_s)' "
    "'(v_(m/s))^2=(v0_(m/s))^2-2*(gloc_(m/s^2))*((y_m)-(y0_m))' "
    "'gloc_(m/s^2)=ⒸG*(Mp_kg)/(r_m)^2' "
    "'gearth_(m/s^2)=9.780327*(1+5.3024E-3*(SIN(φ))^2-5.8E-6*(sin(φ))^2-3.086E-7*(h_m))' "
    "}",

    "Projectile Motion",  "{ "
    "'(x_m)=(x0_m)+(v0_(m/s))*COS(θ0_°)*(t_s)' "
    "'(y_m)=(y0_m)+(v0_(m/s))*SIN(θ0_°)*(t_s)-1/2*Ⓒg*(t_s)^2' "
    "'(vcx_(m/s))=(v0_(m/s))*COS(θ0_°)' "
    "'(vcy_(m/s))=(v0_(m/s))*SIN(θ0_°)-Ⓒg*(t_s)' "
    "'(R_m)=((v0_(m/s))^2)/Ⓒg*SIN(2*(θ0_°))' "
    "'(hmax_m)=((v0_(m/s))^2)/(2*Ⓒg)*(SIN(θ0_r))^2' "
    "'(tf_s)=(2*(v0_(m/s)))/Ⓒg*SIN(θ0_r)' "
    "}",

    "Angular Motion",  "{ "
    "'(θ_°)=(θ0_°)+(ω0_rpm)*(t_s)+1/2*(α_(rpm^2))*(t_s)^2' "
    "'(θ_°)=(θ0_°)+(ω_rpm)*(t_s)-1/2*(α_(rpm^2))*(t_s)^2' "
    "'(θ_°)=(θ0_°)+1/2*((ω0_rpm)+(ω_rpm))*(t_s)' "
    "'(ω_rpm)=(ω0_rpm)+(α_(rpm^2))/(1_turn)*(t_s)' "
    "}",

    "Uniform Circular Motion",  "{ "
    "'(ω_rpm)=(v_(m/s))*(1_r)/(rc_cm)' "
    "'(ar_(m/s^2))=(v_(m/s))^2/(rc_cm)' "
    "'(ω_rpm)=2*(Ⓒπ_r)*(N_rpm)' "
    "}",

    "Terminal Velocity",  "{ "
    "'(vt_(m/s))=√((2*(m_kg)*Ⓒg)/(Cd*(ρ_(kg/m^3))*(Ah_cm^2)))' "
    "'v_(m/s)=(vt_(m/s))*TANH((t_s)*Ⓒg/(vt_(m/s)))' "
    "}",

    "Escape & Orbital Velocity",  "{ "
    "'(ve_(m/s))=√((2*ⒸG*(Mp_kg))/(R_m))' "
    "'(vo_(m/s))=√((ⒸG*(Mp_kg))/(R_m))' "
    "}",


    // ------------------------------------------------------------------------
    "Optics", nullptr,
    // ------------------------------------------------------------------------
    // 29 eqns

    "Refraction Law",  "{ "
    "'n1*SIN(θ1_°)=n2*SIN(θ2_°)' "
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
    "'SIN(θc_°)=n2/n1' "
	"'nf0=Ⓒc/(vf0_(m/s))' "
	"'nf1=Ⓒc/(vf1_(m/s))' "
	"'nf2=Ⓒc/(vf2_(m/s))' "
	"'SIN(θ0_°)=(√(nf1^2-nf2^2))/nf0' "
	"'NA=(√(nf1^2-nf2^2))/nf0'"
    "}",

    "Brewster’s Law",  "{ "
    "'TAN(θB_°)=n2/n1' "
    "'(θB_°)+(θ2_°)=90°' "
    "'n1=Ⓒc/(v1_(m/s))' "
    "'n2=Ⓒc/(v2_(m/s))' "
    "}",

    "Spherical Reflection",  "{ "
    "'1/(u_cm)+1/(v_cm)=1/(f_cm)' "
    "'(f_cm)=(r_cm)/2' "
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
    "'SIN(θr_°)=1.21966989*(λ_nm)/(d_m)' "
    "'TAN(θr_°)=(y_m)/(L_m)' "
    "}",

    "Malus Law",  "{ "
    "'(I_(W/m^2))/(I0_(W/m^2))=(COS(θ_°))^2' "
    "'(Ix_(W/m^2))/I0x_(W/m^2)=((fx_Hz)/(fx0_Hz))*(1+Ⓒλc/Ⓒc*((fx0_Hz)-(fx_Hz)))+(COS(θ_°))^2' "
    "'(I0_(W/m^2))=(1/(2*Ⓒμ0*Ⓒc))*(E0_(V/m))^2' "
    "}",

    "2 Slits Young Interference",  "{ "
    "'(I_(W/m^2))/(Imax_(W/m^2))=4*(COS(Δφ_°)/2)^2' "
    "'(Δφ_r)=(2*(Ⓒπ_r)*(d_μm)*SIN(θ_°))/(λ_nm)' "
    "'TAN(θ_°)=(y_m)/(L_m)' "
    "'(Δyint_m)=(λ_nm)*(L_m)/(d_μm)' "
    "}",

    "One Slit Diffraction",  "{ "
    "'(I_(W/m^2))/(Imax_(W/m^2))=(IFTE(Δα_°;SIN(Δα/2)/(Δα/2);1))^2' "
    "'(Δα_r)=(2*(Ⓒπ_r)*(a_μm)*SIN(θ_°))/(λ_nm)' "
    "'TAN(θ_°)=(y_m)/(L_m)' "
    "'(Δydiff_m)=2*(λ_nm)*(L_m)/(a_μm)' "
    "}",

    // ------------------------------------------------------------------------
    "Oscillations", nullptr,
    // ------------------------------------------------------------------------
    // 36 equs
    "Mass‐Spring System",  "{ "
    "'(ω_(r/s))=(1_r)*√((k_(N/m))/(m_kg))' "
    "'(T_s)=2*(Ⓒπ_r)/(ω_(r/s))' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "}",

    "Simple Pendulum",  "{ "
    "'(ω_(r/s))=(1_r)*√(Ⓒg/(L_cm))' "
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
    "'(ω_(r/s))=(1_r)*√(((G_kPa)*(J_(cm^4)))/((L_cm)*(I_(kg*m^2))))' "
    "'(T_s)=2*(Ⓒπ_r)/(ω_(r/s))' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "}",

    "Simple Harmonic",  "{ "
    "'(x_cm)=(xm_cm)*COS((ω0_(r/s))*(t_s)+(φ_°))' "
    "'(v_(m/s))=-(ω0_(r/s))*(xm_m)*SIN((ω0_(r/s))*(t_s)+(φ_°))' "
    "'(a_(m/s^2))=-(ω0_(r/s))^2*(xm_m)*COS((ω0_(r/s))*(t_s)+(φ_°))' "
    "'(ω0_(r/s))=√((k_(N/m))/(m_kg))' "
    "'(E_J)=(1/2)*(m_kg)*((ω0_(r/s))*(xm_m))^2' "
    "}",
    // Ref.: https://scholar.harvard.edu/files/david-morin/files/waves_oscillations.pdf
    "Underdamped Oscillations",  "{ "
    "'(x_m)=(xm_m)*EXP(-(γ_(r/s))*(t_s)/2)*COS((ωu_(r/s))*(t_s)+(φ_°))' "
    "'(γ_(r/s))=(b_(kg/s))/(m_kg)' "
    "'(ω0_(r/s))=√((k_(N/m))/(m_kg))' "
    "'(ωu_(r/s))=(ω0_(r/s))*√(1-((γ_(r/s))/(2*(ω0_(r/s))))^2)' "
    "'(v_(m/s))=(xm_m)*EXP(-(γ_(r/s))*(t_s)/2)*(-((γ_(r/s))/2)*COS((ωu_(r/s))*(t_s)+(φ_°))-(ωu_(r/s))*SIN((ωu_(r/s))*(t_s)+(φ_°)))' "
    "'(a_(m^2/s))=-((ω0_(r/s))^2*(x_m)+(γ_(r/s))*(v_m/s))' "
    "'(E_J)=(1/2)*(k_(N/m))*(x_m)^2+(1/2)*(m_kg)*(v_(m/s))^2' "
    "'Q=(ω0_(r/s))/(γ_(r/s))' "
    "}",
    // Ref.: https://scholar.harvard.edu/files/david-morin/files/waves_oscillations.pdf
    "Driven Damped Oscillations",  "{ "
    "'(x_m)=(xp_m)*COS((ω_(r/s))*(t_s)+(φ_°))+(xh_m)*EXP(-(γ_(r/s))/(1_r)*(t_s)/2)*COS((ωu_(r/s))*(t_s)+(θ_°))' "
    "'(γ_(r/s))=(1_r)*(b_(kg/s))/(m_kg)' "
    "'(ω0_(r/s))=(1_r)*√((k_(N/m))/(m_kg))' "
    "'(ωu_(r/s))=(ω0_(r/s))*√(1-((γ_(r/s))/(2*(ω0_(r/s))))^2)' "
    "'(xp_m)=(1_r)^2*((Fd_N)/(m_kg))/√(((ω0_(r/s))^2-(ω_(r/s))^2)^2+((γ_(r/s))*(ω_(r/s)))^2)' "
    "'TAN(φ_°)=-((γ_(r/s))*(ω_(r/s)))/((ω0_(r/s))^2-(ω_(r/s))^2)' "
// The next 7th equn doesn't appear (is not drawn on the screen) when doing NextEq in the SOLVER, but it goes on the stack with Eq>
    "'(v_(m/s))=-(xp_m)*(ω_(r/s))/(1_r)*SIN((ω_(r/s))*(t_s)+(φ_°))+(xh_m)*EXP(-(γ_(r/s))/(1_r)*(t_s)/2)*(-((γ_(r/s))/(1_r)/2)*COS((ωu_(r/s))*(t_s)+(θ_°))-(ωu_(r/s))/(1_r)*SIN((ωu_(r/s))*(t_s)+(θ_°)))' "
    "'(a_(m^2/s))=-(((ω0_(r/s))/(1_r))^2*(x_m)+(γ_(r/s))/(1_r)*(v_m/s))+((Fd_N)/(m_kg))*COS((ω_(r/s))*(t_s))' "
    "'(E_J)=(1/2)*(k_(N/m))*(x_m)^2+(1/2)*(m_kg)*(v_(m/s))^2' "
    "'Q=(ω0_(r/s))/(γ_(r/s))' "
    "}",

    // ------------------------------------------------------------------------
    "Plane Geometry", nullptr,
    // ------------------------------------------------------------------------
    // 31 eqns
    "Circle",  "{ "
    "'(A_(cm^2))=Ⓒπ*(r_cm)^2' "
    "'(C_cm)=2*Ⓒπ*(r_cm)' "
    "'(I_(cm^4))=1/4*Ⓒπ*(r_cm)^4' "
    "'(J_(cm^4))=1/2*Ⓒπ*(r_cm)^4' "
    "'(Id_(cm^4))=(I_(cm^4))+(A_(cm^2))*(d_cm)^2' "
    "}",

    "Ellipse",  "{ "
    "'(A_(cm^2))=Ⓒπ*(b_cm)*(h_cm)' "
    "'(C_cm)=2*Ⓒπ*√(((b_cm)^2+(h_cm)^2)/2)' "
    "'(I_(cm^4))=1/4*Ⓒπ*(b_cm)*(h_cm)^3' "
    "'(J_(cm^4))=1/4*Ⓒπ*(b_cm)*(h_cm)*((b_cm)^2+(h_cm)^2)' "
    "'(Id_(cm^4))=(I_(cm^4))+(A_(cm^2))*(d_cm)^2' "
    "}",

    "Rectangle",  "{ "
    "'(A_(cm^2))=(b_cm)*(h_cm)' "
    "'(P_cm)=2*((b_cm)+(h_cm))' "
    "'(I_(m^4))=1/12*(b_cm)*(h_cm)^3' "
    "'(J_(cm^4))=1/12*(b_cm)*(h_cm)*((b_cm)^2+(h_cm)^2)' "
    "'(Id_(cm^4))=(I_(cm^4))+(A_(cm^2))*(d_cm)^2' "
    "}",

    "Regular Polygon",  "{ "
    "'(A_(cm^2))=(1/4*n*(L_yd)^2)/TAN((180_°)/n)' "
    "'(P_cm)=n*(L_cm)' "
    "'(rs_cm)=((L_cm)/2)/TAN((180_°)/n)' "
    "'(rv_cm)=((L_cm)/2)/SIN((180_°)/n)' "
    "'(θ_°)=(n-2)/n*(180_°)' "
    "'(β_°)=(360_°)/n' "
    "}",

    "Circular Ring",  "{ "
    "'(A_(cm^2))=Ⓒπ*((ro_cm)^2-(ri_cm)^2)' "
    "'(I_(cm^4))=Ⓒπ/4*((ro_cm)^4-(ri_cm)^4)' "
    "'(J_(cm^4))=Ⓒπ/2*((ro_cm)^4-(ri_cm)^4)' "
    "'(Id_(cm^4))=(I_(m^4))+(A_(m^2))*(d_cm)^2' "
    "}",

    "Triangle",  "{ "
    "'(A_(in^2))=(b_cm)*(h_cm)/2' "
    "'(P_cm)=(b_cm)+√((v_cm)^2+(h_cm)^2)+√(((b_cm)-(v_cm))^2+(h_cm)^2)' "
    "'(Ix_(cm^4))=1/36*(b_cm)*(h_cm)^3' "
    "'(Iy_(cm^4))=1/36*(b_cm)*(h_cm)*((b_cm)^2+(v_cm)^2-(b_cm)*(v_cm))' "
    "'(J_(cm^4))=1/36*(b_cm)*(h_cm)*((b_cm)^2+(v_cm)^2+(h_cm)^2-(b_cm)*(v_cm))' "
    "'(Id_(cm^4))=(I_(cm^4))+(A_(in^2))*(d_cm)^2' "
    "}",

    // ------------------------------------------------------------------------
    "Solid Geometry", nullptr,
    // ------------------------------------------------------------------------
    //18  eqns
    "Cone",  "{ "
    "'(V_(cm^3))=(Ⓒπ/3)*(r_cm)^2*(h_cm)' "
    "'(A_(cm^2))=Ⓒπ*(r_cm)^2+Ⓒπ*(r_cm)*√((r_cm)^2+(h_cm)^2)' "
    "'(Ixx_(kg*m^2))=(3/20)*(m_kg)*(r_cm)^2+(3/80)*(m_kg)*(h_cm)^2' "
    "'(Izz_(kg*m^2))=(3/10)*(m_kg)*(r_cm)^2' "
    "'(Id_(kg*m^2))=(Ixx_(kg*m^2))+(m_kg)*(d_cm)^2' "
    "}",

    //WARNING mass units in the example of HP50G_AUR (12.2) is lbs...
    // is it lbf or lb (lb probably)
    "Cylinder",  "{ "
    "'(V_(cm^3))=Ⓒπ*(r_cm)^2*(h_cm)' "
    "'(A_(cm^2))=2*Ⓒπ*(r_cm)^2+2*Ⓒπ*(r_cm)*(h_cm)' "
    "'(Ixx_(kg*cm^2))=(1/4)*(m_kg)*(r_cm)^2+(1/12)*(m_kg)*(h_cm)^2' "
    "'(Izz_(kg*cm^2))=(1/2)*(m_kg)*(r_cm)^2' "
    "'(Id_(kg*m^2))=(Ixx_(kg*m^2))+(m_kg)*(d_m)^2' "
    "}",

    "Parallelepiped",  "{ "
    "'(V_(cm^3))=(b_cm)*(h_cm)*(t_cm)' "
    "'(A_(cm^2))=2*((b_cm)*(h_cm)+(b_m)*(t_cm)+(h_cm)*(t_cm))' "
    "'(I_(kg*cm^2))=(1/12)*(m_kg)*((h_cm)^2+(t_cm)^2)' "
    "'(Id_(kg*cm^2))=(I_(kg*cm^2))+(m_kg)*(d_cm)^2' "
    "}",

    "Sphere",  "{ "
    "'(V_(cm^3))=(4/3)*Ⓒπ*(r_cm)^3' "
    "'(A_(cm^2))=4*Ⓒπ*(r_cm)^2' "
    "'(I_(kg*m^2))=(2/5)*(m_kg)*(r_cm)^2' "
    "'(Id_(kg*cm^2))=(I_(kg*m^2))+(m_kg)*(d_cm)^2' "
    "}",

    // ------------------------------------------------------------------------
    "Solid State Device", nullptr,
    // ------------------------------------------------------------------------
    //33  eqns
    // **WARNING: Required function for the parameter ni_(cm^-3)=SIDENS(T_K)
    // And Here q = Ⓒqe.
    // ERROR in gm formula in HP50G_AUR.pdf: μm=μn (OK)
    // ERROR in 4th formula of HP50G_AUR.pdf αR=>αF
    "PN Step Junctions",  "{ "
    "'(Vbi_V)=(Ⓒk*(T_°C))/Ⓒqe*LN((NA_(cm^-3))*(ND_(cm^-3))/((ni_(cm^-3))^2))' "
    "'(ni_(cm^-3))=SIDENS(T_K)' "
    "'(xd_μ)=√((2*Ⓒεsi*Ⓒε0)/Ⓒqe*((Vbi_V)-(Va_V))*(1/(NA_(cm^-3))+1/(ND_(cm^-3))))' "
    "'(Cj_(pF/cm^2))=(Ⓒεsi*Ⓒε0)/(xd_μ)' "
    "'(Emax_(V/m))=2*((Vbi_V)-(Va_V))/(xd_μ)' "
    "'(BV_V)=(Ⓒεsi*Ⓒε0*(E1_(V/m))^2)/(2*Ⓒqe)*(1/(NA_(cm^-3))+1/(ND_(cm^-3)))' "
    "'(J_(A/cm^2))=(Js_(μA/cm^2))*(EXP((Ⓒqe*(Va_V))/(Ⓒk*(T_K)))-1)' "
    "'(Aj_(cm^2))=((W_μ)+2*(ΔW_μ))*((L_μ)+2*(ΔL_μ))+Ⓒπ*((W_μ)+2*(ΔW_μ)+2*(ΔL_μ))*(xj_μ)+2*Ⓒπ*(xj_m)^2' "
    "'(I_mA)=(J_(A/cm^2))*(Aj_(cm^2))' "
    "}",

    // WARNING: In the 9th formula of HP50G it is μn and not μm as described in
    // HP50G_AUR
    "NMOS Transistor",  "{ "
    "'(We_μ)=(W_μ)-2*(ΔW_μ)' "
    "'(Le_μ)=(L_m)-2*(ΔL_μ)' "
    "'(Cox_(pF/cm^2))=(Ⓒεox*Ⓒε0)/(tox_nm)' "
    "'(IDS_mA)=(Cox_(pF/cm^2))*(μn_((cm^2)/(V*s)))*((We_μ)/(Le_μ))*(((VGS_V)-(Vt_V))*(VDS_V)-(VDS_V)^2/2)*(1+(λ_(1/V))*(VDS_V))' "
    "'(γ_(V^(1/2)))=√((2*Ⓒεsi*Ⓒε0)*Ⓒqe*(NA_(cm^-3)))/(Cox_(pF/cm^2))' "
    "'(Vt_V)=(Vt0_V)+(γ_(V^(1/2)))*(√(2*ABS(φp_V)-ABS(VBS_V))-√(2*ABS(φp_V)))' "
    "'(φp_V)=Ⓒk*(T_K)/Ⓒqe*LN((NA_(cm^-3))/(ni_(cm^-3)))' "
    "'(ni_(cm^-3))=SIDENS(T_K)' "
    "'(gds_S)=(IDS_mA)*(λ_(V^-1))' "
    "'(gm_(mA/V))=√((Cox_(pF/cm^2))*(μn_((cm^2)/(V*s)))*((We_m)/(Le_m))*(1+(λ_(V^-1))*(VDS_V))*2*(IDS_mA))' "
    "'(VDsat_V)=(VGS_V)-(Vt_V)' "
    "}",

    // WARNING HP50G has precedence here for the presence of VBC in 2nd exp of
    // 1st eqn (contrary to HP50G_AUR)
    // WARNING HP50G has precedence here for the presence of αF instead of αR in
    // 3rd eqn (contrary to HP50G_AUR)
    "Bipolar Transistors",  "{ "
    "'(IE_mA)=-(IES_nA)*(exp((Ⓒqe*(VBE_V))/(Ⓒk*(T_K)))-1)+αR*(ICS_nA)*(EXP((Ⓒqe*(VBC_V))/(Ⓒk*(T_K)))-1)' "
    "'(IC_mA)=-(ICS_nA)*(exp((Ⓒqe*(VBC_V))/(Ⓒk*(T_K)))-1)+αF*(IES_nA)*(EXP((Ⓒqe*(VBE_V))/(Ⓒk*(T_K)))-1)' "
    "'(IS_nA)=αF*(IES_nA)' "
    "'(IS_nA)=αR*(ICS_nA)' "
    "'(IB_nA)+(IE_nA)+(IC_nA)=0' "
    "'(ICO_nA)=(ICS_nA)*(1-αF*αR)' "
    "'(ICEO_nA)=(ICO_nA)/(1-αF)' "
    "'(VCEsat_V)=((Ⓒk*(T_K)))/Ⓒqe*LN(((1+(IC_mA)/(IB_mA)*(1-αR)))/(αR*(1-(IC_mA)/(IB_mA)*((1-αF)/αF))))' "
    "}",

    "JFETs",  "{ "
    "'(Vbi_V)=(Ⓒk*(T_K))/Ⓒqe*LN((ND_(cm^-3))/(ni_(cm^-3)))' "
    "'(ni_(cm^-3))=SIDENS(T_K)' "
    "'(xdmax_μ)=√((2*Ⓒεsi*Ⓒε0)/(Ⓒqe*(ND_(cm^-3)))*((Vbi_V)-(VGS_V)+(VDS_V)))' "
    "'(G0_S)=Ⓒqe*(ND_(cm^-3))*(μn_((cm^2)/(V*s)))*(((a_μ)*(W_μ))/(L_μ))' "
    "'(ID_mA)=(G0_S)*((VDS_V)-((2/3)*√((2*Ⓒεsi*Ⓒε0)/(Ⓒqe*(ND_(cm^-3))*(a_μ)^2)))*(((Vbi_V)-(VGS_V)+(VDS_V))^(3/2)-((Vbi_V)-(VGS_V))^(3/2)))' "
    "'(VDsat_V)=(Ⓒqe*(ND_(cm^-3))*(a_μ)^2)/(2*Ⓒεsi*Ⓒε0)-((Vbi_V)-(VGS_V))' "
    "'(Vt_V)=(Vbi_V)-(Ⓒqe*(ND_(cm^-3))*(a_μ)^2)/(2*Ⓒεsi*Ⓒε0)' "
    "'(gm_(mA/V))=(G0_S)*(1-√(((2*Ⓒεsi*Ⓒε0)/(Ⓒqe*(ND_(cm^-3))*(a_μ)^2))*((Vbi_V)-(VGS_V))))' "
    "}",

    // ------------------------------------------------------------------------
    "Stress Analysis", nullptr,
    // ------------------------------------------------------------------------
    //16  eqns
    "Normal Stress",  "{ "
    "'(σ_atm)=(E_atm)*ε' "
    "'ε=(δ_cm)/(L_m)' "
    "'(σ_Pa)=(P_N)/(A_cm^2)' "
    "}",

    "Shear Stress",  "{ "
    "'(τ_atm)=(G_atm)*(γ_°)' "
    "'(γ_°)=((r_cm)*(φ_°))/(L_m)' "
    "'(τ_atm)=((T_(cm*N))*(r_cm))/(J_(cm^4))' "
    "}",

    "Stress On An Element",  "{ "
    "'(σx1_kPa)=((σx_kPa)+(σy_kPa))/2+((σx_kPa)-(σy_kPa))/2*COS(2*(θ_°))+(τxy_kPa)*SIN(2*(θ_°))' "
    "'(σx1_kPa)+(σy1_kPa)=(σx_kPa)+(σy_kPa)' "
    "'(τx1y1_kPa)=-(((σx_kPa)-(σy_kPa))/2)*SIN(2*(θ_°))+(τxy_kPa)*(σy_kPa)' "
    "}",

    // WARNING Error in formula 1 of HP50G_AUR square missing HP50G takes
    // precedence
    "Mohr’s Circle",  "{ "
    "'(σ1_atm)=((σx_atm)+(σy_atm))/2+√((((σx_atm)-(σy_atm))^2)/2+(τxy_atm)^2)' "
    "'(σ1_atm)+(σ2_atm)=(σx_atm)+(σy_atm)' "
    "'SIN(2*(θp1_°))=(τxy_atm)/√((((σx_atm)-(σy_atm))/2)^2+(τxy_atm)^2)' "
    "'(θp2_°)=(θp1_°)+(90_°)' "
    "'(τmax_atm)=((σ1_atm)-(σ2_atm))/2' "
    "'(θs_°)=(θp1_°)-(45_°)' "
    "'(σavg_atm)=((σx_atm)+(σy_atm))/2' "
    "}",

    // ------------------------------------------------------------------------
    "Waves", nullptr,
    // ------------------------------------------------------------------------
    //43  eqns
    "Transverse Waves",  "{ "
    "'(y_cm)=(ym_cm)*SIN((k_(r/cm))*(x_cm)-(ω_(r/s))*(t_s)+(φ_r))' "
    "'(v_(cm/s))=(λ_cm)*(f_Hz)' "
    "'(k_(r/cm))=2*(Ⓒπ_r)/(λ_cm)' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "'(vy_(cm/s))=-(ym_cm)*(ω_(r/s))/(1_r)*COS((k_(r/cm))*(x_cm)-(ω_(r/s))*(t_s)+(φ_r))' "
    "'(ay_(cm/(s^2)))=-((ω_(r/s))/(1_r))^2*(ym_cm)' "
    "}",

    "Longitudinal Waves",  "{ "
    "'(s_cm)=(sm_cm)*COS((k_(r/cm))*(x_cm)-(ω_(r/s))*(t_s)+(φ_r))' "
    "'(v_(m/s))=(λ_cm)*(f_Hz)' "
    "'(k_(r/cm))=2*(Ⓒπ_r)/(λ_cm)' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "'(vs_(cm/s))=(sm_cm)*(ω_(r/s))/(1_r)*SIN((k_(r/cm))*(x_cm)-(ω_(r/s))*(t_s)+(φ_r))' "
    "'(as_(cm/(s^2)))=-((ω_(r/s))/(1_r))^2*(s_cm)' "
    "}",

    "Sound Waves",  "{ "
    "'(s_cm)=(sm_cm)*SIN((k_(r/cm))*(x_cm)-(ω_(r/s))*(t_s)+(φ_r))' "
    "'(vs_(cm/s))=(sm_cm)*(ω_(r/s))/(1_r)*COS((k_(r/cm))*(x_cm)-(ω_(r/s))*(t_s)+(φ_r))' "
    "'(as_(cm/(s^2)))=-((ω_(r/s))/(1_r))^2*(s_cm)' "
    "'(Δp_Pa)=-(Δpm_Pa)*COS((k_(r/cm))*(x_cm)-(ω_(r/s))*(t_s)+(φ_r))' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "'(v_(m/s))=√((B_kPa)/(ρ_(kg/(m^3))))' "
    "'(Δpm_Pa)=(ρ_(kg/(m^3)))*(ω_(r/s))/(1_r)*(v_(m/s))*(sm_cm)' "
    "'(I_(W/(m^2)))=1/2*(ρ_(kg/(m^3)))*(v_(m/s))*((ω_(r/s))/(1_r))^2*(sm_cm)^2' "
    "'(I_(W/m^2))=(Ps_W)/(4*Ⓒπ*(r_m)^2)' "
    "'(β_dB)=10*LOG((I_(W/(m^2)))/(ⒸI0_(W/(m^2))))' "
    "}",

    "Doppler Effect",  "{ "
    "'(f_Hz)=(f0_Hz)*(((cair_(m/s))+(vr_(m/s)))/((vsair_(m/s))-(vs_(m/s))))' "
    "'(vsair_(m/s))=√(1.4*((8.314462618153_(J/(mol*K)))/(0.0289645_(kg/mol))*(T_K)))' "
    "}",

    "Mach Number",  "{ "
    "'M=(u_(m/s))/(vsair_(m/s))' "
    "'SIN(θcone_°)=(vsair_(m/s))/(u_(m/s))' "
    "'(vsair_(m/s))=√(1.4*((8.314462618153_(J/(mol*K)))/(0.0289645_(kg/mol))*(T_K)))' "
    "}",

    "String Standing Waves",  "{ "
    "'(y_m)=(ym_m)*SIN((k_(r/m))*(x_m))*COS((ω_(r/s))*(t_s))' "
    "'(v_(m/s))=(λ_m)*(f_Hz)' "
    "'(k_(r/m))=2*(Ⓒπ_r)/(λ_m)' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "'(v_(m/s))=√((T_N)/(μ_(kg/m)))' "
    "'(ffixed-fixed_Hz)=(ninteger*(v_(m/s)))/(2*(L_m))' "
    "'(ffixed-free_Hz)=(nodd*(v_(m/s)))/(4*(L_m))' "
    "}",

    "Sound Wave Harmonics",  "{ "
    "'(s_m)=(sm_m)*SIN((k_(r/m))*(x_m))*COS((ω_(r/s))*(t_s))' "
    "'(vsair_(m/s))=(λ_m)*(f_Hz)' "
    "'(k_(r/m))=2*(Ⓒπ_r)/(λ_m)' "
    "'(ω_(r/s))=2*(Ⓒπ_r)*(f_Hz)' "
    "'(vsair_(m/s))=√(1.4*((8.314462618153_(J/(mol*K)))/(0.0289645_(kg/mol))*(T_K)))' "
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
    //86  eqns in 15 sections
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
    "'γ=1/√(1-β^2)' "
    "}",

    "Time Dilation",  "{ "
    "'(Δtp_s)=γ*(Δt_s)' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ=1/√(1-β^2)' "
    "}",

    "Space Contraction",  "{ "
    "'(Δxp_m)=(Δx_m)/γ' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ=1/√(1-β^2)' "
    "}",

    "Velocity Superposition",  "{ "
    "'(upx_(m/s))=((ux_(m/s))-(v_(m/s)))/(1-((((v_(m/s))*(ux_(m/s))))/Ⓒc^2))' "
    "'(upy_(m/s))=((uy_(m/s))/γ)/(1-((v_(m/s))*(ux_(m/s))/Ⓒc^2))' "
    "'(upz_(m/s))=((uz_(m/s))/γ)/(1-((v_(m/s))*(ux_(m/s))/Ⓒc^2))' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ=1/√(1-β^2)' "
    "}",

    "Acceleration Superposition",  "{ "
    "'(apx_(m/s^2))=(ax_(m/s^2))/(γ^3*(1-((((v_(m/s))*(ux_(m/s))))/Ⓒc^2)^3))' "
    "'(apy_(m/s))=(ay_(m/s))/(γ^2*(1-((v_(m/s))*(ux_(m/s))/Ⓒc^2))^2)+((ax_(m/s))*((v_(m/s))*(uy_(m/s)))/Ⓒc^2)/(γ^2*(1-((v_(m/s))*(ux_(m/s))/Ⓒc^2))^3)' "
    "'(apz_(m/s))=(az_(m/s))/(γ^2*(1-((v_(m/s))*(ux_(m/s))/Ⓒc^2))^2)+((ax_(m/s))*((v_(m/s))*(uz_(m/s)))/Ⓒc^2)/(γ^2*(1-((v_(m/s))*(ux_(m/s))/Ⓒc^2))^3)' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ=1/√(1-β^2)' "
    "}",

    "E & B Fields Transformation",  "{ "
    "'(Epx_(N/C))=(Ex_(N/C))' "
    "'(Epy_(N/C))=γ*((Ey_(N/C))-β*(Bz_T))' "
    "'(Epz_(N/C))=γ*((Ez_(N/C))+β*(By_T))' "
    "'(Bpx_T)=(Bx_T)'"
    "'(Bpy_T)=γ*((By_T)+β*(Ez_(N/C)))' "
    "'(Bpz_T)=γ*((Bz_T)-β*(Ey_(N/C)))' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ=1/√(1-β^2)' "
    "}",

    "Longitudinal Doppler Effect",  "{ "
    "'(frl_Hz)=(fs_Hz)*√((1-β)/(1+β))' "
    "'β=(v_(m/s))/Ⓒc' "
    "}",

    "Transverse Doppler Effect",  "{ "
    "'(frt_Hz)=γ*(fs_Hz)' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ=1/√(1-β^2)' "
    "}",

    "Light Propagation",  "{ "
    "'(fp_Hz)/(f_Hz)=γ*(1+β*COS(α_°))' "
    "'COS(θp_°)=(COS(θ_°)-β)/(1-β*COS(θ_°))' "
    "'Pθ=SIN(θ_°)/(2*γ^2*(1-β*COS(θ_°))^2)' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ=1/√(1-β^2)' "
    "}",

    "Energy & Momentum",  "{ "
    "'(ppx_(kg*(m/s)))=γ*((px_(kg*(m/s)))-v(m/s)*(E_J)/Ⓒc^2)' "
    "'(ppy_(kg*(m/s)))=(py_(kg*(m/s)))' "
    "'(ppz_(kg*(m/s)))=(pz_(kg*(m/s)))' "
    "'(Ep_J)=γ*((E_J)-v(m/s)*(px_(kg*(m/s))))' "
    "'(p_(kg*(m/s)))=γ*(mo_kg)*(v(m/s))' "
    "'(E_J)=γ*(E0_J)' "
    "'(E0_J)=(m0_kg)*Ⓒc^2' "
    "'(E_J)^2=(p_(kg*(m/s)))^2*Ⓒc^2+(m0_kg)^2*Ⓒc^4' "
    "'(K_J)=(γ-1)*(E0_J)' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ=1/√(1-β^2)' "
    "}",

    "Ultrarelativistic Cases",  "{ "
    "'(E0_J)=(m0_kg)*Ⓒc^2' "
    "'(E_J)^2=(p_(kg*(m/s)))^2*Ⓒc^2+(m0_kg)^2*Ⓒc^4' "
    "'(K_J)=(γ-1)*(E0_J)' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ=1/√(1-β^2)' "
    "'(Δtp_s)=γ*(Δt_s)' "
    "'(Δxp_m)=(Δx_m)/γ' "
    "}",

    "Gravitational Time Dilation",  "{ "
    "'(ΔtpG_s)=γG*(Δt_s)' "
    "'βe=(ve_(m/s))/Ⓒc' "
    "'γG=1/√(1-βe^2)' "
    "'(ve_(m/s))=√(2*ⒸG*(M_kg)/r(m))' "
    "}",

    "Gravitational Redshift",  "{ "
    "'z=(λ∞_nm)/(λe_nm)-1' "
    "'(λ∞_nm)/(λe_nm)=1/√(1-((rs_m)/(Rem_m)))' "
    "'(λ1_nm)/(λ2_nm)=√((1-(rs_m)/(R1_m))/(1-(rs_m)/(R2_m)))' "
    "'zNL=ⒸG*(M_kg)/((Ⓒc)^2*(Rem_m))' "
    "'(rs_m)=2*ⒸG*(M_kg)/(Ⓒc)^2' "
    "}",

    "Circumnavigating Airplanes",  "{ "
    "'(ΔτWE_ns)=(ΔτW_ns)-(ΔτE_ns)' "
    "'(ΔτE_ns)=(ΔτpE_ns)-(Δτg_ns)' "
    "'(ΔτW_ns)=(ΔτpW_ns)-(Δτg_ns)' "
    "'(ΔτpE_ns)=(1-(MGu_m)/((R_m)+(hp_m))-(βp+βg)/2)*(Δt_s)' "
    "'(ΔτpW_ns)=(1-(MGu_m)/((R_m)+(hp_m))-(βp-βg)/2)*(Δt_s)' "
    "'(Δτg_ns)=(1-(MGu_m)/(R_m)-βg^2/2)*(Δt_s)' "
    "'(MGu_m)=ⒸG*(M_kg)/Ⓒc^2' "
    "'βp=(vp_(m/s))/Ⓒc' "
    "'βg=(vg_(m/s))/Ⓒc' "
    "'(vg_m/s)=(2*Ⓒπ*(R_km))/(Tday_s)*COS(φ_°)' "
    "}",

    "Clocks at different heights",  "{ "
    "'γ21=(γv1/γv2)*(γG1/γG2)' "
    "'γv1=1/√(1-((v1_(m/s))/Ⓒc)^2)' "
    "'(v1_m/s)=(ω_(r/s))/(1_r)*((R_km)+(h1_m))*COS(φ_°)' "
    "'γv2=1/√(1-((v2_(m/s))/Ⓒc)^2)' "
    "'(v2_m/s)=(ω_(r/s))/(1_r)*((R_km)+(h2_m))*COS(φ_°)' "
    "'γG1=1/√(1-(2*(MGu_m))/(R+h1))' "
    "'γG2=1/√(1-(2*(MGu_m))/(R+h2))' "
    "'(MGu_m)=ⒸG*(M_kg)/Ⓒc^2' "
    "'(ω_(r/s))=(2*(Ⓒπ_r))/(Tday_s)' "
    "}",

    "B H Schwarzschild Geometry",  "{ "
    "'(rs_m)=2*ⒸG*(M_kg)/(Ⓒc)^2' "
    "'(ve_(m/s))=2*ⒸG*(M_kg)/(rs_m)' "
    "'(Vs_(m^3))=(4/3)*Ⓒπ*(rs_m)^3' "
    "'Vxsun=(V_(km^3))/(1.412*10^18_(km^3))' "
    "'rxearth=(r_km)/(6371_km)' "
    "'Mxsun=(M_kg)/(1.9885*10^30_kg)' "
    "'Mxearth=(M_kg)/(5.972168*10^24_kg)' "
    "}",

    "B H Thermodynamics",  "{ "
    "'(TH_K)=(Ⓒℏ*(Ⓒc)^3)/(8*Ⓒπ*ⒸG*Ⓒk*(M_kg))' "
    "'(PBH_W)=(Ⓒℏ*(Ⓒc)^6)/(15360*Ⓒπ*(ⒸG)^2*(M_kg)^2)' "
    "'(SBH_(J/K))=(Ⓒk*(As_(m^2))*(Ⓒc)^3)/(4*ⒸG*Ⓒℏ)' "
    "'(As_(m^2))=4*Ⓒπ*(rs_m)^2' "
    "'(rs_m)=2*ⒸG*(M_kg)/(Ⓒc)^2' "
    "'(tev_s)=(5120*Ⓒπ*(ⒸG)^2*(M_kg)^3)/(Ⓒℏ*(Ⓒc)^4)' "
    "'Mxsun=(M_kg)/(1.9885*10^30_kg)' "
    "'MxSagA=(M_kg)/(8.54*10^36_kg)' "
    "'Mxearth=(M_kg)/(5.972168*10^24_kg)' "
    "'txyr=(t_s)/(3.15576*10^7_s)' "
    "}",

    // ------------------------------------------------------------------------
    "Modern Physics", nullptr,
    // ------------------------------------------------------------------------
    //47 eqns 37 vars in 6 sections
    "Planck & Wien Comparison",  "{ "
    "'(eb_(W/m^2))=Ⓒσ*(T_°K)^4' "
    "'(ebfafb_(W/m^2))=Frfafb*(eb_(W/m^2))' "
    "'(fpeak_Hz)=Ⓒk*ROOT((-3)*EXPM1(-X)-X;X;2)*(T_°K)/Ⓒh' "
    "'(f1_Hz)=Ⓒk*4*(T_K)/Ⓒh' "
    "'(f2_Hz)=Ⓒk*9*(T_K)/Ⓒh' "
    "'FrPl12=15/Ⓒπ^4*∫(Ⓒh*(f1_Hz)/(Ⓒk*(T_K));Ⓒh*(f2_Hz)/(Ⓒk*(T_K));x^3/expm1(x);X)' "
    "'FrWn12=15/Ⓒπ^4*∫(Ⓒh*(f1_Hz)/(Ⓒk*(T_K));Ⓒh*(f2_Hz)/(Ⓒk*(T_K));x^3/expm1(x);X)' "
    "'%rFr12=ABS(FrPl12-FrWn12)/FrPl12' "
    "'(f3_Hz)=Ⓒk*0.01*(T_K)/Ⓒh' "
    "'(f4_Hz)=Ⓒk*2.5*(T_K)/Ⓒh' "
    "'FrPl34=15/Ⓒπ^4*∫(Ⓒh*(f3_Hz)/(Ⓒk*(T_K));Ⓒh*(f3_Hz)/(Ⓒk*(T_K));x^3/exp(x);X)' "
    "'FrWn34=15/Ⓒπ^4*∫(Ⓒh*(f3_Hz)/(Ⓒk*(T_K));Ⓒh*(f4_Hz)/(Ⓒk*(T_K));x^3/exp(x);X)' "
    "'%rFr34=ABS(FrPl34-FrWn34)/FrPl34' "
    "'(q_W)=(ebfafb_(W/m^2))*(A_(cm^2))' "
    "}",

    "Planck & Rayleigh‐Jeans Comparison",  "{ "
    "'(eb_(W/m^2))=Ⓒσ*(T_°K)^4' "
    "'(ebfafb_(W/m^2))=Frfafb*(eb_(W/m^2))' "
    "'(fpeak_Hz)=Ⓒk*ROOT((-3)*EXPM1(-X)-X;X;2)*(T_°K)/Ⓒh' "
    "'(f1_Hz)=Ⓒk*1.7*(T_K)/Ⓒh' "
    "'(f2_Hz)=Ⓒk*3.7*(T_K)/Ⓒh' "
    "'FrPl12=15/Ⓒπ^4*∫(Ⓒh*(f1_Hz)/(Ⓒk*(T_K));Ⓒh*(f2_Hz)/(Ⓒk*(T_K));x^3/expm1(x);X)' "
    "'FrRJ12=15/Ⓒπ^4*∫(Ⓒh*(f1_Hz)/(Ⓒk*(T_K));Ⓒh*(f2_Hz)/(Ⓒk*(T_K));x^2;X)' "
    "'%rFr12=ABS(FrPl12-FrWn12)/FrPl12' "
    "'(f3_Hz)=Ⓒk*0.001*(T_K)/Ⓒh' "
    "'(f4_Hz)=Ⓒk*0.05*(T_K)/Ⓒh' "
    "'FrPl34=15/Ⓒπ^4*∫(Ⓒh*(f3_Hz)/(Ⓒk*(T_K));Ⓒh*(f3_Hz)/(Ⓒk*(T_K));x^3/exp(x);X)' "
    "'FrRJ34=15/Ⓒπ^4*∫(Ⓒh*(f3_Hz)/(Ⓒk*(T_K));Ⓒh*(f4_Hz)/(Ⓒk*(T_K));x^2;X)' "
    "'%rFr34=ABS(FrPl34-FrWn34)/FrPl34' "
    "'(q_W)=(ebfafb_(W/m^2))*(A_(cm^2))' "
    "}",

    "Photoelectric Effect",  "{ "
    "'(Kmax_J)=Ⓒqe*(Vo_V)' "
    "'(Eph)=Ⓒh*(f_Hz)' "
    "'Ⓒh*(f_Hz)=(φ_eV)+(Kmax_eV)' "
    "'(φ_eV)=Ⓒh*(f0_Hz)' "
    "}",

    "Compton Effect",  "{ "
    "'(Kmax_J)=((γ-1))*Ⓒme*Ⓒc^2' "
    "'β=(v_(m/s))/Ⓒc' "
    "'γ=1/√(1-β^2)' "
    "'(Eph)=Ⓒh*(f_Hz)' "
    "'Ⓒh*Ⓒc/(λ_nm)=Ⓒh*Ⓒc/(λp_nm)+(K_J)' "
    "'(λp_nm)-(λ_nm)=Ⓒλc*(1-COS(θ_°))' "
    "}",

     //Ref to Bragg's law  https://en.wikipedia.org/wiki/Bragg%27s_law 	http://hyperphysics.phy-astr.gsu.edu/hbase/quantum/bragg.html
    "De Broglie Wave",  "{ "
    "'(λ_nm)=Ⓒh/(p_(kg*m/s))' "
    "'(K_eV)=(p_(kg*m/s))^2/(2*(m_kg))' "
    "'(p_(kg*m/s))=(m_kg)*(v_(m/s))' "
    "'2*(d_nm)*SIN(θ_°)=n*(λ_nm)' "
    "}",

    "Bohr Atomic Model",  "{ "
    "'(Eph)=Ⓒh*(f_Hz)' "
    "'Ⓒh*(f_Hz)=(Enp_eV)-(En_eV)' "
    "'(En_eV)=-(Ⓒme*Ⓒqe^4*Z^2)/(8*Ⓒε0^2*Ⓒh^2)*(1/n^2)' "
    "'(Enp_eV)=-(Ⓒme*Ⓒqe^4*Z^2)/(8*Ⓒε0^2*Ⓒh^2)*(1/np^2)' "
    "'(r_m)=n^2*(Ⓒε0*Ⓒh^2)/(Ⓒπ*Ⓒme*Ⓒqe^2)' "
    "}",

    // ------------------------------------------------------------------------
    "Nuclear Physics", nullptr,
    // ------------------------------------------------------------------------
    //26 eqns 38 var
    // We need activity units : 1 Bq = 1 disintegration/s & 1 Ci = 3.7e10 Bq
    "Radioactivity",  "{ "
   "'(T1/2_s)=ln(2)/(λ_s^-1)' "
    "'N=N0*exp((λ_s^-1)*(t_s))' "
    "'(A0_Bq)=(λ_s^-1)*N0' "
    "'(A_Bq)=(A0_Bq)*exp((λ_s^-1)*(t_s))' "
    "'N0=(m_kg)*ⒸNA/(MW_(g/mol))' "
    "}",

    // We need the constant for the mass of the neutral H atom
    // mH = 1.007825031898 u
    // The definition of the mass unit u need to be revised:
    // u = 1.6605390689 2e-27 kg
    "Radius & Binding Energy",  "{ "
    "'A=N+Z' "
    "'(R_m)=(1.2e-15_m)*A^(1/3)' "
    "'(V_m^3)=4/3*Ⓒπ*R^3' "
    "'(EB_MeV)=(Z*ⒸmH+N*Ⓒmn-(mX_u))*Ⓒc^2' "
    "'(EBse_MeV)=(1_MeV)*(15.75*A-17.8*A^(2/3)-0.711*Z*(Z-1)/A^(1/3)-23.7*(A-2*Z)^2/A+11.18/(A^(1/2))*IFTE((Z mod 2=0)and(N mod 2=0);1;IFTE((Z mod 2=1)and(N mod 2=1);-1;0)))' "
    "}",

    "α Decay",  "{ "
    "'A=N+Z' "
    "'(Qα_MeV)=((mX_u)-(mY_u)-4.0026032545_u)*Ⓒc^2' "
    "'(AYα)=(AXα)-4' "
    "'(ZYα)=(ZXα)-2' "
    "}",

    "β⊖ Decay",  "{ "
    "'A=N+Z' "
    "'(Qβ⊖_MeV)=((mX_u)-(mY_u))*Ⓒc^2' "
    "'(AYβ⊖)=(AXβ⊖)' "
    "'(ZYβ⊖)=(ZXβ⊖)+1' "
    "}",

    "β⊕ Decay",  "{ "
    "'A=N+Z' "
    "'(Qβ⊕_MeV)=((mX_u)-(mY_u)-2*Ⓒme)*Ⓒc^2' "
    "'(AYβ⊕)=(AXβ⊕)' "
    "'(ZYβ⊕)=(ZXβ⊕)-1' "
    "}",

    "General Nuclear Reaction",  "{ "
    "'A=N+Z' "
    "'(Q_MeV)=((ma_u)+(mX_u)-(mY_u)-(mb_u))*Ⓒc^2' "
    "'(Aa)+(AX)=(AY)+(Ab)' "
    "'(Za)+(ZX)=(ZY)+(Zb)' "
    "}",
    // Total 318 eqns & 675 vars as of 24-09-17 (618/315=1.96; 675/397=1.70 )
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
    .show_builtins  = show_builtin_equations
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
                        grob_g r = expression::infix(g,
                                                     nv, nameg,
                                                     0, ":",
                                                     vv, valg);
                        return r;
                    }
                }
            }
        }
    }
    return object::do_graph(o, g);
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
