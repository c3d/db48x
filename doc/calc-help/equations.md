# Equations library

The DB48X calculator features a library of equations covering mathematics,
physics, chemistry and computer science. The built-in equations can be extended
using the `config/equations.csv` configuration file.


## Columns and Beams

The 21 variables in the Columns and Beams section are:

* `o`: Eccentricity (offset) of load (dim.: length)
* `σcr`: Critical stress (dim.: pressure=force/area, in SI: pascal, Pa)
* `σmax`: Maximum stress (dim.: pressure=force/area, in SI: pascal, Pa)
* `θ`: Slope at `x` (dim.: angle)
* `A`: Cross-sectional area
* `a`: Distance to point load
* `ε`: Eccentricity (dim.: length)
* `c`: Distance to edge fiber ([Eccentric Columns](#Eccentric Columns)), or Distance to applied moment ([Beams](#Beams))
* `E`: Modulus of elasticity (dim.: pressure=force/area, in SI: pascal, Pa)
* `I`: Moment of inertia (dim.: length^4, in SI: m^4)
* `K`: Effective length factor of column
* `L`: Length of column or beam
* `M`: Applied moment (dim.: length·force, in SI: N·m)
* `Mx`: Internal bending moment at `x` (dim.: length·force, in SI: N·m)
* `P`: Load ([Eccentric Columns](#Eccentric Columns)), or Point load ([beams](#Beams)) (dim.: force)
* `Pcr`: Critical load (dim.: force)
* `r`: Radius of gyration
* `V`: Shear force at `x`
* `w`: Distributed load (dim.: force)
* `x`: Distance along beam
* `y`: Deflection at `x` (dim.: length)

For simply supported beams and cantilever beams ([Simple Deflection](#Simple Deflection)) through ([Cantilever Shear](#Cantilever Shear)), the calculations differ depending upon the location of `x` relative to the loads.

* Applied loads are positive downward.
* The applied moment is positive counterclockwise.
* Deflection is positive upward.
* Slope is positive counterclockwise
* Internal bending moment is positive counterclockwise on the left-hand part.
* Shear force is positive downward on the left-hand part.

### Elastic Buckling

These equations apply to a slender column (`K·L/r>100`) with length factor `K`.

![Elastic Buckling](img/ElasticBuckling.bmp)

* To calculate `[Pcr_kN;σcr_kPa]` (Critical load; Critical stress) from 6 known variables:
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
L=6.6542_m  A=187.9351_cm^2  r=8.4836_cm  E=206842718.795_kPa  K=1  P=1908.2571_kN  c=15.24_cm  ε=1.1806_cm
@ Expecting [ σmax=140 853.09700 6 kPa I=135 259 652.161 mm↑4 ]
'ROOT(ⒺEccentric Columns;[σmax;I];[1_kPa;1_mm^4])'
```

### Simple Deflection

![Simple Deflection](img/SimpleDeflection.bmp)

* To calculate `[y_in]` (Deflection at `x`) from 9 known variables:
```rpl
L=20_ft  E=29000000_psi  I=40_in^4  a=10_ft  P=674.427_lbf  c=17_ft  M=3687.81_ft*lbf  w=102.783_lbf/ft  x=9_ft
@ Expecting [ y=-0.60048 54094 96 in ]
'ROOT(ⒺSimple Deflection;[y];[1_in])'
```

### Simple Slope

![Simple Slope](img/SimpleSlope.bmp)

* To calculate `[Θ_°]` (Slope at `x`) from 9 known variables:
```rpl
L=20_ft  E=29000000_psi  I=40_in^4  a=10_ft  P=674.427_lbf  c=17_ft  M=3687.81_ft*lbf
 w=102.783_lbf/ft  x=9_ft
@ Expecting [ θ=-8.76317 82526 7⁳⁻² ° ]
'ROOT(ⒺSimple Slope;[θ];[0_°])'
```

### Simple Moment

![Simple Moment](img/SimpleMoment.bmp)

* To calculate `[Mx_ft*lbf]` (Internal bending moment at `x`) from 7 known variables:
```rpl
L=20_ft  a=10_ft  P=674.427_lbf  c=17_ft  M=3687.81_ft*lbf  w=102.783_lbf/ft  x=9_ft
@ Expecting [ Mx=9 782.1945 ft·lbf ]
'ROOT(ⒺSimple Moment;[Mx];[1_ft*lbf])'
```

### Simple Shear

![Simple Shear](img/SimpleShear.bmp)

* To calculate `[V_lbf]` (Shear force at `x`) from 6 known variables:
```rpl
L=20_ft  a=10_ft  P=674.427_lbf  M=3687.81_ft*lbf  w=102.783_lbf/ft  x=9_ft
@ Expecting [ V=624.387 lbf ]
'ROOT(ⒺSimple Shear;[V];[1_lbf])'
```

### Cantilever Deflection

![Cantilever Deflection](img/CantileverDeflection.bmp)

* To calculate `[y_in]` (Deflection at `x`) from 9 known variables:
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

* To calculate `[Mx_ft*lbf]` (Internal bending moment at `x`) from 7 known variables:
```rpl
L=10_ft  P=500_lbf  M=800_ft*lbf  a=3_ft  c=6_ft  w=100_lbf/ft  x=8_ft
@ Expecting [ Mx=-200 ft·lbf ]
'ROOT(ⒺCantilever Moment;[Mx];[1_ft*lbf])'
```

### Cantilever Shear

![Cantilever Shear](img/CantileverShear.bmp)

* To calculate `[V_lbf]` (Shear force at `x`) from 5 known variables:
```rpl
L=10_ft  P=500_lbf  a=3_ft  x=8_ft  w=100_lbf/ft
@ Expecting [ V=200 lbf ]
'ROOT(ⒺCantilever Shear;[V];[1_lbf])'
```


## Electricity

The 78 variables in the Electricity section are:

* `∈r`: Relative permittivity
* `μr`: Relative permeability
* `ω`: Angular frequency (dim.: angle/time)
* `ω₀`: Resonant angular frequency (dim.: angle/time)
* `φ`: Phase angle
* `φp`: Parallel phase angle
* `φs`: Series phase angle
* `θ₁`: First subtended angle relative to the left end of the wire
* `θ₂`: Second subtended angle relative to the right end of the wire
* `ρ`: Resistivity (dim.: resistance·length; in SI: ohm·meter, Ω·m), or Volumic charge density ([Drift Speed & Current Density](#Drift Speed & Current Density)) (dim.: charge/volume, in SI: C/m^3)
* `ρ0`: Resistivity at the reference temperature `T0` (dim.: resistance·length; in SI: ohm·meter, Ω·m)
* `ΔI`: Current Change (dim.: charge/time; in SI: ampere, A)
* `Δt`: Time Change
* `ΔV`: Voltage Change (dim.: energy/charge; in SI: volt, V)
* `A`: Wire cross-section area ([Wire Resistance](#Wire Resistance)), or Solenoid cross-section area ([Solenoid Inductance](#Solenoid Inductance)), or Plate area ([Plate Capacitor](#Plate Capacitor))
* `C`: Capacitance (dim.: charge^2/energy; in SI: farad, F)
* `C1`: First capacitance (dim.: charge^2/energy; in SI: farad, F)
* `C2`: Second capacitance (dim.: charge^2/energy; in SI: farad, F)
* `Cp`: Parallel capacitance (dim.: charge^2/energy; in SI: farad, F)
* `Cs`: Series capacitance (dim.: charge^2/energy; in SI: farad, F)
* `d`: Plate separation (dim.: length)
* `E`: Energy (dim.: charge·voltage, in SI: joule, J)
* `Er`: Electric field at distance `r` from the source (dim.: force/charge; in SI: N/C = V/m)
* `Ep`: Electric field over an infinite plate (dim.: force/charge; in SI: N/C = V/m)
* `Ein`: Electric field inside a plate capacitor (dim.: force/charge; in SI: N/C = V/m)
* `F`: Force between charges
* `f`: Frequency (dim.: time^-1)
* `f0`: Resonant frequency (dim.: time^-1)
* `I`: Current, or Total current ([Current Divider](#Current Divider)) (dim.: charge/time; in SI: ampere, A)
* `I1`: Current in `R1` (dim.: charge/time; in SI: ampere, A)
* `Imax`: Maximum current (dim.: charge/time; in SI: ampere, A)
* `L`: Inductance (dim.: energy/current^2; in SI: henry, H), or Length ([Wire Resistance](#Wire Resistance)), ([Cylindrical Capacitor](#Cylindrical Capacitor))
* `Lp`: Parallel inductance (dim.: energy/current^2; in SI: henry, H)
* `Ls`: Series inductance (dim.: energy/current^2; in SI: henry, H)
* `N`: Number of turns
* `P`: Power (dim.: energy/time)
* `q`: Charge (in SI: coulomb, C)
* `q1`: First point charge (in SI: coulomb, C)
* `q2`: Second point charge (in SI: coulomb, C)
* `qtest`: Test point charge (in SI: coulomb, C)
* `Qp`: Parallel quality factor
* `Qs`: Series quality factor
* `r`: Charge distance
* `R`: Resistance (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `R1`: First resistance (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `R2`: Second resistance (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `ri`: Inside radius
* `ro`: Outside radius
* `Rp`: Parallel resistance (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `Rs`: Series resistance (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `t`: Time
* `ti`: Initial time
* `tf`: Final time
* `V`: Voltage, or Total voltage (dim.: energy/charge; in SI: volt, V)
* `V1`: Voltage accross `R1` (dim.: energy/charge; in SI: volt, V)
* `Vi`: Initial voltage (dim.: energy/charge; in SI: volt, V)
* `Vf`: Final voltage (dim.: energy/charge; in SI: volt, V)
* `Vmax`: Maximum voltage (dim.: energy/charge; in SI: volt, V)
* `XC`: Reactance of capacitor (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `XL`: Reactance of inductor (dim.: energy·time/charge^2; in SI: ohm, Ω)
* `σ`: Conductivity (dim.: (resistance·length)^-1; in SI: siemens/meter, S/m), or Surface charge density ([E field infinite plate](#E field infinite plate)), ([Plate capacitor](#Plate capacitor)) (dim.: charge/area, in SI: C/m^2)
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

* **Example 1**. To calculate `[F_N;Er_N/C]` (Electric force; Electric Field at position `r`) from 5 known variables:
```rpl
q1=1.6E-19_C  q2=1.6E-19_C  r=4.00E-13_cm  εr=1  qtest=1.6E-19_C
@ Expecting [ F=14.38008 28579 N Er=8.98755 17861 7⁳¹⁹ N/C ]
'ROOT(ⒺCoulomb’s Law & E Field;[F;Er];[1_N;1_N/C])'
```

* **Example 2**. A square metal plate `L = 8_cm` on a side carries a charge of `q1 = 6_μC`. Approximate values of the electric force & electric field for a point charge `q2 = 1_μC` located at `r = 3_m` can be calculated with Coulomb’s law if the separation distance is much greater than the plate dimension `r >> L`. The whole plate is indeed considered as being a point charge providing that `r > 10 · L`. Therefore, to calculate `[F_N;Er_N/C]`:
```rpl
L=8_cm r=3_m q1=6E-6_C  q2=1E-6_C  r=3_m  εr=1  qtest=1E-6_C
@ Expecting [ F=5.99170 11907 8⁳⁻³ N Er=5 991.70119 078 N/C ]
if 'r > 10*L' then
 'ROOT(ⒺCoulomb’s Law & E Field;[F;Er];[1_N;1_N/C])'
end
```

### E Field Infinite Line

The expression for the radial electric field at the distance `r` is approximately valid if this distance is such that `r << L` and therefore also applies to a wire of finite length `L`.

* To calculate `[λ_C/m;Er_N/C]` (Linear charge density; Electric Field at position `r`) from 4 known variables:
```rpl
Q=5E-6_C  L=3_m  r=0.05_m  εr=1
@ Expecting [ λ=1.66666 66666 7⁳⁻⁶ C/m Er=599 170.11907 8 N/C ]
'ROOT(ⒺE Field Infinite Line;[λ;Er];[1_C/m;1_N/C])'
@ Keep
```
The code below saves the reference value for comparison with the example 2 in [E Field Finite Line](#E Field Finite Line):
```rpl
@ Save the reference value for comparison below
Er0=Er
```

### E Field Finite Line

The expression of the radial electric field at the distance `r` depends on the subtended angles `θ₁` and `θ₂` relative to the ends of the wire of finite length `L`.

![E field finite line](img/EFieldFiniteLine.bmp)

* **Example 1.** To calculate `[λ_C/m;Er_N/C]` (Linear charge density; Electric Field at position `r`) from 6 known variables and also with the distance `r=(L/2)/tanθ₁` and angle `θ₂=360°-θ₁` (see figure):
```rpl
r='(3_m)/(2*tan 30_°)' θ₂='360_°-30_°'
Q=5E-6_C  L=3_m  r=2.5981_m  εr=1  θ₁=30_°
@ Expecting [ λ=1.66666 66666 7⁳⁻⁶ C/m Er=5 765.46436 894 N/C ]
'ROOT(ⒺE Field Finite Line;[λ;Er];[1_C/m;1_N/C])'
@ Keep
```

* **Example 2.** To show that the infinite line of the previous section can approximate the finite case (if `r << L` realised when `r < L/10`), we calculate `[λ_C/m;Er_N/C]` (Linear charge density; Electric Field at position `r`) with the angles `θ₁=ATAN((L/2)/r)` and `θ₂=360°-θ₁` (see figure):
```rpl
Q=5E-6_C  L=3_m  r=5_cm  εr=1  θ₁='atan(L/2/r)' θ₂='360_°-θ₁'
if 'r < L/10' then
@ Expecting [ λ=1.66666 66666 7⁳⁻⁶ C/m Er=598 837.52392 7 N/C ]
'ROOT(ⒺE Field Finite Line;[λ;Er];[1_C/m;1_N/C])'
end
@ Keep
```
Verify relative difference under condition `5_cm << 3_m` with the example of [E Field Infinite Line](#E Field Infinite Line)
```rpl
Er0 Er %Ch
@ Expecting -0.05550 93020 85
@ % of relative difference
```

### E Field Infinite Plate

The expression of the perpendicular electric field is constant over an infinite plate and can approximate the field at a distance `d` from a finite plate if it is very small compare to the dimensions (length or width `L`) of the plate. On the contrary if `d >> L`, `Ep` can be approximated if we consider the whole plate as being a point charge with `q = σ·A` (where `σ` is the surface charge density), see example 2 of [Coulomb’s Law & E Field](#Coulomb’s Law & E Field).

* To calculate `[Ep_N/C;σ_C/m^2]` (Electric Field; Linear charge density) at position `[d=5_mm]` above a square plate of width `[L=8_cm]` and surface `A=L^2` where `d << L` when `d < L/10` is verified:
```rpl
L=8_cm A='L^2' d=5_mm Q=6E-6_C  A=64_cm^2  εr=1
@ Expecting [ σ=0.00000 00937 5 C/cm↑2 Ep=52 941 049.9972 N/C ]
if 'd < L/10' then
 'ROOT(ⒺE Field Infinite Plate;[σ;Ep];[1_C/cm^2;1_N/C])'
end
```

### Ohm’s Law & Power

* To calculate `[R_Ω;P_W]` (Resistance; Power) from 2 known variables:
```rpl
V=24_V  I=16_A
@ Expecting [ R=1.5 Ω P=384 W ]
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

* To calculate `[ρ_(Ω*m);σ_(S/m)]` (Resistivity; Conductivity) from 4 known variables:
```rpl
ρ0=1.68E-8_Ω*m  αT=4.04E-3_K^-1  T0=293,15_K  T=373,15_K
@ Expecting [ ρ=2.22297 6⁳⁻⁸ Ω·m σ=44 984 741.1758 S/m ]
'ROOT(ⒺResistivity & Conductivity;[ρ;σ];[1_(Ω*m);1_(S/m)])'
```

### Series & Parallel R

![Series & Parallel R](img/Series&ParallelR.bmp)

* To calculate `[Rs_Ω;Rp_Ω]` (Series & Parallel Resistances) from 2 known variables:
```rpl
R1=2_Ω  R2=3_Ω
@ Expecting [ Rs=5 Ω Rp=1.2 Ω ]
'ROOT(ⒺSeries & Parallel R;[Rs;Rp];[1_Ω;1_Ω])'
```

### Series & Parallel C

![Series & Parallel C](img/Series&ParallelC.bmp)

* To calculate `[Cs_μF;Cp_μF]` (Series & Parallel Capacitances) from 2 known variables:
```rpl
C1=2_μF  C2=3_μF
@ Expecting [ Cs=1.2 μF Cp=5 μF ]
'ROOT(ⒺSeries & Parallel C;[Cs;Cp];[1_μF;1_μF])'
```

### Series & Parallel L

![Series & Parallel L](img/Series&ParallelL.bmp)

* To calculate `[Ls_mH;Lp_mH]` (Series & Parallel Inductances) from 2 known variables:
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

* To calculate `[uE_(J/m^3)]` (Volumic density electric energy) from 2 known variables:
```rpl
E=5_V/m  εr=1
@ Expecting [ uE=1.10677 34773 5⁳⁻¹⁰ J/m↑3 ]
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

![RLC Current Delay](img/RLCCurrentDelay.bmp)

* To calculate `[ω_r/s;φs_°;φp_°;XC_Ω;XL_Ω]` (Series & Parallel phase angles; Reactances of capacitor & inductance) from 4 known variables:
```rpl
f=107_Hz  C=80_μF  L=20_mH  R=5_Ω
@ Expecting [ ω=672.30082 7868 r/s φs=-45.82915 71488 ° φp=-5.87715 65317 1 ° XC=18.59286 71836 Ω XL=13.44601 65574 Ω ]
'ROOT(ⒺRLC Current Delay;[ω;φs;φp;XC;XL];[1_r/s;1_°;1_°;1_Ω;1_Ω])'
```

### DC Capacitor Current

These equations approximate the dc current required to charge the voltage on a capacitor in a certain time interval.

* To calculate `[ΔV_V;Δt_s;tf_s]` (Voltage difference; Time difference; Final time) from 5 known variables:
```rpl
C=15_μF  Vi=2.3_V  Vf=3.2_V  I=10_A  ti=0_μs
@ Expecting [ ΔV=0.9 V Δt=1.35 μs tf=1.35 μs ]
'ROOT(ⒺDC Capacitor Current;[ΔV;Δt;tf];[1_V;1_μs;1_μs])'
```

### Capacitor Charge

* To calculate `[q_C]` (Voltage difference; Time difference; Final time) from 2 known variables:
```rpl
C=20_μF  V=100_V
@ Expecting [ q=0.002 C ]
'ROOT(ⒺCapacitor Charge;[q];[1_C])'
```

### DC Inductor Voltage

These equations approximate the dc voltage induced in an inductor by a change in current in a certain time interval.

![DC Inductor Voltage](img/DCInductorVoltage.bmp)

* To calculate `[ΔIL_A;ILf_A;tf_μs]` (Current difference; Final current; Final time) from 5 known variables:
```rpl
L=100_mH  V=52_V  Δt=32_μs  ILi=23_A  ti=0_μs
@ Expecting [ ΔIL=-1.664⁳⁻² A ILf=22.98336 A tf=32 μs ]
'ROOT(ⒺDC Inductor Voltage;[ΔIL;ILf;tf];[1_A;1_A;1_μs])'
```

### RC Transient

![RC Transient](img/RCTransient.bmp)

* To calculate `[V_V]` (Voltage) from 5 known variables:
```rpl
Vi=0_V  C=50_μF  Vf=10_V  R=100_Ω  t=2_ms
@ Expecting [ V=3.29679 95396 4 V ]
'ROOT(ⒺRC Transient;[V];[1_V])'
```

### RL Transient

![RL Transient](img/RLTransient.bmp)

* To calculate `[I_A]` (Current) from 5 known variables:
```rpl
Vi=0_V  Vf=5_V  R=50_Ω  L=50_mH  t=75_μs
@ Expecting [ I=7.22565 13671 4⁳⁻³ A ]
'ROOT(ⒺRL Transient;[I];[1_A])'
```

### Resonant Frequency

* To calculate `[ω₀;Qs;Qp;f0]` (Resonant angular velocity; Parallel & Series quality factors; Resonant frequency) from 3 known variables:
```rpl
L=500_mH  C=8_μF  R=10_Ω
@ Expecting [ ω₀=500. r/s Qs=25. Qp=0.04 f0=79.57747 15459 Hz ]
'ROOT(ⒺResonant Frequency;[ω₀;Qs;Qp;f0];[1_r/s;1;1;1_Hz])'
```

### Plate Capacitor

![Plate Capacitor](img/PlateCapacitor.bmp)

* To calculate `[d_cm;ΔV_V;Ein_(N/C);σ_(μC/m^2)]` (Distance; Voltage; Internal E field; Surface charge density) from 4 known variables:
```rpl
C=25_μF  εr=2.26  A=1_cm^2  Q=75_μC
@ Expecting [ d=80.04185 7882 pm σ=750 000. μC/m↑2 Ein=3.74803 89378 6⁳¹⁰ N/C ΔV=3. V ]
'ROOT(ⒺPlate Capacitor;[d;σ;Ein;ΔV];[1_pm;1_(μC/m^2);1_(N/C);1_V])'
```

### Cylindrical Capacitor

![Cylindrical Capacitor](img/CylindricalCapacitor.bmp)

* To calculate `[C_μF;ΔV_V]` (Capacitance; Voltage) from 5 known variables:
```rpl
εr=1  Q=75_μC  ro=1_cm  ri=.999_cm  L=10_cm
@ Expecting [ C=5 560.46819 203 pF ΔV=13 488.07284 02 V ]
'ROOT(ⒺCylindrical Capacitor;[C;ΔV];[1_pF;1_V])'
```

### Solenoid Inductance

![Solenoid Inductance](img/SolenoidInductance.bmp)

* To calculate `[L_mH]` (Inductance) from 4 known variables:
```rpl
μr=2.5  n=40_1/cm  A=0.2_cm^2  h=3_cm
@ Expecting [ L=0.03015 92894 7 mH ]
'ROOT(ⒺSolenoid Inductance;[L];[1_mH])'
```

### Toroid Inductance

![Toroid Inductance](img/ToroidInductance.bmp)

* To calculate `[L_mH]` (Inductance) from 4 known variables:
```rpl
μr=1  N=5000  h=2_cm  ri=2_cm  ro=4_cm
@ Expecting [ L=69.31471 80468 mH ]
'ROOT(ⒺToroid Inductance;[L];[1_mH])'
```

### Sinusoidal Voltage

* To calculate `[ω_r/s;V_V]` (Angular velocity; Voltage) from 4 known variables:
```rpl
Vmax=110_V  t=30_μs  f=60_Hz  φ=15_°
@ Expecting [ ω=376.99111 8431 r/s V=29.66992 85671 V ]
'ROOT(ⒺSinusoidal Voltage;[ω;V];[1_r/s;1_V])'
```

### Sinusoidal Current

* To calculate `[I_A;f_Hz]` (Current; Frequency) from 4 known variables:
```rpl
t=3.2_s  Imax=10_A  ω=636_r/s  φ=30_°
@ Expecting [ f=101.22254 3806 Hz I=-0.28436 91656 8 A ]
'ROOT(ⒺSinusoidal Current;[f;I];[1_Hz;1_A])'
```

### Drift Speed & Current Density

* To calculate `[vd_m/s;J_(A/m^2);E_(V/m)]` (Drift speed; Current density; E field) from 5 known variables:
```rpl
I=1_A  n=8.5e28_(m^-3)  r=0.1_cm  A='Ⓒπ*(r_cm)^2'  ρ='Ⓒqe*n' σ='CONVERT(→NUM(Ⓒqe*(n_(m^-3))*40_(cm^2/(V*s)));1_S/m)'
@ Expect [ vd=2.33733 41683 6⁳⁻⁵ m/s J=31.83098 86184 A/cm↑2 E=5.84333 54209⁳⁻³ V/m ]
'ROOT(ⒺDrift Speed & Current Density;[vd;J;E];[1_m/s;1_(A/cm^2);1_(V/m)])'
```

### Electron & Hole Mobilities

In accordance with microscopic Ohm's law, the current density is proportional to the electric field. Holes and electrons therefore move at their average drift speeds during the mean free time between collisions. As long as the electric fields are not very high, the mobilities of holes and electrons are constant.

* To calculate `[μe_(cm^2/(V*s));μe_(cm^2/(V*s));J_(A/m^2);Je_(A/m^2);Jh_(A/m^2);σ_(S/m)]` (Electron & hole mobilities; Current densities; Conductivity) from 5 known variables:
```rpl
τc=4.09365 36801 40e-15_s meeff=1.09312 60456 68e-31 kg mheff=4.55469 18569 5e-31 kg nh=6.0e18_(m^-3) ne=1.04e19_(m^-3) E=6.0e-9_V/m
@ Expecting [ μe=60. cm↑2/(V·s) μh=14.4 cm↑2/(V·s) Je=5.99854 93177⁳⁻¹¹ A/m↑2 Jh=8.30568 36706 6⁳⁻¹² A/m↑2 J=6.82911 76847 6⁳⁻¹¹ A/m↑2 σ=0.01138 18628 08 S/m ]
'ROOT(ⒺElectron & Hole Mobilities;[μe;μh;Je;Jh;J;σ];[1_(cm^2/(V*s));1_(cm^2/(V*s));1_(A/m^2);1_(A/m^2);1_(A/m^2);1_(S/m)])'
```

## Fluids

The 30 variables in the Fluids section are:

* `ε`: Roughness (dim.: length)
* `μ`: Dynamic viscosity (dim.: force·time/volume, in SI: N·s/m^3)
* `ρ`: Density (dim.: mass/volume, in SI: kg/m^3)
* `ΔP`: Pressure change (dim.: force/area, in SI: pascal, Pa)
* `Δy`: Height change
* `ΣK`: Total fitting coefficients
* `A`: Cross-sectional area
* `A1`: Initial cross-sectional area
* `A2`: Final cross-sectional area
* `D`: Diameter
* `D1`: Initial diameter
* `D2`: Final diameter
* `f`: Fanning friction factor
* `h`: Depth relative to P0 reference depth
* `hL`: Head loss (dim.: area/time^2, in SI: m^2/s^2)
* `L`: Length
* `M`: Mass flow rate (dim.: mass/time, in SI: kg/s)
* `n`: Kinematic viscosity (dim.: force·time/volume, in SI: N·s/m^3)
* `P`: Pressure at `h` (dim.: force/area, in SI: pascal, Pa)
* `P0`: Reference pressure (dim.: force/area, in SI: pascal, Pa)
* `P1`: Initial pressure (dim.: force/area, in SI: pascal, Pa)
* `P2`: Final pressure (dim.: force/area, in SI: pascal, Pa)
* `Q`: Volume flow rate (dim.: volume/time, in SI: m^3/s)
* `Reynolds`: Reynolds number
* `v1`: Initial velocitie
* `v2`: Final velocitie
* `vavg`: Average velocity
* `W`: Power input (dim.: energy/time, in SI: watt, W)
* `y1`: Initial height
* `y2`: Final height

### Pressure At Depth

This equation describes hydrostatic pressure for an incompressible fluid. Depth `h` is positive downwards from the reference.

![Pressure At Depth](img/PressureAtDepth.bmp)

* To calculate `[P_kPa]` (Pressure) from 3 known variables:
```rpl
h=100_m  ρ=1025.1817_kg/m^3  P0=1_atm
@ Expecting [ P=1 106.68481 183 kPa ]
'ROOT(ⒺPressure at Depth;[P];[1_kPa])'
```

### Bernoulli Equation

These equations represent the streamlined flow of an incompressible fluid.

![Bernoulli Equation](img/BernoulliEquation.bmp)

* To calculate `[A1_in^2;ΔP_psi;Δy_ft;Q_ft^3/min;M_lb/min;v2_ft/s;A2_in^2;D2_in]` (Initial cross-sectional area; Pressure change; Height change; Volume & Mass flow rates; Final velocity; Final cross-sectional area; Final diameter) from 7 known variables:
```rpl
P2=25_psi  P1=75_psi  y2=35_ft  y1=0_ft  D1=18_in  ρ=64_lb/ft^3  v1=100_ft/s
@ Expect [ A1=254.46900 4941 in↑2 ΔP=-50. psi Δy=35. ft Q=10 602.87520 59 ft↑3/min M=678 584.01317 5 lb/min v2=122.42131 1569 ft/s A2=207.86332 19 in↑2 D2=16.26836 81217 in ]
'ROOT(ⒺBernoulli Equation;[A1;ΔP;Δy;Q;M;v2;A2;D2];[1_in^2;1_psi;1_ft;1_ft^3/min;1_lb/min;1_ft/s;1_in^2;1_in])'
```

### Flow with Losses

These equations extend Bernoulli’s equation to include power input (or output) and head loss.

![Flow with Losses](img/FlowwithLosses.bmp)

* To calculate `[A1_in^2;ΔP_psi;Δy_ft;Q_ft^3/min;M_lb/min;v2_ft/s;A2_in^2;D2_in]` (Initial cross-sectional area; Pressure change; Height change; Volume & Mass flow rates; Final velocity; Final cross-sectional area; Final diameter) from 9 known variables:
```rpl
P2=30_psi  P1=65_psi  y2=100_ft  y1=0_ft  ρ=64_lb/ft^3  D1=24_in  hL=2.0_(ft/s)^2  W=25_hp  v1=100_ft/s
@ Expecting [ A1=452.38934 2117 in↑2 ΔP=-35 psi Δy=100 ft Q=18 849.55592 15 ft↑3/min M=1 206 371.57898 lb/min v2=93.12684 14502 ft/s A2=485.77760 7264 in↑2 D2=24.86988 66004 in ]
'ROOT(ⒺFlow with Losses;[A1;ΔP;Δy;Q;M;v2;A2;D2];[1_in^2;1_psi;1_ft;1_ft^3/min;1_lb/min;1_ft/s;1_in^2;1_in])'
```

### Flow In Full Pipes

These equations adapt Bernoulli’s equation for flow in a round, full pipe, including power input (or output) and frictional losses (with the Fanning friction factor `f`).

![Flow In Full Pipes](img/FlowInFullPipes.bmp)

* To calculate `[ΔP_Psi;Δy_ft;A_in^2;n_ft^2/s;Q_ft^3/min;M_lb/min;Reynolds;f;W_hp]` (Pressure & Height changes; Cross-sectional area; Kinematic viscosity; Volume & Mass flow rates; Reynolds number; Fanning factor; Power input) from 11 known variables:
```rpl
ρ=62.4_lb/ft^3  D=12_in  vavg=8_ft/s  P2=15_psi  P1=20_psi  y2=40_ft  y1=0_ft  μ=0.00002_lbf*s/ft^2  ΣK=2.25  ϵ=0.02_in  L=250_ft
@ Expect [ ΔP=-5. psi Δy=40. ft A=113.09733 5529 in↑2 n=1.03121 95050 1⁳⁻⁵ ft↑2/s Q=376.99111 8431 ft↑3/min M=23 524.24579 01 lb/min Reynolds=775 780.51628 2 f=4.32637 07915 4⁳⁻³ W=24.95162 12864 hp ]
'ROOT(ⒺFlow In Full Pipes;[ΔP;Δy;A;n;Q;M;Reynolds;f;W];[1_psi;1_ft;1_in^2;1_ft^2/s;1_ft^3/min;1_lb/min;1;1;1_hp])'
```
'Reynolds='(D_m)*(vavg_(m/s))*(ρ_(kg/m^3))/(μ_(kg/(m*s)))'  Reynolds=775 780.51628 23186 30725 06
## Forces and Energy

The 37 variables in the Force and Energy section are:

* `α`: Angular acceleration (dim.: angle/time^2, in SI: r/s^2)
* `ω`: Angular velocity (dim.: angle/time, in SI: r/s))
* `ωi`: Initial angular velocitie (dim.: angle/time, in SI: r/s)
* `ωf`: Final angular velocitie (dim.: angle/time, in SI: r/s)
* `ρ`: Fluid density (dim.: mass/volume, in SI: kg/m^3)
* `τ`: Torque (dim.: force·length, in SI: N·m)
* `Θ`: Angular displacement (dim.: angle, in SI: r)
* `A`: Projected area relative to flow
* `ar`: Centripetal acceleration at `r`
* `at`: Tangential acceleration at `r`
* `Cd`: Drag coefficient
* `E`: Energy (dim.: force·length, in SI: joule, J)
* `F`: Force at `r` or `x`, or Spring force ([Hooke’s Law](#Hooke’s Law)), or Attractive
force ([Law of Gravitation](#Law of Gravitation)), or Drag force ([Drag force](#Drag force))
* `I`: Moment of inertia (dim.: mass·length^2, in SI: kg·m^2)
* `k`: Spring constant (dim.: force/length, in SI: N/m)
* `Ki`: Initial kinetic energy (dim.: force·length, in SI: joule, J)
* `Kf`: Final kinetic energy (dim.: force·length, in SI: joule, J)
* `m`: Mass
* `m1`: First mass
* `m2`: Second mass
* `N`: Rotational speed (dim.: turn/time, in SI: rpm)
* `Ni`: Initial rotational speed (dim.: turn/time, in SI: rpm)
* `Nf`: Final rotational speed (dim.: turn/time, in SI: rpm)
* `P`: Instantaneous power (dim.: energy/time, in SI: watt, W)
* `Pavg`: Average power (dim.: energy/time, in SI: watt, W)
* `r`: Radius from rotation axis, or Separation distance ([Law of Gravitation](#Law of Gravitation))
* `t`: Time
* `v`: Velocity
* `vf`: Final velocity
* `v1f`: First final velocity
* `v2f`: Second final velocity
* `vi`: Initial velocity
* `v1i`: Initial velocity of mass `m1`
* `W`: Work (dim.: force·length, in SI: joule, J)
* `x`: Displacement
* `UGi`: Initial gravitational potential energy (dim.: force·length, in SI: joule, J)
* `UGf`: Final gravitational potential energy (dim.: force·length, in SI: joule, J)

### Linear Mechanics

* To calculate `[F_lbf;Ki_ft*lbf;vf_ft/s;Kf_ft*lbf;W_ft*lbf;x_ft;Pavg_hp]` (Force at `x`; Initial kinetic energy; Final speed; Final kinetic energy; Work; Average Power) from 4 known variables:
```rpl
t=10_s  m=50_lb  a=12.5_ft/s^2  vi=0_ft/s
@ Expecting [ F=19.42559 38572 lbf Ki=0. ft·lbf vf=125. ft/s Kf=12 140.99616 08 ft·lbf W=12 140.99616 08 ft·lbf x=625. ft Pavg=2.20745 38474 1 hp ]
@ Note Ki is approximately 0
'ROOT(ⒺLinear Mechanics;[F;Ki;vf;Kf;W;x;Pavg];[1_lbf;1_ft*lbf;1_ft/s;1_ft*lbf;1_ft*lbf;1_ft;1_hp])'
```

### Angular Mechanics

* To calculate `[τ_ft*lbf;Ki_ft*lbf;W_ft*lbf;Kf_ft*lbf;at_ft/s^2;Ni_rpm;ωf_r/min;t_min;Nf_rpm;Pavg_hp]` (Torque; Initial kinetic energy; Work; Final kinetic energy; Tangential acceleration; Initial rotational speed; Final angular velocity; Time; Final rotational speed; Average power) from 5 known variables:
```rpl
I=1750_lb*in^2  θ=360_°  r=3.5_in  α=10.5_r/min^2  ωi=0_r/s
@ Expecting [ τ=1.10168 29849 6⁳⁻³ ft·lbf Ki=0. ft·lbf W=6.92207 83442 6⁳⁻³ ft·lbf Kf=6.92207 83442 6⁳⁻³ ft·lbf at=8.50694 44444 4⁳⁻⁴ ft/s↑2 Ni=0. rpm ωf=11.48681 38076 r/min t=1.09398 22673 9 min Nf=0.29096 43928 17 rpm Pavg=1.91739 80792 8⁳⁻⁷ hp ]
'ROOT(ⒺAngular Mechanics;[τ;Ki;W;Kf;at;Ni;ωf;t;Nf;Pavg];[1_ft*lbf;1_ft*lbf;1_ft*lbf;1_ft*lbf;1_ft/s^2;1_rpm;1_r/min;1_min;1_rpm;1_hp])'
```

### Centripetal Force

* To calculate `[ω_r/s;F_N;v_m/s;ar_m/s]` (Angular velocity; Force; Velocity; Centripetal accélération at `r`) from 3 known variables:
```rpl
m=1_kg  r=5_cm  N=2000_Hz
@ Expecting [ ω=12 566.37061 44 r/s F=7 895 683.52087 N v=628.31853 0718 m/s ar=7 895 683.52087 m/s↑2 ]
'ROOT(ⒺCentripetal Force;[ω;F;v;ar];[1_r/s;1_N;1_m/s;1_m/s^2])'
```

### Hooke’s Law

The force is that exerted by the spring.

![Hooke’s Law](img/Missing name.bmp)

* To calculate `[F_lbf;W_ft*lbf]` (Force; Work) from 2 known variables:
```rpl
k=1725_lbf/in  x=1.25_in
@ Expecting [ F=-2 156.25 lbf W=-112.30468 75 ft·lbf ]
'ROOT(ⒺHooke’s Law;[F;W];[1_lbf;1_ft*lbf])'
```

### 1D Elastic Collisions

![1D Elastic Collisions](img/1DElasticCollisions.bmp)

* To calculate `[v1f_m/s;v2f_m/s]` (Final velocities of mass `m1` & `m2`) from 3 known variables:
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

### Gravitation Law

* To calculate `[F_N;UGf_J;UGi_J;W_J]` (Gravitational force; Work; Final & Initial potential energy) from 5 known variables:
```rpl
m1=2E15_kg  m2=2E18_kg  r=1000000_km  ri=1000000_km  rf=5000000_km
@ Expecting [ F=266 972. N UGf=-5.33944⁳¹³ J UGi=-2.66972⁳¹⁴ J W=2.13577 6⁳¹⁴ J ]
'ROOT(ⒺGravitation Law;[F;UGf;UGi;W];[1_N;1_J;1_J;1_J])'
```

### Relativity Mass Energy

* To calculate `[E_J]` (Relativistic energy) from 1 known variable:
```rpl
m=9.1E-31_kg
@ Expecting [ E=8.17867 21265 1⁳⁻¹⁴ J ]
'ROOT(ⒺRelativity Mass Energy;[E];[1_J])'
```

## Gases

The 38 variables in the Gases section are:

* `λ`: Mean free path (dim.: length)
* `ρ`: Flow density (dim.: mass/volume, in SI: kg/m^3)
* `ρr`: Reduced state factor
* `ρri`: Initial reduced state factor
* `ρrf`: Final reduced state factor
* `ρ0`: Stagnation density (dim.: mass/volume, in SI: kg/m^3)
* `A`: Flow area
* `At`: Throat area
* `d`: Molecular diameter
* `k`: Specific heat ratio
* `M`: Mach number
* `m`: Mass
* `MW`: Molecular weight (dim.: mass/mole, in SI: g/gmol)
* `n`: Number of moles, or Polytropic constant ([Polytropic Processes](#Polytropic Processes))
* `P`: Pressure, or Flow pressure ([Isentropic Flow](#Isentropic Flow)) (dim.: force/area, in SI: pascal, Pa)
* `P0`: Stagnation pressure (dim.: force/area, in SI: pascal, Pa)
* `Pc`: Pseudocritical pressure (dim.: force/area, in SI: pascal, Pa)
* `Pi`: Initial pressure (dim.: force/area, in SI: pascal, Pa)
* `Pf`: Final pressure (dim.: force/area, in SI: pascal, Pa)
* `Pr`: Reduced pressure
* `Pri`: Initial reduced ressure
* `Prf`: Final reduced pressure
* `T`: Temperature, or Flow temperature ([Isentropic Flow](#Isentropic Flow))
* `T0`: Stagnation temperature
* `Tc`: Pseudocritical temperature
* `Tr`: Reduced temperature
* `Tri`: Initial reduced temperature
* `trf`: Final reduced temperature
* `Ti`: Initial temperature
* `Tf`: Final temperature
* `V`: Volume
* `Vi`: Initial volume
* `Vf`: Final volume
* `vrms`: Root-mean-square (rms) velocity
* `W`: Work (dim.: force·length, in SI: joule, J)
* `Z`: Gas compressibility correction factor
* `Zi`: Initial gas compressibility correction factor
* `Zf`: Final gas compressibility correction factor

### Ideal Gas

* To calculate `[n_mol;m_kg]` (Number of moles; Mass) from 4 known variables:
```rpl
T=16.85_°C  P=1_atm  V=25_l  MW=36_g/mol
@ Expecting [ n=1.05056 86529 9 mol m=3.78204 71507 7⁳⁻² kg ]
'ROOT(ⒺIdeal Gas;[n;m];[1_mol;1_kg])'
```

### Ideal Gas Law Change

* To calculate `[Vf_l]` (Volume final) from 5 known variables:
```rpl
Pi=1.5_kPa  Pf=1.5_kPa  Vi=2_l  Ti=100_°C  Tf=373.15_K
@ Expecting [ Vf=2. l ]
'ROOT(ⒺIdeal Gas Law Change;[Vf];[1_l])'
```

### Isothermal Expansion

These equations apply to an ideal gas.

* To calculate `[m_kg;W_J]` (Mass; Work) from 5 known variables:
```rpl
Vi=2_l  Vf=125_l  T=300_°C  n=0.25_mol  MW=64_g/mol
@ Expecting [ m=0.016 kg W=4 926.46608 432 J ]
'ROOT(ⒺIsothermal Expansion;[m;W];[1_kg;1_J])'
```

### Polytropic Processes

These equations describe a reversible pressure-volume change of an ideal gas such that `P·Vn` is constant. Special cases include isothermal processes (`n = 1`), isentropic processes (`n = k`, the specific heat ratio), and constant-pressure processes (`n = 0`).

* To calculate `[n_1;Tf_°F]` (Polytropic number; Final temperature) from 5 known variables:
```rpl
Pi=15_psi  Pf=35_psi  Vi=1_ft^3  Vf=0.50_ft^3  Ti=75_°F
@ Expecting [ n=1.22239 24213 4 Tf=303.98333 3333 K ]
'ROOT(ⒺPolytropic Processes;[n;Tf];[1;1_K])'
```

### Isentropic Flow

![Isentropic Flow](img/IsentropicFlow.bmp)

The calculation differs at velocities below and above Mach 1. The Mach number is based on the speed of sound in the compressible fluid.

* To calculate `[P_kPa;ρ_kg/m^3;At_cm^2]` (Flow pressure; Flow density; Throat area) from 7 known variables:
```rpl
k=2  M=0.9  T0=26.85_°C  T=373.15_K  ρ0=100_kg/m^3  P0=100_kPa  A=1_cm^2
@ Expecting [ P=154.71213 6111 kPa ρ=124.38333 3333 kg/m↑3 At=0.99280 71853 34 cm↑2 ]
'ROOT(ⒺIsentropic Flow;[P;ρ;At];[1_kPa;1_kg/m^3;1_cm^2])'
```

### Real Gas Law

These equations adapt the ideal gas law to emulate real-gas behavior.

* To calculate `[Z_1;n_mol;m_kg]` (Gas compressibility correction factor; Number of mole; Mass) from 7 known variables:
```rpl
Pc=48_atm  Tc=298_K  P=5_kPa  V=10_l  MW=64_g/mol  T=348.15_K
@ Expecting [ Z=0.99977 57972 69 n=1.72769 39032 1⁳⁻² mol m=1.10572 40980 5⁳⁻³ kg ]
'ROOT(ⒺReal Gas Law;[Z;n;m];[1;1_mol;1_kg])'
```

### Real Gas State Change

This equation adapts the ideal gas state-change equation to emulate real-gas behavior.

* To calculate `[Zi_1;Zf_1;Vf_l]` (Initial & Final gas compressibility correction factor; Final volume) from 7 known variables:
```rpl
Pc=48_atm  Pi=100_kPa  Pf=50_kPa  Ti=348.15_K  Tc=298_K  Vi=10_l  Tf=523.15_K
@ Expecting [ Zi=0.99550 62096 36 Zf=0.99938 68303 14 Vf=30.17028 92973 l ]
'ROOT(ⒺReal Gas State Change;[Zi;Zf;Vf];[1;1;1_l])'
```

### Kinetic Theory

These equations describe properties of an ideal gas.

* To calculate `[vrms_m/s;n_mol;m_kg;λ_nm]` (Root-mean-square velocity; Number of mole; Mean free path) from 7 known variables:
```rpl
P=100_kPa  V=2_l  T=300_K  MW=18_g/mol  d=2.5_nm
@ Expecting [ vrms=644.76595 0487 m/s n=0.08018 15700 28 mol m=1.44326 82605 1⁳⁻³ kg λ=1.49162 49859 nm ]
'ROOT(ⒺKinetic Theory;[vrms;n;m;λ];[1_m/s;1_mol;1_kg;1_nm])'
```


## Heat transfer

The 31 variables in the Heat Transfer section are:

* `α`: Expansion coefficient (dim.: 1/temperature, in SI: K^-1)
* `δ`: Elongation (dim.: length)
* `λ1`: Lower wavelength limits (dim.: length)
* `λ2`: Upper wavelength limits (dim.: length)
* `λmax`: Wavelength of maximum emissive power (dim.: length)
* `ΔT`: Temperature difference
* `A`: Area
* `c`: Specific heat (dim.: energy/(mass·temperature), in SI: J/(kg·K))
* `eb12`: Emissive power in the range λ1 to λ2 (dim.: power/area, in SI: W/m^2)
* `eb`: Total emissive power (dim.: power/area, in SI: W/m^2)
* `f`: Fraction of emissive power in the range λ1 to λ2
* `h`: Convective heat-transfer coefficient (dim.: power/(area·temperature) in SI: W/(m^2·K))
* `h1`: Convective heat-transfer coefficient (dim.: power/(area·temperature) in SI: W/(m^2·K))
* `h3`: Convective heat-transfer coefficient (dim.: power/(area·temperature) in SI: W/(m^2·K))
* `k`: Thermal conductivity (dim.: power/(length·temperature) in SI: W/(m·K))
* `k1`: Thermal conductivity (dim.: power/(length·temperature) in SI: W/(m·K))
* `k2`: Thermal conductivity (dim.: power/(length·temperature) in SI: W/(m·K))
* `k3`: Thermal conductivity (dim.: power/(length·temperature) in SI: W/(m·K))
* `L`: Length
* `L1`: Length
* `L2`: Length
* `L3`: Length
* `m`: Mass
* `Q`: Heat capacity (dim.: force·length, in SI: joule, J)
* `qr`: Heat transfer rate (dim.: power=energy/time, in SI: watt, W)
* `T`: Temperature
* `Tc`: Cold surface temperature ([Conduction^](#Conduction)), or Cold fluid temperature ([Convection](#Convection))
* `Th`: Hot surface temperature, or Hot fluid temperature ([Conduction + Convection](#Conduction + Convection))
* `Ti`: Initial temperature
* `Tf`: Final temperature
* `U`: Overall heat transfer coefficient (dim.: power/(area·temperature) in SI: W/(m^2·K))

### Heat Capacity

* To calculate `[Tf_°C;c_kJ/(kg*K)]` (Final temperature; Specific heat) from 4 known variables:
```rpl
ΔT=15_°C  Ti=0_°C  m=10_kg  Q=25_kJ
@ Expecting [ Tf=15 °C c=0.16666 66666 67 kJ/(kg·K) ]
'ROOT(ⒺHeat Capacity;[Tf;c];[1_°C;1_kJ/(kg*K)])'
```

### Thermal Expansion

![Thermal Expansion](img/ThermalExpansion.bmp)

* To calculate `[α_K^-1;Ti_°C]` (Expansion coefficient; Initial temperature) from 4 known variables:
```rpl
ΔT=15_°C  L=10_m  Tf=25_°C  δ=1_cm
@ Expecting [ α=6.66666 66666 7⁳⁻⁵ K⁻¹ Ti=10 °C ]
'ROOT(ⒺThermal Expansion;[α;Ti];[1_K^-1;1_°C])'
```

### Conduction

![Conduction](img/Conduction.bmp)

* To calculate `[qr_W;ΔT_°C]` (Heat transfer rate; Temperature difference) from 5 known variables:
```rpl
Tc=25_°C  Th=75_°C  A=12.5_m^2  L=1.5_cm  k=0.12_W/(m*K)
@ Expecting [ qr=5 000. W ΔT=50 °C ]
'ROOT(ⒺConduction;[qr;ΔT];[1_W;1_°C])'
```

### Convection

![Convection](img/Convection.bmp)

* To calculate `[Th_°C;ΔT_°C]` (Hot surface temperature; Temperature difference) from 4 known variables:
```rpl
Tc=26.85_°C  A=200_m^2  h=0.005_W/(m^2*K)  qr=10_W
@ Expecting [ Th=36.85 °C ΔT=10. °C ]
'ROOT(ⒺConvection;[Th;ΔT];[1_°C;1_°C])'
```

### Conduction & Convection

If you have fewer than three layers, give the extra layers a zero thickness and any nonzero conductivity. The two temperatures are fluid temperatures – if instead you know a surface temperature, set the corresponding convective coefficient to 10^999999.

![Conduction + Convection](img/Conduction+Convection.bmp)

* To calculate `[Tc_°C;qr_W;U_W/(m^2*K)]` (Cold surface temperature; Heat transfer rate; Overall heat transfer coefficient) from 11 known variables:
```rpl
ΔT=35_°C  Th=55_°C  A=10_m^2  h1=0.05_W/(m^2*K)  h3=0.05_W/(m^2*K)  L1=3_cm  L2=5_cm  L3=3_cm  k1=0.1_W/(m*K)  k2=.5_W/(m*K)  k3=0.1_W/(m*K)
@ Expecting [ qr=8.59950 85995 1 W Tc=20 °C U=0.02457 00245 7 W/(m↑2·K) ]
'ROOT(ⒺConduction & Convection;[qr;Tc;U];[1_W;1_°C;1_W/(m^2*K)])'
```

### Black Body Radiation

F0λ(λ_m, T_K) is the black body emissive power Function which returns the fraction of total black-body emissive power at temperature `T_K` between wavelengths 0 and `λ_m`. It is the integral of the Planck distribution.

![Black Body Radiation](img/BlackBodyRadiation.bmp)

* To calculate `[λmax_nm;eb_W/m^2;f_1;eb12_W/m^2;q_W]` (Wavelength of maximal emission; Total emissive power; Fraction of emissive power between λ1 & λ2; Emissive power between λ1 & λ2; Heat transfer rate) from 4 known variables:
```rpl
T=1273,15_K  Tmax=1273,15_K  λ1=1000_nm  λ2=600_nm  A=1_cm^2
@ Expecting [ λmax=2 276.06484 325 nm eb=148 980.70811 W/m↑2 f=3.60911 40468 1⁳⁻³ eb12=537.68836 6343 W/m↑2 q=14.89807 0811 W ]
'ROOT(ⒺBlack Body Radiation;[λmax;eb;f;eb12;q];[1_nm;1_W/m^2;1;1_W/m^2;1_W])'
```


## Magnetism

The 28 variables in the Magnetism section are:

* `α1`: Subtended internal left angle relative to the top ends of the solenoid
* `α2`: Subtended internal right angle relative to the top ends of the solenoid
* `θ₁`: Subtended left angle relative to the top ends of the solenoid
* `θ₂`: Subtended right angle relative to the ends of the wire
* `θ`: Angle between the line of the magnetic field and the speed of the moving charge
* `μr`: Relative permeability
* `B`: Magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `d`: Separation distance
* `Dpitch`: Pitch of the helicoidal motion (dim.: length)
* `Fba`: Force
* `fc`: Cyclotron frequency (dim.: time^-1, in SI: hertx, Hz)
* `I`: Current (dim.: charge/time, in SI: ampere, A)
* `Ia`: Current (dim.: charge/time, in SI: ampere, A)
* `Ib`: Current (dim.: charge/time, in SI: ampere, A)
* `L`: Length
* `m`: Mass
* `N`: Total number of turns
* `nl`: Number of turns per unit length (dim.: length^-1)
* `q`: Charge of the moving charge (dim.: charge, in SI: coulomb, C)
* `r`: Distance from center of wire
* `Rc`: Radius of the circular part of the motion
* `ri`: Inside radius of toroid
* `ro`: Outside radius of toroid
* `rw`: Radius of wire
* `uB`: Volumic density of magnetic energy (dim.: energy/volume, in SI: J/m^3)
* `v`: Speed of the moving charge
* `T`: Period (dim.: time)
* `VH`: Hall tension (dim.: energy/charge, in SI: volt, V)

#### Straight Wire Infinite

The magnetic field expression differs depending upon whether the point at `r` is inside or outside the wire of radius `rw` and the calculations are done accordingly. The expression for the magnetic field at the distance `r` is approximately valid if the distance is such that `r << L` and therefore also applies for a wire of finite length `L` providing that `r < L/10` (see the following example 2 compared to example 2 of "Straight Wire Finite"). Note that if an electric current passes through a straight wire, one must use the following right-hand rule te determine the direction of the `B` field: when the thumb is pointed in the direction of conventional current (from positive to negative), the curled fingers will then point in the direction of the magnetic field (see fig.).


![Straight Wire Infinite](img/StraightWireInfinite.bmp)

* **Example 1.** Inside the wire, to calculate `[B_T]` (Magnetic field) from 4 known variables:
```rpl
μr=1  rw=0.25_cm  r=0.2_cm  I=25_A
@ Expecting [ B=1.59999 99997 9⁳⁻³ T ]
'ROOT(ⒺStraight Wire Infinite;[B];[1_T])'
```

* **Example 2.** Outside the wire, to calculate `[B_T]` (Magnetic field) from 4 known variables:
```rpl
μr=1  rw=0.25_cm  r=5_cm  I=25_A
@ Expecting [ B=9.99999 99986 8⁳⁻⁵ T ]
'ROOT(ⒺStraight Wire Infinite;[B];[1_T])'
@ Save B for later
```

The code below saves the reference value for comparison with the example 2 in [B Field Finite Wire](#B Field Finite Wire):
```rpl
@ Save the reference value for comparison below
B0=B
```

#### Straight Wire Finite

The expression for the magnetic field at the distance `r` depends on the subtended angles `θ₁` and `θ₂` relative to the ends of the wire of finite length `L`. The magnetic field expression differs depending upon whether the point at `r` is inside or outside the wire of radius `rw` and the calculations are done accordingly.

![B Field From Finite Wire](img/B_Field_From_Finite_Wire.bmp)

* **Example 1.** To calculate `[B_T]` (Magnetic field) from 6 known variables:
```rpl
μr=1_1  rw=0.25_cm  r=5_cm  I=25_A  θ₁=30_°  θ₂=150_°
@ Expecting [ B=8.66025 40367⁳⁻⁵ T ]
'ROOT(ⒺStraight Wire Finite;[B];[1_T])'
@ Save for test below
```

* **Example 2.** When 'r << L' which means 'r < L/10', we can verify that the value of B for a infinite wire approximates the exact value calculated for a finite wire of length 'L'.
```rpl
L=3_m  μr=1  rw=0.25_cm  r=5_cm  I=25_A  θ₁='atan(r/L/2)'  θ₂='180_°-θ₁'
if 'r < L/10' then
@ Expecting [ B=9.99965 27945 4⁳⁻⁵ T ]
'ROOT(ⒺStraight Wire Finite;[B];[1_T])'
end
@ Save for test below
```
Verify relative difference under condition `5_cm << 3_m` with the example 2 of [Straight Wire Infinite](#Straight Wire Infinite)
```rpl
B0 B %Ch
@ Expecting -3.47204 13877 8⁳⁻³
@ % of relative difference
```

#### Force Between Wires

The force between wires is positive for an attractive force (for currents having the same sign) and negative otherwise, corresponding to a repulsive force.

![Force Between Wires](img/ForceBetweenWires.bmp)

* To calculate `[Fba_N]` (Magnetic force) from 4 known variables:
```rpl
Ia=10_A  Ib=20_A  μr=1  L=50_cm  d=1_cm
@ Expecting [ Fba=1.99999 99997 4⁳⁻³ N ]
'ROOT(ⒺForce Between Wires;[Fba];[1_N])'
```

#### B Field In Infinite Solenoid

The expression for the magnetic field in the center is approximately valid if the radius of the solenoid < `L` and therefore also applies inside a solenoid of finite length `L`. The right-hand rule applies also here: when the fingers curl around the solenoid in the sense of the current, the thumb points in the direction of the magnetic field (see fig.).

![B Field In Infinite Solenoid](img/BFieldInInfiniteSolenoid.bmp)

* To calculate `[B_T]` (Magnetic field) from 3 known variables:
```rpl
μr=10  nl=5000_m^-1  I=1.25_A
@ Expecting [ B=0.07853 98163 29 T ]
'ROOT(ⒺB Field In Infinite Solenoid;[B];[1_T])'
@ Save B for later use
```
The code below saves the reference value for comparison with the example 2 in [B Field Finite Solenoid](#B Field Finite Solenoid):
```rpl
@ Save the reference value for comparison below
B0=B
@ Save B0 for later use
```

#### B Field In Finite Solenoid

The expression for the magnetic field in the center depends on the subtended internal angles `α1` and `α2` relative to the top ends of the solenoid of finite length `L`.

![B Field In Finite Solenoid](img/B_Field_In_Finite_Solenoid.bmp)

* **Example 1.** Inside the wire, to calculate `[B_T]` (Magnetic field) from 5 known variables:
```rpl
μr=10  nl=5_mm^-1  I=1.25_A  α1=150_°  α2=30_°
@ Expecting [ B=6.80174 76149 8⁳⁻² T ]
'ROOT(ⒺB Field In Finite Solenoid;[B];[1_T])'
@ Save variables for later use
```

* **Example 2.** When `r << L` which means `r < L/10`, we can verify that the value of `B` for a infinite solenoid approximates the exact value calculated for a finite solenoid of length `L`.
```rpl
L=3_m  μr=10  r=10_cm  nl=5000_m^-1  I=1.25_A  α2='atan(r/L/2)'  α1='180_°-α2'
if 'r < L/10' then
@ Expecting [ B=0.07852 89102 94 T ]
'ROOT(ⒺB Field In Finite Solenoid;[B];[1_T])'
end
@ Save variables for later use
```
Verify relative difference under condition 10_cm << 3_m with the example of [B Field In Infinite Solenoid](#B Field In Infinite Solenoid)
```rpl
B0 B %Ch
@ Expecting -0.01388 59960 4
@ % of relative difference
```

#### B Field In Toroid

The magnetic field `B` is calculated in the center of the torroid. The right-hand rule applies also here: when the fingers curl around the outer circle of the torroid following the sense of the current, the thumb points in the direction of the central magnetic field (see fig.).

![B Field In Toroid](img/BFieldInToroid.bmp)

* To calculate `[B_T]` (Magnetic field) from 5 known variables:
```rpl
μr=10  N=50  ri=5_cm  ro=7_cm  I=10_A
@ Expecting [ B=1.66666 66664 5⁳⁻² T ]
'ROOT(ⒺB Field In Toroid;[B];[1_T])'
```

#### Hall Effect

When a conductor carrying a current (to the right) is suddenly plunged into a perpendicular magnetic field (towards the bottom), there is a deviation (towards the top) of the free electrons which unbalances the distribution of electric charges. This transfer of charges from top to bottom gives rise to an electric force which in return balances the magnetic force. And it is precisely the appearance of this new electric field which is revealed by the Hall voltage `VH` measured between the top and the bottom of the conductor (small multimeter on the right).

![Hall Effect](img/Hall Effect VH.bmp)

* To calculate `[VH_V]` (Hall tension) from 5 known variables:
```rpl
n=5e28_(1/m^3)  B=0.641_T  q=1.60217 6634e-19_C  L=2_mm  I=10_A
@ Expecting [ VH=4.00080 73167 3⁳⁻⁷ V ]
'ROOT(ⒺHall Effect;[VH];[1_V])'
```

#### Cyclotron Motion

Under the perpendicular magnetic field, the moving charge has a circular trajectory and turns at the cyclotron frequency with the rotation period `T`.

![Cyclotron Motion](img/Cyclotron_BW.bmp)

* To calculate `[Rc_m;fc_Hz;T_s]` (Radius of the circular path; Cyclotron frequency; Period) from 4 known variables:
```rpl
m=1.67262 19259 5e-27_kg  B=0.8_T  q=1.60217 6634e-19_C  v=4.6e7_m/s
@ Expecting [ Rc=0.60028 18834 15 m fc=12 196 149.1501 Hz T=8.19930 93696 5⁳⁻⁸ s ]
'ROOT(ⒺCyclotron Motion;[Rc;fc;T];[1_m;1_Hz;1_s])'
```

#### Helicoidal Motion

Under the magnetic field lines (at angle `θ` with the speed vector), the moving charge has an helicoidal trajectory of pitch `Dpitch`, radius `Rc` and period `T`.

![Helicoidal Motion](img/Helicoidal Motion BW.bmp)

* To calculate `[Rc_m;T_s;Dpitch_m]` (Radius of the circular path; Period, pitch of the helicoidal motion) from 4 known variables:
```rpl
m=1.67262 19259 5e-27_kg  B=0.8_T  q=1.60217 6634e-19_C  v=4.6e7_m/s  θ=30_°
@ Expecting [ Rc=0.30014 09417 08 m T=8.19930 93696 5⁳⁻⁸ s Dpitch=3.26637 26955 m ]
'ROOT(ⒺHelicoidal Motion;[Rc;T;Dpitch];[1_m;1_s;1_m])'
```

#### Volumic Density Magnetic Energy

* To calculate `[uB_(J/m^3)]` (Volumic density of magnetic energy) from 2 known variables:
```rpl
μr=3.0  B=0.8_T
@ Expecting [ uB=84 882.63632 69 J/m↑3 ]
'ROOT(ⒺVolumic Density Magnetic Energy;[uB];[1_(J/m^3)])'
```


## Motion

The 38 variables in the Motion section are:

* `α`: Angular acceleration (dim.: angle/time^2, in SI: r/s^2)
* `ω`: Angular velocity ([Circular Motion](#Circular Motion)), or Angular velocity at `t` ([Angular Motion](#Angular Motion)) (dim.: angle/time, in SI: r/s)
* `ω₀`: Initial angular velocity (dim.: angle/time, in SI: r/s)
* `ρ`: Fluid density (dim.: mass/volume, in SI: kg/m^3)
* `θ`: Angular position at `t` (dim.: angle)
* `θ0`: Initial angular position ([Angular Motion](#Angular Motion)), or Initial vertical angle ([Projectile Motion](#Projectile Motion))
* `φ`: Latitude (dim.: angle)
* `a`: Acceleration
* `Ah`: Projected horizontal area
* `ar`: Centripetal acceleration at `r`
* `Cd`: Drag coefficient
* `fr`: Fraction of the terminal velocity `vt`
* `gloc`: local gravitational acceleration of a planet or star (dim.: length/time^2)
* `gearth`: local gravitational acceleration on Earth (dim.: length/time^2)
* `h`: altitude (dim.: length)
* `hmax`: maximum height of the projectile Motion
* `m`: Mass
* `Mp`: Planet or star mass
* `N`: Rotational speed (dim.: time^-1, in SI: hertz, Hz)
* `R`: Horizontal range ([Projectile Motion](#Projectile Motion)), or Planet or Star radius ([Object in Free Fall](#Object in Free Fall)) & ([Escape and Orbital Velocities](#Escape and Orbital Velocities))
* `r`: Radius
* `rc`: Radius of circular motion
* `t`: Time
* `tf`: Time of flight of a projectile
* `tfr`: time required to reach the fraction `fr` of the terminal velocity
* `v`: Velocity at `t` ([linear Motion](#linear Motion)), or Tangential velocity at `r` ([Circular Motion¸](#Circular Motion)), or Falling velocity at time `t` ([Terminal Velocity](#Terminal Velocity)) & ([Buoyancy & Terminal Velocity](#Buoyancy & Terminal Velocity))
* `v0`: Initial velocity
* `ve`: Escape velocity in a gravitational field
* `vcx`: Horizontal (component x) of velocity at `t`
* `vcy`: Vertical (component y) of velocity at `t`
* `vo`: Orbital velocity in a gravitational field
* `Vol`: Volume of the moving object
* `vt`: Terminal velocity reached in a vertical fall
* `x`: Horizontal position at `t`
* `x0`: Initial horizontal position
* `xfr`: Displacement during `tfr`
* `y`: Vertical position at `t`
* `y0`: Initial vertical position

#### Linear Motion

* To calculate `[a_m/s^2;v_m/s]` (Acceleration; Velocity at time `t`) from 4 known variables:
```rpl
x0=0_m  x=100_m  t=10_s  v0=1_m/s
@ Expecting [ a=1.8 m/s↑2 v=19. m/s ]
'ROOT(ⒺLinear Motion;[a;v];[1_m/s^2;1_m/s])'
```

#### Object In Free Fall

By definition, an object in free fall only experiences local gravitational acceleration `gloc`. This depends on the mass of the star or planet and the distance `r` center to center (where we assume that the position is greater than the radius of the mass). For the Earth, we can calculate an approximate value `gearth` of the acceleration of gravity as a function of latitude `φ` and for an altitude `h` low compared to the Earth's radius (typically: a few thousand meters, valid in commercial aviation).

* To calculate `[t_s;v_ft/s;r_m;gearth_m/s^2]` (Time; Velocity at time `t`; Radius; Acceleration of gravity at latitude `φ` & altitude `h`) from 7 known variables:
```rpl
y0=1000_ft  y=0_ft  v0=0_ft/s  gloc=9.80665_m/s↑2  φ=45_°  h=1000_m  Mp=5.9722e24_kg
@ Expecting [ t=7.88428 18533 5 s v=-253.66926 7182 ft/s r=6 375 433.14029 m gearth=9.80321 00310 8 m/s↑2 ]
'ROOT(ⒺObject In Free Fall;[t;v;r;gearth];[1_s;1_ft/s;1_m;1_m/s^2])'
```

#### Projectile Motion

During the time of flight `tf`, the motion of a projectile follows a symetric parabole of horizontal range `R` and of maximum height `hmax`.

![Projectile Motion](img/Projectile_BW.bmp)

* To calculate `[R_ft;vcx_ft/s;vcy_ft/s;x_ft;y_ft;hmax_ft;tf_s]` (Range, Components `x` & `y` of velocity at time `t`; Positions `x` & `y` at time `t`; Maximum height; Time of flight) from 5 known variables:
```rpl
x0=0_ft  y0=0_ft  θ0=45_°  v0=200_ft/s  t=10_s
@ Expecting [ R=1 243.23800 686 ft vcx=141.42135 6237 ft/s vcy=-180.31912 9327 ft/s x=1 414.21356 237 ft y=-194.48886 5448 ft hmax=310.80950 1716 ft tf=8.79102 02528 1 s ]
'ROOT(ⒺProjectile Motion;[R;vcx;vcy;x;y;hmax;tf];[1_ft;1_ft/s;1_ft/s;1_ft;1_ft;1_ft;1_s])'
```

#### Angular Motion

* To calculate `[ω_r/min;Θ_°]` (Angular velocity at time `t`; Angular position at time `t`) from 4 known variables:
```rpl
θ0=0_°  ω₀=0_r/min  α=1.5_r/min^2  t=30_s
@ Expecting [ ω=0.75 r/min θ=10.74295 86587 ° ]
'ROOT(ⒺAngular Motion;[ω;θ];[1_r/min;1_°])'
```

#### Uniform Circular Motion

* To calculate `[ω_r/min;ar_ft/s^2;N_rpm]` (Angular velocity; Centripetal acceleration at `r`; Rotational speed) from 2 known variables:
```rpl
rc=25_in  v=2500_ft/s
@ Expecting [ ω=72 000. r/min ar=3 000 000. ft/s↑2 N=11 459.15590 26 rpm ]
'ROOT(ⒺUniform Circular Motion;[ω;ar;N];[1_r/min;1_ft/s^2;1_rpm])'
```

#### Terminal Velocity

Terminal velocity is the maximum speed attainable by an object as it falls through a fluid like air for instance. It is reached when the sum of the increasing drag force is equal to the downward force of gravity acting on the object (neglecting buoyancy), leading to a zero net force at the resulting terminal velocity.

* **Example 1**. For a falling big mass, to calculate `[vt_ft/s;v_ft/s;tfr_s;xfr_ft]` (Terminal velocity; Velocity at time `t`; Time required to reach the fraction `fr` of `vt`; Displacement during `tfr`) from 6 known variables:
```rpl
Cd=0.15  ρ=0.025_lb/ft^3  Ah=100000_in^2  m=1250_lb  t=5_s  fr=0.95
@ Expecting [ vt=175.74722 3631 ft/s v=127.18655 2185 ft/s tfr=10.00590 25332 s xfr=1 117.39339 247 ft ]
'ROOT(ⒺTerminal Velocity;[vt;v;tfr;xfr];[1_ft/s;1_ft/s;1_s;1_ft])'
```

* **Example 2**. For a human skydiving head first, to calculate `[vt_m/s;v_m/s;tfr_s;xfr_m]` (Terminal velocity; Velocity at time `t`; Time required to reach the fraction `fr` of `vt`; Displacement during `tfr`) from 6 known variables:
```rpl
Cd=0.7  ρ=1.29_kg/m^3  Ah=0.18_m^2  m=75_kg  t=5_s  fr=0.95
@ Expecting [ vt=312.11229 4878 ft/s v=147.99138 9715 ft/s tfr=17.76964 17471 s xfr=3 524.12177 429 ft ]
'ROOT(ⒺTerminal Velocity;[vt;v;tfr;xfr];[1_ft/s;1_ft/s;1_s;1_ft])'
```

#### Buoyancy & Terminal Velocity

Terminal velocity is the maximum speed attainable by an object as it falls through a fluid like air for instance. It is reached when the sum of the increasing drag force plus the buoyancy is equal to the downward force of gravity acting on the object, leading to a zero net force at the resulting terminal velocity. If the terminal velocity is found to be negative, the motion is upward because buoyancy dominates gravity (see example 2).

* **Example 1**. For a golf ball falling in water, to calculate `[vt_m/s;v_m/s;tfr_s;xfr_m]` (Terminal velocity; Velocity at time `t`; Time required to reach the fraction `fr` of `vt`; Displacement during `tfr`) from 8 known variables:
```rpl
Cd=0.5  ρ=1077,5_(kg/m^3)  ρf=1000_(kg/m^3)  d=4.282_cm  Ah=14.40068 68745_cm↑2  Vol=41.10916 07978_cm↑3  t=3e-2_s  fr=0.95
@ Expecting [ vt=0.29459 06011 51 m/s v=0.22419 40616 41 m/s tfr=5.50264 78343 2⁳⁻² s xfr=1.03003 49562 7⁳⁻² m ]
'ROOT(ⒺBuoyancy & Terminal Velocity;[vt;v;tfr;xfr];[1_m/s;1_m/s;1_s;1_m])'
```

* **Example 2**. For a CO2 bubble in a glass of champagne, to calculate `[vt_m/s;v_m/s;tfr_s;xfr_m]` (Terminal velocity; Velocity at time `t`; Time required to reach the fraction `fr` of `vt`; Displacement during `tfr`) from 8 known variables:
```rpl
Cd=0.01  ρ=1.98_(kg/m^3)  ρf=998_(kg/m^3)  d=0.1_cm  Ah=7.85398 16339 7e-3_cm↑2  Vol=5.23598 77559 8e-4_cm↑3  t=0.1_s  fr=0.95
@ Expecting [ vt=-1.14234 81034 5 m/s v=-0.79446 37698 68 m/s tfr=0.21337 88142 9 s xfr=0.15488 56277 51 m ]
'ROOT(ⒺBuoyancy & Terminal Velocity;[vt;v;tfr;xfr];[1_m/s;1_m/s;1_s;1_m])'
```

#### Escape and Orbital Velocities

The escape velocity is the speed required to completely free oneself from the gravitational field of a star, planet, etc. It is defined as the initial speed allowing you to move away to infinity. The orbital velocity is the speed nneded to maintain a stable circular orbit in a gravitational field.

* To calculate `[ve_m/s;vo_m/s]` (Escape velocity; Orbital velocity) from 2 known variables:
```rpl
Mp=1.5E23_lb  R=5000_mi
@ Expecting [ ve=3 485.55509 628 ft/s vo=2 464.65964 478 ft/s ]
'ROOT(ⒺEscape & Orbital Velocity;[ve;vo];[1_ft/s;1_ft/s])'
```


## Optics

The 40 variables in the Optics section are:

* `Δα`: Phase difference between top and bottom rays of light from a single slit (dim.: angle, in SI: r)
* `Δφ`: Phase difference between two rays of light seperated by an angle θ (dim.: angle, in SI: r)
* `Δyint`: Distance between bright fringes on the interference screen
* `Δydiff`: Width of the central maximum of diffraction
* `λ`: Light wavelength
* `θ`: Angle between initial light polarisation direction and polarizer transmission axis ([Malus Law](#Malus Law)), or Angle subtended by two points separated by y on the screen and the middle of one slit ([2 Slits Young Interference](#2 Slits Young Interference)), or two slits ([One Slit Diffraction](#One Slit Diffraction))
* `θ0`: Acceptance angle to enter an optical fiber in the outer medium of refraction index `n0f`
* `θ₁`: Angle of incidence in the medium of refraction index n1
* `θ₂`: Angle of refraction in the medium of refraction index n2
* `θB`: Brewster angle
* `θc`: Critical angle
* `θr`: Rayleigh's criterion limiting
* `a`: Width of a diffraction slit
* `d`: Distance between two interference slits
* `E₀`: Incident light electrtc field
* `f`: Focal length
* `fx₀`: Frequency of the incident X-ray (dim.: time^-1, in SI: hertz, Hz)
* `fx`: Frequency of the transmitted X-ray (dim.: time^-1, in SI: hertz, Hz)
* `I`: Transmitted irradiance or polarized light radiance flux (dim.: power/area, in SI: W/m^2)
* `I₀`: Incident irradiance or polarized light radiance flux (dim.: power/area, in SI: W/m^2)
* `Ix`: Transmitted irradiance or polarized X rays radiance flux (dim.: power/area, in SI: W/m^2)
* `Ix0`: Incident irradiance or polarized X rayx radiance flux (dim.: power/area, in SI: W/m^2)
* `m`: Magnification
* `n`: Index of refraction
* `n1`: Index of refraction of medium 1
* `n2`: Index of refraction of medium 2
* `nf0`: Index of refraction of the outer medium at the entry of an optic fiber
* `nf1`: Index of refraction of the core medum of an optic fiber
* `nf2`: Index of refraction of the cladding medium of an optic fiber
* `r`: Radius of curvature
* `r1`: Radius of curvature
* `r2`: Radius of curvature
* `u`: Distance to object
* `v`: Distance to image
* `v1`: Speed of light in medium of refraction index n1
* `v2`: Speed of light in medium of refraction index n2
* `vn`: Speed of light in medium of refraction index n
* `L`: distance to the observation screen or to the light detector
* `NA`: Numerical aperture of the optic fiber
* `y`: distance between two image points on the observation screen (perpendicular to the optical axis) or distance between two point source

For reflection and refraction problems, the focal length and radius of curvature are positive in the direction of the outgoing light (reflected or refracted). The object distance is positive in front of the surface. The image distance is positive in the direction of the outgoing light (reflected or refracted). The magnification is positive for an upright image.

#### Refraction Law

![Refraction Law](img/RefractionLaw.bmp)

* To calculate `[θ₂_°;v1_m/s;v2_m/s]` (Refraction angle; Speed of light in media of refraction index `n1` & `n2`) from 3 known variables:
```rpl
n1=1  n2=1.333  θ₁=45_°
@ Expecting [ θ₂=32.03672 30399 ° v1=299 792 458 m/s v2=224 900 568.642 m/s ]
'ROOT(ⒺRefraction Law;[θ₂;v1;v2];[1_°;1_m/s;1_m/s])'
```

#### Critical Angle

![Critical Angle](img/CriticalAngle.bmp)

* To calculate `[θc_°;v1_m/s;v2_m/s]` (Critical angle; Speed of light in media of refraction index `n1` & `n2`) from 2 known variables:
```rpl
n1=1  n2=1.5
@ Expecting [ θc=41.81031 48958 ° v1=299 792 458 m/s v2=199 861 638.667 m/s ]
'ROOT(ⒺCritical Angle;[θc;v1;v2];[1_°;1_m/s;1_m/s])'
```

#### Fiber Optic

![Fiber Optic](img/Fiber Optic BW.bmp)

* To calculate `[vf0_m/s;vf1_m/s;vf2_m/s;NA;θ0_°]` (Speed of light in media of refraction index `n1f` & `n2f`; Numerical aperture; Acceptance angle) from 3 known variables:
```rpl
nf0=1.2  nf1=1.5  nf2=1.45
@ Expecting [ vf0=249 827 048.333 m/s vf1=199 861 638.667 m/s vf2=206 753 419.31 m/s NA=0.32004 77394 95 θ0=18.66581 19909 ° ]
'ROOT(ⒺFiber Optic;[vf0;vf1;vf2;NA;θ0];[1_m/s;1_m/s;1_m/s;1;1_°])'
```

#### Brewster’s Law

The Brewster angle is the angle of incidence at which the reflected wave is completely polarized.

![Brewster’s Law](img/Brewster’sLaw.bmp)

* To calculate `[θB_°;θ₂_°;v1_m/s;v2_m/s]` (Brewster input angle; Refraction angle; Speed of light in media of refraction index `n1` & `n2`) from 2 known variables:
```rpl
n1=1  n2=1.5
@ Expecting [ v1=299 792 458 m/s v2=199 861 638.667 m/s θB=56.30993 2474 ° θ₂=33.69006 7526 ° ]
'ROOT(ⒺBrewster’s Law;[v1;v2;θB;θ₂];[1_m/s;1_m/s;1_°;1_°])'
```

#### Spherical Reflection

![Spherical Reflection](img/SphericalReflection.bmp)

* To calculate `[m;f_cm]` (Magnification; Focal distance) from 3 known variables:
```rpl
u=10_cm  v=300_cm  r=19.35_cm
@ Expecting [ m=-30 f=9.67741 93548 4 cm ]
'ROOT(ⒺSpherical Reflection;[m;f];[1;1_cm])'
```

#### Spherical Refraction

![Spherical Refraction](img/SphericalRefraction.bmp)

* To calculate `[n2;v1_m/s;v2_m/s]` (Refraction index in medium 2, speed of light in media of refraction index `n1`, `n2`) from 4 known variables:
```rpl
u=8_cm  v=12_cm  r=2_cm  n1=1
@ Expecting [ n2=1.5 v1=299 792 458 m/s v2=199 861 638.667 m/s ]
'ROOT(ⒺSpherical Refraction;[n2;v1;v2];[1;1_m/s;1_m/s])'
```

#### Thin Lens

![Thin Lens](img/ThinLens.bmp)

* To calculate `[f_cm;v_cm;m;vn_m/s]` (Focal distance, distance to image, magnification, speed of light in media of refraction index `n`) from 4 known variables:
```rpl
r1=5_cm  r2=20_cm  n=1.5  u=50_cm
@ Expecting [ f=13.33333 33333 cm v=18.18181 81818 cm m=-0.36363 63636 36 vn=199 861 638.667 m/s ]
'ROOT(ⒺThin Lens;[f;v;m;vn];[1_cm;1_cm;1;1_m/s])'
```

#### Rayleigh’s Criterion

![Rayleigh’s Criterion](img/Rayleigh's Criterion_BW.bmp)

* To calculate `[θr_°;y_m]` (Rayleigh’s criterion angle; Distance between two point source) from 3 known variables:
```rpl
λ=550_nm  d=9_mm  L=18.7_km
@ Expecting [ θr=4.27056 28265⁳⁻³ ° y=1.39381 16503 9 m ]
'ROOT(ⒺRayleigh’s Criterion;[θr;y];[1_°;1_m])'
```

#### Malus Law

If lineraly polarized light is incident on a perfect linear polarizer the transmitted light is the component at angle `θ` between the light polarisation direction and the polarizer transmission axis. The Malus law is given in terms of light irradiances. A relavistic version of the laws applies for X rays and more energetic electromagnetic radiations (with loss up to 10% in irradiance). The decrease in frequency (`fx < fx₀`) and therefore in energy (`h·fx`) of a transmitted photon is due to the movement of the interacting electron of the polarizer (Compton scattering).

![Malus Law](img/Malus Law BW.bmp)

* To calculate `[I_(W/m^2);Ix_(W/m^2),E₀_V/m]` (Polarized light radiance flux; Polarized radiance flux of emitted Xrays; Electric field) from 5 known variables:
```rpl
θ=30_°  I₀=10_(W/m^2)  fx₀=3e17_Hz  fx=2.7e17_Hz  I₀x=0.1_(W/m^2)
@ Expecting [ I=7.5 W/m↑2 Ix=6.75163 88932 1⁳⁻² W/m↑2 E₀=86.80210 98145 V/m ]
'ROOT(ⒺMalus Law;[I;Ix;E₀];[1_(W/m^2);1_(W/m^2);1_V/m])'
```

#### 2 Slits Young Interference

* To calculate `[Δφ_r;I_(W/m^2);y_m;Δyint_m]` (Phase difference between two rays of light seperated by an angle θ; Irradiance of emitted light; Distance between two image points on the observation screen, Distance between bright fringes) from 5 known variables:
```rpl
L=2_m  d=800._μm  λ=600_nm  θ='ASIN(0.6*(λ_nm)/(d_μm))'  Imax=10_(W/m^2)
@ Expecting [ Δφ=3.76991 11843 1 r I=6.54508 49718 7 W/m↑2 y=9.00000 09112 5⁳⁻⁴ m Δyint=0.0015 m ]
'ROOT(Ⓔ2 Slits Young Interference;[Δφ;I;y;Δyint];[1_r;1_(W/m^2);1_m;1_m])'
```

#### One Slit Diffraction

* To calculate `[Δα_r;I_(W/m^2);y_m;Δydiff_m]` (Phase difference between top and bottom rays of light; Irradiance of emitted light; Distance between two image points on the observation screen; Width of the central maximum) from 5 known variables:
```rpl
L=3_m  a=1000._μm  λ=600_nm  θ='ASIN(0.3*(λ_nm)/(a_μm))'  Imax=10_(W/m^2)
@ Expecting [ Δα=1.88495 55921 5 r I=7.36839 72932 2 W/m↑2 y=5.40000 00874 8⁳⁻⁴ m Δydiff=0.0036 m ]
'ROOT(ⒺOne Slit Diffraction;[Δα;I;y;Δydiff];[1_r;1_(W/m^2);1_m;1_m])'
```


## Oscillations

The 25 variables in the Oscillations section are:

* `ω`: Angular frequency (dim.: angle/time, in SI: r!s)
* `ω₀`: Natural angular frequency (dim.: angle#time, in SI: r!s)
* `ωu`: Underdamped angular frequency (dim.: angle/time, in SI: r!s)
* `γ`: Reduced damping coefficient (dim.: angle/time, in SI: r/s)
* `φ`: Phase angle
* `θ`: Cone angle
* `a`: Acceleration at `t`
* `E`: Total energy (kinetic plus potential) (dim.: force·length, in SI: joule, J)
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
* `T`: Period, or Period for small amplitude ([Simple Pendulum](#Simple Pendulum))
* `Treal`: Real period for large amplitude ([Simple Pendulum](#Simple Pendulum))
* `v`: Velocity at `t`
* `x`: Displacement at `t`
* `xm`: Displacement amplitude
* `xh`: Displacement amplitude of harmonic motion
* `xp`: Resulting displacement amplitude of driven & damped oscillations

#### Mass-Spring System

![Mass-Spring System](img/Mass-SpringSystem.bmp)

* To calculate `[ω_(r/s);T_s;f_Hz]` (Angular frequency; Oscillation period; Frequency) from 2 known variables:
```rpl
k=20_N/m  m=5_kg
@ Expecting [ ω=2. r/s T=3.14159 26535 9 s f=0.31830 98861 84 Hz ]
'ROOT(ⒺMass‐Spring System;[ω;T;f];[1_(r/s);1_s;1_Hz])'
```

#### Simple Pendulum

![Simple Pendulum](img/SimplePendulum.bmp)

* To calculate `[ω_(r/s);Treal_s;T_s;f_Hz]` (Angular frequency; Real period for large amplitude; Period for small amplitude; Frequency) from 2 known variables:
```rpl
L=15_cm  θmax=80_°
@ Expecting [ ω=8.08564 57173 6 r/s  Treal=0.88361 42622 96 s   T=0.77707 89775 87 s  f=1.28687 04840 1 Hz ]
'ROOT(ⒺSimple Pendulum;[ω;Treal;T;f];[1_(r/s);1_s;1_s;1_Hz])'
```
```rpl
@ Without Treal, MSOLVER works fine for the remaining 3 unknowns:
L=15_cm  θmax=80_°
@ Expecting [ ω=8.08564 57173 6 r/s T=0.77707 89775 87 s f=1.28687 04840 1 Hz ]
'ROOT(ⒺSimple Pendulum;[ω;T;f];[1_(r/s);1_s;1_Hz])'
```

#### Conical Pendulum

![Conical Pendulum](img/ConicalPendulum.bmp)

* To calculate `[θ_°;ω_r/s;T_s;f_Hz]` (Cone angle; Angular frequency; Oscillation period; Frequency) from 2 known variables:
```rpl
L=25_cm  h=20_cm
@ Expecting [ θ=36.86989 76458 ° ω=7.00237 45972 3 r/s T=0.89729 35137 83 s f=1.11446 25305 3 Hz ]
'ROOT(ⒺConical Pendulum;[θ;ω;T;f];[1_°;1_r/s;1_s;1_Hz])'
```

#### Torsional Pendulum

![Torsional Pendulum](img/TorsionalPendulum.bmp)

* To calculate `[ω_r/s;T_s;f_Hz]` (Angular frequency; Oscillation period; Frequency) from 4 known variables:
```rpl
G=1000_kPa  J=17_mm^4  L=26_cm  I=50_kg*m^2
@ Expecting [ ω=1.14354 37497 9⁳⁻³ r/s T=5 494.48616 051 s f=1.82000 64042 2⁳⁻⁴ Hz ]
'ROOT(ⒺTorsional Pendulum;[ω;T;f];[1_r/s;1_s;1_Hz])'
```

#### Simple Harmonic

* To calculate `[x_cm;v_cm/s;a_m/s^2;m_kg;E_J]` (Displacement; Velocity & Acceleration at `t`; Mass; Total energy) from 5 known variables:
```rpl
xm=10_cm  ω₀=15_r/s  φ=25_°  t=25_μs  k=10_N/m
@ Expecting [ x=9.06149 24146 7 cm v=-63.44371 46156 cm/s a=-20.38835 7933 m/s↑2 m=4.44444 44444 4⁳⁻² kg E=0.05 J ]
'ROOT(ⒺSimple Harmonic;[x;v;a;m;E];[1_cm;1_cm/s;1_m/s^2;1_kg;1_J])'
```

#### Underdamped Oscillations

We are considering here a damped mass-spring oscillator having the natural angular frequency `ω₀`. The corresponding differential equation : `−k*x − b*dx/dt = m*d^2x/dt^2` describes the underdamped oscillations.

* To calculate `[m_kg;γ_(r/s);ωu_(r/s);x_cm;v_cm/s;a_m/s^2;E_J;Q]` (Mass; Reduced damping coefficient; Underdamped angular frequency; Displacement; Velocity & Acceleration at `t`; Mass; Total energy at `t`; Quality factor) from 6 known variables:
```rpl
xm=10_cm  ω₀=15_r/s  φ=25_°  t=25_μs  k=10_N/m  b=0.2_(kg/s)
@ Expecting [ m=4.44444 44444 4⁳⁻² kg γ=4.5 r/s ωu=14.83028 995 r/s x=9.06100 06640 3 cm v=-83.10906 53488 cm/s a=-16.64734 35534 m/s↑2 E=0.05640 00148 35 J Q=3.33333 33333 3 ]
'ROOT(ⒺUnderdamped Oscillations;[m;γ;ωu;x;v;a;E;Q];[1_kg;1_(r/s);1_(r/s);1_cm;1_cm/s;1_m/s^2;1_J;1])'
@ Save E for later use
```

The code below saves the reference value for comparison with the example in [Driven Damped Oscillations](#Driven Damped Oscillations):
```rpl
@ Save the reference value for comparison below
E₀=E
@ Save E₀ for later
```

#### Driven Damped Oscillations

We are considering here a damped mass-spring oscillator where the external driving force is of the form `Fdriving = Fd*cos(ω*t)` acting at the angular frequency `ω`. The corresponding differential equation : `−k*x − b*dx/dt + Fd*cos(ω*t) = m*d^2x/dt^2` describes the driven damped oscillations. When the driving frequency `ω` comes close to the natural frequency `ω₀` this is the onset of resonance with amplitude increase and the total energy accumulates up to a possible catastrophy when the structure is overcome (see fig)

![Driven Damped Oscillations](img/Driven Damped Oscillations2_BW.bmp)

* To calculate `[m_kg;γ_(r/s);ωu_(r/s);φ_°;xp_m;x_cm;v_cm/s;a_m/s^2;E_J;Q]` (Mass; Reduced damping coefficient; Underdamped angular frequency; Phase angle; Resulting amplitude; Displacement; Velocity & Acceleration at `t`; Total energy at `t`; Quality factor) from 9 known variables which correspond to the values of the previous section:
```rpl
ω=14.99_r/s  ω₀=15_r/s  θ=25_°  t=500_s  k=10_N/m  b=0.2_(kg/s)  xh=10_cm  Fd=0.9_N
@ Expecting [ m=4.44444 44444 4⁳⁻² kg γ=4.5 r/s ωu=14.83028 995 r/s φ=-89.74526 88301 ° xp=0.30019 71665 48 m x=-22.26611 11734 cm v=301.81823 6224 cm/s a=50.03197 40728 m/s↑2 E=0.45032 15149 87 J Q=3.33333 33333 3 ]
'ROOT(ⒺDriven Damped Oscillations;[m;γ;ωu;φ;xp;x;v;a;E;Q];[1_kg;1_(r/s);1_(r/s);1_°;1_m;1_cm;1_cm/s;1_m/s^2;1_J;1])'
@ Save E for comparison
```

Verify relative difference with the total energy of the case [Underdamped Oscillations](#Underdamped Oscillations)
```rpl
E₀ E %Ch
@ Expecting 698.44219 2445
@ % of relative augmentation which illustrates the huge energy gain due to the driving force acting very near the resonance frequency.
```


## Plane Geometry

The 21 variables in the Plane Geometry section are:

* `β`: Central angle of polygon
* `θ`: Vertex angle of polygon
* `A`: Area
* `b`: Base length ([Rectangle](#Rectangle)) & ([Triangle](#Triangle)), or Length of semiaxis in x direction ([Ellipse](#Ellipse))
* `C`: Circumference
* `d`: Distance to rotation axis in y direction
* `h`: Height (Rectangle, Triangle), or Length of semiaxis in y direction (Ellipse)
* `I`: Moment of inertia about x axis (dim.: mass·area, in SI: kg·m^2)
* `Ix`: Moment of inertia about x axis (dim.: mass·area, in SI: kg·m^2)
* `Id`: Moment of inertia in x direction at distance `d` (dim.: mass·area, in SI: kg·m^2)
* `Iy`: Moment of inertia about y axis (dim.: mass·area, in SI: kg·m^2)
* `J`: Polar moment of inertia at centroid (dim.: mass·area, in SI: kg·m^2)
* `L`: Side length of polygon
* `n`: Number of sides
* `P`: Perimeter
* `r`: Radius
* `ri`: Inside radius
* `ro`: Outside radius
* `rs`: Distance to side of polygon
* `rv`: Distance to vertex of polygon
* `v`: Horizontal distance to vertex

#### Circle

![Circle](img/Circle.bmp)

* To calculate `[C_cm;A_cm^2;I_mm^4;J_mm^4;Id_mm^4]` (Circonference; Area; Moment of inertia about `x` axis; Polar moment of inertia at centroid; Moment of inertia in `x` direction at distance `d`) from 2 known variables:
```rpl
r=5_cm  d=1.5_cm
@ Expecting [ C=31.41592 65359 cm A=78.53981 63397 cm↑2 I=4 908 738.52123 mm↑4 J=9 817 477.04247 mm↑4 Id=6 675 884.38888 mm↑4 ]
'ROOT(ⒺCircle;[C;A;I;J;Id];[1_cm;1_cm^2;1_mm^4;1_mm^4;1_mm^4])'
```

#### Ellipse

![Ellipse](img/Ellipse.bmp)

* To calculate `[C_cm;A_cm^2;I_mm^4;J_mm^4;Id_mm^4]` (Circonference; Area; Moment of inertia about `x` axis; Polar moment of inertia at centroid; Moment of inertia in `x` direction at distance `d`) from 3 known variables:
```rpl
b=17.85_μm  h=78.9725_μin  d=.00000012_ft
@ Expecting [ C=7.98046 33593 6⁳⁻³ cm A=1.12485 79868⁳⁻⁶ cm↑2 I=1.13150 61302 6⁳⁻¹⁰ mm↑4 J=9.07327 72104 7⁳⁻⁹ mm↑4 Id=1.13301 09695 2⁳⁻¹⁰ mm↑4 ]
'ROOT(ⒺEllipse;[C;A;I;J;Id];[1_cm;1_cm^2;1_mm^4;1_mm^4;1_mm^4])'
```

#### Rectangle

![Rectangle](img/Rectangle.bmp)

* To calculate `[C_cm;A_cm^2;I_km^4;J_km^4;Id_km^4]` (Perimeter; Area; Moment of inertia about `x` axis; Polar moment of inertia at centroid; Moment of inertia in `x` direction at distance `d`) from 3 known variables:
```rpl
b=4_chain  h=7_rd  d=39.26_in
@ Expecting [ P=23 134.32 cm A=28 327 994.9568 cm↑2 I=2.92569 11916 2⁳⁻⁷ km↑4 J=1.82109 34968 2⁳⁻⁶ km↑4 Id=2.95386 09978 8⁳⁻⁷ km↑4 ]
'ROOT(ⒺRectangle;[P;A;I;J;Id];[1_cm;1_cm^2;1_km^4;1_km^4;1_km^4])'
```

#### Regular Polygon

![Regular Polygon](img/RegularPolygon.bmp)

* To calculate `[P_cm;A_cm^2;rs_cm;rv_cm;θ_°;β_°]` (Perimeter; Area; Distance to side; Distance to vertex of polygon; Vertex and central angles of polygon) from 2 known variables:
```rpl
n=8  L=0.5_yd
@ Expecting [ P=365.76 cm A=10 092.95006 19 cm↑2 rs=55.18892 20358 cm rv=59.73605 87541 cm θ=135. ° β=45 ° ]
'ROOT(ⒺRegular Polygon;[P;A;rs;rv;θ;β];[1_cm;1_cm^2;1_cm;1_cm;1_°;1_°])'
```

#### Circular Ring

![Circular Ring](img/CircularRing.bmp)

* To calculate `[A_cm^2;I_mm^4;J_mm^4;Id_mm^4]` (Area; Moment of inertia about `x` axis; Polar moment of inertia at centroid; Moment of inertia in `x` direction at distance `d`) from 3 known variables:
```rpl
ro=4_μm  ri=25_Å  d=0.1_mil
@ Expecting [ A=5.02654 62822 5⁳⁻⁷ cm↑2 I=2.01061 92983⁳⁻¹⁰ mm↑4 J=4.02123 85965 9⁳⁻¹⁰ mm↑4 Id=5.25354 58977 5⁳⁻¹⁰ mm↑4 ]
@ Note these answers are OK, the manual HP50g_AUR (p.5-46) is definitively in error (as calculated  directly)
'ROOT(ⒺCircular Ring;[A;I;J;Id];[1_cm^2;1_mm^4;1_mm^4;1_mm^4])'
```

#### Triangle

![Triangle](img/Triangle.bmp)

* To calculate `[b_in;A_in^2;Ix_in^4;Iy_in^4;J_in^4;Id_in^4]` (Base length; Area; Moment of inertia about `x` and `y` axis; Polar moment of inertia at centroid; Moment of inertia in `x` direction at distance `d`) from 4 known variables:
```rpl
h=4.33012781892_in  v=2.5_in  P=15_in  d=2_in
@ Expecting [ b=4.99999 90762 4 in A=10.82531 75473 in↑2 Ix=11.27637 66118 in↑4 Iy=11.27636 82785 in↑4 J=22.55274 48902 in↑4 Id=54.57764 6801 in↑4 ]
'ROOT(ⒺTriangle;[b;A;Ix;Iy;J;Id];[1_in;1_in^2;1_in^4;1_in^4;1_in^4;1_in^4])'
```


## Solid geometry
The 12 variables in the Solid Geometry section are:

* `A`: Total surface area
* `b`: Base length
* `d`: Distance to rotation axis in z direction
* `h`: Height in z direction ([Cone](#Cone)) & ([Cylinder](#Cylinder)), or Height in y direction ([Parallelepiped](#Parallelepiped))
* `I`: Moment of inertia about x axis (dim.: mass·area, in SI: kg·m^2)
* `Ixx`: Moment of inertia about x axis (dim.: mass·area, in SI: kg·m^2)
* `Id`: Moment of inertia in x direction at distance `d` (dim.: mass·area, in SI: kg·m^2)
* `Izz`: Moment of inertia about z axis (dim.: mass·area, in SI: kg·m^2)
* `m`: Mass
* `r`: Radius
* `t`: Thickness in z direction (dim.: length)
* `V`: Volume

#### Cone

![Cone](img/Cone.bmp)

* To calculate `[V_cm^3;A_cm^2;Ixx_kg*m^2;Izz_kg*m^2;Id_kg*m^2]` (Volume; Area; Moment of inertia about `x` axis & `z` axis; Moment of inertia in `x` direction at distance `d`) from 4 known variables:
```rpl
r=7_cm  h=12.5_cm  m=12.25_kg  d=3.5_cm
@ Expecting [ V=641.40850 0108 cm↑3 A=468.99530 2857 cm↑2 Ixx=0.01618 14843 75 kg·m↑2 Izz=0.01800 75 kg·m↑2 Id=0.03118 77343 75 kg·m↑2 ]
'ROOT(ⒺCone;[V;A;Ixx;Izz;Id];[1_cm^3;1_cm^2;1_kg*m^2;1_kg*m^2;1_kg*m^2])'
```

#### Cylinder

![Cylinder](img/Cylinder.bmp)

* To calculate `[V_in^3;A_in^2;Ixx_lb*in^2;Izz_lb*in^2;Id_lb*in^2]` (Volume; Area; Moment of inertia about `x` axis & `z` axis; Moment of inertia in `x` direction at distance `d`) from 4 known variables:
```rpl
r=8.5_in  h=65_in  m=12000_lb  d=2.5_in
@ Expecting [ V=14 753.70449 94 in↑3 A=3 925.42002 066 in↑2 Ixx=4 441 750. lb·in↑2 Izz=433 500. lb·in↑2 Id=4 516 750. lb·in↑2 ]
'ROOT(ⒺCylinder;[V;A;Ixx;Izz;Id];[1_in^3;1_in^2;1_lb*in^2;1_lb*in^2;1_lb*in^2])'
```

#### Parallelepiped

![Parallelepiped](img/Parallelepiped.bmp)

* To calculate `[V_in^3;A_in^2;I_lb*in^2;Id_lb*in^2]` (Volume; Area; Moment of inertia about `x` axis; Moment of inertia in `x` direction at distance `d`) from 4 known variables:
```rpl
b=36_in  h=12_in  t=72_in  m=83_lb  d=7_in
@ Expecting [ V=31 104 in↑3 A=7 776 in↑2 I=36 852. lb·in↑2 Id=40 919. lb·in↑2 ]
'ROOT(ⒺParallelepiped;[V;A;I;Id];[1_in^3;1_in^2;1_lb*in^2;1_lb*in^2])'
```

#### Sphere

![Sphere](img/Sphere.bmp)

* To calculate `[I_kg*m^2;r_cm;V_cm^3;A_cm^2]` (Moment of inertia about `x` axis; Radius; Volume; Area) from 3 known variables:
```rpl
d=14_cm  m=3.75_kg  Id=486.5_lb*in^2
@ Expecting [ I=0.06886 91913 93 kg·m↑2 r=21.42727 10027 cm V=41 208.72679 31 cm↑3 A=5 769.57188 64 cm↑2 ]
'ROOT(ⒺSphere;[I;r;V;A];[1_kg*m^2;1_cm;1_cm^3;1_cm^2])'
```


## Solid State Devices
The 54 variables in the Solid State Devices section are:

* `αF`: Forward common-base current gain
* `αR`: Reverse common-base current gain
* `γ`: Body factor (dim.: (energy/charge)^0.5, in SI: V^0.5)
* `λ`: Modulation parameter (dim.: charge/energy, in SI: V^-1)
* `μn`: Electron mobility (dim.: speed/(electric field), in SI: m^2/(V·s))
* `φp`: Fermi potential (dim.: energy/charge, in SI: volt, V)
* `ΔL`: Length adjustment ([PN Step Junctions](#PN Step Junctions)), or Channel encroachment ([NMOS Transistors](#NMOS Transistors)) (dim.: length)
* `ΔW`: Width adjustment ([PN Step Junctions](#PN Step Junctions)), or Width contraction ([NMOS Transistors](#NMOS Transistors))
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
* `L`: Drawn mask length ([PN Step Junctions](#PN Step Junctions)), or Drawn gate length ([NMOS Transistors](#NMOS Transistors)), or Channel length ([JFETs](#JFETs))
* `Le`: Effectives gate length
* `NA`: P-side doping ([PN Step Junctions](#PN Step Junctions)), or Substrate doping ([NMOS Transistors](#NMOS Transistors)) (dim.: 1/volume, in SI: m^-3)
* `ND`: N-side doping ([PN Step Junctions](#PN Step Junctions)), or N-channel doping ([JFETs](#JFETs)) (dim.: 1/volume, in SI: m^-3)
* `ni`: Silicon density (dim.: 1/volume, in SI: m^-3)
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
* `W`: Drawn mask width ([PN Step Junctions](#PN Step Junctions)), or Drawn width ([NMOS Transistors](#NMOS Transistors)), or Channel width ([JFETs](#JFETs))
* `We`: Effective width
* `xd`: Depletion-region width
* `xdmax`: Depletion-layer width
* `xj`: Junction depth

#### PN Step Junctions

These equations for a silicon PN-junction diode use a “two-sided step-junction” model–the doping density changes abruptly at the junction. The equation assume the current density is determined by minority carries injected across the depletion region and the PN junction is rectangular in its layout, The temperature should be between 77 and 500 K.

![PN Step Junctions](img/PNStepJunctions.bmp)

* To calculate `[ni_m^-3;Vbi_V;xd_μ;Cj_pF/cm^2;Emax_V/cm;BV_V;J_A/cm^2;Aj_cm^2;I_mA]` (Silicon density; Built-in voltage; Depletion-region width; Junction capacitance per unit area; Maximum electric field; Breakdown voltage; Current density; Effective junction area; Diode current) from 11 known variables:
```rpl
ND=1E22_cm^-3  NA=1E15_cm^-3  T=300_K  Js=1e-6_μA/cm^2  Va=-20_V  E1=3.3E5_V/cm  W=10_μ  ΔW=1_μ  L=10_μ  ΔL=1_μ  xj=2_μ
@ Expecting [ ni=9.64987 39813 5⁳⁹ (cm↑3)⁻¹ Vbi=1.01379 20414 3 V xd=5.25726 51776 7 μ Cj=2 004.17577 358 pF/cm↑2 Emax=79 941.91402 29 V/cm BV=358.08260 5881 V J=-1.⁳⁻¹² A/cm↑2 Aj=2.57097 33552 9⁳⁻⁶ cm↑2 I=-2.57097 33552 9⁳⁻¹⁵ mA ]
'ROOT(ⒺPN Step Junctions;[ni;Vbi;xd;Cj;Emax;BV;J;Aj;I];[1_cm^-3;1_V;1_μ;1_pF/cm^2;1_V/cm;1_V;1_A/cm^2;1_cm^2;1_mA])'
```

#### NMOS Transistor

These equations for a silicon NMOS transistor use a two-port network model. They include linear and nonlinear regions in the device characteristics and are based on a gradual-channel approximation (the electric fields in the direction of current flow are small compared to those perpendicular to the flow). The drain current and transconductance calculations differ depending on whether the transistor is in the linear, saturated, or cutoff region. The equations assume the physical geometry of the device is a rectangle, second-order length-parameter effects are negligible, shot-channel, hot-carrier, and velocity-saturation effects are negligible, and subthreshold currents are negligible.

![NMOS Transistor](img/NMOSTransistor.bmp)

* To calculate `[ni_(cm^-3);We_μ;Le_μ;Cox_pF/cm^2;γ_V^.5;φp_V;Vt_V;VDsat_V;IDS_mA;gds_S;gm_mA/V]` (Silicon density; Effective width; Effectives gate length; Silicon dioxide capacitance per unit area; Body factor; Fermi potential; Threshold voltage; Saturation voltage; Drain current; Output conductance; Transconductance) from 13 known variables:
```rpl
tox=700_Å  NA=1e15_1/cm^3  μn=600_(cm^2)/(V*s)  T=26.85_°C  Vt0=0.75_V  VGS=5_V  VBS=0_V  VDS=5_V  W=25_μ  ΔW=1_μ  L=4_μ  ΔL=0.75_μ  λ=0.05_1/V
@ Expecting [ ni=9.64987 39813 5⁳⁹ (cm↑3)⁻¹ We=23 μ Le=2.5 μ Cox=49 330.47499 05 pF/cm↑2 γ=0.37247 98153 1 V↑(¹/₂) φp=-0.29855 35180 52 V Vt=0.75 V VDsat=4.25 V IDS=2.97832 74275 5 mA gds=1.48916 37137 7⁳⁻⁴ S gm=1.42391 28597 5 mA/V ]
'ROOT(ⒺNMOS Transistor;[ni;We;Le;Cox;γ;φp;Vt;VDsat;IDS;gds;gm];[1_cm^-3;1_μ;1_μ;1_pF/cm^2;1_V^(1/2);1_V;1_V;1_V;1_mA;1_S;1_mA/V])'
```

#### Bipolar Transistors

These equations for an NPN silicon bipolar transistor are based on large-signal models developed by J.J. Ebers and J.L. Moll. The offset-voltage calculation differs depending on whether the transistor is saturated or not. The equations also include the special conditions when the emitter-base or collector-base junction is open, which are convenient for measuring transistor parameters.

![Bipolar Transistors](img/BipolarTransistors.bmp)

* To calculate `[VBE_V;IS_nA;ICO_nA;ICEO_nA;IE_mA;IC_mA;IB_mA;VCEsat_V]` (Base-to-emitter voltage; Transistor saturation current; Collector current (emitter-to-base open); Collector current (collector-to-base open); Total emitter current; Total collector current; Total base current; Collector-to-emitter saturation voltage) from 7 known variables:
```rpl
IES=1e-5_nA  ICS=2e-5_nA  T=26.85_°C  αF=0.98  αR=0.49  IC=1_mA  VBC=-10_V
@ Expecting [ VBE=0.63739 37623 73 V IS=0.00000 98 nA ICO=0.00001 0396 nA ICEO=0.00051 98 nA IE=-0.51020 40835 27 mA IB=-0.48979 59164 73 mA VCEsat=-5.95264 29332 3 V ]
'ROOT(ⒺBipolar Transistors;[VBE;IS;ICO;ICEO;IE;IB;VCEsat];[1_V;1_nA;1_nA;1_nA;1_mA;1_mA;1_V])'
```

#### JFETs

These equations for a silicon N-channel junction field-effect transistor (JFET) are based on the single-sided stepjunction
approximation, which assumes the gates are heavily doped compared to the channel doping,. The draincurrent
calculation differs depending on whether the gate-junction depletion-layer thickness is less than or greater
than the channel thickness. The equations assume the channel is uniformly doped and end effects (such as contact,
drain, and source resistances) are negligible.

![JFETs](img/JFETs.bmp)

* To calculate `[ni_(cm^-3);Vbi_V;xdmax_μ;G0_S;ID_mA;VDsat_V;Vt_V;gm_mA/V]` (Silicon density; Built-in voltage; Depletion-layer width; Channel conductance; Drain current; Saturation voltage; Threshold voltage; Transconductance) from 8 known variables:
```rpl
ND=1e16_1/cm^3  W=6_μ  a=1_μ  L=2_μ  μn=1248_cm^2/(V*s)  VGS=-4_V  VDS=4_V  T=26.85_°C
@ Expecting [ ni=9.64987 39813 5⁳⁹ (cm↑3)⁻¹ Vbi=0.35807 99473 84 V xdmax=1.04848 18266 6 μ G0=5.99854 93177⁳⁻⁴ S ID=0.21443 71687 23 mA VDsat=3.24491 50809 6 V Vt=-7.24491 50809 6 V gm=0.14570 26745 73 mA/V ]
'ROOT(ⒺJFETs;[ni;Vbi;xdmax;G0;ID;VDsat;Vt;gm];[1_(cm^-3);1_V;1_μ;1_S;1_mA;1_V;1_V;1_mA/V])'
```


## Stress Analysis
The 28 variables in the Stress Analysis section are:

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
* `θp1`: Angle 1 to plane of maximum principal normal stress
* `θp2`: Angle 2 to plane of minimum principal normal stress
* `θs`: Angle to plane of maximum shear stress
* `A`: Area
* `E`: Modulus of elasticity (dim.: pressure, in SI: pascal, Pa)
* `G`: Shear modulus of elasticity (dim.: pressure, in SI: pascal, Pa)
* `J`: Polar moment of inertia (dim.: length^4, in SI: m^4)
* `L`: Length
* `P`: Load (dim.: force, in SI: newton, N)
* `r`: Radius
* `T`: Torque (dim.: force·length, in SI: N·m)

#### Normal Stress

![Normal Stress](img/NormalStress.bmp)

* To calculate `[δ_in;ε;σ_psi]` (Elongation; Normal strain; Normal stress) from 4 known variables:
```rpl
P=40000_lbf  L=1_ft  A=3.14159265359_in^2  E=10e6_psi
@ Expecting [ σ=12 732.39544 73 psi δ=0.01527 88745 37 in ε=1.27323 95447 3⁳⁻³ ]
'ROOT(ⒺNormal Stress;[σ;δ;ε];[1_psi;1_in;1])'
```

#### Shear Stress

![Shear Stress](img/ShearStress.bmp)

* To calculate `[T_ft*lbf;γ_°;φ_°]` (Torque; Shear strain; Angle of twist) from 5 known variables:
```rpl
L=6_ft  r=2_in  J=10.4003897419_in^4  G=12000000_psi  τ=12000_psi
@ Expecting [ T=5 200.19487 095 ft·lbf γ=0.05729 57795 13 ° φ=2.06264 80624 7 ° ]
'ROOT(ⒺShear Stress;[T;γ;φ];[1_ft*lbf;1_°;1_°])'
```

#### Stress On An Element

Stresses and strains are positive in the directions shown in the picture.

![Stress On An Element](img/StressOnAnElement.bmp)

* To calculate `[σx1_kPa;σy1_kPa;τx1y1_kPa]` (Normal stress in rotated-`x` direction; Normal stress in rotated-`y` direction; Rotated shear stress) from 4 known variables:
```rpl
σx=15000_kPa  σy=4755_kPa  τxy=7500_kPa  θ=30_°
@ Expecting [ σx1=18 933.94052 84 kPa σy1=821.05947 1617 kPa τx1y1=-686.21513 0886 kPa ]
'ROOT(ⒺStress On An Element;[σx1;σy1;τx1y1];[1_kPa;1_kPa;1_kPa])'
```

#### Mohr’s Circle

![Mohr’s Circle](img/Mohr’sCircle.bmp) ```![Mohr’s Circle](img/Mohr’sCircle.bmp)```

* To calculate `[σ1_psi;σ2_psi;θp1_°;θp2_°;τmax_psi;θs_°;σavg_psi]` (Maximum principal normal stress; Minimum principal normal stress; Angle to plane of maximum principal normal stress; Angle to plane of minimum principal normal stress; Maximum shear stress; Angle to plane of maximum shear stress; Normal stress on place of maximum shear stress) from 3 known variables:
```rpl
σx=-5600_psi  σy=-18400_psi  τxy=4800_psi
@ Expecting [ σ1=-1 755.00122 011 psi σ2=-22 244.99877 99 psi θp1=18.43494 88229 ° θp2=108.43494 8823 ° τmax=10 244.99877 99 psi θs=-26.56505 11771 ° σavg=-12 000 psi ]
'ROOT(ⒺMohr’s Circle;[σ1;σ2;θp1;θp2;τmax;θs;σavg];[1_psi;1_psi;1_°;1_°;1_psi;1_°;1_psi])'
```


## Waves
The 39 variables in the Waves section are:

* `β`: Sound intensity level (dim.: intensity in logarithmic scale, in SI: decibel, dB)
* `Δp`: Sound pressure variafion around atmospheric pressure (dim.: force/area, in SI: pascal, Pa)
* `Δpm`: Amplitude of sound pressure variafion around atmospheric pressure (dim.: force/area, in SI: pascal, Pa)
* `λ`: Wavelength
* `ω`: Angular frequency (dim.: angle/time. in SI: r/s^2)
* `ρ`: Volumic density of medium (dim.: mass/volume, in SI: kg/m^3)
* `θcone`: Angle at the tip of the cone formed by the supersonic shockwave
* `μ`: Linear density of a string (dim.: mass/length, in SI: kg/m)
* `as`: Acceleration at `x` and `t` of vibrating particles ([Longitudinal Waves](#Longitudinal Waves)), or Acceleration at `x` and `t` of air particles ([Sound Waves](#Sound Waves))
* `ay`: Acceleration at `x` and `t` of vibrating particles for transversal waves
* `B`: Bulk modulus of elasticity ([Sound Waves](#Sound Waves)) (dim.: pressure, in SI: Pa), or Magnetic field ([Electromagnetic Waves](#Electromagnetic Waves)) (dim.: mass/(time^2·current), in SI: T)
* `E`: Electric field at `x` and `t` of an electromagnetic wave (dim.: force/charge, in SI: N/C=V/m)
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
* `M`: Mach number
* `ninteger`: Harmonics number being an integer number
* `nodd`: Harmonics number being an odd number
* `Ps`: Power of the source (dim.: energy/time, in SI: watt, W)
* `s`: Longitudinal displacement at `x` and `t` of vibrating particles ([Longitudinal Waves](#Longitudinal Waves)), or Longitudinal displacement at `x` and `t` of air particles ([Sound Waves](#Sound Waves))
* `sm`: Longitudinal displacement amplitude of vibrating particles ([Longitudinal Waves](#Longitudinal Waves)), or Longitudinal displacement amplitude of air particles ([Sound Waves](#Sound Waves))
* `t`: Time
* `T`: Tension ([String Standing Waves](#String Standing Waves))
* `Tair`: Air temperature ([Doppler Effect](#Doppler Effect)), ([Mach Number](#Mach Number)) & ([Sound Wave Harmonics](#Sound Wave Harmonics))
* `u`: Mass or flow velocity ([Mach Number](#Mach Number))
* `v`: Velocity of the propagating sound in medium ([Sound Waves](#Sound Waves)), or Wave speed ([Transverse Waves](#Transverse Waves)) & ([Longitudinal Waves](#Longitudinal Waves))
* `vr`: Speed of the receiver relative to the medium
* `vs`: Velocity at `x` and `t` of vibrating particles ([Longitudinal Waves](#Longitudinal Waves)), or Velocity at `x` and `t` of air particles ([Sound Waves](#Sound Waves)), or Speed of the source relative to the medium ([Doppler Effect](#Doppler Effect))
* `vsair`: Velocity of the propagating sound in the ait as a function of temperature
* `vy`: Velocity at `x` and `t` of vibrating particles for transversal waves
* `x`: Position
* `y`: Transverse displacement at `x` and `t`

#### Transverse Waves

* To calculate `[f_Hz;λ_cm;v_cm/s;y_cm;vy_cm/s;ay_cm/s^2]` (Frequency; Wavelength; Transverse displacement at `x` and `t`; Wave speed; Velocity & Acceleration at `x` and `t` of vibrating particles) from 5 known variables:
```rpl
ym=6.37_cm  k=32.11_r/cm  x=0.03_cm  ω=7000_r/s  t=1_s φ=0_r
@ Expecting [ f=1 114.08460 164 Hz λ=0.19567 69015 cm v=218.00062 2859 cm/s y=2.66549 26430 5 cm vy=-40 498.52342 4 cm/s ay=-312 130 000. cm/s↑2 ]
'ROOT(ⒺTransverse Waves;[f;λ;v;y;vy;ay];[1_Hz;11_cm;1_cm/s;1_cm;1_cm/s;1_cm/s^2])'
```

#### Longitudinal Waves

* To calculate `[s_cm;λ_cm;f_Hz;v_m/s;vs_cm/s;as_cm/s^2]` (Frequency; Wavelength; Transverse displacement at `x` and `t`; Wave speed; Velocity & Acceleration at `x` and `t` of vibrating particles) from 5 known variables:
```rpl
sm=6.37_cm  k=32.11_r/cm  x=0.03_cm  ω=7000_r/s  t=1_s φ=0_r
@ Expecting [ s=5.78550 33462 8 cm λ=0.19567 69015 cm f=1 114.08460 164 Hz v=2.18000 62285 9 m/s vs=18 658.44850 13 cm/s as=-283 489 663.968 cm/s↑2 ]
'ROOT(ⒺLongitudinal Waves;[s;λ;f;v;vs;as];[1_cm;1_cm;1_Hz;1_m/s;1_cm/s;1_cm/s^2])'
```

#### Sound Waves

* **Example 1**. To calculate for an unknomn substance `[v_m/s;f_Hz;I_W/m^2;s_cm;vs_cm/s;as_cm/s^2;Δpm_Pa;Δp_Pa;Ps_W;β_dB]` (Wave speed; Frequency; Sound intensity; Longitudinal displacement, Velocity & Acceleration at `x` and `t` of vibrating particles; Amplitude of sound pressure variafion around atmospheric pressure; Sound pressure variafion; Power of the source; Sound level in dB) from 9 known variables:
```rpl
sm=10_cm  ω=6000_r/s  B=12500_kPa  ρ=65_kg/m^3   x=2_cm   t=0_s  r=10_m   k=13.6821_r/m  φ=2_r
@ Expecting [ v=438.52900 9654 m/s f=954.92965 8551 Hz I=5.13078 94129 5⁳⁹ W/m↑2 s=7.63005 85995 9 cm vs=-38 783.49400 98 cm/s as=-274 682 109.585 cm/s↑2 Δpm=17 102 631.3765 Pa Δp=11 054 996.6924 Pa Ps=6.44754 01307 3⁳¹² W β=217.10184 1899 dB ]
'ROOT(ⒺSound Waves;[v;f;I;s;vs;as;Δpm;Δp;Ps;β];[1_m/s;1_Hz;1_(W/(m^2));1_cm;1_cm/s;1_cm/s^2;1_Pa;1_Pa;1_W;1_dB])'
```
* **Example 2**. For a sound wave propagating in air at a pressure of 1 atm and temperature of 20°C, to calculate `[v_m/s;f_Hz;I_W/m^2;s_cm;vs_cm/s;as_cm/s^2;Δpm_Pa;Δp_Pa;Ps_W;β_dB]` (Wave speed; Frequency; Sound intensity; Sound intensity; Longitudinal displacement, Velocity & Acceleration at `x` and `t` of vibrating particles; Amplitude of sound pressure variafion around atmospheric pressure; Sound pressure variafion; Power of the source; Sound level in dB) from 9 known variables:
```rpl
sm=1e-5_m  ω=2 513.27412 287_r/s  B=141_kPa  ρ=1.29_kg/m^3   x=2_cm   t=0_s  r=5_m   k=7.59297 31808 8_r/m  φ=2_r
@ Expecting [ v=330.60902 2232 m/s f=400. Hz I=0.13469 58251 68 W/m↑2 s=8.35879 55288 8⁳⁻⁴ cm vs=-1.37956 85902 cm/s as=-5 279.87232 893 cm/s↑2 Δpm=10.71875 31947 Pa Δp=5.88366 19129 6 Pa Ps=42.31594 14818 W β=111.29354 1352 dB ]
'ROOT(ⒺSound Waves;[v;f;I;s;vs;as;Δpm;Δp;Ps;β];[1_m/s;1_Hz;1_(W/(m^2));1_cm;1_cm/s;1_cm/s^2;1_Pa;1_Pa;1_W;1_dB])'
```
#### Doppler Effect

In the classical Doppler effect it is assumed that the speed of the observer and the source are lower than the speed of sound in the air. The speed of the receiver relative to the air `vr` is added to the speed of sound in the air if the receiver is moving towards the source, subtracted if the receiver is moving away from the source whatever the movement of the source. The speed of the source relative to the air `vs` is subtracted from the speed of sound in the air if the source is moving towards the receiver, added if the source is moving away from the receiver whatever the movement of the receiver.

* **Example 1**. A police car with a siren frequency of 1200 Hz is driving at 180 km/h in the same direction as a truck moving at 90 km/h. To calculate the frequency heard by the trucker when the police are behind him: `[vsair_(m/s);f_Hz]` (Propagation speed of sound waves; Frequency) from 4 known variables:
```rpl
Tair=20_°C  f0=1200_Hz  vr=-90_km/h  vs=180_km/h
@ Expecting [ vsair=343.23616 5602 m/s f=1 302.30661 671 Hz ]
'ROOT(ⒺDoppler Effect;[vsair;f];[1_(m/s);1_Hz])'
```

* **Example 2**. A police car with a siren frequency of 1200 Hz is driving at 180 km/h in the same direction as a truck moving at 90 km/h. To calculate the frequency heard by the trucker when the police are in front of him: `[vsair_(m/s);f_Hz]` (Propagation speed of sound waves; Frequency) from 4 known variables:
```rpl
Tair=20_°C  f0=1200_Hz  vr=90_km/h  vs=-180_km/h
@ Expecting [ vsair=343.23616 5602 m/s f=1 123.70996 713 Hz ]
'ROOT(ⒺDoppler Effect;[vsair;f];[1_(m/s);1_Hz])'
```

#### Mach Number

For an object moving at a supersonic speed, the shockwave describes a cone having the angle `θcone` at its tip where the opposite side is the distance travelled by the sound and the hypothenuse is the distance travelled by the object. The Mach number `M` is the speed ratio to the speed of sound.

* To calculate `[vsair_(m/s);M;θcone_°]` (Propagation speed of sound waves; Mach number; Angle at the tip of the cone formed by the supersonic shockwave) from 2 known variables:
```rpl
Tair=-20_°C  u=2200_km/h
@ Expecting [ vsair=318.96061 3718 m/s M=1.91594 53701 4 θcone=31.46217 41236 ° ]
'ROOT(ⒺMach Number;[vsair;M;θcone];[1_(m/s);1;1_°])'
```

#### String Standing Waves

A string being fixed or free at its ends admits only discrete harmonics as standing waves on the string. A string being fixed (or free) at both ends admits all integer harmonics. A string being being fixed at one end and free at the other end admits only all odd integer harmonics.

* To calculate `[v_m/s;k_(r/m);ω_(r/s);Ts_N;y_m;ffixedfixed_Hz;ffixedfree_Hz]` (Propagation speed of waves, Wave number; Angular frequency; Tension; Frequency of harmonics on a string fixed at both ends; Frequency of harmonics on a string fixed at one end and free at the other end) from 9 known variables:
```rpl
λ=1.2_m  f=112_Hz  μ=1.8_(g/m)  L=0.6_m  ninteger=2  nodd=3  x=10_cm  t=5_s  ym=2_cm
@ Expecting [ v=134.4 m/s k=5.23598 77559 8 r/m ω=703.71675 4404 r/s Ts=32.51404 8 N y=1.⁳⁻² m ffixedfixed=224. Hz ffixedfree=168. Hz ]
'ROOT(ⒺString Standing Waves;[v;k;ω;Ts;y;ffixedfixed;ffixedfree];[1_m/s;1_(r/m);1_(r/s);1_N;1_m;1_Hz;1_Hz])'
```

#### Sound Wave Harmonics

A tube being open or closed at its ends admits only discrete harmonics as standing waves of the sound in the air within the tube. A tube being open (or closed) at both ends admits all integer harmonics. A tube being being open at one end and closed at the other end admits only all odd integer harmonics.

* To calculate `[v_m/s;k_(r/m);ω_(r/s);Tair_°C;s_m;fopenopen_Hz;fopenclose_Hz]` (Propagation speed of sound waves; Wave number; Angular frequency, Temperature; Frequency of harmonics in a tube open at both ends; Frequency of harmonics in a tube open at one end and close at the other end) from 8 known variables:
```rpl
λ=3_m f=110_Hz L=0.6_m ninteger=2 nodd=3 x=10_cm t=5_s sm=2e-6_m
@ Expecting [ vsair=330 m/s k=2.09439 51023 9 r/m ω=691.15038 379 r/s Tair=-2.17345 88932 4 °C s=4.15823 38163 6⁳⁻⁷ m fopenopen=550. Hz fopenclose=412.5 Hz ]
'ROOT(ⒺSound Wave Harmonics;[vsair;k;ω;Tair;s;fopenopen;fopenclose];[1_m/s;1_(r/m);1_(r/s);1_°C;1_m;1_Hz;1_Hz])'
```

#### Beat Acoustics

In acoustics, a beat is an interference pattern between two sounds of slightly different frequencies, perceived as a periodic variation in amplitude whose rate is the difference of the two frequencies. The sum of two unit-amplitude sine waves can be expressed as a carrier wave of frequency `favg` whose amplitude is modulated by an envelope wave of frequency `fbeat`.

* To calculate `[favg_Hz;fbeat_Hz;s_m]` (Frequency average; Beat frequency; Longitudinal displacement) from 4 known variables:
```rpl
f1=400_Hz f2=402_Hz t=5_s sm=2e-6_m
@ Expecting [ favg=401 Hz fbeat=2 Hz s=0.00000 2 m ]
'ROOT(ⒺBeat Acoustics;[favg;fbeat;s];[1_Hz;1_Hz;1_m])'
```

#### Electromagnetic Waves

* To calculate `[f_Hz;k_(r/m);ω_(r/s);E_(N/C);B_T]` (Frequency; Wave number; Angular Frequency; Electric & Magnetic fields at `s` & `t`) from 5 known variables:
```rpl
λ=500_nm  Em=5_N/C  x=1e-8_m  t=5e-13_s  φ=25_°
@ Expecting [ f=5.99584 916⁳¹⁴ Hz k=12 566 370.6144 r/m ω=3.76730 31346 2⁳¹⁵ r/s E=4.78368 41812 N/C B=1.59566 52856 2⁳⁻⁸ T ]
'ROOT(ⒺElectromagnetic Waves;[f;k;ω;E;B];[1_Hz;1_(r/m);1_(r/s);1_(N/C);1_T])'
```


## Relativity
The 110 variables in the Relativity section are:

* `α`: Light Doppler effect, light arrival angle in the rest frame
* `β`: Relativistic speed ratio
* `βe`: Escape velocity relativistic speed ratio
* `βp`: Plane velocity relativistic speed ratio
* `βg`: Ground velocity relativistic speed ratio
* `Δt`: Proper time interval ([Time Dilation](#Time Dilation)), or Duration of the circumnavigation trip at latitude `φ` ([Circumnavigating Airplanes](#Circumnavigating Airplanes))
* `Δtp`: Dilated time interval
* `ΔtpG`: Gravitationally dilated time interval
* `Δx`: Proper space interval
* `Δxp`: Contracted space interval
* `λ1`: Wavelength of the photon as measured by the observer at position `R1`
* `λ2`: Wavelength of the photon as measured by the observer at position `R2`
* `λ∞`: Wavelength of the photon as measured by the observer at infinity
* `γ`: Lorentz factor
* `γ21`: Factor of combined special and general relativity effects
* `γv1`: Lorentz factor for velocity `v1`
* `γv2`: Lorentz factor for velocity `v2`
* `γG`: Lorentz factor associated to gravitational dilation
* `γG1`: Lorentz factor associated to gravitational dilation at height `h1`
* `γG2`: Lorentz factor associated to gravitational dilation at height `h2`
* `ω`: Angular velocity associated to planet rotation (dim.: angle/time, in SI: r/s)
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
* `Bp`: Norm of the magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `Bpx`: Transformed x component of the magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `Bpy`: Transformed y component of the magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `Bpz`: Transformed z component of the magnetic field (dim.: mass/(time^2·current), in SI: tesla, T)
* `E`: Total energy (dim.: force·length, in SI: joule, J) or, Norm of the Electric field ([E & B Fields Transformation](#E & B Fields Transformation)) (dim.: force/charge, in SI: N/C=V/m)
* `Ep`: Transformed total energy (dim.: force·length, in SI: joule, J)
* `E₀`: Total energy associated to the rest mass (dim.: force·length, in SI: joule, J)
* `Ex`: X component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `Ey`: Y component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `Ez`: Z component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `Epx`: Transformed x component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `Epy`: Transformed y component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `Epz`: Transformed z component of the electric field (dim.: force/charge, in SI: N/C=V/m)
* `f`: Light Doppler effect, frequency received in the frame at rest (dim.: 1/time, in SI: hertz, Hz)
* `fpr`: Light Doppler effect, frequency emitted in the moving frame (dim.: 1/time, in SI: hertz, Hz)
* `fs`: Wave frequency of the source (dim.: 1/time, in SI: hertz, Hz)
* `frl`: Relativistic longitudinal Doppler frequency (dim.: 1/time, in SI: hertz, Hz)
* `frt`: Relativistic transverse Doppler frequency (dim.: 1/time, in SI: hertz, Hz)
* `h1`: Height of point 1
* `h2`: Height of point 2
* `hp`: Altitude of plane (dim.: length)
* `K`: Kinetic energy (dim.: force·length, in SI: joule, J)
* `M`: Mass of planet
* `m0`: Rest mass
* `MGu`: Reduced gravitational mass given in geometrized units (dim.: length)
* `Mxearth`: Mass factor as a multiple of Earth mass
* `Mxsun`: Mass factor as a multiple of Sun mass
* `MxSagA`: Mass factor as a multiple of SagitariusA* mass
* `Pθ` = Light relativistic beaming, angular distribution of photon in the moving frame from a source isotropic and stationary
* `p`: Momentum (dim.: mass·speed, in SI: kg·m/s)
* `px`: X component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `py`: Y component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `pz`: Z component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `ppx`: Transformed x component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `ppy`: Transformed y component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `ppz`: Transformed z component of the momentum (dim.: mass·speed, in SI: kg·m/s)
* `PBH`: Black hole evaporation power (dim.: energy/time, in SI: watt, W)
* `R`: Planet radius
* `R1`: Radius to point 1
* `R2`: Radius to point 2
* `Remp`: Radius at which the photon is emitted
* `rs`: Schwarzschild  radius
* `rxearth`: Radius factor as a multiple of Earth radius
* `SBH`: Black hole entropy (dim.: energy/temperature, in SI: J/K)
* `Tday`: Day duration
* `tev`: Evaporation time of a black hole
* `TH`: Black hole temperature
* `tp`: Transformed time coordinate in the moving frame
* `txyr`: Duration factor as a multiple of a year
* `ux`: X component of the velocity
* `uy`: Y component of the velocity
* `uz`: Z component of the velocity
* `upx`: Transformed x component of the velocity observed in the moving frame
* `upy`: Transformed y component of the velocity observed in the moving frame
* `upz`: Transformed z component of the velocity observed in the moving frame
* `v1`: Velocity at height `h1`
* `v2`: Velocity at height `h2`
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
* `ve`: Escape velocity in a gravitational field
* `z`: Gravitational redshift parameter
* `znl`: Newtonian limit of the gravitational redshift parameter

The relativistic transformations are parametrized by the real constant `v` representing a velocity confined to the x-direction. The respective inverse transformation is then parameterized by the negative of this velocity.

#### Lorentz Transformation

The primed reference frame `[xp yp zp]` is travelling with velocity `v` in the positive x direction. Therefore, the y and z coordinates of the rest frame remain unchanged.

* To calculate `[β;γ;xp_m;tp_s;yp_m;zp_m]` (Relativistic speed ratio; Lorentz factor; Transformed time, x, y & z coordinate in the moving frame) from 5 known variables:
```rpl
x=1_m  y=2_m  z=3_m  t=4_s  v=239 833 966.4_m/s
@ Expecting [ β=0.8 γ=1.66666 66666 7 xp=-1.59889 31076 7⁳⁹ m tp=6.66666 66622 2 s yp=2 m zp=3 m ]
'ROOT(ⒺLorentz Transformation;[β;γ;xp;tp;yp;zp];[1;1;1_m;1_s;1_m;1_m])'
```

#### Time Dilation

The dilation comes from the fact that the Lorentz factor `γ` is greater or equal to one and the proper time interval is multiplied by this factor.

* To calculate `[β;γ;Δtp_s]` (Relativistic speed ratio; Lorentz factor; dilated time interval) from 3 known variables:
```rpl
Δt=4_s  v=239 833 966.4_m/s
@ Expecting [ β=0.8 γ=1.66666 66666 7 Δtp=6.66666 66666 7 s ]
'ROOT(ⒺTime Dilation;[β;γ;Δtp];[1;1;1_s])'
```

#### Space Contraction

The contraction comes from the fact that the Lorentz factor `γ` is greater or equal to one and the proper space interval is divided by this factor.

* To calculate `[β;γ;Δxp_m]` (Relativistic speed ratio; Lorentz factor; Contracted space interval) from 3 known variables:
```rpl
Δx=2_m  v=284 802 835.1_m/s
@ Expecting [ β=0.95 γ=3.20256 30761 Δxp=0.62449 97998 4 m ]
'ROOT(ⒺSpace Contraction;[β;γ;Δxp];[1;1;1_m])'
```

#### Velocity Superposition

These expressions replace the usual Galilean addition of velocities. It can be checked that superposing with `v = c` leads to `upx = c`, hence the impossibility to superpose velocities to go beyond the velocity limit `c`. Even if the velocity `v` is confined to the x-direction, all components of the observed velocity are transformed in the moving frame.

* **Example 1** To calculate `[β;γ;upx_(m/s);upy_(m/s);upz_(m/s)]` (Relativistic speed ratio; Lorentz factor; Transformed x, y & z component of the velocity observed in the moving frame) from 3 known variables:
```rpl
v=296 794 533.42_m/s  ux=284 802 835.1_(m/s)  uy=200 000 000_(m/s)  uz=250 000 000_(m/s)
@ Expecting [ β=0.99 γ=7.08881 20500 8 upx=-201 541 148.235 m/s upy=474 175 999.317 m/s upz=592 719 999.146 m/s ]
'ROOT(ⒺVelocity Superposition;[β;γ;upx;upy;upz];[1;1;1_(m/s);1_(m/s);1_(m/s)])'
```
* **Example 2** First attempt to cross the `c` speed limit, to calculate `[upx_(m/s)]` (Relativistic speed ratio; Lorentz factor; Transformed x, y & z component of the velocity observed in the moving frame) from 2 known variables:
```rpl
v=299 792 458_m/s  ux=284 802 835.1_m/s
@ Expecting [ upx=-299 792 458. m/s ]
'ROOT(ⒺVelocity Superposition;[upx];[1_(m/s)])'
```
* **Example 3** Second attempt to cross the `c` speed limit, to calculate `[upx_(m/s)]` (Relativistic speed ratio; Lorentz factor; Transformed x, y & z component of the velocity observed in the moving frame) from 2 known variables:
```rpl
ux=299 792 458_m/s  v=284 802 835.1_m/s
@ Expecting [ upx=299 792 458. m/s ]
'ROOT(ⒺVelocity Superposition;[upx];[1_(m/s)])'
```


#### Acceleration Superposition

Even if the velocity `v` is confined to the x-direction, all components of the observed acceleration are transformed in the moving frame.

* To calculate `[β;γ;apx_(m/s^2);apy_(m/s^2);apz_(m/s^2)]` (Relativistic speed ratio; Lorentz factor; Transformed x, y & z component of the acceleration observed in the moving frame) from 3 known variables:
```rpl
v=298 293 495.71_m/s  ax=100_(m/s^2)  ay=200_(m/s^2)  az=300_(m/s^2)  ux=284 802 835.1_(m/s)  uy=200 000 000_(m/s)  uz=250 000 000_(m/s)
@ Expecting [ β=0.995 γ=10.01252 34864 apx=607.03954 8234 m/s↑2 apy=4 700.07036 316 m/s↑2 apz=6 041.47314 191 m/s↑2 ]
'ROOT(ⒺAcceleration Superposition;[β;γ;apx;apy;apz];[1;1;1_(m/s^2);1_(m/s^2);1_(m/s^2)])'
```

#### E & B Fields Transformation

Even if the velocity `v` is confined to the x-direction, only the `y` & `z` transformed components of both B & E fields are modified. One can check that ratio the `EDB = E/B = c` & `EpDBp = Ep/Bp = c` remains the same.

* To calculate `[β;γ;Epx_(N/C);Epy_(N/C);Epz_(N/C);Bpx_T;Bpy_T;Bpz_T;E_(N/C);B_T;Ep_(N/C);Bp_T]` (Relativistic speed ratio; Lorentz factor; Transformed x, y & z component of the electric field & of the magnetic field;Norm of the Electric field & Magnetic field;Norm of the transformed Electric field & Magnetic field) from 7 known variables:
```rpl
v=298 293 495.71_m/s  Ex=100_(N/C)  Ey=200_(N/C)  Ez=300_(N/C)  Bx=50e-8_T  By=80e-8_T Bz=8.17135 28774 3⁳⁻⁷ T
@ Expecting [ β=0.995 γ=10.01252 34864 Epx=100 N/C Epy=-438.00926 8698 N/C Epz=5 393.09355 125 N/C Bpx=0.00000 05 T Bpy=1.79793 76526 3⁳⁻⁵ T Bpz=1.53534 77686 9⁳⁻⁶ T E=374.16573 8677 N/C B=1.24808 25607 6⁳⁻⁶ T Ep=5 411.77514 056 N/C Bp=1.80517 38781 8⁳⁻⁵ T EDB=299 792 458. m/s EpDBp=299 792 458. m/s ]
'ROOT(ⒺE & B Fields Transformation;[β;γ;Epx;Epy;Epz;Bpx;Bpy;Bpz;E;B;Ep;Bp;EDB;EpDBp];[1;1;1_(N/C);1_(N/C);1_(N/C);1_T;1_T;1_T;1_(N/C);1_T;1_(N/C);1_T;1;1])'
```

#### Longitudinal Doppler Effect

* To calculate `[β;frl]` (Relativistic speed ratio; Transformed longitudinal Doppler frequency) from 2 known variables:
```rpl
v=298 293 495.71_m/s  fs=2e3_Hz
@ Expecting [ β=0.995 frl=100.12523 4864 Hz ]
'ROOT(ⒺLongitudinal Doppler Effect;[β;frl];[1;1_Hz])'
```

#### Transverse Doppler Effect

* To calculate `[β;γ;frt]` (Relativistic speed ratio; Lorentz factor; Transformed transverse Doppler frequency) from 2 known variables:
```rpl
v=298 293 495.71_m/s  fs=2e3_Hz
@ Expecting [ β=0.995 γ=10.01252 34864 frt=20 025.04697 29 Hz ]
'ROOT(ⒺTransverse Doppler Effect;[β;γ;frt];[1;1;1_Hz])'
```

#### Light Propagation

* To calculate `[β;γ;fp;θp;Pθ]` (Relativistic speed ratio; Lorentz factor; Transformed Doppler frequency; Emission angle in the moving frame for light aberration; Angular distribution of photon in the moving frame from a source isotropic and stationary) from 4 known variables:
```rpl
v=298 293 495.71_m/s  f=2e3_Hz  α=20_°  θ=10_°
@ Expecting [ β=0.995 γ=10.01252 34864 fpr=38 748.34889 98 Hz θp=120.44203 7302 ° Pθ=2.14021 57038 4 ]
'ROOT(ⒺLight Propagation;[β;γ;fpr;θp;Pθ];[1;1;1_Hz;1_°;1])'
```

#### Energy & Momentum

The total relativistic energy `E` and the norm of the momentum `p` form the invariant `mo·c^2` which remains the same in all frames. The kinetic energy `K` is the difference between the total relativistic energy `E` and the rest energy `E₀ = mo·c^2`.

* To calculate `[β;γ;ppx_(kg*(m/s));ppy_(kg*(m/s));ppz_(kg*(m/s));Ep_J;E_J;K_J]` (Relativistic speed ratio; Lorentz factor; Transformed x, y & z component of the momentum, Transformed total energy; Total & Kinetic energy of the moving mass) from 5 known variables:
```rpl
v=299 192 873.084 m/s  px=10_(kg*(m/s))  py=20_(kg*(m/s))  pz=30_(kg*(m/s))  E=1.42176 77735 4e19_J
@ Expecting [ β=0.998 γ=15.81929 99292 ppx=-7.48730 91346 7⁳¹¹ kg·m/s ppy=20 kg·m/s ppz=30 kg·m/s Ep=2.24913 70834 6⁳²⁰ J E₀=8.98755 17873 9⁳¹⁷ J m0=10. kg p=4.73302 17960 1⁳¹⁰ kg·m/s K=1.33189 22556 7⁳¹⁹ J ]
'ROOT(ⒺEnergy & Momentum;[β;γ;ppx;ppy;ppz;Ep;E₀;m0;p;K];[1;1;1_(kg*(m/s));1_(kg*(m/s));1_(kg*(m/s));1_J;1_J;1_kg;1_(kg*(m/s));1_J])'
```

#### Ultrarelativistic Cases

* **Example 1** In the 27 km circonference accelerator of LHC, protons are accelerated to kinetic energy of 6.8 TeV. To calculate `[E₀_J;γ;β;v_m/s;Δt_s;Δxp_m]` (Rest energy; Lorentz factor; Relativistic speed ratio; Speed; Proper time; Contracted space interval) from 4 known variables, one can calculate the speed, the contracted space interval and proper time of the protons:
```rpl
K=6.8_TeV  m0='Ⓒmp'  Δx=27_km  Δtp='Δx/(299 792 455.147_m/s)'  Δtp=0.00009 00623 07_s
@ Expecting [ E₀=1.50327 76180 2⁳⁻¹⁰ J γ=7 248.36782 709 β=0.99999 99904 83 v=299 792 455.147 m/s Δt=1.24251 84420 6⁳⁻⁸ s Δxp=3.72497 65249 3 m ]
'ROOT(ⒺUltrarelativistic Cases;[E₀;γ;β;v;Δt;Δxp];[1_J;1;1;1_(m/s);1_s;1_m])'
```
* **Example 2** The "Oh-My-God" particle (probably a proton) had a kinetic energy of 3.2e20 eV. To calculate `[E₀_J;γ;β;v_m/s;Δt_s;Δxp_m]` (Rest energy; Lorentz factor; Relativistic speed ratio; Speed; Proper time; Contracted space interval) from 4 known variables, in order to calculate the speed, the contracted space interval and proper time of the proton, the precision needs to be set to 32 digits and 28 significant digits:
```rpl
Modes 'MyModes' STO
32 Precision 28 Sig @ Need high precision for this one
K=3.2e20_eV  m0='Ⓒmp'  Δx=100_km  Δtp='Δx/(299 792 457.99999 99999 99998 7113_m/s)'  Δtp=0.00033 35640 95198 15204 95755 781 s
781_s
@ Expecting [ E₀=1.50327 76180 2⁳⁻¹⁰ J γ=3.41052 60362 9⁳¹¹ β=1. v=299 792 458 m/s Δt=9.78042 95187 6⁳⁻¹⁶ s Δxp=2.93209 90057 2⁳⁻⁷ m ]
'ROOT(ⒺUltrarelativistic Cases;[E₀;γ;β;v;Δt;Δxp];[1_J;1;1;1_(m/s);1_s;1_m])'
ResetModes MyModes @ Restore initial state
```

#### Gravitational Time Dilation

* To calculate `[ve_m/s;βe;γG;ΔtpG_s]` (Excape speed; Relativistic escape speed ratio; Lorentz factor associated to gravitational dilation; Gravitational dilated time interval) from 3 known variables:
```rpl
M=2.32e30_kg  r=6.96e3_m  Δt=1e6_s
@ Expecting [ ve=210 939 169.746 m/s βe=0.70361 73329 83 γG=1.40730 28724 7 ΔtpG=1 407 302.87247 s ]
'ROOT(ⒺGravitational Time Dilation;[ve;βe;γG;ΔtpG];[1_m/s;1;1;1_s])'
```

#### Gravitational Redshift

* To calculate `[rs_m;λ∞_nm;z;λ1_nm;zNL]` (Schwarzschild  radius; Wavelength of the photon as measured by the observer at infinity; Gravitational redshift; Wavelength of the photon as measured by the observer at position `R1`) from 7 known variables:
```rpl
λe=550_nm  λ2=550_nm  M=2.32e30_kg  Remp=70_km  R2=50e6_km  R1=10e6_km  M=4.10227 55e30_kg
@ Expecting [ rs=6 092.83106 622 m λ∞=575.62138 4944 nm z=0.04658 43362 62 λ1=549.99986 5958 nm zNL=4.35202 21901 5⁳⁻² ]
'ROOT(ⒺGravitational Redshift;[rs;λ∞;z;λ1;zNL];[1_m;1_nm;1;1_nm;1])'
```

#### Circumnavigating Airplanes

It is assumed that the planes are circumnavigating at the same altitude `h`, same latitude `φ` and the during same flight duration `Δt` measured in the airplanes. The ground is rotating with the planet at the angular frequency `ω`. The Schwarzschild metric is taken into account. The calculation should formally invokes an integral for the elapsed proper time along a path and is approximated here to the first order in speed since the velocities of the plane `vp` and the planet ground `vg` are << `c` (slow airplanes over a slow rotating planet).

* **Example 1** To calculate for a standard jet (500_mph) `[Δt_s;vg_m/s;βp;βg;MGu_m;Δτg_ns;ΔτpE_ns;ΔτpW_ns;ΔτE_ns;ΔτW_ns;ΔτWE_ns]` (Flight time duration of the circumnavigation trip at latitude `φ`; Ground speed of rotating earth at latitude `φ`; Plane speed ratio; Ground speed ratio; Reduced gravitational mass given in geometrized units; Elapsed time variation due to the ground tangential velocity; Elapsed time variation due to the plane altitude and velocity in the Eastward & Westward direction; Flight time in the Eastward & Westward direction; Time difference between westward and eastward flights) from 6 known variables:
```rpl
vp=500_mph  Tday=86400_s  R=6371_km  hp=1e4_m  M=5.972168e24_kg  φ=7_°
@ Expecting [ Δt=177 754.98724 2 s vg=459.85873 5513 m/s βp=7.45582 46558 7⁳⁻⁷ βg=1.53392 36303⁳⁻⁶ MGu=4.43502 76722 1⁳⁻³ m Δτg=177 754.98711 8 s ΔτpE=177 754.98711 8 s ΔτpW=177 754.98711 8 s ΔτE=-58.77919 8501 ns ΔτW=347.80567 188 ns ΔτWE=406.58487 0381 ns ]
'ROOT(ⒺCircumnavigating Airplanes;[Δt;vg;βp;βg;MGu;Δτg;ΔτpE;ΔτpW;ΔτE;ΔτW;ΔτWE];[1_s;1_m/s;1;1;1_m;1_s;1_s;1_s;1_ns;1_ns;1_ns])'
```
* **Example 2** To calculate for the circumnavigation of the Concorde at maximal speed (Mach 2.04) flying at an altitude of 60000 feet `[Δt_s;vg_m/s;βp;βg;MGu_m;Δτg_ns;ΔτpE_ns;ΔτpW_ns;ΔτE_ns;ΔτW_ns;ΔτWE_ns]` (Flight time duration of the circumnavigation trip at latitude `φ`; Ground speed of rotating earth at latitude `φ`; Plane speed ratio; Ground speed ratio; Reduced gravitational mass given in geometrized units; Elapsed time variation due to the ground tangential velocity; Elapsed time variation due to the plane altitude and velocity in the Eastward & Westward direction; Flight time in the Eastward & Westward direction; Time difference between westward and eastward flights) from 6 known variables:
```rpl
vp=605.27777 77777 77777 77777_m/s  Tday=86400_s  R=6371e3_m  hp=18288_m  M=5.972168e24_kg  φ=12_°
@ Expecting [ Δt=64 689.99803 66 s vg=453.18771 1296 m/s βp=2.01898 93428 8⁳⁻⁶ βg=1.51167 14887 4⁳⁻⁶ MGu=4.43502 76722 1⁳⁻³ m Δτg=64 689.99799 14 s ΔτpE=64 689.99799 12 s ΔτpW=64 689.99799 16 s ΔτE=-200.38946 8271 ns ΔτW=194.48457 0936 ns ΔτWE=394.87403 9207 ns ]
'ROOT(ⒺCircumnavigating Airplanes;[Δt;vg;βp;βg;MGu;Δτg;ΔτpE;ΔτpW;ΔτE;ΔτW;ΔτWE];[1_s;1_m/s;1;1;1_m;1_s;1_s;1_s;1_ns;1_ns;1_ns])'
```

#### Clocks at different heights

It is assumed that the two clocks are at rest with respect to the ground at a latitude `φ` and are rotating with the planet at the angular frequency `ω`. The clocks are at their respective heights `h1` and `h2` for instance at the bottom & top of a mountain. For simplicity, the planet is assumed to have a spherical distribution. The Schwarzschild metric is taken into account. The calculation should formally invoke an integral for the elapsed proper time along a path and is approximated here to the first order in speed since the tangential velocities at height `h1` and `h2` are << `c` (slow rotating planet). As a consequence, the rate of time dilation per meter of height is calculated.

* **Example 1a)** (Earth): To calculate `[ω_r/s;v1_m/s;v2_m/s;MGu_m;γv1;γv2;γG1;γG2;γ21]` (Angular velocity associated to planet rotation; Velocity at height `h1` & `h2` and latitude `φ`; Reduced gravitational mass given in geometrized units; Lorentz factor for velocity `v1` & `v2`; Lorentz factor associated to gravitational dilation at height `h1` & `h2`; Factor of combined special and general relativity effects) from 6 known variables (choose `h2 > h1`):
```rpl
Tday=86400_s  R=6371e3_m  h1=0_m  h2=2000_m  M=5.972168e24_kg  φ=15_°
@ Expecting [ ω=7.27220 52166 4⁳⁻⁵ r/s v1=447.52521 416 m/s v2=447.66570 2376 m/s MGu=4.43502 76722 1⁳⁻³ m γv1=1. γv2=1. γG1=1.00000 00007 γG2=1.00000 00007 γ21=1. ]
'ROOT(ⒺClocks at different heights;[ω;v1;v2;MGu;γv1;γv2;γG1;γG2;γ21];[1_r/s;1_m/s;1_m/s;1_m;1;1;1;1;1])'
@ Save result for later use
```
* **Consequence** To check the validity of a well-known APPROXIMATION which is valid when `Δh/R < 0.1%` => special relativity corrections are negligible which means `ABS(γv1/γv2-1) < 0.1%` => `γ21=γG1/γG2` Then the APPROXIMATE RESULT is `γG1/γG2 ≈ 1 + gloc*Δh/Ⓒc^2` with `gloc=ⒸG*M/R^2`. Let's verify precisely these relations in 3 steps with the final CONSEQUENCE:

* **Example 1b)** The prerequisite conditions & their direct consequence are given by:
```rpl
Δh='ABS((h2_m)-(h1_m))'  gloc='ⒸG*(M_kg)/((R_m)+(h1_m))^2'  approx1='ABS(γG1/γG2-1)'  approx2='gloc*Δh/Ⓒc^2'
"1a) Prerequisite conditions:"
"(Δh_m)/((R_m)+(h1_m)) < 0.1/100  AND  ABS(γv1/γv2-1) < 0.1/100"
'(Δh_m)/((R_m)+(h1_m)) < 0.1/100  AND  ABS(γv1/γv2-1) < 0.1/100'
@ Expecting True
@ Save result for later use
```
* **Example 1c)** EVAL => True, then APPROXIMATION can be checked (EVAL => True) by:
```rpl
"1b) APPROXIMATION check:"
"→NUM(ABS(approx1/approx2-1)) < 0.1/100"
'→NUM(ABS(approx1/approx2-1)) < 0.1/100'
@ Expecting True
@ Save result for later use
```
* **Example 1d)** The important CONSEQUENCE is that the following value is the RATE OF TIME DILATION per meter of height due to a gravitational field `gloc` at height `h1`:
```rpl
"=> Rate of t Dilation /m - vert :"
"ABS(γ21-1)/Δh≈gloc/Ⓒc^2="
@ Expecting 1.09265 01350 9⁳⁻¹⁶ m⁻¹
'→NUM(gloc/Ⓒc^2)'
@ Save result for later use
```
* **Example 2a)** (Earth, Mount Everest):  This mount has an height of 3660_m with repect to the surrounding ground which is at an altitude of 5200_m. To calculate `[ω_r/s;v1_m/s;v2_m/s;MGu_m;γv1;γv2;γG1;γG2;γ21]` (Angular velocity associated to planet rotation; Velocity at height `h1` & `h2` and latitude `φ`; Reduced gravitational mass given in geometrized units; Lorentz factor for velocity `v1` & `v2`; Lorentz factor associated to gravitational dilation at height `h1` & `h2`; Factor of combined special and general relativity effects) from 6 known variables (maintain 24 digits of precision & choose `h2 > h1`):
```rpl
Tday=86400_s  R=6371e3_m  h1=5200_m  h2=8860_m  M=5.972168e24_kg  φ=15_°
@ Expecting [ ω=7.27220 52166 4⁳⁻⁵ r/s v1=447.89048 3523 m/s v2=448.14757 6959 m/s MGu=4.43502 76722 1⁳⁻³ m γv1=1. γv2=1. γG1=1.00000 00007 γG2=1.00000 00007 γ21=1. ]
'ROOT(ⒺClocks at different heights;[ω;v1;v2;MGu;γv1;γv2;γG1;γG2;γ21];[1_r/s;1_m/s;1_m/s;1_m;1;1;1;1;1])'
@ Save result for later use
```
* **Consequence** To check the validity of a well-known APPROXIMATION which is valid when `Δh/R < 0.1%` => special relativity corrections are negligible which means `ABS(γv1/γv2-1) < 0.1%` => `γ21=γG1/γG2` Then the APPROXIMATE RESULT is `γG1/γG2 ≈ 1 + gloc*Δh/Ⓒc^2` with `gloc=ⒸG*M/R^2`. Let's verify precisely these relations in 3 steps with the final CONSEQUENCE:
* **2b)** The prerequisite conditions & their direct consequence are given by:
```rpl
Δh='ABS((h2_m)-(h1_m))'  gloc='ⒸG*(M_kg)/((R_m)+(h1_m))^2'  approx1='ABS(γG1/γG2-1)'  approx2='gloc*Δh/Ⓒc^2'
"2a) Prerequisite CONDITIONS:"
"(Δh_m)/((R_m)+(h1_m)) < 0.1/100  AND  ABS(γv1/γv2-1) < 0.1/100"
'(Δh_m)/((R_m)+(h1_m)) < 0.1/100  AND  ABS(γv1/γv2-1) < 0.1/100'
@ Expecting True
@ Save result for later use
```
* **Example 2c)** EVAL => True, then APPROXIMATION can be checked (EVAL => True) by:
```rpl
"1b) APPROXIMATION check:"
"→NUM(ABS(approx1/approx2-1)) < 0.1/100"
'→NUM(ABS(approx1/approx2-1)) < 0.1/100'
@ Expecting True
@ Save result for later use
```
* **Example 2d)** The important CONSEQUENCE is that the following value is the RATE OF TIME DILATION per meter of height due to a gravitational field  `gloc` at height `h1`:
```rpl
"=> Rate of t Dilation /m - vert :"
"ABS(γ21-1)/Δh≈gloc/Ⓒc^2="
'→NUM(gloc/Ⓒc^2)'
@ Expecting 1.09086 86778 4⁳⁻¹⁶ m⁻¹
```

#### B H Schwarzschild Geometry

* For Sagittarius A*, the supermassive black hole at the Galactic Center of the Milky Way to calculate `[rs_m;ve_(m/s);Vs_(m^3);Vxsun;rxearth;Mxsun;Mxearth]` (Schwarzschild radius; Escape velocity; Schwarzschild volume; Factor multiplicative of Sun volume, of Earth radius, of Sun mass & of Earth mass) from 3 known variables (maintain 24 digits of precision):
```rpl
M=8.54e36_kg  r=12e9_m  V=8.54105 09309e30_m^3
@ Expecting [ rs=1.26838 81739 7⁳¹⁰ m ve=299 792 458. m/s Vs=8.54761 91182 1⁳³⁰ m↑3 Vxsun=6 048.90292 557 rxearth=1 883.53476 691 Mxsun=4 294 694.49334 Mxearth=1.42996 64711 4⁳¹² ]
'ROOT(ⒺB H Schwarzschild Geometry;[rs;ve;Vs;Vxsun;rxearth;Mxsun;Mxearth];[1_m;1_(m/s);1_(m^3);1;1;1;1])'
```

#### B H Thermodynamics

* **Example 1** For M31*, the supermassive black hole at the Galactic Center of Andromeda Galaxy to calculate `[rs_m;As_(m^2);TH_K;PBH_W;SBH_(J/K);tev_s;Mxsun;MxSagA;Mxearth;txyr]` (Schwarzschild radius; Schwarzschild area; Black hole temperature; Black hole evaporation power; Black hole entropy; Evaporation time; Multiplicative factor of Sun mass, of Sagittarius A* mass & of Earth mass; Multiplicative factor of a year) from 2 known variables (maintain 24 digits of precision):
```rpl
M=1.708e45_kg  t=4.18902 53989e119_s
@ Expecting [ rs=2.53677 63479 3⁳¹⁸ m As=8.08675 38442 8⁳³⁷ m↑2 TH=5.⁳⁻²³ K PBH=1.22087 75567 7⁳⁻⁵⁸ W SBH=1.06850 79039 3⁳⁸⁴ J/K tev=4.19117 60841 4⁳¹¹⁹ s Mxsun=8.58938 89866 7⁳¹⁴ MxSagA=200 000 000. Mxearth=2.85993 29422 7⁳²⁰ txyr=1.32742 20469 6⁳¹¹² ]
'ROOT(ⒺB H Thermodynamics;[rs;As;TH;PBH;SBH;tev;Mxsun;MxSagA;Mxearth;txyr];[1_m;1_(m^2);1_K;1_W;1_(J/K);1_s;1;1;1;1])'
```
* **Example 2** For a very small black hole having the mass of 1000_kg, to calculate `[rs_m;As_(m^2);TH_K;PBH_W;SBH_(J/K);tev_s;Mxsun;MxSagA;Mxearth;txyr]` (Schwarzschild radius; Schwarzschild area; Black hole temperature; Black hole evaporation power; Black hole entropy; Evaporation time; Factor multiplicative of Sun mass, of Sagittarius A* mass & of Earth mass; Multiplicative factor of a year) from 2 known variables (maintain 24 digits of precision):
```rpl
M=1000_kg  t=8.40716 15834 7⁳⁻⁸ s
@ Expecting [ rs=1.48523 20538 2⁳⁻²⁴ m As=2.77203 36055 6⁳⁻⁴⁷ m↑2 TH=1.22690 06705 9⁳²⁰ K PBH=3.56162 21447 8⁳²⁶ W SBH=0.36627 05548 53 J/K tev=8.41147 78997⁳⁻⁸ s Mxsun=0. MxSagA=0. Mxearth=1.67⁳⁻²² txyr=2.66406 8745⁳⁻¹⁵ ]
'ROOT(ⒺB H Thermodynamics;[rs;As;TH;PBH;SBH;tev;Mxsun;MxSagA;Mxearth;txyr];[1_m;1_(m^2);1_K;1_W;1_(J/K);1_s;1;1;1;1])'
```

## Modern Physics

The 43 variables in the Modern Physics section are :

* `β`: Relativistic speed ratio
* `φ`: Work function of the substance([Photoelectric Effect](#Photoelectric Effect))(dim.: charge·voltage, in SI : eV); or Angle of the scattered electron ([Compton Scattering](#Compton Scattering))
* `θ`: Scattered photon angle of deflection ([Photoelectric Effect](#Photoelectric Effect)) & ([Compton Scattering](#Compton Scattering)), or Angle between incident photon and cristallographic plane ([DeBroglie Wave](#DeBroglie Wave))
* `γ`: Lorentz relativistic factor
* `%rFr12`: Relative % of change between distribution fractions integrated from `f1` to `f2`
* `%rFr34`: Relative % of change between distribution fractions integrated from `f3` to `f4`
* `λ`: Wavelength of the incident photon ([Compton Scattering](#Compton Scattering))
* `λp`: Wavelength of the diffused photon
* `A`: Area, or Total activity ([Radioactivity](#Radioactivity)) (dim.: number of decay/time, in SI: becquerel, Bq)
* `d`: Interatomic distance between cristallographic plane
* `En`: Electron energy in level `n` (dim.: charge·voltage, in SI: eV)
* `Enp`: Electron energy in level `np` (dim.: charge·voltage, in SI: eV)
* `ebfafb`: Emissive power in the range `fa` to `fb` (dim.: energy/time·area, in SI: W/m^2)
* `eb`: Total emissive power for the entire Planck spectrum (dim.: energy/time·area, in SI: W/m^2)
* `Eph`: Inident photon energy (dim.: charge·voltage; in SI: eV)
* `Epph`: Diffused photon energy (dim.: charge·voltage; in SI: eV)
* `f`: Frequency of the photon (dim.: 1/time, in SI: hertz, Hz)
* `f0`: Threshold frequency (dim.: 1/time, in SI: hertz, Hz)
* `f1`: Lower frequency limit of integration (dim.: 1/time, in SI: hertz, Hz)
* `f2`: Upper frequency limit of integration (dim.: 1/time, in SI: hertz, Hz)
* `f3`: Lower frequency limit of integration (dim.: 1/time, in SI: hertz, Hz)
* `f4`: Upper frequency limit of integration (dim.: 1/time, in SI: hertz, Hz)
* `fa`: Lower frequency limit of integration (dim.: 1/time, in SI: hertz, Hz)
* `fb`: Upper frequency limit of integration (dim.: 1/time, in SI: hertz, Hz)
* `fpeak`: Frequency of maximum emissive power for the Planck distribution (dim.: 1/time, in SI: hertz, Hz)
* `Frfafb`: Fraction of total emissive power in the range `fa` to `fb`
* `Frfafb`: Fraction of Planck emissive power in the range `fa` to `fb`
* `FrPl12`: Fraction of Planck emissive power in the range `f1` to `f2`
* `FrWn12`: Fraction of Wien emissive power in the range `f1` to `f2`
* `FrRJ12`: Fraction of Rayleign-Jeans emissive power in the range `f1` to `f2`
* `FrPl34`: Fraction of Planck emissive power in the range `f3` to `f4`
* `FrWn34`: Fraction of Wien emissive power in the range `f3` to `f4`
* `FrRJ34`: Fraction of Rayleign-Jeans emissive power in the range `f3` to `f4`
* `K`: Kinetic energy of scattered electron (dim.: charge·voltage; in SI: eV)
* `Kmax`: Maximum kinetic energy of photoelectron ([Photoelectric Effect](#Photoelectric Effect)) (dim.: charge·voltage; in SI: eV)
* `p`: Momentum (dim.: mas·speed, in SI: kg·m/s)
* `m`: Mass
* `q`: Heat transfer rate (dim.: energy/time, in SI: watt, W)
* `r`: Radius of electron orbit in level `n`
* `T`: Temperature
* `V0`: Stopping potential (dim.: energy/charge, In SI: volt,V)
* `v`: Speed of emitted electron
* `vmax`: Maximum speed of ejected photoelectron

#### Planck & Wien Comparison

In this section, two comparisons are done between the Planck and Wien spectral distributiona. Based on a incomplete thermodynamic argument, the latter is an approximation of the true Planck law describing the spectral distribution for the light emitted by a black-body. The choice of temperature `T` determines the frequency ranges for integration between `f1` & `f2`, or between `f3` & `f4`. One shall determine in which frequency interval both distribution differs notably or agree. The asymptotic agreement for large frequency is clearly illustrated in the picture. The user is free to choose one or the other comparison fractions (replacing it in `Frfafb`) to compute the corresponding enissive power `ebfafb` and the heat transfer rate `q` from the black-body. Or, For the Planck distribution, one can choose any other values for `fa` & `fb` and compute the corresponding `FrPlab`, `ebfafb` and `q`.

![Planck & Wien Comparison](img/Planck_and_Wien_Distributions.bmp)

* To calculate `[fpeak_THz;f1_THz;f2_THz;FrPl12;FrWn12;%rFr12;f3_THz;f4_THz;FrPl34;FrWn34;%rFr34;FrPlab;eb_(W/m^2);ebfafb_(W/m^2);q_W]` (Frequency of maximum emissive power for the Planck distribution; Lower & Upper frequency limits of integration; Fractions of Planck & Wien emissive power in the range `f1` to `f2`; Relative % of change between distribution fractions integrated from `f3` to `f4`; Lower & Upper frequency limits of integration; Fractions of Planck & Wien emissive power in the range `fa` to `fb`; Total emissive power for the entire Planck spectrum; Emissive power in the range `fa` to `fb`; Heat transfer rate) from 5 known variables:
```rpl
T=1273.15_K  A=100_cm^2  fa=7.48475 43283 5⁳¹³ Hz  fb=3.18337 69964 2⁳¹⁴ Hz  Frfafb=0.64388 90934 2
@ Expecting [ fpeak=74.84754 32835 THz f1=106.11256 6547 THz f2=238.75327 4732 THz FrPl12=0.38336 04816 94 FrWn12=0.38088 77248 71 %rFr12=0.64502 13155 81 f3=0.26528 14163 69 THz f4=66.32035 40922 THz FrPl34=0.28402 76245 74 FrWn34=0.22398 47200 01 %rFr34=21.13981 15457 FrPlab=0.64388 90934 2 eb=148 980.70811 W/m↑2 ebfafb=95 927.05308 19 W/m↑2 q=959.27053 0819 W ]
'ROOT(ⒺPlanck & Wien Comparison;[fpeak;f1;f2;FrPl12;FrWn12;%rFr12;f3;f4;FrPl34;FrWn34;%rFr34;FrPlab;eb;ebfafb;q];[1_THz;1_THz;1_THz;1;1;1;1_THz;1_THz;1;1;1;1;1_(W/m^2);1_(W/m^2);1_W])'
```

#### Planck & Rayleigh-Jeans Comparison

In this section, two comparisons are done between the Planck and Rayleigh-Jeans spectral distributiona. Based on the equipartition theorem argument, the latter is an approximation of the true Planck law describing the spectral distribution for the light emitted by a black-body. The choice of temperature `T` determines the frequency ranges for integration between `f1` & `f2`, or between `f3` & `f4`. One shall determine in which frequency interval both distribution agree or differs considerably, leading to a divergence called UV catastrophy corresponding to unphysical fractions greather than one. The asymptotic agreement for small frequency is clearly illustrated in the picture. The user is free to choose one or the other comparison fractions (replacing it in `Frfafb`) to compute the corresponding enissive power `ebfafb` and the heat transfer rate `q` from the black-body. Or, For the Planck distribution, one could choose any other values for `fa` & `fb` and computes the corresponding `FrPlab`, `ebfafb` and `q`.

![Planck & Rayleigh-Jeans Comparison](img/Planck_and_Rayleigh-Jeans_Distributions.bmp)

* To calculate `[fpeak_THz;f1_THz;f2_THz;FrPl12;FrRJ12;%rFr12;f3_THz;f4_THz;FrPl34;FrRJ34;%rFr34;FrPlab;eb_(W/m^2);ebfafb_(W/m^2);q_W]` (Frequency of maximum emissive power for the Planck distribution; Lower & Upper frequency limits of integration; Fractions of Planck & Rayleigh‐Jeans emissive power in the range `f1` to `f2`; Relative % of change between distribution fractions integrated from `f3` to `f4`; Lower & Upper frequency limits of integration; Fractions of Planck & Rayleigh‐Jeans emissive power in the range `fa` to `fb`; Total emissive power for the entire Planck spectrum; Emissive power in the range `fa` to `fb`; Heat transfer rate) from 5 known variables:
```rpl
T=1273.15_K  A=100_cm^2  fa=2.65281 41636 9⁳¹⁰ Hz  fb=7.48475 43283 5⁳¹³ Hz  Frfafb=0.35399 34269 15
@ Expecting [ fpeak=74.84754 32835 THz f1=45.09784 07827 THz f2=98.15412 40564 THz FrPl12=0.41306 62386 78 FrRJ12=2.34783 01416 5 %rFr12=468.39071 3596 f3=2.65281 41636 9⁳⁻² THz f4=1.32640 70818 4 THz FrPl34=6.29668 51249 6⁳⁻⁶ FrRJ34=6.41618 75792 7⁳⁻⁶ %rFr34=1.89786 29538 3 FrPlab=0.35399 34269 15 eb=148 980.70811 W/m↑2 ebfafb=52 738.19140 8 W/m↑2 q=527.38191 408 W ]
'ROOT(ⒺPlanck & Rayleigh‐Jeans Comparison;[fpeak;f1;f2;FrPl12;FrRJ12;%rFr12;f3;f4;FrPl34;FrRJ34;%rFr34;FrPlab;eb;ebfafb;q];[1_THz;1_THz;1_THz;1;1;1;1_THz;1_THz;1;1;1;1;1_(W/m^2);1_(W/m^2);1_W])'
```

#### Photoelectric Effect

Einstein explained the photoelectric effect with the energy quantification of the electromagnetic wave. The photoelectron is then emitted only if the energy `E` of the incident photon is greather or equal to the work function `φ` of the material. A simple energy budget equation determines the maximum kinetic energy `Kmax` of the photoelectron.

* To calculate `[f_THz;Eph_eV;f0_THz;λ0_nm;Kmax_eV;Vo_V;vmax_m/s]` (Frequency, Photon energy; Threshold frequency; Threshold wavelength; Maximum kinetic energy of photoelectron; Stoping potential; Maximum speed of photoelectron) from 2 known variables:
```rpl
φ=4.01_eV  λ=207_nm
@ Expecting [ f=1 448.27274 396 THz Eph=5.98957 22016 1 eV f0=969.61410 7285 THz λ0=309.18739 2951 nm Kmax=1.97957 22016 1 eV Vo=1.97957 30615 6 V vmax=834 471.41457 5 m/s ]
'ROOT(ⒺPhotoelectric Effect;[f;Eph;f0;λ0;Kmax;Vo;vmax];[1_THz;1_eV;1_THz;1_nm;1_eV;1_V;1_m/s])'
```

#### Compton Scattering

In the Compton Scattering, both energy and momentum are conserved during the collision of the incident photon and the electron, which underlines the fact that the photon must henceforth be considered as a particle. When a high frequency `f` (or energy `E = hf`) photon scatters due to an interaction with an electron, there is a decrease in the energy of the photon scattered at an angle `θ` and thus, an increase in its wavelength `λp`. The kinetic energy of the scattered electron `Kmax` is relativist.

![Compton Scattering_BW](img/Compton_Scattering_BW.bmp)

* To calculate `[λp_nm;K_eV;γ;β;v_m/s;Eph_eV;Epph_eV;p_(kg*m/s);φ_°]` (Wavelength of scattered photon; Kinetic energy of scattered electron; Lorentz factor; Speed of the scattered electron; Energy of the Incident & Scattered photon; Momentum of the scattered electron; Angle of scattering of the electron) from 2 known variables:
```rpl
θ=40_°  λ=0.024_nm
@ Expecting [ λp=2.45676 48762 3⁳⁻² nm K=1 193.63352 749 eV γ=1.00233 58835 6 β=6.82308 49980 3⁳⁻² v=20 455 094.227 m/s Eph=51 660.06023 89 eV Epph=50 466.42671 14 eV p=1.86768 55511 5⁳⁻²³ kg·m/s φ=68.16075 25239 ° ]
'ROOT(ⒺCompton Scattering;[λp;K;γ;β;v;Eph;Epph;p;φ];[1_nm;1_eV;1;1;1_m/s;1_eV;1_eV;1_(kg*m/s);1_°])'
```

#### De Broglie Wave

At all scales where measurements have been possible, matter exhibits wave-like behavior (e.g. Young interference for protons, even for molecules). More precisely, a beam of neutron can be diffracted just like a beam of light or a water wave as it is the case in the Bragg diffraction. Here, the interference is constructive when the phase difference between the matter wave reflected off different atomic planes at an angle `θ` is a multiple of 2π, giving the Following condition: `2·d·SINθ = n·λ` with `n` integer and where `θ` is mesured between the refected waves trajectory and the atomic plane.

* To calculate `[λ_nm;K_eV;v_m/s;d_nm]` (De Broglie wavelength of scattered photon; Kinetic energy of scattered electron; Speed of the scattered electron; Distance between adjacent atomic planes) from 3 known variables:
```rpl
θ=40_°  p=1e-23_kg*m/s m=Ⓒme n=2
@ Expecting [ λ=0.06626 07015 nm K=342.58664 2473 eV v=10 977 691.0426 m/s d=0.10308 33521 08 nm ]
'ROOT(ⒺDe Broglie Wave;[λ;K;v;d];[1_nm;1_eV;1_m/s;1_nm])'
```

#### Bohr Atomic Model

Since the hydrogen atom is a bound system between the proton of the nucleus and the electron located at a level `n`, the binding energy is calculated with a negative expression, the value 0 corresponding to a free electron or occupying a level n which tends to infinity. A transition which gives a positive energy difference corresponds to the emission of a final photon `np` > initial `n`, or if it is negative, to the absorption of a photon.

* **Example 1** In the case of an emission, to calculate `[Enp_eV;En_eV;r_m;f_THz;Eph_eV;λ_nm;r_m]` (Energy of the final atomic level `np`; Energy of the initial atomic level `n`; Radius of the initial atomic level `n`; Frequency, Energy & Wavelength of the emitted photon) from 3 known variables:
```rpl
np=2  n=1  Z=1
@ Expecting [ Enp=-3.40142 18031 3 eV En=-13.60568 72125 eV r=5.29177 21054 4⁳⁻¹¹ m f=2 467.38147 018 THz Eph=10.20426 54094 eV λ=121.50227 3411 nm ]
'ROOT(ⒺBohr Atomic Model;[Enp;En;r;f;Eph;λ];[1_eV;1_eV;1_m;1_THz;1_eV;1_nm])'
```
* **Example 2** In the case of an absorption, to calculate `[Enp_eV;En_eV;r_m;f_THz;Eph_eV;λ_nm]` (Energy of the final atomic level `np`; Energy of the initial atomic level `n`; Radius of the initial atomic level n; Frequency, Energy & Wavelength of the absorbed photon) from 3 known variables (Note: instead to `n→∞` one can choose `n=9.99999E999999`):
```rpl
np=2  n=9.99999E999999  Z=1
@ Expecting [ Enp=-3.40142 18031 3 eV En=-1.36057 14423 9⁳⁻¹⁹⁹⁹⁹⁹⁹ eV r=5.29176 15219⁳¹⁹⁹⁹⁹⁸⁹ m f=-822.46049 0061 THz Eph=-3.40142 18031 3 eV λ=-364.50682 0234 nm ]
'ROOT(ⒺBohr Atomic Model;[Enp;En;r;f;Eph;λ];[1_eV;1_eV;1_m;1_THz;1_eV;1_nm])'
```

## Nuclear Physics
The 52 variables in the Nuclear Physics section are:

* `λ`: Decay constant (Radioactivity) (dim.: 1/time, in SI: s^-1)
* `Δm`: Mass default
* `A`: Total activity at time `t` ([Radioactivity](#Radioactivity)) (dim.: number of decay/time, in SI: becquerel, Bq), or Mass number ([Nuclear Physics](#Nuclear Physics))
* `abdaughter`: Abondance of the daughter nuclide
* `abparent`: Abondance of the parent radionuclide
* `abtot`: Total abondance of the nuclide
* `AY1`: Mass number of the first nuclide `Y1` produced by a fission
* `AY2`: Mass number of the second nuclide `Y2` produced by a fission
* `AXα`: Mass number of the radionuclide `X` undergoing α decay
* `AYα`: Mass number of the daughter nuclide `Y` from α decay
* `AXβ⊖`: Mass number of the radionuclide `X` undergoing β⊖ decay
* `AYβ⊖`: Mass number of the daughter nuclide `Y` from β⊖ decay
* `AXβ⊕`: Mass number of the radionuclide `X` undergoing β⊕ decay
* `AYβ⊕`: Mass number of the daughter nuclide `Y` from β⊕ decay
* `Aa`: Mass number of the incident nuclide or particle `a`
* `Ab`: Mass number of the product nuclide or particle `b`
* `AX`: Mass number of the reactant nuclide `X`
* `AY`: Mass number of the product nuclide `Y`
* `A0`: Initial total activity (dim.: number of decay/time, in SI: becquerel, Bq)
* `EB`: Nuclear binding energy (dim.: charge·voltage, in SI: MeV)
* `EBse`: Semiempirical nuclear binding energy (dim.: charge·voltage, in SI: MeV)
* `K`: Kinetic energy of the emitted `α` particle (dim.: charge·voltage, in SI: MeV)
* `Kmax`: Maximum kinetic energy of the electron ([β⊖ Decay](#β⊖ Decay)), or of the positron ([β⊕ Decay](#β⊕ Decay)) (dim.: charge·voltage, in SI: MeV)
* `ΔKtot`: Variation of total kinetic energy (dim.: charge·voltage, in SI: MeV)
* `ma`: Mass of incident nuclide or particle `a` (In SI: u)
* `mb`: Mass of outgoing nuclide or particle `b` (In SI: u)
* `mX`: Mass of reactant neutral atom `X` (In SI: u)
* `mY`: Mass of product neutral atom `X` (In SI: u)
* `MW`: Mass weight (dim.: mass/mol, in SI: g/mol)
* `N`: Number of nuclide at time `t` ([Radioactivity](#Radioactivity)), or Number of neutron ([Nuclear Physics](#Nuclear Physics))
* `nn`: Number of neutrons produced in a fission reaction
* `N0`: Initial number of nuclide
* `Q`: Reaction energy of a nuclear reaction (dim.: charge·voltage, in SI: MeV)
* `Qα`: Reaction energy of an α decay (dim.: charge·voltage, in SI: MeV)
* `Qβ⊖`: Reaction energy of a β⊖ decay (dim.: charge·voltage, in SI: MeV)
* `Qβ⊕`: Reaction energy of a β⊕ decay (dim.: charge·voltage, in SI: MeV)
* `R`: Radius of the nucleus having `A` nucleons
* `t`: Age of the decaying nuclide
* `Thl`: Half-life of radionuclide (dim.: time)
* `Z`: Number of proton
* `ZXα`: Proton number of the radionuclide `X` undergoing α decay
* `ZYα`: Proton number of the daughter nuclide `Y` from α decay
* `ZY1`: Proton number of the first nuclide `Y1` produced by a fission
* `ZY2`: Proton number of the second nuclide `Y2` produced by a fission
* `ZXβ⊖`: Proton number of the radionuclide `X` undergoing β⊖ decay
* `ZYβ⊖`: Proton number of the daughter nuclide `Y` from β⊖ decay
* `ZXβ⊕`: Proton number of the radionuclide `X` undergoing β⊕ decay
* `ZYβ⊕`: Proton number of the daughter nuclide `Y` from β⊕ decay
* `Za`: Proton number of the incident nuclide or charge of the incident particle `a`
* `Zb`: Proton number of the incident nuclide or charge of the product particle `b`
* `ZX`: Proton number of the reactant nuclide `X`
* `ZY`: Proton number of the product nuclide `Y`

For all nuclear reactions, including nuclear decays, we have charge conservation `Zp = Z` and mass number conservation `Mp = M` (the same number of nucleons). This therefore requires assigning numbers `A` and `Z` to incident particles `a` and `b` whether they are for example gamma photons or neutrinos (`A = 0`, `Z = 0`), positrons (`A = 0`, `Z = +1`) or others. The reaction energy `Q` is always calculated with the mass-energy equivalence by the mass default `Δm` between the reactants and the products. For instance, spontaneous decays and fissions are always exothermic `Q > 0` while some nuclear reactions can also be endothermic `Q < 0` (like nucleosynthesis of heavy elements). The mass difference should be computed with at least 9 significant digits because reactants and products masses can be of the same order of magnitude.

#### Radioactivity

* **Example 1** For a sample of 1 g of radium, to calculate `[Thl_s;abtot;N0;A0_Bq;N;A_Bq]` (Half-life of radionuclide, Total abundance, Initial number of nuclide, Initial total activity; Final number of nuclide at time `t`; Final total activity at time `t`) from 6 known variables:
```rpl
m=1_g  MW=226_(g/mol)  λ=1.37364 03205 5⁳⁻¹¹_s^-1  abparent=1  abdaughter=1  t=400_yr
@ Expecting [ Thl=5.04606 02400 1⁳¹⁰ s abtot=1 N0=2.66466 40531⁳²¹ A0=3.66028 99840 5⁳¹⁰ Bq N=2.24047 19403 2⁳²¹ A=3.07760 25942 9⁳¹⁰ Bq ]
'ROOT(ⒺRadioactivity;[Thl;abtot;N0;A0;N;A];[1_s;1;1;1_Bq;1;1_Bq])'
```
* **Example 2** For the C14 datation (present in the atmosphere) of a 10 g sample of carbon having an activity of 30 decays/min, to calculate `[λ_s^-1;abtot;N0;A0_Bq;t_yr]` (Decay constant; Total abundance, Initial number of nuclide, Initial total activity; Final number of nuclide at time `t`; Age of the sample) from 6 known variables:
```rpl
m=10_g  MW=12.01_(g/mol)  Thl=5730_yr  abparent=0.989  abdaughter=1.3e-12  A=30_min^-1
@ Expecting [ λ=3.83332 95627⁳⁻¹² s⁻¹ abtot=1.2857⁳⁻¹² N0=6.44684 96046 1⁳¹¹ A0=2.47128 99175 6 Bq t=13 209.16426 31 yr ]
'ROOT(ⒺRadioactivity;[λ;abtot;N0;A0;t];[1_s^-1;1;1;1_Bq;1_yr])'
```

#### Radius & Binding Energy

* **Example 1** For the C12, to calculate `[A;R_m;V_m^3;EB_MeV;EBse_MeV]` (Mass number of the nuclide; Radius & Volume of the nucleus; Binding energy; Semiempirical binding energy) from 3 known variables:
```rpl
N=6  Z=6  mX=12_u
@ Expecting [ A=12 R=2.74731 41821 3⁳⁻¹⁵ m V=8.68587 53686 5⁳⁻⁴⁴ m↑3 EB=92.16169 75587 MeV EBse=89.61225 87145 MeV ]
'ROOT(ⒺRadius & Binding Energy;[A;R;V;EB;EBse];[1;1_m;1_m^3;1_MeV;1_MeV])'
```
* **Example 2** For the U238, to calculate `[A;R_m;V_m^3;EB_MeV;EBse_MeV]` (Mass number of the nuclide; Radius & Volume of the nucleus; Binding energy; Semiempirical binding energy) from 3 known variables:
```rpl
N=92  Z=146  mX=238.0507847_u
@ Expecting [ A=238 R=7.43658 53216 9⁳⁻¹⁵ m V=1.72269 86147 8⁳⁻⁴² m↑3 EB=1 759.44468 491 MeV EBse=346.41011 9506 MeV ]
'ROOT(ⒺRadius & Binding Energy;[A;R;V;EB;EBse];[1;1_m;1_m^3;1_MeV;1_MeV])'
```

#### α Decay

* **Example 1** For the α decay of U238 into Th234, to calculate `[N;Δm_u;Qα_MeV;Kα_MeV;γ;β;AYα;ZYα]` (Number of neutron of U238; Mass default; Net energy balance; Kinetic energy of the α particle, Lorentz factor; Relativistic speed ratio; Mass number & Proton number of the daughter nuclide) from 6 known variables:
```rpl
A=238  Z=92  AXα=238  ZXα=92  mX=238.0507847_u  mY=234.0436014_u
@ Expecting [ N=146 Δm=0.00458 00458 7 u Qα=4.26628 38693 5 MeV Kα=4.19573 64506 9 MeV γ=1.00112 53451 3 β=4.74014 42424 1⁳⁻² AYα=234 ZYα=90 ]
'ROOT(Ⓔα Decay;[N;Δm;Qα;Kα;γ;β;AYα;ZYα];[1;1_u;1_MeV;1_MeV;1;1;1;1])'
```
* **Example 2** For the α decay of Pu239 into U235, to calculate `[N;Δm_u;Qα_MeV;Kα_MeV;γ;β;AYα;ZYα]` (Number of neutron of Pu239; Mass default; Net energy balance; Kinetic energy of the α particle, Lorentz factor; Relativistic speed ratio; Mass number & Proton number of the daughter nuclide) from 6 known variables:
```rpl
A=239  Z=94  AXα=239  ZXα=94  mX=239.052157_u  mY=235.043924_u
@ Expecting[ [ N=145 Δm=0.0064 u Qα=5.9614 MeV Kα=5.86324 MeV γ=1.00157 β=0.05684 AYα=235 ZYα=92 ]
'ROOT(Ⓔα Decay;[N;Δm;Qα;Kα;γ;β;AYα;ZYα];[1;1_u;1_MeV;1_MeV;1;1;1;1])'
```

#### β⊖ Decay

The β⊖ Decay reaction has 3 products: the daughter nuclide, an electron & an antineutrino having a negligeable mass. Due to their weak masses, the resulting kinetic energy is essentially splitted between the last two particles. The energy spectrum of the antineutrino being continuous and possibly very small, the energy reaction can therefore be the maximum kinetic energy `Kmax` of the emitted elevtron.

* **Example 1** For the β⊖ decay of Th234 into Pa234, to calculate `[N;Δm_u;Qβ⊖_MeV;Kmax_MeV;γmax;βmax;AYβ⊖;ZYβ⊖]` (Number of neutron of Th234; Mass default; Reaction energy; Maximum kinetic energy of the electron, Maximum Lorentz factor; Maximum relativistic speed ratio; Mass number & Proton number of the daughter nuclide) from 6 known variables:
```rpl
A=234  Z=90  AXβ⊖=234  ZXβ⊖=90  mX=234.0436014_u  mY=234.0433085_u
@ Expecting [ N=144 Δm=0.00029 29 u Qβ⊖=0.27283 45044 57 MeV Kmax=0.27283 45044 57 MeV γmax=1.53392 40376 3 βmax=0.75828 53739 86 AYβ⊖=234 ZYβ⊖=91 ]
'ROOT(Ⓔβ⊖ Decay;[N;Δm;Qβ⊖;Kmax;γmax;βmax;AYβ⊖;ZYβ⊖];[1;1_u;1_MeV;1_MeV;1;1;1;1])'
```
* **Example 2** For the β⊖ decay of C14 into N14, to calculate `[N;Δm_u;Qβ⊖_MeV;Kmax_MeV;γmax;βmax;AYβ⊖;ZYβ⊖]` (Number of neutron of C14; Mass default; Reaction energy; Maximum kinetic energy of the electron, Maximum Lorentz factor; Maximum relativistic speed ratio; Mass number & Proton number of the daughter nuclide) from 6 known variables:
```rpl
A=14  Z=6  AXβ⊖=14  ZXβ⊖=6  mX=14.0032419884_u  mY=14.00307400443_u
@ Expecting [ N=8 Δm=0.00016 79839 7 u Qβ⊖=0.15647 60096 MeV Kmax=0.15647 60096 MeV γmax=1.30621 60447 9 βmax=0.64335 24392 36 AYβ⊖=14 ZYβ⊖=7 ]
'ROOT(Ⓔβ⊖ Decay;[N;Δm;Qβ⊖;Kmax;γmax;βmax;AYβ⊖;ZYβ⊖];[1;1_u;1_MeV;1_MeV;1;1;1;1])'
```

#### β⊕ Decay

The β⊕ Decay reaction has 3 products: the daughter nuclide, a positron & a neutrino having a negligeable mass. Due to their weak masses, the resulting kinetic energy is essentially splitted between the last two particles. The energy spectrum of the neutrino being continuous and possibly very small, the energy reaction can therefore be the maximum kinetic energy `Kmax` of the emitted positron.

* For the β⊕ decay of K40 (present in bananas) into Ar40, to calculate `[N;Δm_u;Qβ⊕_MeV;Kmax_MeV;γmax;βmax;AYβ⊕;ZYβ⊕]` (Number of neutron of K40; Mass default; Reaction energy; Maximum kinetic energy of the electron, Maximum Lorentz factor; Maximum relativistic speed ratio; Mass number & Proton number of the daughter nuclide) from 6 known variables:
```rpl
A=40  Z=19  AXβ⊕=40  ZXβ⊕=19  mX=39.963998166_u  mY=39.9623831237_u
@ Expecting [ N=21 Δm=5.17882 48191 5⁳⁻⁴ u Qβ⊕=0.48240 42687 62 MeV Kmax=0.48240 42687 62 MeV γmax=1.94404 20135 3 βmax=0.85755 50653 48 AYβ⊕=40 ZYβ⊕=18 ]
'ROOT(Ⓔβ⊕ Decay;[N;Δm;Qβ⊕;Kmax;γmax;βmax;AYβ⊕;ZYβ⊕];[1;1_u;1_MeV;1_MeV;1;1;1;1])'
```

#### General Nuclear Reaction

* **Example 1**  For the nuclear reaction: α + N14 → O17 + p (represented as a + X → Y + b), to calculate `[N;Δm_u;Q_MeV;ΔKtot_MeV;AY;ZY]` (Number of neutron of N14; Mass default; Reaction energy; Variation of total kinetic energy, Mass number & Proton number of the daughter nuclide) from 12 known variables (note: to balance `Z`, the α & p are replaced here by the neutral atoms He4 & H1):
```rpl
A=14  Z=7  AX=14  ZX=7  Aa=4  Ab=1  Za=2  Zb=1  mX=14.00307400443_u  mY=16.99913175650_u  ma=4.00260325413_u  mb=1.00782503223_u
@ Expecting [ N=7 Δm=-0.00127 95301 7 u Q=-1.19187 42911 2 MeV ΔKtot=-1.19187 42911 2 MeV AY=17 ZY=8 ]
'ROOT(ⒺGeneral Nuclear Reaction;[N;Δm;Q;ΔKtot;AY;ZY];[1;1_u;1_MeV;1_MeV;1;1])'
```
* **Example 2**  For the nuclear reaction: α + Al27 → P30 + n (represented as a + X → Y + b), to calculate `[N;Δm_u;Q_MeV;ΔKtot_MeV;AY;ZY]` (Number of neutron of Al27; Mass default; Reaction energy; Variation of total kinetic energy, Mass number & Proton number of the daughter nuclide) from 12 known variables (note: to balance `Z`, α is replaced by the neutral atom He4):
```rpl
A=27  Z=13  AX=27  ZX=13  Aa=4  Ab=1  Za=2  Zb=0  mX=14.00307400443_u  mY=16.99913175650_u  ma='ⒸmHe'  mb='Ⓒmn'
@ Expecting [ N=14 Δm=-2.11941 39960 6⁳⁻³ u Q=-1.97422 07830 5 MeV ΔKtot=-1.97422 07830 5 MeV AY=30 ZY=15 ]
'ROOT(ⒺGeneral Nuclear Reaction;[N;Δm;Q;ΔKtot;AY;ZY];[1;1_u;1_MeV;1_MeV;1;1])'
```
* **Example 3**  For the fusion nuclear reaction: D2 + Li6 → He4 + He4 (represented as a + X → Y + b), to calculate `[N;Δm_u;Q_MeV;ΔKtot_MeV;AY;ZY]` (Number of neutron of Li6; Mass default; Reaction energy; Variation of total kinetic energy, Mass number & Proton number of one of the daughter nuclide) from 12 known variables :
```rpl
A=6  Z=3  AX=6  ZX=3  Aa=2  Ab=4  Za=1  Zb=2  mX=6.0151228874_u  mY=4.00260325413_u  ma=2.01410177812_u  mb='ⒸmHe'
@ Expecting [ N=3 Δm=0.02401 81572 6 u Q=22.37276 21509 MeV ΔKtot=22.37276 21509 MeV AY=4 ZY=2 ]
'ROOT(ⒺGeneral Nuclear Reaction;[N;Δm;Q;ΔKtot;AY;ZY];[1;1_u;1_MeV;1_MeV;1;1])'
```

#### Fission Reaction

* **Example 1** For the fission reaction: n + U235 → Xe140 + Sr94 + 2n (represented as n + X → Y1 + Y2 + `nn`*n where `nn` is the number of produced neutrons), to calculate `[N;Δm_u;Q_MeV;ΔKtot_MeV;AY1;ZY1]` (Number of neutron of U235; Mass default; Reaction energy; Variation of total kinetic energy; Mass number & Proton number of one of the two products of fission, Xe140) from 10 known variables:
```rpl
A=235  Z=92  AX=235  ZX=92  AY2=94  ZY2=38  mX=235.043924_u  mY1=139.9216458_u  mY2=93.915361_u  nn=2
@ Expecting [ N=143 Δm=0.19825 22839 44 u Q=184.67075 332 MeV ΔKtot=184.67075 332 MeV AY1=140 ZY1=54 ]
'ROOT(ⒺFission Reaction;[N;Δm;Q;ΔKtot;AY1;ZY1];[1;1_u;1_MeV;1_MeV;1;1])'
```
* **Example 2** For the fission reaction: n + Pu239 → Xe134 + Zr103 + 3n (represented as n + X → Y1 + Y2 + `nn`*n where `nn` is the number of produced neutrons), to calculate `[N;Δm_u;Q_MeV;ΔKtot_MeV;AY1;ZY1]` (Number of neutron of Pu239; Mass default; Reaction energy; Variation of total kinetic energy; Mass number & Proton number of one of the two products of fission, Xe134) from 10 known variables:
```rpl
A=239  Z=94  AX=239  ZX=94  AY2=103  ZY2=40  mX=239.052157_u  mY1=133.90539466_u  mY2=102.926597_u  nn=3
@ Expecting [ N=145 Δm=0.20283 55078 88 u Q=188.93999 7545 MeV ΔKtot=188.93999 7545 MeV AY1=134 ZY1=54 ]
'ROOT(ⒺFission Reaction;[N;Δm;Q;ΔKtot;AY1;ZY1];[1;1_u;1_MeV;1_MeV;1;1])'
```


## Finance

The `Finance` section contains equations to compute time value of money (TVM).
The variables in the Finance section include:

* `n`: The number of compounding periods. For example, a 30 years loan with
       monthly payments has 360 compounding periods.
* `I%Yr`: Interest rate per year in percent
* `PYr`: Payments per year.
* `Pmt`: The periodic payment amount.
* `PV`: The present value of a series of future cash flows,
        or the initial cash flow.
* `FV`: The future value, that is the final cash flow
        (balloon payment or remaining balance),
        or the compounded value of a series of prior cash flows.

### TVMBeg

Time value of money equation when payments are made at the beginning of the
period.

```rpl
n='9*12' I%Yr=5.75 PV=-155 Pmt=0 PYr=12
@ Expecting FV=259.74210 1025
'ROOT(ⒺTVMBeg;FV;0)'
```

### TVMEnd

Time value of money equation when payments are made at the end of the period.

```rpl
n='5*12' I%Yr=13 PV=-63000 FV=10000 PYr=12
@ Expecting Pmt=1 314.24620 468
'ROOT(ⒺTVMEnd;Pmt;0)'
```
