# Equations library

The DB48X calculator features a library of equations covering mathematics,
physics, chemistry and computer science. The built-in equations can be extended
using the `config/equations.csv` configuration file.


## Columns and Beams

The variables in the Columns and Beams section are:

* `o`: Eccentricity (offset) of load (dim.: length)
* `σcr`: Critical stress (dim.: pressure=force/area, in SI: pascal, Pa)
* `σmax`: Maximum stress (dim.: pressure=force/area, in SI: pascal, Pa)
* `θ`: Slope at `x` (dim.: angle)
* `A`: Cross-sectional area
* `a`: Distance to point load
* `ε`: Eccentricity (dim.: length)
* `c`: Distance to edge fiber ([Eccentric Columns](#Eccentric Columns)), or Distance to applied moment (beams)
* `E`: Modulus of elasticity (dim.: pressure=force/area, in SI: pascal, Pa)
* `I`: Moment of inertia (dim.: length^4, in SI: m^4)
* `K`: Effective length factor of column
* `L`: Length of column or beam
* `M`: Applied moment (dim.: length·force, in SI: N·m)
* `Mx`: Internal bending moment at x (dim.: length·force, in SI: N·m)
* `P`: Load (Eccentric Columns), or Point load (beams) (dim.: force)
* `Pcr`: Critical load (dim.: force)
* `r`: Radius of gyration
* `V`: Shear force at x
* `w`: Distributed load (dim.: force)
* `x`: Distance along beam
* `y`: Deflection at x (dim.: length)

For simply supported beams and cantilever beams ([Simple Deflection](#Simple Deflection) through [Cantilever Shear](#Cantilever Shear)), the calculations differ depending upon the location of `x` relative to the loads.

* Applied loads are positive downward.
* The applied moment is positive counterclockwise.
* Deflection is positive upward.
* Slope is positive counterclockwise
* Internal bending moment is positive counterclockwise on the left-hand part.
* Shear force is positive downward on the left-hand part.

### Elastic Buckling

These equations apply to a slender column (`K·L/r>100`) with length factor `K`.

![Elastic Buckling](img/ElasticBuckling.bmp)

* To calculate [Pcr_kN;σcr_kPa] (Critical load; Critical stress) from 6 known variables:
```rpl
L=7.3152_m  r=4.1148_cm  E=199947961.502_kPa  A=53.0967_cm^2  K=0.7  I=8990598.7930_mm^4
@ Expecting [ Pcr=676.60192 6324 kN σcr=127 428.24437 8 kPa ]
'ROOT(ⒺElastic Buckling;[Pcr;σcr];[1_kN;1_kPa])'
```

### Eccentric Columns

These equations apply to a slender column (`K·L/r>100`) with length factor `K`.

![Eccentric Columns](img/EccentricColumns.bmp)

* To calculate `[σmax_kPa;I_mm^4]` (Maximum stress; Moment of inertia) from 8 known variables:
```rpl
L=6.6542_m  A=187.9351_cm^2  r=8.4836_cm  E=206842718.795_kPa  K=1  P=1908.2571_kN  c=15.24_cm
 ε=1.1806_cm
@ Expecting [ σmax=140 853.09700 6 kPa I=135 259 652.161 mm↑4 ]
'ROOT(ⒺEccentric Columns;[σmax;I];[1_kPa;1_mm^4])'
```

### Simple Deflection

![Simple Deflection](img/SimpleDeflection.bmp)

* To calculate `[y_in]` (Deflection at x) from 9 known variables:
```rpl
L=20_ft  E=29000000_psi  I=40_in^4  a=10_ft  P=674.427_lbf  c=17_ft  M=3687.81_ft*lbf
 w=102.783_lbf/ft  x=9_ft
@ Expecting [ y=-0.60048 54094 96 in ]
'ROOT(ⒺSimple Deflection;[y];[1_in])'
```

### Simple Slope

![Simple Slope](img/SimpleSlope.bmp)

* To calculate `[Θ_°]` (Slope at `x`) from 9 known variables:
```rpl
L=20_ft  E=29000000_psi  I=40_in^4  a=10_ft  P=674.427_lbf  c=17_ft  M=3687.81_ft*lbf
 w=102.783_lbf/ft  x=9_ft
@ Expecting [ θ=-0.08763 17825 27 ° ]
'ROOT(ⒺSimple Slope;[θ];[0_°])'
```

### Simple Moment

![Simple Moment](img/SimpleMoment.bmp)

* To calculate `[Mx_ft*lbf]` (Internal bending moment at x) from 7 known variables:
```rpl
L=20_ft  a=10_ft  P=674.427_lbf  c=17_ft  M=3687.81_ft*lbf  w=102.783_lbf/ft  x=9_ft
@ Expecting [ Mx=9 782.1945 lbf·ft ]
'ROOT(ⒺSimple Moment;[Mx];[1_ft*lbf])'
```

### Simple Shear

![Simple Shear](img/SimpleShear.bmp)

* To calculate `[V_lbf]` (Shear force at x) from 6 known variables:
```rpl
L=20_ft  a=10_ft  P=674.427_lbf  M=3687.81_ft*lbf  w=102.783_lbf/ft  x=9_ft
@ Expecting [ V=624.387 lbf ]
'ROOT(ⒺSimple Shear;[V];[1_lbf])'
```

### Cantilever Deflection

![Cantilever Deflection](img/CantileverDeflection.bmp)

* To calculate `[y_in]` (Deflection at x) from 9 known variables:
```rpl
L=10_ft  E=29000000_psi  I=15_in^4  P=500_lbf  M=800_ft*lbf  a=3_ft  c=6_ft  w=100_lbf/ft  x=8_ft
@ Expecting [ y=-0.33163 03448 28 in ]
'ROOT(ⒺCantilever Deflection;[y];[0_in])'
```

### Cantilever Slope

![Cantilever Slope](img/CantileverSlope.bmp)

* To calculate `[Θ_°]` (Slope at `x`) from 9 known variables:
```rpl
L=10_ft  E=29000000_psi  I=15_in^4  P=500_lbf  M=800_ft*lbf  a=3_ft  c=6_ft  w=100_lbf/ft  x=8_ft
@ Expecting [ θ=-0.26522 01876 49 ° ]
'ROOT(ⒺCantilever Slope;[θ];[0_°])'
```

### Cantilever Moment

![Cantilever Moment](img/CantileverMoment.bmp)

* To calculate `[Mx_ft*lbf]` (Internal bending moment at x) from 7 known variables:
```rpl
L=10_ft  P=500_lbf  M=800_ft*lbf  a=3_ft  c=6_ft  w=100_lbf/ft  x=8_ft
@ Expecting [ Mx=-200. lbf·ft ]
'ROOT(ⒺCantilever Moment;[Mx];[1_ft*lbf])'
```

### Cantilever Shear

![Cantilever Shear](img/CantileverShear.bmp)

* To calculate `[V_lbf]` (Shear force at x) from 5 known variables:
```rpl
L=10_ft  P=500_lbf  a=3_ft  x=8_ft  w=100_lbf/ft
@ Expecting [ V=200. lbf ]
'ROOT(ⒺCantilever Shear;[V];[1_lbf])'
```


## Electricity

The variables in the Electricity section are:

* `∈r`: Relative permittivity
* `μr`: Relative permeability
* `ω`: Angular frequency (dim.: angle/time)
* `ω0`: Resonant angular frequency (dim.: angle/time)
* `φ`: Phase angle
* `φp, φs`: Parallel and series phase angles
* `θ1, θ2`: Subtended angles relative to the ends of the wire
* `ρ`: Resistivity (dim.: resistance·length; in SI: ohm·meter, Ω·m), or Volumic charge density (Drift speed and current density) (dim.: charge/volume, in SI: C/m^3)
* `ρ0`: Resistivity at the reference temperature T0 (dim.: resistance·length; in SI: ohm·meter, Ω·m)
* `ΔI`: Current Change (dim.: charge/time; in SI: ampere, A)
* `Δt`: Time Change
* `ΔV`: Voltage Change (dim.: energy/charge; in SI: volt, V)
* `A`: Wire cross-section area (Wire Resistance), os Solenoid cross-section area (Solenoid Inductance), or Plate area (Plate Capacitor)
* `C, C1, C2`: Capacitance (dim.: charge^2/energy; in SI: farad, F)
* `Cp, Cs`: Parallel and series capacitances (dim.: charge^2/energy; in SI: farad, F)
* `d`: Plate separation (dim.: length)
* `E`: Energy
* `Er`: Electric field at distance r from the source (dim.: force/charge; in SI: N/C = V/m)
* `Ep`: Electric field over an infinite plate (dim.: force/charge; in SI: N/C = V/m)
* `Ein`: Electric field inside a plate capacitor (dim.: force/charge; in SI: N/C = V/m)
* `F`: Force between charges
* `f`: Frequency (dim.: time^-1)
* `f0`: Resonant frequency (dim.: time^-1)
* `I`: Current, or Total current (Current Divider) (dim.: charge/time; in SI: ampere, A)
* `I1`: Current in R1 (dim.: charge/time; in SI: ampere, A)
* `Imax`: Maximum current (dim.: charge/time; in SI: ampere, A)
* `L`: Inductance (dim.: energy/current^2; in SI: henry, H), or Length (Wire Resistance, Cylindrical Capacitor)
* `Lp, Ls`: Parallel and series inductances (dim.: energy/current^2; in SI: henry, H)
* `N`: Number of turns
* `P`: Power (dim.: energy/time)
* `q`: Charge (in SI: coulomb, C)
* `q1, q2, qtest`: Point charge (in SI: coulomb, C)
* `Qp, Qs`: Parallel and series quality factors
* `r`: Charge distance
* `R, R1, R2`: Resistance (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `ri, ro`: Inside and outside radii
* `Rp, Rs`: Parallel and series resistances (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `t`: Time
* `ti, tf`: Initial and final times
* `V`: Voltage, or Total voltage (dim.: energy/charge; in SI: volt, V)
* `V1`: Voltage accross R1 (dim.: energy/charge; in SI: volt, V)
* `Vi, Vf`: Initial and final voltages (dim.: energy/charge; in SI: volt, V)
* `Vmax`: Maximum voltage (dim.: energy/charge; in SI: volt, V)
* `XC`: Reactance of capacitor (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `XL`: Reactance of inductor (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `σ`: Conductivity (dim.: (resistance·length)^-1; in SI: siemens/meter, S/m), or Surface charge density (E field infinite plate, Plate capacitor) (dim.: charge/area, in SI: C/m^2)
* `T`: Temperature (in SI: K)
* `T0`: Fixed reference temperature for the measurement of resistivity (In SI: K)
* `uE`: Volumic density of electric energy (dim.: energy/volume)
* `αT`: Temperature coefficient of resistivity (dim.: temperature^-1, in SI: K^-1)
* `λ`: Linear charge density (dim.: charge/length, in SI: C/m)
* `J`: Current density (dim.: current/area, in SI: A/m^2)
* `Je`: Electron current density (dim.: current/area, in SI: A/m^2)
* `Jh`: Hole current density (dim.: current/area, in SI: A/m^2)
* `vd`: Drift speed
* `n`: Charge-carrier number density (dim.: volume^-1)
* `ne`: Electron number density (dim.: volume^-1)
* `nh`: Hole number density (dim.: volume^-1)
* `μe`: Electron mobility (dim.: speed/(electric field), in SI: m^2/(V·s))
* `μh`: Hole mobility (dim.: speed/(electric field), in SI: m^2/(V·s))
* `τc`: Mean free time between collisions
* `meeff`: Electron effective mass
* `mheff`: Hole effective mass

### Coulomb’s Law & E Field

These equations describe the electrostatic force between two point charged particles and the electric field observed at the position of a test charge which replaces one of the two charges `q1` or `q2` in the expression of the electric force. A finite object carrying a net charge `q1` can be considered as a point charge if the distance to the position of the point charge `q2` is much greater than the object dimension, see example 2, for the approximate calculations of the electric force and electric field far away from a charged plate.

* **Example 1**. To calculate `[F_N;Er_N/C]` (Electric force; Electric Field) from 5 known variables:

```rpl
q1=1.6E-19_C  q2=1.6E-19_C  r=4.00E-13_cm  εr=1  qtest=1.6E-19_C
@ Expecting [ F=14.38008 28598 N Er=8.98755 17873 8⁳¹⁹ N/C ]
'ROOT(ⒺCoulomb’s Law & E Field;[F;Er];[1_N;1_N/C])'
```

* **Example 2**. A square metal plate `L = 8_cm` on a side carries a charge of `q1 = 6_μC`. Approximate values of the electric force & electric field for a point charge `q2 = 1_μC` located at `r = 3_m` can be calculated with Coulomb’s law if the separation distance is much greater than the plate dimension `r >> L`. The whole plate is indeed considered as being a point charge providing that `r > 10 · L`. Therefore, to calculate `[F_N;Er_N/C]`:

```rpl
L=8_cm r=3_m q1=6E-6_C  q2=1E-6_C  r=3_m  εr=1  qtest=1E-6_C
@ Expecting [ F=5.99170 11915 9⁳⁻³ N Er=5 991.70119 159 N/C ]
if 'r > 10*L' then
 'ROOT(ⒺCoulomb’s Law & E Field;[F;Er];[1_N;1_N/C])'
end
```

### E Field Infinite Line

The expression for the radial electric field at the distance `r` is approximately valid if this distance is such that `r << L` and therefore also applies to a wire of finite length `L`.

* To calculate `[λ_C/m;Er_N/C]` (Linear charge density; Electric Field) from 4 known variables:

```rpl
Q=5E-6_C  L=3_m  r=0.05_m  εr=1
@ Expecting [ λ=1.66666 66666 7⁳⁻⁶ C/m Er=599 170.11915 9 N/C ]
'ROOT(ⒺE Field Infinite Line;[λ;Er];[1_C/m;1_N/C])'
```

The code below saves the reference value for comparison with the example in [E Field Finite Line](#E Field Finite Line):
```rpl
@ Save the reference value for comparison below
Er0=Er
```

### E Field Finite Line

The expression of the radial electric field at the distance `r` depends on the subtended angles `θ1` and `θ2` relative to the ends of the wire of finite length `L`.

![E field finite line](img/EFieldFiniteLine.bmp)

* **Example 1.** To calculate `[λ_C/m;Er_N/C]` (Electric Field; Linear charge density) from 6 known variables and also with the distance `r=(L/2)/tanθ1` and angle `θ2=360°-θ1` (see figure):

```rpl
r='(3_m)/(2*tan 30_°)' θ2='360_°-30_°'
Q=5E-6_C  L=3_m  r=2.5981_m  εr=1  θ1=30_°
@ Expecting [ λ=1.66666 66666 7⁳⁻⁶ C/m Er=5 765.46436 972 N/C ]
'ROOT(ⒺE Field Finite Line;[λ;Er];[1_C/m;1_N/C])'
```

* **Example 2.** To show that the infinite line of the previous section can approximate the finite case (if `r << L` realised when `r < L/10`), we calculate `[λ_C/m;Er_N/C]` with the angles `θ1=ATAN((L/2)/r)` and `θ2=360°-θ1` (see figure):

```rpl
Q=5E-6_C  L=3_m  r=5_cm  εr=1  θ1='atan(L/2/r)' θ2='360_°-θ1'
if 'r < L/10' then
@ Expecting [ λ=1.66666 66666 7⁳⁻⁶ C/m Er=598 837.52400 7 N/C ]
'ROOT(ⒺE Field Finite Line;[λ;Er];[1_C/m;1_N/C])'
end
```

```rpl
@ Verify relative difference under condition 5_cm << 3_m.
Er0 Er %Ch
@ Expecting -5.55093 02084 6⁳⁻²
```

### E Field Infinite Plate

The expression of the perpendicular electric field is constant over an infinite plate and can approximate the field at a distance `d` from a finite plate if it is very small compare to the dimensions (length or width `L`) of the plate. On the contrary if `d >> L`, `Ep` can be approximated if we consider the whole plate as being a point charge with `q = σ·A` (where `σ` is the surface charge density), see example 2 of [Coulomb’s Law & E Field](#Coulomb’s Law & E Field).

* To calculate `[Ep_N/C;σ_C/m^2]` (Electric Field; Linear charge density) at position `[d=5_mm]` above a square plate of width `[L=8_cm]` and surface `A=L^2` where `d << L` when `d < L/10` is verified:

```rpl
L=8_cm A='L^2' d=5_mm Q=6E-6_C  A=64_cm^2  εr=1
@ Expecting [ σ=9.375⁳⁻⁸ C/cm↑2 Ep=52 941 050.0044 N/C ]
if 'd < L/10' then
 'ROOT(ⒺE Field Infinite Plate;[σ;Ep];[1_C/cm^2;1_N/C])'
end
```

### Ohm’s Law & Power

* To calculate `[R_Ω;P_W]` (Resistance; Powe) from 2 known variables:

```rpl
V=24_V  I=16_A
@ Expecting [ R=1.5 Ω P=384. W ]
'ROOT(ⒺOhm’s Law & Power;[R;P];[1_Ω;1_W])'
```

### Volt Divider

* To calculate `[V1_V]` (Voltage) from 3 known variables:

```rpl
R1=40_Ω  R2=10_Ω  V=100_V
@ Expecting [ V1=80. V ]
'ROOT(ⒺVolt Divider;[V1];[1_V])'
```

### Current Divider

* To calculate `[I1_A]` (Current) from 3 known variables:

```rpl
R1=10_Ω  R2=6_Ω  I=15_A
@ Expecting [ I1=5.625 A ]
'ROOT(ⒺCurrent Divider;[I1];[1_A])'
```

### Wire Resistance

* To calculate `[R_Ω]` (Resistance) from 3 known variables:

```rpl
ρ=0.0035_Ω*cm  L=50_cm  A=1_cm^2
@ Expecting [ R=0.175 Ω ]
'ROOT(ⒺWire Resistance;[R];[1_Ω])'
```

### Resistivity & Conductivity

The electrical resistivity `ρ` of most materials changes with temperature. If the temperature `T` does not vary too much, a linear approximation can be used around the reference point (`ρ0`; `T0`).

* To calculate `[ρ_(Ω*m);σ_(S/m)]` (Resistance) from 4 known variables:

```rpl
ρ0=1.68E-8_Ω*m  αT=4.04E-3_K^-1  T0=293,15_K  T=373,15_K
@ Expecting [ ρ=2.22297 6⁳⁻⁸ Ω·m σ=44 984 741.1758 S/m ]
'ROOT(ⒺResistivity & Conductivity;[ρ;σ];[1_(Ω*m);1_(S/m)])'
```

### Series & Parallel R

![Series & Parallel R](img/Missing name.bmp)

* To calculate `[Rs_Ω;Rp_Ω]` (Resistance) from 2 known variables:

```rpl
R1=2_Ω  R2=3_Ω
@ Expecting [ Rs=5. Ω Rp=1.2 Ω ]
'ROOT(ⒺSeries & Parallel R;[Rs;Rp];[1_Ω;1_Ω])'
```

### Series & Parallel C

![Series & Parallel C](img/Missing name.bmp)

* To calculate `[Cs_μF;Cp_μF]` (Capacitance) from 2 known variables:

```rpl
C1=2_μF  C2=3_μF
@ Expecting [ Cs=1.2 μF Cp=5. μF ]
'ROOT(ⒺSeries & Parallel C;[Cs;Cp];[1_μF;1_μF])'
```

### Series & Parallel L

![Series & Parallel L](img/Missing name.bmp)

* To calculate `[Ls_mH;Lp_mH]` (Inductance) from 2 known variables:

```rpl
L1=17_mH  L2=16.5_mH
@ Expecting [ Ls=33.5 mH Lp=8.37313 43283 6 mH ]
'ROOT(ⒺSeries & Parallel L;[Ls;Lp];[1_mH;1_mH])'
```

### Capacitive Energy

* To calculate `[V_V;q_μC]` (Potential; Charge) from 2 known variables:

```rpl
E=0.025_J  C=20_μF
@ Expecting [ V=50. V q=1 000. μC ]
'ROOT(ⒺCapacitive Energy;[V;q];[1_V;1_μC])'
```

### Volumic Density Electric Energy

* To calculate `[uE_(J/m^3)]` (Volumic Density Electric Energy) from 2 known variables:

```rpl
E=5_V/m  εr=1
@ Expecting [ uE=1.10677 34772⁳⁻¹⁰ J/m↑3 ]
'ROOT(ⒺVolumic Density Electric Energy;[uE];[1_(J/m^3)])'
```

### Inductive Energy

* To calculate `[I_A]` (Current) from 2 known variables:

```rpl
E=4_J L=15_mH
@ Expecting [ I=23.09401 07676 A ]
'ROOT(ⒺInductive Energy;[I];[1_A])'
```

### RLC Current Delay

![RLC Current Delay](img/Missing name.bmp)

* To calculate `[ω_r/s;φs_°;φp_°;XC_Ω;XL_Ω]` (Phases and inpedances) from 4 known variables:

```rpl
f=107_Hz  C=80_μF  L=20_mH  R=5_Ω
@ Expecting [ ω=672.30082 7868 r/s φs=-45.82915 71488 ° φp=-5.87715 65317 1 ° XC=18.59286 71836 Ω XL=13.44601 65574 Ω ]
'ROOT(ⒺRLC Current Delay;[ω;φs;φp;XC;XL];[1_r/s;1_°;1_°;1_Ω;1_Ω])'
```

### DC Capacitor Current

These equations approximate the dc current required to charge the voltage on a capacitor in a certain time interval.

* To calculate `[ΔV_V;Δt_s;tf_s]` (Voltage difference, time difference, final time) from 5 known variables:

```rpl
C=15_μF  Vi=2.3_V  Vf=3.2_V  I=10_A  ti=0_μs
@ Expecting [ ΔV=0.9 V Δt=1.35 μs tf=1.35 μs ]
'ROOT(ⒺDC Capacitor Current;[ΔV;Δt;tf];[1_V;1_μs;1_μs])'
```

### Capacitor Charge

* To calculate `[q_C]` (Voltage difference, time difference, final time) from 2 known variables:

```rpl
C=20_μF  V=100_V
@ Expecting [ q=2.⁳⁻³ C ]
'ROOT(ⒺCapacitor Charge;[q];[1_C])'
```

### DC Inductor Voltage

These equations approximate the dc voltage induced in an inductor by a change in current in a certain time interval.

![DC Inductor Voltage](img/Missing name.bmp)

* To calculate `[ΔIL_A;ILf_A;tf_μs]` (Current difference, final current, final time) from 5 known variables:

```rpl
L=100_mH  V=52_V  Δt=32_μs  ILi=23_A  ti=0_μs
@ Expecting [ ΔIL=-1.664⁳⁻² A ILf=22.98336 A tf=32. μs ]
'ROOT(ⒺDC Inductor Voltage;[ΔIL;ILf;tf];[1_A;1_A;1_μs])'
```

### RC Transient

![RC Transient](img/Missing name.bmp)

* To calculate `[V_V]` (Voltage) from 5 known variables:

```rpl
Vi=0_V  C=50_μF  Vf=10_V  R=100_Ω  t=2_ms
@ Expecting [ V=3.29679 95396 4 V ]
'ROOT(ⒺRC Transient;[V];[1_V])'
```

### Cylindrical Capacitor

![Cylindrical Capacitor](img/Missing name.bmp)

* To calculate `[C_μF;ΔV_V]` (Capacitance, voltage) from 5 known variables:

```rpl
εr=1  Q=75_μC  ro=1_cm  ri=.999_cm  L=10_cm
@ Expecting [ C=5 560.46819 129 pF ΔV=13 488.07284 21 V ]
'ROOT(ⒺCylindrical Capacitor;[C;ΔV];[1_pF;1_V])'
```

### Solenoid Inductance

![Solenoid Inductance](img/Missing name.bmp)

* To calculate `[L_mH]` (Inductance) from 4 known variables:

```rpl
μr=2.5  n=40_1/cm  A=0.2_cm^2  h=3_cm
@ Expecting [ L=3.01592 89474 6⁳⁻² mH ]
'ROOT(ⒺSolenoid Inductance;[L];[1_mH])'
```

### Toroid Inductance

![Toroid Inductance](img/Missing name.bmp)

* To calculate `[L_mH]` (Inductance) from 4 known variables:

```rpl
μr=1  N=5000  h=2_cm  ri=2_cm  ro=4_cm
@ Expecting [ L=69.31471 80562 mH ]
@ Error in ri input data of HP50gAUR.pdf
'ROOT(ⒺToroid Inductance;[L];[1_mH])'
```

### Drift Speed & Current Density

* To calculate `[vd_m/s;J_(A/m^2);E_(V/m)]` (Drift speed, current density, E field) from 5 known variables:

```rpl
@ A='Ⓒπ*(0.1_cm)^2'  ρ='Ⓒqe*n'  σ='Ⓒqe*n*40_(cm^2/(V*s))'
@ Expecting [ vd=2.33733 41683 6⁳⁻⁵ m/s J=31.83098 86184 A/cm↑2 E=5.84333 54209⁳⁻³ V/m ]
I=1_A  n=8.5e28_(m^-3) A=3.14159 26535 90E-2_cm↑2 ρ=1.36185 01389E10_C/m↑3 σ=54 474 005.556_S/m
'ROOT(ⒺDrift Speed & Current Density;[vd;J;E];[1_m/s;1_(A/cm^2);1_(V/m)])'
```

### Electron & Hole Mobilities

In accordance with microscopic Ohm's law, the current density is proportional to the electric field. Holes and electrons therefore move at their average drift speeds during the mean free time between collisions. As long as the electric fields are not very high, the mobilities of holes and electrons are constant.

* To calculate `[μe_(cm^2/(V*s));μe_(cm^2/(V*s));J_(A/m^2);Je_(A/m^2);Jh_(A/m^2);σ_(S/m)]` (Electron & hole mobilities, current densities, conductivity) from 5 known variables:

```rpl
@ In Ge typical values can be μe=500_(cm^2/(V*s)) μh=200_(cm^2/(V*s)) meeff_kg='0.12*Ⓒme'
@ mheff_kg='0.5*Ⓒme'  ne=1.04e19_(cm^-3)  nh=6.0e18_(m^-3) E=6.0e-9_V/m

τc=4.09365 36801 40e-15_s meeff=1.09312 60456 68e-31 kg mheff=4.55469 18569 5e-31 kg nh=6.0e18_(m^-3) ne=1.04e19_(m^-3) E=6.0e-9_V/m
@ Wanted [ μe=49.03846 15384 64976 91445 22_cm↑2/(s·V) μh=60.00000 00000 03822 512o15 7_cm↑2/(s·V) Je=4.90266 05000 386⁳-11_A/m↑2 Jh=3.46070 15294 402⁳-11_A/m↑2 J=8.36336 20294 771⁳-11_A/m↑2 σ=0.01393 89367 15800 94039 7294_S/m ]
@ The solution below is different, but seems to validate4 with EvalEq
@ Expecting [ μe=60. cm↑2/(V·s) μh=14.4 cm↑2/(V·s) Je=5.99854 93176 9⁳⁻¹¹ A/m↑2 Jh=8.30568 36706 4⁳⁻¹² A/m↑2 J=6.82911 76847 6⁳⁻¹¹ A/m↑2 σ=1.13818 62807 9⁳⁻² S/m ]
'ROOT(ⒺElectron & Hole Mobilities;[μe;μh;Je;Jh;J;σ];[1_(cm^2/(V*s));1_(cm^2/(V*s));1_(A/m^2);1_(A/m^2);1_(A/m^2);1_(S/m)])'
```

Notes about manually-ordered solution:
```
@ 'ROOT(ⒺElectron & Hole Mobilities;[μe #4;μh #5;Je #2;Jh #3;J #1;σ #6];[1_(cm^2/(V*s));1_(cm^2/(V*s));1_(A/m^2);1_(A/m^2);1_(A/m^2);1_(S/m)])'

```
## Fluids

The variables in the Fluids section are:

* `ε`: Roughness (dim.: length)
* `μ`: Dynamic viscosity (dim.: force·time/volume, in SI: N·s/m^3)
* `ρ`: Density (dim.: mass/volume, in SI: kg/m^3)
* `ΔP`: Pressure change (dim.: force/area, in SI: pascal, Pa)
* `Δy`: Height change
* `ΣK`: Total fitting coefficients
* `A`: Cross-sectional area
* `A1, A2`: Initial and final ross-sectional areas
* `D`: Diameter
* `D1, D2`: Initial and final diameters
* `f`: Fanning friction factor
* `h`: Depth relative to P0 reference depth
* `hL`: Head loss (dim.: area/time^2, in SI: m^2/s^2)
* `L`: Length
* `M`: Mass flow rate (dim.: mass/time, in SI: kg/s)
* `n`: Kinematic viscosity (dim.: force·time/volume, in SI: N·s/m^3)
* `P`: Pressure at h (dim.: force/area, in SI: pascal, Pa)
* `P0`: Reference pressure (dim.: force/area, in SI: pascal, Pa)
* `P1, P2`: Initial and final pressures (dim.: force/area, in SI: pascal, Pa)
* `Q`: Volume flow rate (dim.: volume/time, in SI: m^3/s)
* `Reynolds`: Reynolds number
* `v1, v2`: Initial and final velocities
* `vavg`: Average velocity
* `W`: Power input (dim.: energy/time, in SI: watt, W)
* `y1, y2`: Initial and final heights

### Pressure At Depth

This equation describes hydrostatic pressure for an incompressible fluid. Depth `h` is positive downwards from the reference.

![Pressure At Depth](img/Missing name.bmp)

* To calculate `[P_kPa]` (Pressure) from 3 known variables:

```rpl
h=100_m  ρ=1025.1817_kg/m^3  P0=1_atm
@ Expecting [ P=1 106.68481 183 kPa ]
'ROOT(ⒺPressure at Depth;[P];[1_kPa])'
```

## Forces and Energy

The variables in the Force and Energy section are:

* `α`: Angular acceleration (dim.: angle/time^2, in SI: r/s^2)
* `ω`: Angular velocity (dim.: angle/time, in SI: r/s))
* `ωi, ωf`: Initial and final angular velocities (dim.: angle/time, in SI: r/s)
* `ρ`: Fluid density (dim.: mass/volume, in SI: kg/m^3)
* `τ`: Torque (dim.: force·length, in SI: N·m)
* `Θ`: Angular displacement (dim.: angle, in SI: r)
* `A`: Projected area relative to flow
* `ar`: Centripetal acceleration at r
* `at`: Tangential acceleration at r
* `Cd`: Drag coefficient
* `E`: Energy (dim.: force·length, in SI: joule, J)
* `F`: Force at r or x, or Spring force (Hooke’s Law), or attractive
force (Law of Gravitation), or Drag force (Drag force)
* `I`: Moment of inertia (dim.: mass·length^2, in SI: kg·m^2)
* `k`: Spring constant (dim.: force/length, in SI: N/m)
* `Ki, Kf`: Initial and final kinetic energies (dim.: force·length, in SI: joule, J)
* `m, m1, m2`: Mass
* `N`: Rotational speed (dim.: time^-1, in SI: hertz, Hz)
* `Ni, Nf`: Initial and final rotational speeds (dim.: time^-1, in SI: hertz, Hz)
* `P`: Instantaneous power (dim.: energy/time, in SI: watt, W)
* `Pavg`: Average power (dim.: energy/time, in SI: watt, W)
* `r`: Radius from rotation axis, or Separation distance (Law of Gravitation)
* `t`: Time
* `v`: Velocity
* `vf, v1f, v2f`: Final velocity
* `vi, v1i`: Initial velocity
* `W`: Work (dim.: force·length, in SI: joule, J)
* `x`: Displacement
* `UGf, UGi`: Final and initial gravitational potential energy (dim.: force·length, in SI: joule, J)

### 1D Elastic Collisions

![1D Elastic Collisions](img/Missing name.bmp)

* To calculate `[v1f_m/s;v2f_m/s]` (Final velocities of mass m1 & m2) from 3 known variables:

```rpl
m1=10_kg  m2=25_kg  v1i=100_m/s
@ Expecting [ v1f=-42.85714 28571 m/s v2f=57.14285 71429 m/s ]
'ROOT(Ⓔ1D Elastic Collisions;[v1f;v2f];[1_m/s;1_m/s])'
```

### Drag Force

* To calculate `[F_N]` (Drag force) from 4 known variables:

```rpl
Cd=0.05  ρ=1000_kg/m^3  A=7.5E6_cm^2  v=35_m/s
@ Expecting [ F=22 968 750. N ]
'ROOT(ⒺDrag Force;[F];[1_N])'
```

### Relativity Mass Energy

* To calculate `[E_J]` (Relativistic energy) from 1 known variable:

```rpl
m=9.1E-31_kg
@ Expecting [ E=8.17867 21247⁳⁻¹⁴ J ]
'ROOT(ⒺRelativity Mass Energy;[E];[1_J])'
```

## Gases

The variables in the Gases section are:

* `λ`: Mean free path (dim.: length)
* `ρ`: Flow density (dim.: mass/volume, in SI: kg/m^3)
* `ρr, ρri, ρrf`: Initial and final reduced state factors
* `ρ0`: Stagnation density (dim.: mass/volume, in SI: kg/m^3)
* `A`: Flow area
* `At`: Throat area
* `d`: Molecular diameter
* `k`: Specific heat ratio
* `M`: Mach number
* `m`: Mass
* `MW`: Molecular weight (dim.: mass/mole, in SI: g/gmol)
* `n`: Number of moles, or Polytropic constant (Polytropic Processes)
* `P`: Pressure, or Flow pressure (Isentropic Flow) (dim.: force/area, in SI: pascal, Pa)
* `P0`: Stagnation pressure (dim.: force/area, in SI: pascal, Pa)
* `Pc`: Pseudocritical pressure (dim.: force/area, in SI: pascal, Pa)
* `Pi, Pf`: Initial and final pressure (dim.: force/area, in SI: pascal, Pa)
* `Pr, Pri, Prf`: Initial and final Reduced Pressures
* `T`: Temperature, or Flow temperature (Isentropic Flow)
* `T0`: Stagnation temperature
* `Tc`: Pseudocritical temperature
* `Tr, Tri, trf`: Initial and final reduced temperatures
* `Ti, Tf`: Initial and final temperatures
* `V, Vi, Vf`: Initial and final volumes
* `vrms`: Root-mean-square (rms) velocity
* `W`: Work (dim.: force·length, in SI: joule, J)
* `Z, Zi, Zf`: Initial and final gas compressibility correction factors

### Ideal Gas Law Change

* To calculate `[Vf_l]` (Volume final) from 5 known variables:

```rpl
Pi=1.5_kPa  Pf=1.5_kPa  Vi=2_l  Ti=373.15_K  Tf=373.15_K
@ Expecting [ Vf=2. l ]
'ROOT(ⒺIdeal Gas Law Change;[Vf];[1_l])'
```

### Isothermal Expansion

These equations apply to an ideal gas.

* To calculate `[m_kg;W_J]` (Mass, work) from 5 known variables:

```rpl
Vi=2_l  Vf=125_l  T=573.15_°C  n=0.25_mol  MW=64_g/mol
@ Expecting [ m=1.6⁳⁻² kg W=7 274.34704 123 J ]
'ROOT(ⒺIsothermal Expansion;[m;W];[1_kg;1_J])'
```

### Polytropic Processes

These equations describe a reversible pressure-volume change of an ideal gas such that `P·Vn` is constant. Special cases include isothermal processes (`n = 1`), isentropic processes (`n = k`, the specific heat ratio), and constant-pressure processes (`n = 0`).

* To calculate `[n_1;Tf_°F]` (Polytropic number, final temperature) from 5 known variables:

```rpl
Pi=15_psi  Pf=35_psi  Vi=1_ft^3  Vf=0.50_ft^3  Ti=75_°F
@ Expecting [ n=1.22239 24213 4 Tf=346.54537 037 K ]
'ROOT(ⒺPolytropic Processes;[n;Tf];[1;1_°F])'
```

### Isentropic Flow

![Isentropic Flow](img/Missing name.bmp)

The calculation differs at velocities below and above Mach 1. The Mach number is based on the speed of sound in the compressible fluid.

* To calculate `[P_kPa;ρ_kg/m^3;At_cm^2]` (Flow pressure, flow density, throat area) from 7 known variables:

```rpl
k=2  M=0.9  T0=300_K  T=373.15_K  ρ0=100_kg/m^3  P0=100_kPa  A=1_cm^2
@ Expecting [ P=154.71213 6111 kPa ρ=124.38333 3333 kg/m↑3 At=0.99280 71853 34 cm↑2 ]
'ROOT(ⒺIsentropic Flow;[P;ρ;At];[1_kPa;1_kg/m^3;1_cm^2])'
```

## Heat transfer

The variables in the Heat Transfer section are:

* `α`: Expansion coefficient (dim.: 1/temperature, in SI: K^-1)
* `δ`: Elongation (dim.: length)
* `λ1, λ2`: Lower and upper wavelength limits (dim.: length)
* `λmax`: Wavelength of maximum emissive power (dim.: length)
* `ΔT`: Temperature difference
* `A`: Area
* `c`: Specific heat (dim.: energy/(mass·temperature), in SI: J/(kg·K))
* `eb12`: Emissive power in the range λ1 to λ2 (dim.: power/area, in SI: W/m^2)
* `eb`: Total emissive power (dim.: power/area, in SI: W/m^2)
* `f`: Fraction of emissive power in the range λ1 to λ2
* `h, h1, h3`: Convective heat-transfer coefficient (dim.: power/(area·temperature) in SI: W/(m^2·K))
* `k, k1, k2, k3`: Thermal conductivity (dim.: power/(length·temperature) in SI: W/(m·K))
* `L, L1, L2, L3`: Length
* `m`: Mass
* `Q`: Heat capacity (dim.: energy, in SI: joule, J)
* `qr`: Heat transfer rate (dim.: power=energy/time, in SI: watt, W)
* `T`: Temperature
* `Tc`: Cold surface temperature (Conduction), or Cold fluid temperature (Convection)
* `Th`: Hot surface temperature, or Hot fluid temperature (Conduction + Convection)
* `Ti, Tf`: Initial and final temperatures
* `U`: Overall heat transfer coefficient (dim.: power/(area·temperature) in SI: W/(m^2·K))

### Thermal Expansion

![Thermal Expansion](img/Missing name.bmp)

* To calculate `[α_K^-1;Ti_°C]` (Expansion coefficient, initial temperature) from 4 known variables:

```rpl
ΔT=15_°C  L=10_m  Tf=25_°C  δ=1_cm
@ Expecting [ α=6.66666 66666 7⁳⁻⁵ K⁻¹ Ti=10. °C ]
'ROOT(ⒺThermal Expansion;[α;Ti];[1_K^-1;1_°C])'
```

### Convection

![Convection](img/Missing name.bmp)

* To calculate `[ΔT_°C;Th_°C]` (Temperature difference, hot surface temperature) from 4 known variables:

```rpl
Tc=26.85_°C  A=200_m^2  h=0.005_W/(m^2*K)  qr=10_W
@ Expecting [ ΔT=10. °C Th=36.85 °C ]
'ROOT(ⒺConvection;[ΔT;Th];[1_°C;1_°C])'
```

### Conduction + Convection

If you have fewer than three layers, give the extra layers a zero thickness and any nonzero conductivity. The two temperatures are fluid temperatures – if instead you know a surface temperature, set the corresponding convective coefficient to 10^999999.

![Conduction + Convection](img/Missing name.bmp)

* To calculate `[qr_W;Tc_°C;U_W/(m^2*K)]` (Heat transfer rate, cold surface temperature, overall heat transfer coefficient) from 11 known variables:

```rpl
ΔT=35_°C  Th=55_°C  A=10_m^2  h1=0.05_W/(m^2*K)  h3=0.05_W/(m^2*K)  L1=3_cm  L2=5_cm  L3=3_cm  k1=0.1_W/(m*K)  k2=.5_W/(m*K)  k3=0.1_W/(m*K)
@ Expecting [ qr=8.59950 85995 1 W Tc=20. °C U=0.02457 00245 7 W/(m↑2·K) ]
'ROOT(ⒺConduction & Convection;[qr;Tc;U];[1_W;1_°C;1_W/(m^2*K)])'
```

## Magnetism

The variables in the Magnetism section are:

* `α1, α2`: Subtended internal angles relative to the top ends of the solenoid
* `θ1, θ2`: Subtended angles relative to the ends of the wire
* `θ`: Angle between the line of the magnetic field and the speed of the moving charge
* `μr`: Relative permeability
* `B`: Magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `d`: Separation distance
* `Dpitch`: Pitch of the helicoidal mouvement (dim.: length)
* `Fba`: Force
* `fc`: Cyclotron frequency (dim.: time^-1, in SI: hertx, Hz)
* `I, Ia, Ib`: Current (dim.: charge/time, in SI: ampere, A)
* `L`: Length
* `m`: Mass
* `N`: Total number of turns
* `nl`: Number of turns per unit length (dim.: length^-1)
* `q`: Charge of the moving charge (dim.: charge, in SI: coulomb, C)
* `r`: Distance from center of wire
* `Rc`: Radius of the circular part of the movement
* `ri, ro`: Inside and outside radii of toroid
* `rw`: Radius of wire
* `uB`: Volumic density of magnetic energy (dim.: energy/volume, in SI: J/m^3)
* `v`: Speed of the moving charge
* `T`: Period (dim.: time)
* `VH`: Hall tension (dim.: energy/charge, in SI: volt, V)

#### Straight Wire Infinite

The magnetic field expression differs depending upon whether the point at `r` is inside or outside the wire of radius `rw` and the calculations are done accordingly. The expression for the magnetic field at the distance `r` is approximately valid if the distance is such that `r << L` and therefore also applies for a wire of finite length `L`.

#### Straight Wire Finite

The expression for the magnetic field at the distance r depends on the subtended angles θ1 and θ2 relative to the ends of the wire of finite length L. The magnetic field expression differs depending upon whether the point at `r` is inside or outside the wire of radius `rw` and the calculations are done accordingly.

![B Field From Finite Wire](img/B Field From Finite Wire.bmp)


#### Force Between Wires

The force between wires is positive for an attractive force (for currents having the same sign) and negative otherwise, corresponding to a repulsive force.

#### B Field In Infinite Solenoid

The expression for the magnetic field in the center is approximately valid if the radius of the solenoid < `L` and therefore also applies inside a solenoid of finite length `L`.

#### B Field In Finite Solenoid

The expression for the magnetic field in the center depends on the subtended internal angles `α1` and `α2` relative to the top ends of the solenoid of finite length `L`.

![B Field In Finite Solenoid](img/B Field In Finite Solenoid.bmp)

#### B Field In Toroid

#### Hall Effect

The moving charge carrier is deflected by the magnetic field to create the Hall tension `VH` between the opposite sides of the conductor.

#### Cyclotron Mouvement

Under the perpendicular magnetic field, the moving charge has a circular trajectory and turns at the cyclotron frequency with the rotation period `T`.

#### Helicoidal Mouvement

Under the magnetic field lines (at angle `θ` with the speed vector), the moving charge has an helicoidal trajectory of pitch `Dpitch`, radius `Rc` and period `T`.

#### Volumic Density Magnetic Energy

## Motion

The variables in the Motion section are:

* `α`: Angular acceleration (dim.: angle/time^2, in SI: r/s^2)
* `ω`: Angular velocity (Circular Motion), or Angular velocity at t (Angular Motion) (dim.: angle/time, in SI: r/s)
* `ω0`: Initial angular velocity (dim.: angle/time, in SI: r/s)
* `ρ`: Fluid density (dim.: mass/volume, in SI: kg/m^3)
* `θ`: Angular position at t (dim.: angle)
* `θ0`: Initial angular position (Angular Motion), or Initial vertical angle (Projectile Motion)
* `φ`: Latitude (dim.: angle)
* `a`: Acceleration
* `Ah`: Projected horizontal area
* `ar`: Centripetal acceleration at r
* `Cd`: Drag coefficient
* `gloc`: local gravitational acceleration of a planet or star (dim.: length/time^2)
* `gearth`: local gravitational acceleration on Earth (dim.: length/time^2)
* `h`: altitude (dim.: length)
* `hmax`: maximum height (Projectile Motion)
* `m`: Mass
* `Mp`: Planet or star mass
* `N`: Rotational speed (dim.: time^-1, in SI: hertz, Hz)
* `R`: Horizontal range (Projectile Motion), or Planet or Star radius (Object in Free Fall and Escape Velocity)
* `rc`: Radius of circular motion
* `t`: Time
* `tf`: Time of flight of a projectile
* `v`: Velocity at t (linear Motion), or Tangential velocity at r (Circular Motion), or Falling velocity at time t (Terminal Velocity)
* `v0`: Initial velocity
* `ve`: Escape velocity in a gravitational field
* `vcx`: Horizontal (component x) of velocity at t
* `vcy`: Vertical (component y) of velocity at t
* `vo`: Orbital velocity in a gravitational field
* `vt`: Terminal velocity reached in a vertical fall
* `x`: Horizontal position at t
* `x0`: Initial horizontal position
* `y`: Vertical position at t
* `y0`: Initial vertical position

#### Linear Motion

#### Object In Free Fall

By definition, an object in free fall only experiences local gravitational acceleration `gloc`. This depends on the mass of the star or planet and the distance `r` center to center (where we assume that the position is greater than the radius of the mass). For the Earth, we can calculate an approximate value `gearth` of the acceleration of gravity as a function of latitude `φ` and for an altitude `h` low compared to the Earth's radius (typically: a few thousand meters, valid in commercial aviation).

// Reference: Commissions romandes de mathématique, de physique et de chimie, Formulaires et tables : Mathématiques, Physique, Chimie, Tricorne, 2000, 278

#### Projectile Motion

During the time of flight `tf`, the motion of a projectile follows a symetric parabole of horizontal range `R` and of maximum height `hmax`.

#### Angular Motion

#### Uniform Circular Motion

#### Terminal Velocity

Terminal velocity is the maximum speed attainable by an object as it falls through a fluid like air for instance. It is reached when the sum of the increasing drag force plus the buoyancy is equal to the downward force of gravity acting on the object, leading to a zero net force at the resulting terminal velocity.

#### Escape and Orbital Velocities

The escape velocity is the speed required to completely free oneself from the gravitational field of a star, planet, etc. It is defined as the initial speed allowing you to move away to infinity. The orbital velocity is the speed nneded to maintain a stable circular orbit in a gravitational field.

## Optics

The variables in the Optics section are:

* `Δα`: Phase difference between top and bottom rays of light from a single slit (dim.: angle, in SI: r)
* `Δφ`: Phase difference between two rays of light seperated by an angle θ (dim.: angle, in SI: r)
* `Δyint`: Distance between bright fringes on the interference screen
* `Δydiff`: Width of the central maximum of diffraction
* `λ`: Light wavelength
* `θ`: Angle between initial light polarisation direction and polarizer transmission axis, or Angle subtended by two points separated by y on the screen and the middle of one or two slits
* `θ0`: Acceptance angle to enter an optical fiber in the outer medium of refraction index n0
* `θ1`: Angle of incidence in the medium of refraction index n1
* `θ2`: Angle of refraction in the medium of refraction index n2
* `θB`: Brewster angle
* `θc`: Critical angle
* `θr`: Rayleigh's criterion limiting
* `a`: Width of a diffraction slit
* `d`: Distance between two interference slits
* `E0`: Incident light electrtc field
* `f`: Focal length
* `fx0`: Frequency of the incident X-ray (dim.: time^-1, in SI: hertz, HZ)
* `fx`: Frequency of the transmitted X-ray (dim.: time^-1, in SI: hertz, HZ)
* `I`: Transmitted irradiance or polarized light radiance flux (dim.: power/area, in SI: W/m^2)
* `I0`: Incident irradiance or polarized light radiance flux (dim.: power/area, in SI: W/m^2)
* `Ix`: Transmitted irradiance or polarized X rays radiance flux (dim.: power/area, in SI: W/m^2)
* `Ix0`: Incident irradiance or polarized X rayx radiance flux (dim.: power/area, in SI: W/m^2)
* `m`: Magnification
* `n, n1, n2`: Index of refraction
* `nf0`: Index of refraction of the outer medium at the entry of an optic fiber
* `nf1`: Index of refraction of the core medum of an optic fiber
* `nf2`: Index of refraction of the cladding medium of an optic fiber
* `r, r1, r2`: Radius of curvature
* `u`: Distance to object
* `v`: Distance to image
* `v1, v2`: Speed of light in media of refraction index n1, n2
* `vn`: Speed of light in medium of refraction index n
* `L`: distance to the observation screen or to the light detector
* `NA`: Numerical aperture of the optic fiber
* `y`: distance between two image points on the observation screen (perpendicular to the optical axis)

For reflection and refraction problems, the focal length and radius of curvature are positive in the direction of the outgoing light (reflected or refracted). The object distance is positive in front of the surface. The image distance is positive in the direction of the outgoing light (reflected or refracted). The magnification is positive for an upright image.

#### Refraction Law

#### Critical Angle

#### Fiber Optic

#### Brewster’s Law

The Brewster angle is the angle of incidence at which the reflected wave is completely polarized.

#### Spherical Reflection

#### Spherical Refraction

#### Thin Lens

#### Rayleigh’s Criterion

#### Malus Law

If lineraly polarized light is incident on a perfect linear polarizer the transmitted light is the component at angle `θ` between the light polarisation direction and the polarizer transmission axis. The Malus law is given in terms of light irradiances. A relavistic version of the laws applies for X rays and more energetic electromagnetic radiations (with loss up to 10% in irradiance). The decrease in frequency (`fx < fx0`) and thefore in energy (hfx) of a transmitted photon is due to the movement of the interacting electron of the polarizer (Compton scattering).

#### 2 Slits Young Interference

#### One Slit Diffraction

## Oscillations

The variables in the Oscillations section are:

* `ω`: Angular frequency (dim.: angle/time, in SI: r!s)
* `ω0`: Natural angular frequency (dim.: angle#time, in SI: r!s)
* `ωu`: Undamped angular frequency (dim.: angle/time, in SI: r!s)
* `γ`: Reduced damping coefficient (dim.: angle/time, in SI: r/s)
* `φ`: Phase angle
* `θ`: Cone angle
* `a`: Acceleration at t
* `E`: Total energy (kinetic plus potential)
* `f`: Frequency (dim.: time^-1; in SI: hertz, Hz)
* `G`: Shear modulus of elasticity (dim.: pressure, in SI: pascal, Pa)
* `h`: Cone height
* `I`: Moment of inertia (dim.: mass·area, in SI: kg·m^2)
* `J`: Polar moment of inertia (dim.: length^4; in SI: m^4)
* `k`: Spring constant (dim.: force/length, in SI: N/m)
* `L`: Length of pendulum
* `m`: Mass
* `Q`: Quality factor
* `t`: Time
* `T`: Period
* `v`: Velocity at t
* `x`: Displacement at t
* `xm`: Displace amplitude

#### Mass-Spring System

#### Simple Pendulum

#### Conical Pendulum

#### Torsional Pendulum

#### Simple Harmonic

#### Underdamped Oscillations

#### Driven Damped Oscillations

## Plane Geometry

The variables in the Plane Geometry section are:

* `β`: Central angle of polygon
* `θ`: Vertex angle of polygon
* `A`: Area
* `b`: Base length (Rectangle, Triangle), or Length of semiaxis in x direction (Ellipse)
* `C`: Circumference
* `d`: Distance to rotation axis in y direction
* `h`: Height (Rectangle, Triangle), or Length of semiaxis in y direction (Ellipse)
* `I, Ix`: Moment of inertia about x axis (dim.: mass·area, in SI: kg·m^2)
* `Id`: Moment of inertia in x direction at distance d (dim.: mass·area, in SI: kg·m^2)
* `Iy`: Moment of inertia about y axis (dim.: mass·area, in SI: kg·m^2)
* `J`: Polar moment of inertia at centroid (dim.: mass·area, in SI: kg·m^2)
* `L`: Side length of polygon
* `n`: Number of sides
* `P`: Perimeter
* `r`: Radius
* `ri, ro`: Inside and outside radii
* `rs`: Distance to side of polygon
* `rv`: Distance to vertex of polygon
* `v`: Horizontal distance to vertex

#### Circle

#### Ellipse

#### Rectangle

#### Regular Polygon

#### Circular Ring

#### Triangle

## Solid geometry
The variables in the Solid Geometry section are:

* `A`: Total surface area
* `b`: Base length
* `d`: Distance to rotation axis in z direction
* `h`: Height in z direction (Cone, Cylinder), or Height in y direction (Parallelepiped)
* `I, Ixx`: Moment of inertia about x axis (dim.: mass·area, in SI: kg·m^2)
* `Id`: Moment of inertia in x direction at distance d (dim.: mass·area, in SI: kg·m^2)
* `Izz`: Moment of inertia about z axis (dim.: mass·area, in SI: kg·m^2)
* `m`: Mass
* `r`: Radius
* `t`: Thickness in z direction (dim.: length)
* `V`: Volume

#### Cone

#### Cylinder

#### Parallelepiped

#### Sphere

## Solid State Devices
The variables in the Solid State Devices section are:

* `αF`: Forward common-base current gain
* `αR`: Reverse common-base current gain
* `γ`: Body factor (dim.: (energy/charge)^0.5, in SI: V^0.5)
* `λ`: Modulation parameter (dim.: charge/energy, in SI: V^-1)
* `μn`: Electron mobility (dim.: speed/(electric field), in SI: m^2/(V·s))
* `φp`: Fermi potential (dim.: energy/charge, in SI: volt, V)
* `ΔL`: Length adjustment (PN Step Junctions), or Channel encroachment (NMOS Transistors) (dim.: length)
* `ΔW`: Width adjustment (PN Step Junctions), or Width contraction (NMOS Transistors)
* `a`: Channel thickness
* `Aj`: Effective junction area
* `BV`: Breakdown voltage (dim.: energy/charge, in SI: volt, V)
* `Cj`: Junction capacitance per unit area (dim.: charge^2/(energy·area); in SI: F/m^2)
* `Cox`: Silicon dioxide capacitance per unit area (dim.: charge^2/(energy·area); in SI: F/m^2)
* `E1`: Breakdown-voltage field factor (dim.: force/charge, in SI: N/C=V/m)
* `Emax`: Maximum electric field (dim.: force/charge, in SI: N/C=V/m)
* `G0`: Channel conductance (dim.: resistance^-1, in SI: siemens, S)
* `gds`: Output conductance (dim.: resistance^-1, in SI: siemens, S)
* `gm`: Transconductance (dim.: resistance^-1, in SI: siemens, S)
* `I`: Diode current (dim.: charge/time, in SI: ampere, A)
* `IB`: Total base current (dim.: charge/time, in SI: ampere, A)
* `IC`: Total collector current (dim.: charge/time, in SI: ampere, A)
* `ICEO`: Collector current (collector-to-base open) (dim.: charge/time, in SI: ampere, A)
* `ICO`: Collector current (emitter-to-base open) (dim.: charge/time, in SI: ampere, A)
* `ICS`: Collector-to-base saturation current (dim.: charge/time, in SI: ampere, A)
* `ID, IDS`: Drain current (dim.: charge/time, in SI: ampere, A)
* `IE`: Total emitter current (dim.: charge/time, in SI: ampere, A)
* `IES`: Emitter-to-base saturation current (dim.: charge/time, in SI: ampere, A)
* `IS`: Transistor saturation current (dim.: charge/time, in SI: ampere, A)
* `J`: Current density (dim.: current/area, in SI: A/m^2)
* `Js`: Saturation current density (dim.: current/area, in SI: A/m^2)
* `L`: Drawn mask length (PN Step Junctions), or Drawn gate length (NMOS Transistors), or Channel length (JFETs)
* `Le`: Effectives gate length
* `NA`: P-side doping (PN Step Junctions), or Substrate doping (NMOS Transistors) (dim.: 1/volume, in SI: m^-3)
* `ND`: N-side doping (PN Step Junctions), or N-channel doping (JFETs) (dim.: 1/volume, in SI: m^-3)
* `T`: Temperature
* `tox`: Gate silicon dioxide thickness
* `Va`: Applied voltage (dim.: energy/charge, in SI: volt, V)
* `VBC`: Base-to-collector voltage (dim.: energy/charge, in SI: volt, V)
* `VBE`: Base-to-emitter voltage (dim.: energy/charge, in SI: volt, V)
* `Vbi`: Built-in voltage (dim.: energy/charge, in SI: volt, V)
* `VBS`: Substrate voltage (dim.: energy/charge, in SI: volt, V)
* `VCEsat`: Collector-to-emitter saturation voltage (dim.: energy/charge, in SI: volt, V)
* `VDS`: Applied drain voltage (dim.: energy/charge, in SI: volt, V)
* `VDsat`: Saturation voltage (dim.: energy/charge, in SI: volt, V)
* `VGS`: Applied gate voltage (dim.: energy/charge, in SI: volt, V)
* `Vt`: Threshold voltage (dim.: energy/charge, in SI: volt, V)
* `Vt0`: Threshold voltage (at zero substrate voltage) (dim.: energy/charge, in SI: volt, V)
* `W`: Drawn mask width (PN Step Junctions), or Drawn width (NMOS Transistors), or Channel width (JFETs)
* `We`: Effective width
* `xd`: Depletion-region width
* `xdmax`: Depletion-layer width
* `xj`: Junction depth

#### PN Step Junctions

These equations for a silicon PN-junction diode use a “two-sided step-junction” model–the doping density changes abruptly at the junction. The equation assume the current density is determined by minority carries injected across the depletion region and the PN junction is rectangular in its layout, The temperature should be between 77 and 500 K.

#### NMOS Transistor

These equations for a silicon NMOS transistor use a two-port network model. They include linear and nonlinear regions in the device characteristics and are based on a gradual-channel approximation (the electric fields in the direction of current flow are small compared to those perpendicular to the flow). The drain current and transconductance calculations differ depending on whether the transistor is in the linear, saturated, or cutoff region. The equations assume the physical geometry of the device is a rectangle, second-order length-parameter effects are negligible, shot-channel, hot-carrier, and velocity-saturation effects are negligible, and subthreshold currents are negligible.

#### Bipolar Transistors

These equations for an NPN silicon bipolar transistor are based on large-signal models developed by J.J. Ebers and J.L. Moll. The offset-voltage calculation differs depending on whether the transistor is saturated or not. The equations also include the special conditions when the emitter-base or collector-base junction is open, which are convenient for measuring transistor parameters.

#### JFETs

These equations for a silicon N-channel junction field-effect transistor (JFET) are based on the single-sided stepjunction
approximation, which assumes the gates are heavily doped compared to the channel doping,. The draincurrent
calculation differs depending on whether the gate-junction depletion-layer thickness is less than or greater
than the channel thickness. The equations assume the channel is uniformly doped and end effects (such as contact,
drain, and source resistances) are negligible.

## Stress Analysis
The variables in the Stress Analysis section are:

* `δ`: Elongation
* `ϵ`: Normal strain
* `γ`: Shear strain (dim.: angle)
* `φ`: Angle of twist
* `σ`: Normal stress (dim.: pressure, in SI: pascal, Pa)
* `σ1`: Maximum principal normal stress (dim.: pressure, in SI: pascal, Pa)
* `σ2`: Minimum principal normal stress (dim.: pressure, in SI: pascal, Pa)
* `σavg`: Normal stress on place of maximum shear stress (dim.: pressure, in SI: pascal, Pa)
* `σx`: Normal stress in x direction (dim.: pressure, in SI: pascal, Pa)
* `σx1`: Normal stress in rotated-x direction (dim.: pressure, in SI: pascal, Pa)
* `σy`: Normal stress in y direction (dim.: pressure, in SI: pascal, Pa)
* `σy1`: Normal stress in rotated-y direction (dim.: pressure, in SI: pascal, Pa)
* `τ`: Shear stress (dim.: pressure, in SI: pascal, Pa)
* `τmax`: Maximum shear stress (dim.: pressure, in SI: pascal, Pa)
* `τx1y1`: Rotated shear stress (dim.: pressure, in SI: pascal, Pa)
* `τxy`: Shear stress (dim.: pressure, in SI: pascal, Pa)
* `θ`: Rotation angle
* `θp1`: Angle to plane of maximum principal normal stress
* `θp2`: Angle to plane of minimum principal normal stress
* `θs`: Angle to plane of maximum shear stress
* `A`: Area
* `E`: Modulus of elasticity (dim.: pressure, in SI: pascal, Pa)
* `G`: Shear modulus of elasticity (dim.: pressure, in SI: pascal, Pa)
* `J`: Polar moment of inertia (dim.: length^4, in SI: m^4)
* `L`: Length
* `P`: Load (dim.: force, in SI: newton, N)
* `r`: Radius
* `T`: Torque (dim.: force·length, in SI: N·m)

#### Stress Analysis

#### Shear Stress

#### Stress On An Element

Stresses and strains are positive in the directions shown in the picture.

#### Mohr’s Circle

## Waves
The variables in the Waves section are:

* `β`: Sound intensity level (dim.: intensity in logarithmic scale, in SI: decibel, dB)
* `Δp`: Sound pressure variafion around atmospheric pressure (dim.: force/area, in SI: pascal, Pa)
* `Δpm`: Amplitude of sound pressure variafion around atmospheric pressure (dim.: force/area, in SI: pascal, Pa)
* `λ`: Wavelength
* `ω`: Angular frequency (dim.: angle/time. in SI: r/s^2)
* `ρ`: Volumic density of medium (dim.: mass/volume, in SI: kg/m^3)
* `θcone`: Angle at the tip of the cone formed by the supersonic shockwave
* `μ`: Linear density of a string (dim.: mass/length, in SI: kg/m)
* `as`: Acceleration at x and t of vibrating particles (Longitudinal Waves), or Acceleration at x and t of air particles (Sound Waves)
* `ay`: Acceleration at x and t of vibrating particles (transversal waves)
* `B`: Bulk modulus of elasticity (Sound Waves) (dim.: pressure, in SI: Pa), or Magnetic field (Electromagnetic Waves) (dim.: mass/(time^2·current), in SI: T)
* `E`: Electric field at x and t of an electromagnetic wave (dim.: force/charge, in SI: N/C=V/m)
* `Em`: Electric Field amplitude (dim.: force/charge, in SI: N/C=V/m)
* `f`: Frequency (dim.: 1/time; in SI: hertz, Hz)
* `favg`: Frequency average (dim.: 1/time; in SI: hertz, Hz)
* `fbeat`: Beat frequency (dim.: 1/time; in SI: hertz, Hz)
* `f0`: Emission frequency (dim.: 1/time; in SI: hertz, Hz)
* `ffixed-fixed`: frequency of harmonics on a string fixed at both ends (dim.: 1/time; in SI: hertz, Hz)
* `ffixed-free`: frequency of harmonics on a string fixed at one end and free at the other end (dim.: 1/time; in SI: hertz, Hz)
* `fopen-open`: frequency of harmonics in a tube open at both ends (dim.: 1/time; in SI: hertz, Hz)
* `fopen-close`: frequency of harmonics in a tube open at one end and close at the other end (dim.: 1/time; in SI: hertz, Hz)
* `I`: Sound intensity (dim.: energy/(area·time), in SI: W/m^2)
* `k`: Angular wave number (dim.: agle/length, in SI: r/m)
* `ninteger`: Harmonics number being an integer number
* `nodd`: Harmonics number being an odd number
* `Ps`: Power of the source (dim.: energy/time, in SI: watt, W)
* `s`: Longitudinal displacement at x and t of vibrating particles (Longitudinal Waves), or Longitudinal displacement at x and t of air particles (Sound Waves)
* `sm`: Longitudinal displacement amplitude of vibrating particles (Longitudinal Waves), or Longitudinal displacement amplitude of air particles (Sound Waves)
* `t`: Time
* `u`: Mass or flow velocity (Mach Number)
* `v`: Velocity of the propagating sound in medium (Sound Waves), or Wave speed (Transverse Waves, Longitudinal Waves)
* `vs`: Velocity at x and t of vibrating particles (Longitudinal Waves), or Velocity at x and t of air particles (Sound Waves)
* `vsair`: Velocity of the propagating sound in the ait as a function of temperature

#### Transverse Waves

#### Longitudinal Waves

#### Sound Waves

#### Doppler Effect

In the classical Doppler effect it is assumed that the speed of the observer and the source are lower than the speed of sound in the air. The speed of the receiver relative to the air `vr` is added to the speed of sound in the air if the receiver is moving towards the source, subtracted if the receiver is moving away from the source whatever the movement of the source. The speed of the source relative to the air `va` is subtracted from the speed of sound in the air if the source is moving towards the receiver, added if the source is moving away from the receiver whatever the movement of the receiver.

#### Mach Number

For an object moving at a supersonic speed, the shockwave describes a cone having the angle `θcone` at its tip where the opposite side is the distance travelled by the sound and the hypothenuse is the distance travelled by the object.

#### String Standing Waves

A string being fixed or free at its ends admits only discrete harmonics as standing waves on the string. A string being fixed (or free) at both ends admits all integer harmonics. A string being being fixed at one end and free at the other end admits only all odd integer harmonics.

#### Sound Wave Harmonics

A tube being open or closed at its ends admits only discrete harmonics as standing waves of the sound in the air within the tube. A tube being open (or closed) at both ends admits all integer harmonics. A tube being being open at one end and closed at the other end admits only all odd integer harmonics.

#### Beat Acoustics

In acoustics, a beat is an interference pattern between two sounds of slightly different frequencies, perceived as a periodic variation in amplitude whose rate is the difference of the two frequencies. The sum of two unit-amplitude sine waves can be expressed as a carrier wave of frequency `favg` whose amplitude is modulated by an envelope wave of frequency `fbeat`.

#### Electromagnetic Waves

## Relativity
The variables in the Relativity section are:

* `α`: Light Doppler effect, light arrival angle in the rest frame
* `β`: Velocity relativistic speed ratio
* `βe`: Escape velocity relativistic speed ratio
* `βp`: Plane velocity relativistic speed ratio
* `βg`: Ground velocity relativistic speed ratio
* `Δt`: Proper time interval (Time Dilation), or Duration of the circumnavigation trip (Circumnavigating Airplanes)
* `Δtp`: Dilated time interval
* `ΔtpG`: Gravitationally dilated time interval
* `Δx`: Proper space interval
* `Δxp`: Contracted space interval
* `λ1, λ2`: Wavelength of the photon as measured by the observer at position R1 and R2
* `λ∞`: Wavelength of the photon as measured by the observer at infinity
* `γ`: Lorentz factor
* `γ21`: Factor of combined special and general relativity effects
* `γv1, γv2`: Lorentz factor for velocities v1 and v2
* `γG`: Lorentz factor associated to gravitational dilation
* `γG1, γG2`: Lorentz factor associated to gravitational dilation at heights h1 and h2
* `ω`: Angular velocity of a rotating planet during a day (dim.: angle/time, in SI: r/s)
* `φ`: Latitude (dim.: angle)
* `θ` : Aberration of light, emission angle in the frame at rest
* `θp` : Aberration of light, emission angle in the moving frame
* `ΔτWE`: Time difference between westward and eastward flights
* `ΔτE`: Flight time in the eastward direction
* `ΔτW`: Flight time in the westward direction
* `Δτg`: Elapsed time variation due to the ground tangential velocity
* `ΔτpE`: Elapsed time variation due to the plane altitude and velocity in the eastward direction
* `ΔτpW`: Elapsed time variation due to the plane altitude and velocity in the westward direction
* `As`: Schwarzschild  black hole surface area
* `ax`: X component of the acceleration
* `ay`: Y component of the acceleration
* `az`: Z component of the acceleration
* `apx`: Transformed x component of the acceleration observed in the moving frame
* `apy`: Transformed y component of the acceleration observed in the moving frame
* `apz`: Transformed z component of the acceleration observed in the moving frame
* `Bx`: X component of the magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `By`: Y component of the magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `Bz`: Z component of the magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `Bpx`: Transformed x component of the magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `Bpy`: Transformed y component of the magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `Bpz`: Transformed z component of the magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `E`: Total energy
* `Ep`: Transformed total energy
* `E0`: Rest energy associated to the rest mass
* `Ex`: X component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `Ey`: Y component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `EZ`: Z component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `Epx`: Transformed x component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `Epy`: Transformed y component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `Epz`: Transformed z component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `f` = Light Doppler effect, frequency received in the frame at rest (dim.: 1/time, in SI: hertz, Hz)
* `fp` = Light Doppler effect, frequency emitted in the moving frame (dim.: 1/time, in SI: hertz, Hz)
* `fs`: Wave frequency of the source (dim.: 1/time, in SI: hertz, Hz)
* `frl`: Relativistic longitudinal Doppler frequency (dim.: 1/time, in SI: hertz, Hz)
* `frt`: Relativistic transverse Doppler frequency (dim.: 1/time, in SI: hertz, Hz)
* `h1, h2`: Height of points 1 and 2
* `hp`: Altitude of plane (dim.: length)
* `K`: Kinetic energy
* `M`: Mass of planet
* `m0`: Rest mass
* `MGu`: Reduced gravitational mass (dim.: length)
* `Mxearth`: Mass factor as a multiple of Earth mass
* `Mxsun`: Mass factor as a multiple of Sun mass
* `MxSagA`: Mass factor as a multiple of SagitariusA* mass
* `Pθ` = Light relativistic beaming, angular distribution of photon in the moving frame from a source isotropic and stationary
* `p`: Momentum (dim.: mass·speed, in SI: kg·m/s)
* `px`: X component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `pz`: Y component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `pz`: Z component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `ppx`: Transformed x component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `ppz`: Transformed y component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `ppz`: Transformed z component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `PBH`: Black hole evaporation power (dim.: energy/time, in SI: watt, W)
* `R`: Planet radius
* `R1, R2`: Radius to points 1 and 2
* `Rem`: Radius at which the photon is emitted
* `rs`: Schwarzschild  radius
* `rxearth`: Radius factor as a multiple of Earth radius
* `SBH`: Black hole entropy (dim.: energy/temperature, in SI: J/K)
* `Tday`: Day duration
* `tev`: Evaporation time of a black hole
* `TH`: Black hole temperature
* `txyr`: Duration factor as a multiple of a year
* `ux`: X component of the velocity
* `uy`: Y component of the velocity
* `uz`: Z component of the velocity
* `upx`: Transformed x component of the velocity observed in the moving frame
* `upy`: Transformed y component of the velocity observed in the moving frame
* `upz`: Transformed z component of the velocity observed in the moving frame
* `v1, v2`: Velocity at heights h1 and h2
* `vg`: Tangential speed of the ground of a rotatong planet
* `Vs`: Schwarzschild  volume of a black hole
* `Vxsun`: Volume factor as a multiple of Sun volume
* `x`: X coordinate in the rest frame
* `y`: Y coordinate in the rest frame
* `z`: Z coordinate in the rest frame
* `xp`: Transformed X coordinate in the moving frame
* `yp`: Transformed Y coordinate in the moving frame
* `zp`: Transformed Z coordinate in the moving frame
* `v`: Velocity along the x axis
* `ve`: Escape velocity
* `z`: Gravitational redshift parameter

The relativistic transformations are parametrized by the real constant `v` representing a velocity confined to the x-direction. The respective inverse transformation is then parameterized by the negative of this velocity.

#### Lorentz Transformation

The primed reference frame `[xp yp zp]` is travelling with velocity `v` in the positive x direction. Therefore, the y and z coordinates of the rest frame remain unchanged.

#### Time Dilation

The dilation comes from the fact that the Lorentz factor `γ` is greater or equal to one and the proper time interval is multiplied by this factor.

#### Space Contraction

The contraction comes from the fact that the Lorentz factor `γ` is greater or equal to one and the proper space interval is divided by this factor.

#### Velocity Superposition

These expressions replace the usual Galilean addition of velocities. It can be checked that superposing with `v = c` leads to `upx = c`, hence the impossibility to superpose velocities to go beyond the velocity limit `c`. Since the velocity `v` is confined to the x-direction, the y and z components of velocity remain unchanged.

#### Acceleration Superposition

Even if the velocity `v` is confined to the x-direction, all components of the observed acceleration are transformed in the moving frame.

#### E & B Fields Transformation

#### Longitudinal Doppler Effect

#### Transverse Doppler Effect

#### Energy & Momentum

The total relativistic energy `E` and the norm of the momentum `p` form the invariant `mo·c^2` which remains the same in all frames. The kinetic energy `K` is the difference between the total relativistic energy `E` and the rest energy `E0 = mo·c^2`.

#### Gravitational Time Dilation

#### Gravitational Redshift

#### Circumnavigating Airplanes

It is assumed that the planes are circumnavigating at the same altitude `h`, same latitude `φ` and the during same flight duration `Δt` measured in the airplanes. The ground is rotating with the planet at the angular frequency `ω`. The Schwarzschild metric is taken into account. The calculation should formally invokes an integral for the elapsed proper time along a path and is approximated here to the first order in speed since the velocities of the plane and the planet surface are << `c` (slow airplanes over a slow rotating planet).

#### Clocks at different heights

It is assumed that the two clocks are at rest with respect to the ground at a latitude `φ` and are rotating with the planet at the angular frequency `ω`. The clocks are at their respective heights `h1` and `h2` for inxtance at the top and bottom of a mountain. For simplicity, the planet is assumed to have a spherical distribution. The Schwarzschild metric is taken into account. The calculation should formally invokes an integral for the elapsed proper time along a path and is approximated here to the first order in speed since the tangential velociies at height `h1` and `h2` are << `c` (slow rotating planet).

#### B H Schwarzschild Geometry

#### B H Thermodynamics

## Modern Physics
The variables in the Modern Physics section are:

* `β`: Velocity relativistic speed ratio
* `φ`: Work function of the substance (dim.: energy, in SI: eV)
* `θ`: Diffused photon angle of deflection (Photoelectric Effect, Compton Effect), or Angle between incident photon and cristallographic plane (DeBroglie Wave)
* `γ`: Lorentz relativistic factor
* `%rFr12`: Relative % of change between distribution fractions integrated from f1 to f2
* `%rFr34`: Relative % of change between distribution fractions integrated from f3 to f4
* `λ`: Wavelength of the incident photon (Compton Effect)
* `λp`: Wavelength of the diffused photon
* `A`: Area, or Total activity (Radioactivity) (dim.: number of decay/time, in SI: becquerel, Bq)
* `d`: Interatomic distance between cristallographic plane
* `En`: Electron energy in level n (dim.: energy, in SI: eV)
* `Enp`: Electron energy in level np (dim.: energy, in SI: eV)
* `ebfafb`: Emissive power in the range fa to fb (dim.: energy/time·area, in SI: W/m^2)
* `eb`: Total emissive power for the entire Planck spectrum (dim.: energy/time·area, in SI: W/m^2)
* `Eph`: Photon energy (dim.: energy; in SI: eV)
* `f`: Frequency of the photon (dim.: 1/time, in SI: hertz, Hz)
* `f0`: Threshold frequency (dim.: 1/time, in SI: hertz, Hz)
* `f1, f2`: Lower and upper frequency limits of integration (dim.: 1/time, in SI: hertz, Hz)
* `f3, f4`: Lower and upper frequency limits of integration (dim.: 1/time, in SI: hertz, Hz)
* `fpeak`: Frequency of maximum emissive power for the Planck distribution (dim.: 1/time, in SI: hertz, Hz)
* `Frfafb`: Fraction of total emissive power in the range fa to fb
* `FrPl12`: Fraction of Planck emissive power in the range f1 to f2
* `FrWn12`: Fraction of Wien emissive power in the range f1 to f2
* `FrRJ12`: Fraction of Rayleign-Jeans emissive power in the range f1 to f2
* `FrPl34`: Fraction of Planck emissive power in the range f3 to f4
* `FrWn34`: Fraction of Wien emissive power in the range f3 to f4
* `FrRJ34`: Fraction of Rayleign-Jeans emissive power in the range f3 to f4
* `K`: Kinetic energy of ejected electron (dim.: energy; in SI: eV)
* `Kmax`: Maximum kinetic energy of photoelectron (dim.: energy; in SI: eV)
* `p`: Momentum (dim.: mas·speed, in SI: kg·m/s)
* `m`: Mass
* `q`: Heat transfer rate (dim.: energy/time, in SI: watt, W)
* `r`: Radius of electron orbit in level n
* `T`: Temperature
* `V0`: Stopping potential (dim.: energy/charge, In SI: volt,V)

#### Planck & Wien Comparison

In this section, two comparisons are done between the Planck and Wien spectral distributiona. Based on a incomplete thermodynamic argument, the latter is an approximation of the true Planck law describing the spectral distribution for the light emitted by a black-body. The choice of temperature `T` determines the frequency ranges for integration between `f1` and `f2`, or between `f3` and `f4`. One shall determine in which frequency interval both distribution differs notably or agree. The asymptotic agreement for large frequency is clearly illustrated in the picture. The user is free to choose one or the other comparison fractions (replacing it in `Frfafb`) to compute the corresponding enissive power and the heat transfer rate from the black-body.

![Planck & Wien Comparison](img/Planck&Wien_Distributions.bmp)

#### Planck & Rayleigh-Jeans Comparison

In this section, two comparisons are done between the Planck and Rayleigh-Jeans spectral distributiona. Based on the equipartition theorem argument, the latter is an approximation of the true Planck law describing the spectral distribution for the light emitted by a black-body. The choice of temperature `T` determines the frequency ranges for integration between `f1` and `f2`, or between `f3` and `f4`. One shall determine in which frequency interval both distribution agree or differs considerably, leading to a divergence called UV catastrophy corresponding to unphysical fractions greather than one. The asymptotic agreement for small frequency is clearly illustrated in the picture. The user is free to choose one or the other comparison fractions (replacing it in `Frfafb`) to compute the corresponding enissive power and the heat transfer rate from the black-body.

![Planck & Rayleigh-Jeans Comparison](img/Planck&Rayleigh-Jeans_Distributions.bmp)

#### Photoelectric Effect

Einstein explained the photoelectric effect with the energy quantification of the electromagnetic wave. The photoelectron is then emitted only if the energy `E` of the incident photon is greather or equal to the work function `φ` of the material. A simple energy budget equation determines the maximum kinetic energy `Kmax` of the photoelectron.

#### Compton Effect

In the Compton effect, both energy and momentum are conserved during the collision of the incident photon and the electron, which underlines the fact that the photon must henceforth be considered as a particle. When a high frequency `f` (or energy `E = hf`) photon scatters due to an interaction with a charged particle, there is a decrease in the energy of the photon emitted at an angle `θ` and thus, an increase in its wavelength `λp`. The energy of the ejected electron `Kmax` is relativist.

#### De Broglie Wave

At all scales where measurements have been possible, matter exhibits wave-like behavior (e.g. Young interference for protons, even for molecules). More precisely, a beam of neutron can be diffracted just like a beam of light or a water wave as it is the case in the Bragg diffraction. Here, the interference is constructive when the phase difference between the matter wave reflected off different atomic planes at an angle `θ` is a multiple of 2π, giving the Following condition: `2·d·SINθ = n·λ` with `n` integer and where `θ` is mesured between the refected waves trajectory and the atomic plane.

#### Bohr Atomic Model

## Nuclear Physics
The variables in the Nuclear Physics section are:
//38 variables; 26 eqns
* `λ`: Decay constant (Radioactivity) (dim.: 1/time, in SI: s^-1)
* `A`: Total activity (Radioactivity) (dim.: number of decay/time, in SI: becquerel, Bq), or Mass number (Nuclear Physics)
* `AXα`: Mass number of the radionuclide X undergoing α decay
* `AYα`: Mass number of the daughter nuclide Y from α decay
* `AXβ-`: Mass number of the radionuclide X undergoing β- decay
* `AYβ-`: Mass number of the daughter nuclide Y from β- decay
* `AXβ+`: Mass number of the radionuclide X undergoing β+ decay
* `AYβ+`: Mass number of the daughter nuclide Y from β+ decay
* `Aa`: Mass number of the incident nuclide or particle a
* `Ab`: Mass number of the product nuclide or particle b
* `AX`: Mass number of the reactant nuclide X
* `AY`: Mass number of the product nuclide Y
* `A0`: Initial total activity (dim.: number of decay/time, in SI: becquerel, Bq)
* `EB`: Nuclear binding energy (dim.: energy, in SI: MeV)
* `EBse`: Semiempirical nuclear binding energy (dim.: energy, in SI: MeV)
* `ma`: Mass of incident nuclide or particle a (In SI: u)
* `mb`: Mass of outgoing nuclide or particle b (In SI: u)
* `mX`: Mass of reactant neutral atom X (In SI: u)
* `mY`: Mass of product neutral atom X (In SI: u)
* `MW`: Mass weight (dim.: mass/mol, in SI: g/mol)
* `N`: Number of nucleid (Radioactivity), or Number of neutron (Nuclear Physics)
* `N0`: Initial number of nucleid
* `Q`: Net energy balance of a nuclear reaction (dim.: energy, in SI: MeV)
* `Qα`: Net energy balance of an α decay (dim.: energy, in SI: MeV)
* `Qβ⊖`: Net energy balance of a β- decay (dim.: energy, in SI: MeV)
* `Qβ⊕`: Net energy balance of a β+ decay (dim.: energy, in SI: MeV)
* `R`: Radius of the nucleus having A nucleons
* `T1/2`: Half-life of radionuclide (dim.: time)
* `Z`: Number of proton
* `ZXα`: Proton number of the radionuclide X undergoing α decay
* `ZYα`: Proton number of the daughter nuclide Y from α decay
* `ZXβ⊖`: Proton number of the radionuclide X undergoing β- decay
* `ZYβ⊖`: Proton number of the daughter nuclide Y from β- decay
* `ZXβ⊕`: Proton number of the radionuclide X undergoing β+ decay
* `ZYβ⊕`: Proton number of the daughter nuclide Y from β+ decay
* `Za`: Proton number of the incident nuclide or charge of the incident particle a
* `Zb`: Proton number of the incident nuclide or or charge of the product particle b
* `ZX`: Proton number of the reactant nuclide X
* `ZY`: Proton number of the product nuclide Y

For all nuclear reactions, including nuclear decays, we have charge conservation `Zp = Z` and mass number conservation `Mp = M` (the same number of nucleons). This therefore requires assigning numbers A and Z to incident particles `a` and `b` whether they are for example gamma photons (`A = 0`, Z = 0`), positrons (`A = 0`, Z = -1`) or others. The reaction energy `Q` is always calculated with the mass-energy equivalence by the mass difference between the reactants and the products. Spontaneous decays are for instance always exothermic `Q > 0` while some nuclear reactions can be endothermic `Q < 0`.

#### Radioactivity

#### Radius & Binding Energy

#### α Decay

#### β⊖ Decay

#### β+ Decay

#### General Nuclear Reaction
