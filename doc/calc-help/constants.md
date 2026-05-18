# Constants library

The DB48X calculator features a library of constants covering mathematics,
physics, chemistry, astronomy, materials science and computer science.
The built-in constants can be extended using the `config/constants.csv`
configuration file.

## Mathematics constants

### π constant

The ratio between the circumference and the diameter of a circle.

![Definition of π](img/definition_of_pi.bmp)

### e constant

Euler's constant is the base for the natural logarithm.

![Definition of e](img/definition_of_e.bmp)

### ⅈ constant

The imaginary unit, such that ⅈ²=-1.

The picture below shows imaginary unit ⅈ in the complex plane: Real numbers are
conventionally drawn on the horizontal axis, and imaginary numbers on the
vertical axis.

![Imaginary unit](img/imaginary_unit.bmp)

### ⅉ constant

An alternative notation, often preferred by physicists, for the imaginary unit ⅈ, verifying ⅉ²=-1.

### ∞ constant

Infinity is something that is larger than any natural number.
Its numerical value is an arbitrary large number that is not really infinite.

### ? constant

The undefined constant is used to represent undefined values, such as the result of undefined operations.

### rad constant

This constant defines corresponds to one radian.

### twoπ constant

This constant holds the value of two times [π](#π-constant).

### angl constant

This constants holds one half-turn expressed in degrees.

### −∞ constant

Minus infinity, the negative counterpart of ∞.
Its numerical value is an arbitrary large negative number that
is not really infinite.

### γ constant

The Euler-Mascheroni constant is a mathematical constant
approximately equal to 0.5772, appearing in analysis and
number theory, notably in the Laurent expansion of the Riemann
zeta function and in integrals involving the natural logarithm.
Its exact nature (rational or irrational) remains unknown.


## Chemistry constant

### NA constant

Avogradro constant is the number of constituent particles per mole.

Since the redefinition of the mole in 2019, the `NA` constant is therefore
defined as an exact value. [CODATA](#codata)

### k constant

The Boltzmann constant is the proportionality factor that relates the average relative thermal
energy of particles in a gas with the thermodynamic temperature of the gas.

Since the 2019 redefinition of the SI units, the `k` constant is therefore
defined as an exact value. [CODATA](#codata)

### Vm constant

Molar volume of an ideal gas at 1 atmosphere of pressure and 0°C.

By convention, it is chosen as to be the exact result of `R·StdT/StdP`.

### R constant

The universal gas constant is the molar equivalent to the Boltzmann constant,
expressed in units of energy per temperature increment per amount of substance,
rather than energy per temperature increment per particle.

Since the 2019 redefinition of the SI units, the `R` constant is therefore
defined as an exact quantity.

Since the 2019 redefinition of the SI units, the `σ` constant is defined as an
exact result. [CODATA](#codata)

### F constant

Faraday constant. In physical chemistry, the Faraday constant is a
physical constant defined as the quotient of the total electric charge
(`qe`) by the amount (`n`) of elementary charge carriers in any given
sample of matter. It's an exact constant. [CODATA](#codata)

### StdT constant

Standard temperature as defined by IUPAC in 1982.

Standard temperature and pressure (STP) or Standard conditions for temperature
and pressure are various standard sets of conditions for experimental
measurements used to allow comparisons to be made between different sets of
data. The most used standards are those of the International Union of Pure and
Applied Chemistry (IUPAC, used by DB48X) and the National Institute of Standards
and Technology (NIST). Other organizations have established a variety of other
definitions. By convention, it is chosen as the exact value `StdT=273.15_K`.

### StdP constant

Standard pressure as defined by IUPAC in 1982, corresponding to 1 atm

Standard temperature and pressure (STP) or Standard conditions for temperature
and pressure are various standard sets of conditions for experimental
measurements used to allow comparisons to be made between different sets of
data. The most used standards are those of the International Union of Pure and
Applied Chemistry (IUPAC, used by DB48X) and the National Institute of Standards
and Technology (NIST). Other organizations have established a variety of other
definitions. By convention, it is chosen as the exact value `StdP=101.325_kPa`.

### Mu constant

Molar mass constant, defined as one twelfth of the molar mass of carbon-12.

The molar mass of an element or compound is its relative atomic mass (or atomic
weight, noted `Ar`) or relative molecular mass multiplied by the molar mass
constant.

Following the 2019 revision of the SI system, the
[Avogadro constant](#NA-constant) became exact. As a result the molar mass
constant is no longer exactly `1_g/mol`. For internal consistency this value
depends therefore on the mass unit `u` [u-constant](#u-constant) and is
calculated by the following expression: `NA·u`. [CODATA](#codata)

### MC12 constant

Molar mass of carbon-12. Since 1960, mole is the amount of substance of
a system which contains as many elementary entities as there are atoms in
12 gram of carbon-12. Since 2019, the SI definition of mole changed such
that the molar mass of carbone-12 remains nearly but no longer exactly
12 g/mol. For internal consistency this value depends therefore on the
mass unit `u` [u-constant](#u-constant) and is calculated by the following
expression: `12·Mu`. [CODATA](#codata)

### Da constant

The Dalton constant is the unit mass defined as one twelfth the mass of an
unbound neutral atom of carbon-12 in its nuclear and electronic ground state and
at rest. [CODATA](#codata)

### n0 constant

Loschmidt constant or Loschmidt's number is the number of particles
(atoms or molecules) of an ideal gas per volume (the number density),
and usually quoted at standard temperature and pressure. Since 2019 with
the redifinition of the mole, it is calculated exactly as: `NA·Vm`. [CODATA](#codata)

### SoR constant

Sakur-Tetrode constant gives the absolute entropy at an absolute temperature
of `T=1_K` and standard atmospheric pressure `StdP` for one mole of an ideal
monoatomic gas composed of particles of mass equal to the atomic mass constant.

This constant is used in the Sakur-Tetrode equation expression the entropy of a
monoatomic ideal gas in terms of its thermodynamic state. [CODATA](#codata)

### kq constant

This constant expresses the ratio of Boltzmann constant [k](#k-constant) to the
elementary charge [qe](#qe-constant). It has the exact value `kq=k/qe`.


## Physics constants

### ⅉ constant

Notation often used in physics for the imaginary verifying  `ⅉ²=-1`.

### c constant

Speed of light in vaccuum, a universal physical constant that is exactly equal
to 299,792,458 metres per second (by definition of the metre).

According to the [special theory of relativity](http://en.wikipedia.org/wiki/Special_relativity),
`c` is the upper limit for the speed at which conventional matter or energy
(and thus any signal carrying information) can travel through space.

The [theory of incomplete measurements](http://physics.dinechin.org) presents
space and time as having no existence on their own. We only derive them from
*measurements* of distance and duration made using photons. In that viewpoint,
`c` is the limit of what can be measured using photons. Anything travelling
faster than light is indistinguishable from anti-matter. This is illustrated by
the thought experiment known (or not) as the *bat and the supersonic jet*.
This little tidbit is only mentioned here because this particular theory was
devised by the primary author of DB48X.

### G constant

The gravitational constant is an empirical physical constant involved in the
calculation of gravitational effects in Sir Isaac Newton's law of universal
gravitation and in Albert Einstein's theory of general relativity.

According to Newton's law of universal gravitation, the magnitude of the
attractive force `F` between two bodies each with a spherically symmetric
density distribution is directly proportional to the product of their masses,
`m₁` and `m₂`, and inversely proportional to the square of the distance, `r`,
directed along the line connecting their centres of mass [CODATA](#codata):

![Newton Gravitation](img/NewtonGravitation.bmp)

### g constant

Acceleration of Earth gravity, equivalent to the free-fall acceleration.
Its value is exactly defined by convention as `g=9.80665_m/s²`.  It is also
a comparative units for the common concept of g-force.

### σ constant

Stefan-Boltzmann constant, the factor of proportionality in the Stefan-Boltzmann
law describing the intensity of the thermal radiation emitted by matter in terms
of that matter's temperature.

For an ideal absorber/emitter or black body, the Stefan–Boltzmann law states
that the total energy radiated per unit surface area per unit time (also known
as the radiant exitance) is directly proportional to the fourth power of the
black body's temperature, T:`M°=σ·T⁴`. [CODATA](#codata)

### ε₀ constant

Vacuum permittivity, commonly denoted ε₀ (pronounced "epsilon nought" or
"epsilon zero"), is the value of the absolute dielectric permittivity of
classical vacuum. It may also be referred to as the permittivity of free space,
the electric constant, or the distributed capacitance of the vacuum. It is an
ideal (baseline) physical constant. It is a measure of how dense of an electric
field is "permitted" to form in response to electric charges and relates the
units for electric charge to mechanical quantities such as length and force.
Its value depends on the vaccuum permeability constant `μ0`. [CODATA](#codata)

### μ₀ constant

The vacuum magnetic permeability (variously vacuum permeability, permeability of
free space, permeability of vacuum, magnetic constant) is the magnetic
permeability in a classical vacuum. It is a physical constant, conventionally
written as μ₀ (pronounced "mu nought" or "mu zero"). It quantifies the strength
of the magnetic field induced by an electric current. Its value depends on the
fine structure constant measurement `α`. [CODATA](#codata)

### ke constant

Coulomb constant as it appears in the expression of the Coulomb force:
`Fe=ke·q1·q2/r^2`. Its value depends on the vacuum permittivity `ε0`. [CODATA](#codata)

### Z₀ constant

Vaccuum characteristic impedance, also called impedance of free space.

This constant relates the magnitudes of the electric and magnetic fields of
electromagnetic radiation travelling through free space: `Zo=|E|/|H|`. Its
value depends on the vacuum permittivity `μ0`. [CODATA](#codata)

### ε₀q constant

Ratio of the vacuum permittivity to the elementary charge: `ε₀q=ε₀/qe`.
Its uncertainty is inherited from `ε₀`. [CODATA](#codata)

### qε₀ constant

Product of the vacuum permittivity by the elementary charge: `qε₀=ε₀·qe`.
Its uncertainty inherited from `ε₀`. [CODATA](#codata)

### I₀ constant

The reference sound intensity corresponds to the level of the intensity
of a sound relative to a minimal reference value, most often expressed
in decibels `dB`. `I₀=1E-12 w/m²` corresponds then to `β=0_dB` in the
definition `β=10·LOG10(I/I₀)`. By convention it has an exact value.


## Mass constants

### u constant

Unified atomic mass unit.

A unit of mass defined as one twelfth of the mass of an unbound neutral atom of
carbon-12 in its nuclear and electronic ground state and at rest. It is a non-SI
unit accepted for use with SI. It is identical to the
[Dalton](#dalton constant). [CODATA](#codata)

### me constant

Electron mass. In particle physics, the electron mass is the mass of a
stationary electron, also known as the invariant mass of the electron and it is
one of the fundamental constants of physics. Its value is closely related to
the unit mass measurement through the electron relative atomic mass:
`me=u·Ar(e)` where `Ar(e)`, i.e. [Are](#Are-constant), is determined
iteratively by frequency measurements using Penning trap. Its value can also be
calculated with the fine structure constant `α` and the Rysberg constant `R∞`. [CODATA](#codata)

### mn constant

Neutron mass measurement. Its value is determined experimentally by mass
spectrometry. [CODATA](#codata)

### mp constant

Proton mass measurement which is the hydrogen H-1 nucleus. Its value is
determined by particle drag race experiments. [CODATA](#codata)

### mH constant

Hydrogen mass measurement. The electrically neutral hydrogen H-1
atom contains a single positively charged proton in the nucleus, and
a single negatively charged electron bound to the nucleus by the Coulomb
force. Its value is measured by spectrometry.

Its value can be determined from the calculation of the electron rest mass `me`
and the measurement of the electron relative atomic mass `Ar(e)`
[Are-constant](#Are-constant) (that is, the mass of electron divided by the
atomic mass constant). [CODATA](#codata)

### mD constant

Mass of the neutral deuterium atom which is a stable isotope of hydrogen.
The electrically neutral deuterium atom H-2 (also known as heavy hydrogen)
contains a single positively charged proton and a neutron in the nucleus,
and a single negatively charged electron bound to the nucleus by the
Coulomb force. Precise measurements of deuterium is obtained by
spectrometry. [CODATA](#codata)

### mT constant

Mass of the neutral tritium atom which is an unstable isotope of hydrogen
H-3 . Its nucleus contains a single positively charged proton and two
neutrons, surrounded by a single negatively charged electron bound to
the nucleus by the Coulomb force. Its mass is measured by spectrometry. [CODATA](#codata)

### mHe constant

Mass of the neutral helium atom. The electrically neutral helium atom
He-4 contains two positively charged protons and two neutrons, and two
negatively charged electrons bound to the nucleus by the Coulomb force.
Its mass is measured by spectrometry. [CODATA](#codata)

### mμ constant

Mass of the muon which is an unstable elementary particle similar
to the electron (both are classified as leptons), with an electric
charge of `−qe` and spin -1/2, but with a much greater mass. Its mass
is evaluated from energy conservation budget in pair creation reaction. [CODATA](#codata)

### mτ constant

Mass of the tau which is an elementary particle similar to the
electron (both are classidief as leptons), with an electric charge
of `−qe` and spin -1/2, but it is heaviest of leptons. Its mass is
evaluated from energy conservation budget in pair creation reaction. [CODATA](#codata)

### mpme constant

Dimensionless ratio between the mass of the proton `mp` and the mass
of the electron `me`. Currently, the most precise measurements of the
charge-to-mass ratio of a proton still use a magnetic field like Thompson
did, but rely on measuring (cyclotron) frequencies rather than deflection. [CODATA](#codata)

### Are constant

Electron relative atomic mass. The experimental value of the electron
relative atomic mass is an important dimensionless constant (usually noted
as `Ar(e)`) which is needed to calculate the unit mass value `u`
[u-constant](#u-constant). According to COTATA2022, it is a quantity which
is determined iteratively by frequency measurements using Penning trap. [CODATA](#codata)

### mQtop constant

The top quark is the heaviest known elementary particle and decays before
it can hadronize. Its mass is measured at particle colliders via decay
product reconstruction. [Particle Data Group 2024](#particle-data-group-2024)

### mQbot constant

Bottom quark mass (also called beauty quark). It is a heavy quark whose
mass is determined from quarkonium spectroscopy and lattice QCD
calculations. [Particle Data Group 2024](#particle-data-group-2024)

### mQcha constant

Charm quark mass. A heavy quark whose mass is determined from charmonium
spectroscopy and QCD sum rules. [Particle Data Group 2024](#particle-data-group-2024)

### mQstr constant

Strange quark mass. A light quark whose mass is determined from QCD sum
rules and lattice calculations. Value in `MeV/c²`. [Particle Data Group 2024](#particle-data-group-2024)

### mQdwn constant

Down quark mass. One of the two lightest quarks forming ordinary matter.
Its mass is determined from lattice QCD calculations. Value in `MeV/c²`. [Particle Data Group 2024](#particle-data-group-2024)

### mQup constant

Up quark mass. The lightest quark and a constituent of protons and neutrons.
Its mass is determined from lattice QCD calculations. Value in `MeV/c²`. [Particle Data Group 2024](#particle-data-group-2024)

### mνmax constant

Maximum neutrino mass upper bound from the KATRIN experiment (April 2025),
expressed in `eV/c²`. Neutrino mass is non-zero but extremely small; only
upper bounds are currently measurable. [Particle Data Group 2024](#particle-data-group-2024)

### mνmin constant

Minimum neutrino mass lower bound consistent with neutrino oscillation
experiments, expressed in `eV/c²`. [Particle Data Group 2024](#particle-data-group-2024)

### mHig constant

Combined measurement of the Higgs boson mass from the `H → γγ` and
`H → ZZ* → 4ℓ` (4 leptons) decay channels with the ATLAS detector using 7,
8 and 13 TeV proton-proton collision data. [Reference 15](#reference-15)

### mW constant

The new 2026 CMS measurement, the W boson mass is considered the most precise
single measurement to date at the LHC and strongly confirms the Standard Model,
decreasing the likelihood that the 2022 CDF II result indicated new physics. [Reference 16](#reference-16)

### mZ° constant

The measured value of the Z° boson mass is determined primarily by the scan
method performed at e+e- colliders (LEP) using: resonance scan, cross-section
mapping & Breit-Wigner fit. [Reference 17](#reference-17)


## Electromagnetism

### qe constant

The elementary electric charge is a fundamental physical constant, defined as
the electric charge carried by a single proton or, equivalently, the magnitude
of the negative electric charge carried by a single electron, which has charge
`−qe`.

In the SI system of units, the value of the elementary charge is exactly defined
as `qe=1.602176634⁳⁻¹⁹` coulombs. Since the 2019 redefinition of SI base units,
the seven SI base units are defined by seven fundamental physical constants, of
which the elementary charge is one. As a consequence of this change, the value
of that constant in DB48X differs from the value in the HP50G, which named it q,
with value `1.60217733⁳⁻¹⁹` coulombs. [CODATA](#codata)

### λ₀ constant

Photon wavelength. Photon energy can be expressed using any unit of energy
such as the electronvolt (eV) or the Joule (J). For short wavelength sources,
researchers often discuss photon energies in units of eV (or keV for hard
X-rays) out of convenience.  The SI definition for 1 eV derives from the
[definitional value of the electron charge](#me-constant). Photon energy `E`
in eV can be computed from wavelength `λ` in nm as: `E=λ0/λ`. This is an
exact constant.

### f₀ constant

Photon frequency. This is the exact frequency associated to the
[photon wavelength λ0][#λ₀-constant].

### ge constant

Electron g-factor. It is a dimensionless quantity that characterizes
the magnetic moment and angular momentum of an electron. It is the
ratio of the magnetic moment (or, equivalently, the gyromagnetic ratio)
of the electron to that expected of a classical particle of the same
charge and angular momentum. The electron g-factor is one of the most
precisely measured values in physics. [CODATA](#codata)

### R∞ constant

Rydberg constant. In spectroscopy, the Rydberg constant is a physical
constant relating to the electromagnetic spectra of an atom. The constant
first arose as an empirical fitting parameter in the Rydberg formula for
the hydrogen spectral series, but Niels Bohr later showed that its value
is related to more fundamental constants according to his model of the
atom. The Rydberg constant value is inferred from measurements of atomic
transition frequencies in three different atoms (hydrogen, deuterium,
and antiprotonic helium). [CODATA](#codata)

### qme constant

Ratio between the electron charge `qe` and its mass `me`. The uncertainty
of `qme` dépends on the one of `me`. [CODATA](#codata)

### μe constant

Electron magnetic moment. The electron magnetic moment, or more
specifically the electron magnetic dipole moment, is the magnetic
moment of an electron resulting from its intrinsic properties of spin
and electric charge. Its angular momentum comes from two types of
rotation: spin and orbital motion. Therefore an external magnetic field
exerts a torque on the electron magnetic moment revealing its existence.
It's a measured quantity. [CODATA](#codata)

### μp constant

Proton magnetic moment. It is the magnetic dipole moment of the proton
resulting from its intrinsic properties of spin and electric charge. Its
angular momentum comes from two types of rotation: spin and orbital motion.
Therefore an external magnetic field exerts a torque on the proton magnetic
moment revealing its existence. It's a measured quantity.

### μn constant

Neutron magnetic moment. It is the magnetic dipole moment of the meutron
resulting from its intrinsic properties of spin. Normally it sould be
zero for an elementary neutral particle because of zero charge. The fact
that it was non-vanishing prooves that the neutron is a composite particle.
Its angular momentum comes from two types of rotation: spin and orbital
motion. Therefore an external magnetic field exerts a torque on the
neutron magnetic moment revealing its existence.
It's a measured quantity. [CODATA](#codata)

### μμ constant

Muon magnetic moment. It is the magnetic dipole moment of the meutron
resulting from its intrinsic properties of spin and electric charge.
Its angular momentum comes from two types of rotation: spin and orbital
motion. Therefore an external magnetic field exerts a torque on the muon
magnetic moment revealing its existence. It's a measured quantity. [CODATA](#codata)

### μB constant

Bohr magneton. In atomic physics, the Bohr magneton is a physical constant
and the natural unit for expressing the magnetic moment of an electron caused
by its orbital or spin angular momentum. In SI units, the Bohr magneton
depends on the electron mass `me`. [CODATA](#codata)

### μN constant

The nuclear magneton is a physical constant of magnetic moment. It
is the standard unit used to measure the magnetic dipole moment of
atomic nuclei and nucleons (protons and neutrons), essentially acting
as a scale to quantify their magnetic strength. Defined in SI units,
it depends on the measured value of the proton mass `mp`. [CODATA](#codata)

### γe constant

Electron gyromagnetic ratio. It is the ratio of the electron's magnetic
moment to its angular momentum. It can be used to determine the direction
of precession and the resonance frequency of an electron in a magnetic
field. Its value depends on the electron magnetic moment `μe`. [CODATA](#codata)

### γp constant

Proton gyromagnetic ratio. It is the ratio of the proton's magnetic
moment to its angular momentum. It can be used to determine the direction
of precession and the resonance frequency of a proton in a magnetic field.
Its value depends on the proton magnetic moment `μp`. [CODATA](#codata)

### γn constant

Neutron gyromagnetic ratio. It is the ratio of the Neutron's magnetic
moment to its angular momentum. It is a characteristic of the neutron's
nuclear spin and its sign determines the direction of precession. Its
value depends on the neutron magnetic moment `μn`. [CODATA](#codata)

### c1 constant

First radiation constant. This constant appears in the Radiance
expression of the Planck's law: `Bλ(λ;T)=c1/λ^5/EXPM1(c2/λT)`.
It's an exact constant. [CODATA](#codata)

### c2 constant

Second radiation constant. This constant appears in the Radiance
expression of the Planck's law: `Bλ(λ;T)=c1/λ^5/EXPM1(c2/λT)`.
It's an exact constant. [CODATA](#codata)

### c3 constant

Wien's constant also knowm as the third radiation constant. In physics, Wien's
displacement law states that the black-body radiation curve for different
temperatures will peak at different wavelengths that are inversely proportional
to the temperature. The shift of that peak is a direct consequence of the
Planck radiation law, which describes the spectral brightness or intensity of
black-body radiation as a function of wavelength at any given temperature.
However, it had been discovered by German physicist Wilhelm Wien several years
before Max Planck developed that more general equation, and describes the
entire shift of the spectrum of black-body radiation toward shorter
wavelengths as temperature increases.

Formally, the wavelength version of Wien's displacement law states that the
spectral radiance of black-body radiation per unit wavelength, peaks at the
wavelength `λpeak=c3/T` where `T` is absolute temperature. From a theoretic
expression (using Lambert W function), it's an exact constant. [CODATA](#codata)

### c3f constant

Wien's frequency constant is the frequency version of the third radiation
constant. In physics, Wien's frequency displacement law states that the
black-body radiation curve for different temperatures will peak at different
frequencies that are directly proportional to the temperature. The shift of
that peak is a direct consequence of the Planck radiation law, which describes
the spectral brightness or intensity of black-body radiation as a function
of frequency at any given temperature.

Formally, the frequency version of Wien's displacement law states that the
spectral radiance of black-body radiation per unit frequency, peaks at the
frequency `fpeak=Ⓒc3f·T` where `T` is absolute temperature. From a theoretic
expression (using Lambert W function), it's an exact constant. [CODATA](#codata)

### Rk constant

Von Klitzing constant. It appears in the expression of the Hall resistance
`Rxy=Rk/ν` (`ν` being either an integer or a fraction) of the quantum Hall
effect, a quantized version of the Hall effect which is observed in
two-dimensional electron systems subjected to low temperatures and strong
magnetic fields. It's an exact constant. [CODATA](#codata)

### G0 constant

Conductance quantum constant. It is the quantized unit of electrical
conductance. It is required when measuring the conductance of a quantum
point contact, and also, it appears explicitly in the Landauer formula:
`G(μ)=G0·ΣTn(μ) sum over n` which relates the electrical conductance of
a quantum conductor to its quantum properties. It's an exact constant. [CODATA](#codata)

### G0F constant

Fermi reduced coupling constant. It is a fundamental physical constant
that represents the strength of the weak nuclear interaction, essentially
indicating how readily particles can interact via the weak force; a
larger value signifies a stronger interaction, and it is a key parameter
in the Standard Model of particle physics, primarily used to calculate
the decay rates of particles involved in weak interactions like beta
decay. It's a measured quantity. [CODATA](#codata)

### ø constant

Magnetic flux quantum. The (superconducting) magnetic flux quantum is a
combination of fundamental physical constants: the Planck constant `h`
and the electron charge `qe`. Its value is, therefore, the same for
any superconductor. It's an exact constant. [CODATA](#codata)

### KJ constant

Josephson constant. The Josephson constant is a constant of
proportionality that relates the potential difference across a
Josephson junction to the frequency of irradiation. It's also
the inverse of the magnetic flux quantum `ø`. It's an exact
constant. [CODATA](#codata)

### Kc constant

Quantum of circulation constant. It represents the discrete unit
of circulation in a superfluid, meaning that such circulation around
a vortex can only occur in multiples of this value. The existence
of quantum vortices was first predicted by Lars Onsager in 1949 in
connection with superfluid helium. It is defined as the ratio of
Planck's constant `h` to the mass of the relevant particle `m`
chosen here as the electron. [CODATA](#codata)

## Size

### re constant

The classical electron radius. Through the Bohr radius `a0`, it
depends on fine structure constant `α`. [CODATA](#codata)

### rp constant

Proton charge radius. A direct measure of the proton radius. Since 2010,
the measure was done using either spectroscopy method with muonic hydrogen,
and then with deuterium atom, or either using a more recent electron-proton
scattering experiment. [CODATA](#codata)

### a0 constant

Bohr radius. The Bohr radius is a physical constant, approximately equal to
the most probable distance between the nucleus and the electron in a hydrogen
atom in its ground state. Its value depends on the vacuum electric
permittivity `ε0`. [CODATA](#codata)


## Scattering

### σe constant

The Thomson cross-section. This type of scattering is valid when the field
energy `h·ν` is much less than the rest mass of the electron `m0·c^2`, the
electric field of the incident wave accelerates the charged target-particle,
causing it, in turn, to emit radiation at the same frequency `ν` as the
incident wave, hence the scattering of the wave. Through the classical
electron radius `re`, its value depends on fine structure constant `α`. [CODATA](#codata)

### λc constant

Electron Compton wavelength. The Compton wavelength is a quantum
mechanical property of a particle, defined as the wavelength of a
photon whose energy is the same as the rest energy of that particle
(based on the mass–energy equivalence). The standard Compton wavelength
`λ` of a particle of mass `m` is given by `λ=h/(m·c)`. Since it is
defined here for the electron, it depends on the value of the electron
mass `me`. [CODATA](#codata)

### λcp constant

Proton Compton wavelength. The Compton wavelength is a quantum
mechanical property of a particle, defined as the wavelength of
a photon whose energy is the same as the rest energy of that particle
(based on the mass–energy equivalence). The standard Compton wavelength
`λ` of a particle of mass `m` is given by `λ=h/(m·c)`. Since it is
defined here for the proton, it depends on the measured value of the
proton mass `mp`. [CODATA](#codata)

### λcn constant

Neutron Compton wavelength. The Compton wavelength is a quantum
mechanical property of a particle, defined as the wavelength of
a photon whose energy is the same as the rest energy of that particle
(based on the mass–energy equivalence). The standard Compton wavelength
`λ` of a particle of mass `m` is given by `λ=h/(m·c)`. Since it is
defined here for the neutron, it depends on the measured value of the
neutron mass `mn`. [CODATA](#codata)

### λcμ constant

Muon Compton wavelength. The Compton wavelength is a quantum
mechanical property of a particle, defined as the wavelength of
a photon whose energy is the same as the rest energy of that particle
(based on the mass–energy equivalence). The standard Compton wavelength
`λ` of a particle of mass `m` is given by `λ=h/(m·c)`. Since it is
defined here for the muon particle, it depends on the measured value of
the muon mass `mμ`. [CODATA](#codata)

### λcτ constant

Tau Compton wavelength. The Compton wavelength is a quantum
mechanical property of a particle, defined as the wavelength of
a photon whose energy is the same as the rest energy of that particle
(based on the mass–energy equivalence). The standard Compton wavelength
`λ` of a particle of mass `m` is given by `λ=h/(m·c)`. Since it is
defined here for the tau particle, it depends on the measured value of
the tau mass `mτ`. [CODATA](#codata)


## Quantum constants

### h constant

The Planck constant is a fundamental physical constant that appears in quantum
mechanics. A photon's energy is equal to its frequency multiplied by the Planck
constant (`E=h·ν`), and the wavelength of a matter wave equals the Planck
constant divided by the associated particle momentum (`λ=h/p`). Since the 2019
SI redefinition, it has an exact value. [CODATA](#codata)

### ℏ constant

The reduced Planck constant, ℏ, also known as the Dirac Constant, is
exactly defined as `ℏ=h/2π`. [CODATA](#codata)

### α constant

Fine-structure constant. In physics, the fine-structure constant, also known as
the Sommerfeld constant, commonly denoted by α (the Greek letter alpha), is a
fundamental physical constant which quantifies the strength of the
electromagnetic interaction between elementary charged particles. It is measured
by observing the recoil frequency of atoms, like cesium or rubidium, when they
absorb a photon, essentially gauging how strongly the atoms recoil, which
provides an accurate determination of the constant's value using high precision
measurements achieved through techniques like matter-wave interferometry.

It is a dimensionless quantity, independent of the system of units used, which
is related to the strength of the coupling of an elementary charge `qe` with
the electromagnetic field. [CODATA](#codata)

### ΔfCs constant

Caesium (Cs) hyperfine transition. It is the transition between the two
hyperfine ground states of the caesium atom. The frequency `ΔfCs` of this
transition is used to define the second as the official time unit of the
International System of Units (SI): one second is therefore the duration
of exactly 9192631770 cycles of this radiation. It represents the "tick"
of an extremely accurate atomic clock based on the properties of the
Caesium atom. By convention this constant is exact. [CODATA](#codata)

### θw constant

The weak mixing angle or Weinberg angle. It is a parameter in the
Weinberg–Salam theory of the electroweak interaction, part of the
Standard Model of particle physics. It is the angle by which spontaneous
symmetry breaking rotates the original W0 and B0 vector boson plane,
producing as a result the Z0 boson, and the photon. Its value is
calculated from the mesurement of the following dimensionless parameter
for the W and Z bosons: `(sinθw)^2 = 1 - (mW/mZ)^2 = 0.22305 ± 0.00023`. [CODATA](#codata)

### Lpl constant

Planck length unit. As an attempt to devise a universal and natural units
system, the Planck units are combinations of basic universal constants. It
is the smallest distance that can be measured, and it represents the
scale at which quantum gravity effects become dominant. It is the distance
travelled by light during one Planck time `Tpl`. Its value depends on the
measured value of the gravitational constant `G`. [CODATA](#codata)

### Tpl constant

Planck time unit. As an attempt to devise a universal and natural units
system, the Planck units are combinations of basic universal constants.
It is the shortest time interval that can be measured and it is fundamental
in the study of the universe beginning. It is the time required for light
to travel one Planck length `Lpl`. Its value depends on the measured value
of the gravitational constant `G`. [CODATA](#codata)

### Mpl constant

Planck mass unit. As an attempt to devise a universal and natural units
system, the Planck units are combinations of basic universal constants.
It can be viewed as the mass of a black hole with a Swarzhchild radius
of 2 Planck lengths (`rs=2·Lpl`) or, the minimum mass of a black hole
is one half of the Planck mass, the latter having a Planck Length radius.
Its value depends on the measured value of the gravitational constant
`G`. [CODATA](#codata)

### Epl constant

Planck energy unit. As an attempt to devise a universal and natural units
system, the Planck units are combinations of basic universal constants.
According to the mass-energy equivalence: `Epl=Mpl·c^2` it is the energy
equivalent to the Planck mass. Considered to be the smallest possible unit
of energy, which is theoretically meaningful within the framework of quantum
gravity, where both effects of quantum mechanics and general relativity
become significant; and also the energy scale at which the universe is thought
to have existed at times near the start of the Big Bang, characterized by
extremely high densities and temperatures. Its value depends on the measured
value of the gravitational constant `G`. [CODATA](#codata)

### T°pl constant

Planck temperature unit. As an attempt to devise a universal and natural
units system, the Planck units are combinations of basic universal constants.
It is the highest temperature that conventional physics can describe. It's a
fundamental limit of quantum mechanics and is considered the temperature of
the universe during the Big Bang when quantum gravity effects became
dominant. Its value depends on the measured value of the gravitational
constant `G`. [CODATA](#codata)

### qpl — Planck charge

The Planck charge unit is the natural unit of electric charge in Planck units,
defined as `qpl=√(4π·ε₀·ℏ·c)`. Its value depends on the vacuum permittivity `ε₀`. [CODATA](#codata)

### ppl — Planck momentum

The Planck momentum unit is the natural unit of momentum in Planck units, defined
as `ppl=√(ℏ·c³/G)`. Its value depends on the gravitational constant `G`. [CODATA](#codata)

### Fpl — Planck force

The Planck force unit is the natural unit of force in Planck units, defined as
`Fpl=c⁴/G`. It is the maximum force postulated in general relativity. Its value
depends on the gravitational constant `G`. [CODATA](#codata)

### ρvpl — Planck density

The Planck density unit is the natural unit of density in Planck units, defined
as `ρvpl=c⁵/(ℏ·G²)`. Its value depends on the gravitational constant `G`. [CODATA](#codata)

### Eh constant

Hartree energy constant. It is a unit of energy used in atomic physics
and computational chemistry, which is also used in molecular orbital
calculations. It is approximately the negative electric potential energy
of an electron in a hydrogen atom's ground state, and also approximately
twice the ionization energy of a hydrogen atom. Its value depends on the
measured value of the Rydberg constant `R∞`. [CODATA](#codata)


## Astronomy

## Mercury ☿

### GM☿ — Mercury gravitational parameter

It is measured by radio tracking of spacecraft (Mariner 10, MESSENGER). [Reference 4](#reference-4)

### Req☿ — Mercury equatorial radius

It is measured by radar altimetry and spacecraft imaging. [Reference 22](#reference-22)

### Rp☿ — Mercury polar radius

Mercury's polar radius is also measured, since Mercury being essentially
spherical; equatorial and polar radii are equal within measurement precision. [Reference 22](#reference-22)

### f☿ — Mercury oblateness

Mercury's oblateness is calculated from its equatorial and polar radii measurement.
Due to Mercury's near-spherical shape, this value is effectively zero. [Reference 22](#reference-22)

### ρv☿ — Mercury mean density

It is calculated from measurement of its gravitational parameter and volume.
Its high density indicates a large metallic core. [Reference 4](#reference-4) [Reference 22](#reference-22)

### g☿ — Mercury surface gravity

It is calculated from measurement of `GM☿` and the equatorial radius
`Req☿`. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Ve☿ — Mercury escape velocity

Mercury's escape velocity at the equatorial surface is calculated from
measured quantities. It is the minimum speed needed to break free from
Mercury's gravitational pull and never return. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Prot☿ — Mercury sidereal rotation period

Mercury's sidereal rotation period is a measured quantity. It is the true time
it takes to spin 360° on its axis. Mercury is in a 3:2 spin-orbit resonance with
the Sun, rotating three times for every two orbits. [Reference 22](#reference-22)

### ϵ☿ — Mercury axial tilt

Mercury's axial tilt (obliquity) is measured from spacecraft observations.
The nearly zero value is consistent with its spin-orbit resonance. It is the
angle of the planet's rotational axis relative to its orbital plane. [Reference 22](#reference-22)

### a☿ — Mercury semi-major axis

Mercury's orbital semi-major axis is a measured quantity. It is half the length
of the longest diameter of the elliptical orbit, which is also the average
distance from one of the radii to the orbit. [Reference 23](#reference-23)

### e☿ — Mercury orbital eccentricity

Mercury's orbital eccentricity is a measured quantity, it is the largest of
all major planets. When different from 0, it is a measure of how much a
planet's elliptical path deviates from a perfect circle. [Reference 23](#reference-23)

### i☿ — Mercury orbital inclination

Mercury's orbital inclination with respect to the ecliptic plane is a measured
angle. [Reference 23](#reference-23)

### Ω☿ — Mercury longitude of ascending node

Mercury's longitude of the ascending node is a measured angle quantifying the
horizontal orientation of the orbit. It represents where Mercury crosses
northward through Earth's orbital plane. [Reference 23](#reference-23)

### ω☿ — Mercury argument of perihelion

It is the measured angle between the ascending node (where the orbit crosses
the reference plane northward) and the perihelion (the closest point to the
Sun), measured in the direction of motion. [Reference 23](#reference-23)

### T₀☿ — Mercury last perihelion passage

Mercury's most recent time of perihelion passage is a measured quantity.
Mercury passes perihelion approximately four times per year. Value in JDN
(Julian Day Number, Gregorian calendar). It is the most recent point in its
orbit when it was closest to the Sun. [Materials 20](#materials-20)


## Venus ♀

### GM♀ — Venus gravitational parameter

Venus's gravitational parameter, measured by radio tracking of spacecraft
(Magellan, Venus Express). [Reference 4](#reference-4)

### Req♀ — Venus equatorial radius

Venus's equatorial radius is a measured quantity. Venus is essentially spherical.
[Reference 22](#reference-22)

### Rp♀ — Venus polar radius

Venus's polar radius is a measured quantity, essentially equal to the equatorial
radius. [Reference 22](#reference-22)

### f♀ — Venus oblateness

Venus's oblateness obtained from measurement is effectively zero given its near-
spherical shape. [Reference 22](#reference-22)

### ρv♀ — Venus mean density

Venus's mean density, calculated from measurement of its gravitational parameter and
volume. [Reference 4](#reference-4) [Reference 22](#reference-22)

### g♀ — Venus surface gravity

Venus's surface gravity is calculated from measurement of `GM♀` and `Req♀`.
[Reference 4](#reference-4) [Reference 22](#reference-22)

### Ve♀ — Venus escape velocity

Venus's escape velocity at the equatorial surface is calculated from measured
quantities. It is the minimum speed needed to break free from Venus’s gravitational
pull and never return. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Prot♀ — Venus sidereal rotation period

Measured. Venus's sidereal rotation period is the true time it takes to spin 360°
on its axis. Venus rotates retrograde (opposite to its orbital motion), with a
rotation period longer than its orbital period. [Reference 22](#reference-22)

### ϵ♀ — Venus axial tilt

Measured. Venus's axial tilt. The near-180° value reflects its retrograde rotation.
It is the angle of the planet's rotational axis relative to its orbital plane. [Reference 22](#reference-22)

### a♀ — Venus semi-major axis

Measured. Venus's orbital semi-major axis is half the length of the longest
diameter of the elliptical orbit, which is also the average distance from one
of the radii to the orbit. [Reference 23](#reference-23)

### e♀ — Venus orbital eccentricity

Measured. Venus's orbital eccentricity, nearly circular (therefore, near 0). [Reference 23](#reference-23)

### i♀ — Venus orbital inclination

Venus's orbital inclination with respect to the ecliptic plane is a measured angle.
[Reference 23](#reference-23)

### Ω♀ — Venus longitude of ascending node

Venus's longitude of the ascending node is a measured angle quantifying the
horizontal orientation of the orbit. It represents where Venus crosses northward
through Earth's orbital plane. [Reference 23](#reference-23)

### ω♀ — Venus argument of perihelion

It is the measured angle between the ascending node (where the orbit crosses
the reference plane northward) and the perihelion (the closest point to the Sun),
measured in the direction of motion. [Reference 23](#reference-23)

### T₀♀ — Venus last perihelion passage

Measured. Venus's most recent time of perihelion passage. Venus passes perihelion
approximately twice per year. Value in JDN. It is the most recent point in its
orbit when it was closest to the Sun. [Materials 20](#materials-20)


## Earth ♁

### GM♁ — Earth gravitational parameter

Exact nominal value (IAU 2015). Earth's gravitational parameter. An exact nominal
value defined by the IAU (2015). [Particle Data Group 2023](#particle-data-group-2023)

### Req♁ — Earth equatorial radius

Exact nominal value (IAU 2015). Earth's equatorial radius. An exact nominal value
defined by the IAU (2015). [Particle Data Group 2023](#particle-data-group-2023)

### Rp♁ — Earth polar radius

Exact nominal value (IAU 2015). Earth's polar radius. An exact nominal value
defined by the IAU (2015). [Particle Data Group 2023](#particle-data-group-2023)

### f♁ — Earth oblateness

Calculated from nominal value. Earth's oblateness, reflecting its equatorial
bulge due to rotation. Calculated from `Req♁` and `Rp♁`. When greater than 0, it
describes how much a celestial body deviates from a perfect sphere, forming an
"oblate spheroid". [Particle Data Group 2023](#particle-data-group-2023)

### ρv♁ — Earth mean density

Calculated from nominal value. Earth's mean density, calculated from its gravitational
parameter and volume. [Particle Data Group 2023](#particle-data-group-2023)

### g♁ — Earth surface gravity

Calculated from nominal value. Earth's surface gravity at the equator, calculated
from `GM♁` and `Req♁`. [Particle Data Group 2023](#particle-data-group-2023)

### Ve♁ — Earth escape velocity

Calculated from nominal value, it is valid at the equatorial surface. It is the
minimum speed needed to break free from Earth's gravitational pull and never return. [Particle Data Group 2023](#particle-data-group-2023)

### Prot♁ — Earth sidereal rotation period

Measured. Earth's sidereal rotation period (one sidereal day), the time for one
rotation relative to the fixed stars. [Reference 24](#reference-24)

### ϵ♁ — Earth axial tilt

Measured. Earth's axial tilt (obliquity of the ecliptic), the primary driver of the
seasons. It is the angle of the planet's rotational axis relative to its orbital plane.
[Reference 24](#reference-24)

### a♁ — Earth semi-major axis

Exact nominal value (1 AU by definition). Earth's orbital semi-major axis, defined
as exactly 1 astronomical unit (AU). It is half the length of the longest diameter
of the elliptical orbit, which is also the average distance from one of the radii
to the orbit. [Particle Data Group 2023](#particle-data-group-2023)

### e♁ — Earth orbital eccentricity

Measured. Earth's orbital eccentricity (low since the orbit is nearly circular) . [Reference 23](#reference-23)

### i♁ — Earth orbital inclination

Measured (reference plane — near zero relative to the J2000.0 ecliptic plane).
The Earth's orbital inclination is often defined by convention with respect to
the reference plane (effectively zero) as being the ecliptic plane (in such case
this angle is zero). [Reference 23](#reference-23)

### Ω♁ — Earth longitude of ascending node

Earth’s longitude of the ascending node is a measured angle quantifying the
horizontal orientation of the orbit. It represents where the Earth crosses
northward through Earth's orbital plane. [Reference 23](#reference-23)

### ω♁ — Earth argument of perihelion

It is the measured angle between the ascending node (where the orbit crosses
the reference plane northward) and the perihelion (the closest point to the
Sun), measured in the direction of motion. [Reference 23](#reference-23)

### T₀♁ — Earth perihelion passage

Measured. Earth's most recent time of perihelion passage (early January each
year). Value in JDN. It is the most recent point in its orbit when it was
closest to the Sun. [Materials 20](#materials-20)


## Moon ☽

### GM☽ — Moon gravitational parameter

Measured. Moon's gravitational parameter, measured by lunar laser ranging
and spacecraft radio tracking. [Reference 4](#reference-4)

### Req☽ — Moon equatorial radius

Measured. Moon's equatorial radius, measured by laser altimetry (LRO mission). [Reference 22](#reference-22)

### Rp☽ — Moon polar radius

Measured. Moon's polar radius. [Reference 22](#reference-22)

### f☽ — Moon oblateness

Calculated from measurement. Moon's oblateness, reflecting its slight equatorial
bulge. Calculated from `Req☽` and `Rp☽`. When greater than 0, it describes how
much a celestial body deviates from a perfect sphere, forming an "oblate spheroid".
[Reference 22](#reference-22)

### ρv☽ — Moon mean density

Calculated from measurement. Moon's mean density, reflecting its predominantly
silicate composition with a small iron core. [Reference 4](#reference-4) [Reference 22](#reference-22)

### g☽ — Moon surface gravity

Calculated from measurement. Moon's surface gravity, approximately one sixth
of Earth's. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Ve☽ — Moon escape velocity

Calculated from measurement, it is valid at the equatorial surface. It is
the minimum speed needed to break free from Moon's gravitational pull and
never return. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Prot☽ — Moon sidereal rotation period

Measured. Moon's sidereal rotation period is the true time it takes to spin
360° on its axis. The Moon is tidally locked to Earth, so its rotation period
equals its orbital period. [Reference 22](#reference-22)

### ϵ☽ — Moon axial tilt

Measured. Moon's axial tilt is the angle of the planet's rotational axis
relative to its orbital plane. [Reference 22](#reference-22)

### a☽ — Moon semi-major axis

Measured. Moon's orbital semi-major axis (mean Earth-Moon distance). It is half
the length of the longest diameter of the elliptical orbit, which is also the
average distance from one of the radii to the orbit. [Reference 25](#reference-25)

### e☽ — Moon orbital eccentricity

Measured. Moon's orbital eccentricity. The Moon's elliptical orbit causes
noticeable variation in its apparent size (supermoon and micromoon effects).
When different from 0, it is a measure of how much the moon's elliptical path
deviates from a perfect circle. [Reference 25](#reference-25)

### i☽ — Moon orbital inclination

Moon's orbital inclination is a measured angle with respect to the ecliptic
plane. [Reference 25](#reference-25)

### Ω☽ — Moon longitude of ascending node

Moon's longitude of the ascending node is a measured angle quantifying the
horizontal orientation of the orbit. It represents where the Moon crosses
northward through Earth's orbital plane.. [Reference 25](#reference-25)

### ω☽ — Moon argument of perigee

Measured. Moon's argument of perigee. It is the most recent point in its
orbit when it was closest to the Earth. [Reference 25](#reference-25)

### T₀☽ — Moon perigee passage

Calculated from measurement (Meeus ch.50, ±1 hour). See MPERC.txt. Moon's
most recent time of perigee passage (closest approach to Earth). The perigee
recurs approximately every 27.55 days. Computed using the Meeus algorithm
(15 terms, ±1 hour accuracy). Value in JDN. [Materials 20](#materials-20) [Materials 22](#materials-22)


## Mars ♂

### GM♂ — Mars gravitational parameter

Measured. Mars system gravitational parameter, including the contribution of
its moons Phobos and Deimos, measured by spacecraft radio tracking. [Reference 4](#reference-4)

### Req♂ — Mars equatorial radius

Measured. Mars's equatorial radius. [Reference 22](#reference-22)

### Rp♂ — Mars polar radius

Measured. Mars's polar radius. Mars is noticeably oblate due to its rotation. [Reference 22](#reference-22)

### f♂ — Mars oblateness

Calculated from measurement. When greater than 0, it describes how much a
celestial body deviates from a perfect sphere, forming an "oblate spheroid". [Reference 22](#reference-22)

### ρv♂ — Mars mean density

Calculated from measurement. Mars's mean density, intermediate between rocky
and metallic compositions. [Reference 4](#reference-4) [Reference 22](#reference-22)

### g♂ — Mars surface gravity

Calculated from measurement. Mars's surface gravity, approximately 38% of
Earth's. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Ve♂ — Mars escape velocity

Calculated from measurement, it is valid at the equatorial surface. It is the
minimum speed needed to break free from Mars's gravitational pull and never
return. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Prot♂ — Mars sidereal rotation period

Measured. Mars's sidereal rotation period is the true time it takes to spin
360° on its axis. A Martian day (sol) is very similar in length to an Earth day.
[Reference 22](#reference-22)

### ϵ♂ — Mars axial tilt
Measured. Mars's axial tilt, similar to Earth's, gives Mars comparable
seasonal variations. It is the angle of the planet's rotational axis
relative to its orbital plane. [Reference 22](#reference-22)

### a♂ — Mars semi-major axis

Measured. Mars's orbital semi-major axis is half the length of the longest
diameter of the elliptical orbit, which is also the average distance from
one of the radii to the orbit. [Reference 23](#reference-23)

### e♂ — Mars orbital eccentricity

Measured. Mars's orbital eccentricity, significantly larger than Earth's.
When different from 0, it is a measure of how much a planet's elliptical
path deviates from a perfect circle. [Reference 23](#reference-23)

### i♂ — Mars orbital inclination

Mars's orbital inclination is a measured angle with respect to the ecliptic
plane. [Reference 23](#reference-23)

### Ω♂ — Mars longitude of ascending node
Mars's longitude of the ascending node is a measured angle quantifying the
horizontal orientation of the orbit. It represents where Mars crosses
northward through Earth's orbital plane. [Reference 23](#reference-23)

### ω♂ — Mars argument of perihelion

It is the measured angle between the ascending node (where the orbit crosses
the reference plane northward) and the perihelion (the closest point to the
Sun), measured in the direction of motion. [Reference 23](#reference-23)

### T₀♂ — Mars perihelion passage

Measured. Mars's most recent time of perihelion passage. Mars's orbital
period is approximately 1.88 years. Value in JDN. It is the most recent point
in its orbit when it was closest to the Sun. [Materials 20](#materials-20)


## Jupiter ♃

### GM♃ — Jupiter gravitational parameter

Exact nominal value (IAU 2015). Jupiter system gravitational parameter.
An exact nominal value defined by the IAU (2015). [Particle Data Group 2023](#particle-data-group-2023)

### Req♃ — Jupiter equatorial radius

Exact nominal value (IAU 2015). Jupiter's equatorial radius. An exact
nominal value defined by the IAU (2015). [Particle Data Group 2023](#particle-data-group-2023)

### Rp♃ — Jupiter polar radius

Exact nominal value (IAU 2015). Jupiter's polar radius. An exact nominal
value defined by the IAU (2015). [Particle Data Group 2023](#particle-data-group-2023)

### f♃ — Jupiter oblateness

Calculated from nominal value. It is the largest of any planet due to its
rapid rotation and gaseous composition. When greater than 0, it describes
how much a celestial body deviates from a perfect sphere, forming an
"oblate spheroid". [Particle Data Group 2023](#particle-data-group-2023)

### ρv♃ — Jupiter mean density

Calculated from nominal value. Jupiter's mean density, lower than water
due to its gaseous composition. [Particle Data Group 2023](#particle-data-group-2023)

### g♃ — Jupiter surface gravity

Calculated from nominal value. Jupiter's surface gravity at the 1-bar
pressure level. [Particle Data Group 2023](#particle-data-group-2023)

### Ve♃ — Jupiter escape velocity

Calculated from nominal value, it is valid at the equatorial surface.
It is the minimum speed needed to break free from Jupiter's gravitational
pull and never return. [Particle Data Group 2023](#particle-data-group-2023)

### Prot♃ — Jupiter sidereal rotation period

Measured. Jupiter's sidereal rotation period is the true time it takes to
spin 360° on its axis (System III, based on radio emissions from its
magnetosphere). [Reference 22](#reference-22)

### ϵ♃ — Jupiter axial tilt

Measured. Jupiter's axial tilt is the angle of the planet's rotational
axis relative to its orbital plane. It is very small compared to Earth's.
[Reference 22](#reference-22)

### a♃ — Jupiter semi-major axis

Measured. Jupiter's orbital semi-major axis is half the length of the
longest diameter of the elliptical orbit, which is also the average
distance from one of the radii to the orbit. [Reference 23](#reference-23)

### e♃ — Jupiter orbital eccentricity

Measured. Jupiter's orbital eccentricity. When different from 0, it
is a measure of how much a planet's elliptical path deviates from a
perfect circle. [Reference 23](#reference-23)

### i♃ — Jupiter orbital inclination

Jupiter's orbital inclination is a measured angle with respect to the
ecliptic plane. [Reference 23](#reference-23)

### Ω♃ — Jupiter longitude of ascending node

Jupiter's longitude of the ascending node is a measured angle
quantifying the horizontal orientation of the orbit. It represents
where Jupiter crosses northward through Earth's orbital plane. [Reference 23](#reference-23)

### ω♃ — Jupiter argument of perihelion

It is the measured angle between the ascending node (where the orbit
crosses the reference plane northward) and the perihelion (the closest
point to the Sun), measured in the direction of motion. [Reference 23](#reference-23)

### T₀♃ — Jupiter perihelion passage

Measured. Jupiter's most recent time of perihelion passage. Jupiter's
orbital period is approximately 11.86 years. It is the most recent point
in its orbit when it was closest to the Sun. Value in JDN. [Reference 4](#reference-4) [Materials 21](#materials-21)


## Saturn ♄

### GM♄ — Saturn gravitational parameter

Measured. Saturn system gravitational parameter, measured by radio
tracking of the Cassini spacecraft. [Reference 4](#reference-4)

### Req♄ — Saturn equatorial radius

Measured. Saturn's equatorial radius at the 1-bar pressure level. [Reference 22](#reference-22)

### Rp♄ — Saturn polar radius

Measured. Saturn's polar radius at the 1-bar pressure level. [Reference 22](#reference-22)

### f♄ — Saturn oblateness

Calculated from measurement. It is the second largest of any planet.
When greater than 0, it describes how much a celestial body deviates
from a perfect sphere, forming an "oblate spheroid". [Reference 22](#reference-22)

### ρv♄ — Saturn mean density

Calculated from measurement. Saturn's mean density, the lowest of any
planet — less than that of water. [Reference 4](#reference-4) [Reference 22](#reference-22)

### g♄ — Saturn surface gravity

Calculated from measurement. Saturn's surface gravity at the 1-bar
pressure level. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Ve♄ — Saturn escape velocity

Calculated from measurement, it is valid at the equatorial surface.
It is the minimum speed needed to break free from Saturn's gravitational
pull and never return. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Prot♄ — Saturn sidereal rotation period

Measured. Saturn's sidereal rotation period is the true time it takes
to spin 360° on its axis (System III, based on Cassini radio
measurements). [Reference 22](#reference-22)

### ϵ♄ — Saturn axial tilt

Measured. Saturn's axial tilt, which causes its rings to appear at
varying angles from Earth, It is the angle of the planet's rotational
axis relative to its orbital plane.. [Reference 22](#reference-22)

### a♄ — Saturn semi-major axis

Measured. Saturn's orbital semi-major axis is half the length of the
longest diameter of the elliptical orbit, which is also the average
distance from one of the radii to the orbit. [Reference 23](#reference-23)

### e♄ — Saturn orbital eccentricity

Measured. Saturn's orbital eccentricity. When different from 0, it is
a measure of how much a planet's elliptical path deviates from a perfect
circle. [Reference 23](#reference-23)

### i♄ — Saturn orbital inclination

Saturn's orbital inclination is a measured angle with respect to the
ecliptic plane. [Reference 23](#reference-23)

### Ω♄ — Saturn longitude of ascending node

Saturn's longitude of the ascending node is a measured angle quantifying
the horizontal orientation of the orbit. It represents where Saturn
crosses northward through Earth's orbital plane. [Reference 23](#reference-23)

### ω♄ — Saturn argument of perihelion

It is the measured angle between the ascending node (where the orbit
crosses the reference plane northward) and the perihelion (the closest
point to the Sun), measured in the direction of motion. [Reference 23](#reference-23)

### T₀♄ — Saturn perihelion passage

Measured. Saturn's most recent time of perihelion passage. Saturn's
orbital period is approximately 29.46 years. It is the most recent point
in its orbit when it was closest to the Sun. Value in JDN. [Reference 4](#reference-4) [Materials 21](#materials-21)


## Uranus ⛢

### GM⛢ — Uranus gravitational parameter

Measured. Uranus system gravitational parameter, measured by Voyager
2 radio tracking. [Reference 4](#reference-4)

### Req⛢ — Uranus equatorial radius

Measured. Uranus's equatorial radius, measured by Voyager 2 and stellar
occultations. [Reference 22](#reference-22)

### Rp⛢ — Uranus polar radius

Measured. Uranus's polar radius. [Reference 22](#reference-22)

### f⛢ — Uranus oblateness

Calculated from measurement. When greater than 0, it describes how much
a celestial body deviates from a perfect sphere, forming an "oblate
spheroid". [Reference 22](#reference-22)

### ρv⛢ — Uranus mean density

Calculated from measurement. Uranus's mean density, characteristic of
an ice giant. [Reference 4](#reference-4) [Reference 22](#reference-22)

### g⛢ — Uranus surface gravity

Calculated from measurement. Uranus's surface gravity at the 1-bar
pressure level. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Ve⛢ — Uranus escape velocity

Calculated from measurement, it is valid at the equatorial surface.
It is the minimum speed needed to break free from Uranus's gravitational
pull and never return. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Prot⛢ — Uranus sidereal rotation period

Measured. Uranus's sidereal rotation period is the true time it takes
to spin 360° on its axis. Uranus rotates retrograde relative to its
orbital motion. [Reference 22](#reference-22)

### ϵ⛢ — Uranus axial tilt

Measured. Uranus's axial tilt is the angle of the planet's rotational
axis relative to its orbital plane. Its extreme obliquity (≈98°) means
it rotates almost on its side. [Reference 22](#reference-22)

### a⛢ — Uranus semi-major axis

Measured. Uranus's orbital semi-major axis is half the length of the
longest diameter of the elliptical orbit, which is also the average
distance from one of the radii to the orbit. [Reference 23](#reference-23)

### e⛢ — Uranus orbital eccentricity

Measured. Uranus's orbital eccentricity. When different from 0, it
is a measure of how much a planet's elliptical path deviates from a
perfect circle. [Reference 23](#reference-23)

### i⛢ — Uranus orbital inclination

Uranus's orbital inclination is a measured angle with respect to the
ecliptic plane. [Reference 23](#reference-23)

### Ω⛢ — Uranus longitude of ascending node

Uranus's longitude of the ascending node is a measured angle quantifying
the horizontal orientation of the orbit. It represents where Uranus
crosses northward through Earth's orbital plane. [Reference 23](#reference-23)

### ω⛢ — Uranus argument of perihelion

It is the measured angle between the ascending node (where the orbit
crosses the reference plane northward) and the perihelion (the closest
point to the Sun), measured in the direction of motion. [Reference 23](#reference-23)

### T₀⛢ — Uranus perihelion passage

Calculated from measurement (JPL DE440 orbital propagation). Uranus's
predicted next time of perihelion passage (~2050). Uranus's orbital
period is approximately 84 years. Last perihelion: 1966. It is the most
recent point in its orbit when it was closest to the Sun. Value in JDN.
[Reference 4](#reference-4) [Materials 21](#materials-21)


## Neptune ♆

### GM♆ — Neptune gravitational parameter

Measured. Neptune system gravitational parameter, measured by Voyager
2 radio tracking and Hubble Space Telescope astrometry of Triton. [Reference 4](#reference-4)

### Req♆ — Neptune equatorial radius

Measured. Neptune's equatorial radius, measured by Voyager 2 and stellar
occultations. [Reference 22](#reference-22)

### Rp♆ — Neptune polar radius

Measured. Neptune's polar radius. [Reference 22](#reference-22)

### f♆ — Neptune oblateness

Calculated from measurement. When greater than 0, it describes how
much a celestial body deviates from a perfect sphere, forming an
"oblate spheroid". [Reference 22](#reference-22)

### ρv♆ — Neptune mean density

Calculated from measurement. Neptune's mean density, characteristic of
an ice giant. [Reference 4](#reference-4) [Reference 22](#reference-22)

### g♆ — Neptune surface gravity

Calculated from measurement. Neptune's surface gravity at the 1-bar
pressure level. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Ve♆ — Neptune escape velocity

Calculated from measurement, it is valid at the equatorial surface. It
is the minimum speed needed to break free from Neptune's gravitational
pull and never return. [Reference 4](#reference-4) [Reference 22](#reference-22)

### Prot♆ — Neptune sidereal rotation period

Measured. Neptune's sidereal rotation period is the true time it takes
to spin 360° on its axis (System III, from Voyager 2 radio measurements).
[Reference 22](#reference-22)

### ϵ♆ — Neptune axial tilt

Measured. Neptune's axial tilt, comparable to Earth's, is the angle of
the planet's rotational axis relative to its orbital plane. [Reference 22](#reference-22)

### a♆ — Neptune semi-major axis

Measured. Neptune's orbital semi-major axis is half the length of the
longest diameter of the elliptical orbit, which is also the average
distance from one of the radii to the orbit. [Reference 23](#reference-23)

### e♆ — Neptune orbital eccentricity

Measured. Neptune's orbital eccentricity. When different from 0, it
is a measure of how much a planet's elliptical path deviates from a
perfect circle. [Reference 23](#reference-23)

### i♆ — Neptune orbital inclination

Neptune's orbital inclination is a measured angle with respect to
the ecliptic plane. [Reference 23](#reference-23)

### Ω♆ — Neptune longitude of ascending node

Neptune's longitude of the ascending node is a measured angle quantifying
the horizontal orientation of the orbit. It represents where Neptune
crosses northward through Earth's orbital plane. [Reference 23](#reference-23)

### ω♆ — Neptune argument of perihelion

It is the measured angle between the ascending node (where the orbit
crosses the reference plane northward) and the perihelion (the closest
point to the Sun), measured in the direction of motion. [Reference 23](#reference-23)

### T₀♆ — Neptune perihelion passage

Calculated from measurement (JPL DE440 orbital propagation). Neptune's
predicted next time of perihelion passage (~2042). Neptune's orbital
period is approximately 164.8 years. Last perihelion: 1876. It is the
most recent point in its orbit when it was closest to the Sun. Value
in JDN. [Reference 4](#reference-4) [Materials 21](#materials-21)


## Pluton ♇

### GM♇ — Pluto gravitational parameter

Measured. Pluto system gravitational parameter, measured by New Horizons
radio tracking. [Reference 4](#reference-4)

### Req♇ — Pluto equatorial radius

Measured. Pluto's equatorial radius, measured by New Horizons occultation
and imaging. [Reference 26](#reference-26)

### Rp♇ — Pluto polar radius

Measured. Pluto's polar radius. Pluto is essentially spherical. [Reference 26](#reference-26)

### f♇ — Pluto oblateness

Calculated from measurement. Pluto being spherical, its oblateness is
effectively zero. [Reference 26](#reference-26)

### ρv♇ — Pluto mean density

Calculated from measurement. Pluto's mean density, consistent with
a mixture of rock and ice. [Reference 4](#reference-4) [Reference 26](#reference-26)

### g♇ — Pluto surface gravity

Calculated from measurement. Pluto's surface gravity. [Reference 4](#reference-4) [Reference 26](#reference-26)

### Ve♇ — Pluto escape velocity

Calculated from measurement, it is valid at the equatorial surface.
It is the minimum speed needed to break free from Pluto's gravitational
pull and never return. [Reference 4](#reference-4) [Reference 26](#reference-26)

### Prot♇ — Pluto sidereal rotation period

Measured. Pluto's sidereal rotation period is the true time it takes to
spin 360° on its axis. Pluto rotates retrograde and is tidally locked to
its moon Charon. [Reference 26](#reference-26)

### ϵ♇ — Pluto axial tilt

Measured. Pluto's axial tilt is the angle of the planet's rotational
axis relative to its orbital plane. [Reference 26](#reference-26)

### a♇ — Pluto semi-major axis

Measured. Pluto's orbital semi-major axis is half the length of the
longest diameter of the elliptical orbit, which is also the average
distance from one of the radii to the orbit. [Reference 23](#reference-23)

### e♇ — Pluto orbital eccentricity

Measured. Pluto's orbital eccentricity, large enough to bring it inside
Neptune's orbit near perihelion. When different from 0, it is a measure
of how much a planet's elliptical path deviates from a perfect circle. [Reference 23](#reference-23)

### i♇ — Pluto orbital inclination

Pluto's orbital inclination is a measured angle with respect to the ecliptic
plane, ir is the highest of any solar system body traditionally listed. [Reference 23](#reference-23)

### Ω♇ — Pluto longitude of ascending node

Pluto's longitude of the ascending node is a measured angle quantifying
the horizontal orientation of the orbit. It represents where Pluto crosses
northward through Earth's orbital plane. [Reference 23](#reference-23)

### ω♇ — Pluto argument of perihelion

It is the measured angle between the ascending node (where the orbit crosses
the reference plane northward) and the perihelion (the closest point to the
Sun), measured in the direction of motion. [Reference 23](#reference-23)

### T₀♇ — Pluto perihelion passage

Measured. Pluto's last time of perihelion passage (1989 Sep 05). Pluto's
orbital period is approximately 248 years. It is the most recent point in
its orbit when it was closest to the Sun. Next perihelion: ~2237. Value
in JDN. [Reference 4](#reference-4) [Materials 21](#materials-21)

## Solar ☉

### M☉ — Solar mass

It is derived from the exact nominal solar gravitational parameter
`GM☉` and the measured gravitational constant `G`. [Particle Data Group 2024](#particle-data-group-2024) [Particle Data Group 2023](#particle-data-group-2023) [Reference 5](#reference-5)

### GM☉ — Solar gravitational parameter

It is an exact nominal value defined by the International Astronomical
Union (IAU 2015 Resolution B3). [Particle Data Group 2023](#particle-data-group-2023)

### L☉ — Solar luminosity

It is an exact nominal value defined by the IAU (2015). [Particle Data Group 2023](#particle-data-group-2023)

### Req☉ — Solar equatorial radius

It is an exact nominal value defined by the IAU (2015). [Particle Data Group 2023](#particle-data-group-2023)

### Rp☉ — Solar polar radius

It is measured by helioseismology and limb-darkening observations.
[Reference 18](#reference-18) [Reference 19](#reference-19)

### f☉ — Solar oblateness

The Solar oblateness (or flattening) is calculated from the nominal
equatorial radius and the polar radius measurement as
`f☉=1−Rp☉/Req☉`. Its uncertainty is inherited from Jupiter `♃Rp☉`.
When greater than 0, it describes how much a celestial body deviates
from a perfect sphere, forming an "oblate spheroid". [Reference 18](#reference-18) [Reference 19](#reference-19)

### ρv☉ — Solar mean density

It is calculated from the nominal gravitational parameter `GM☉`, the
gravitational constant `G`, and the solar radii measurement. [Particle Data Group 2023](#particle-data-group-2023)

### g☉ — Solar surface gravity

The Solar surface gravity at the equator is calculated from `GM☉`
and the measurement of the equatorial radius `Req☉`. [Particle Data Group 2023](#particle-data-group-2023)

### Ve☉ — Solar escape velocity

The Solar escape velocity at the equatorial surface is calculated from
`GM☉` and the measurement of solar radii. It is the minimum speed needed
to break free from Sun's gravitational pull and never return. [Particle Data Group 2023](#particle-data-group-2023)

### Prot☉ — Solar sidereal rotation period

The Solar sidereal rotation period at the equator (System I) is measured
by tracking surface features using Doppler techniques. It is the true
time it takes to spin 360° on its axis. [Reference 20](#reference-20) [Reference 21](#reference-21)


## Cosmology & Varia

### Λ — Cosmological constant

It represents the measured energy density of empty space (dark energy)
in Einstein's field equations. Its value is inferred from observations
of the large-scale structure and expansion of the universe. [Planck 2020](#planck-2020) [Particle Data Group 2024](#particle-data-group-2024)

### H₀ — Hubble constant

It is the measured current rate of expansion of the universe, expressed
in `km/(s·Mpc)`. Its value is measured from observations of the cosmic
microwave background and Type Ia supernovae. [Planck 2020](#planck-2020)

### ωb — Baryon density parameter

It is the measured physical density of baryonic matter (ordinary matter)
in the universe, measured from the cosmic microwave background power
spectrum. [Planck 2020](#planck-2020)

### ωc — Dark matter density parameter

It is the measured physical density of cold dark matter in the universe,
measured from the cosmic microwave background power spectrum. [Planck 2020](#planck-2020)

### 100θ* — CMB acoustic scale

The CMB Acoustic scale corresponds to the measure of one hundred times
the angular size of the sound horizon at recombination, measured from
the position of the first acoustic peak in the CMB power spectrum. [Planck 2020](#planck-2020)

### τr — Reionization optical depth

It is the measured integrated optical depth to Thomson scattering from
the epoch of reionization, measured from the CMB polarization spectrum.
[Planck 2020](#planck-2020)

### ns — Scalar spectral index

It is the measured tilt of the primordial power spectrum of density
fluctuations, where ns=1 corresponds to a scale-invariant Harrison-
Zel'dovich spectrum. [Planck 2020](#planck-2020)

### (ln(10¹⁰As)) — Power spectrum amplitude

Measured amplitude of the primordial power spectrum. The logarithm
of the scalar power spectrum amplitude, normalized at a pivot scale
of `0.05 Mpc⁻¹`. [Planck 2020](#planck-2020)

### MG — Milky Way total mass

Measured total mass of the Milky Way galaxy, including baryonic
and dark matter components, estimated from stellar kinematics and
satellite galaxy dynamics. [Reference 8](#reference-8) [Reference 9](#reference-9) [Reference 10](#reference-10)

### fDMG — Milky Way dark matter fraction

Measured fraction of dark matter in the Milky Way galaxy. Estimated
from rotation curves, satellite dynamics, and gravitational lensing.
[Reference 8](#reference-8) [Reference 9](#reference-9)

### RG — Milky Way disk radius

Measure of the equatorial radius of the Milky Way galaxy's disk,
estimated from star count surveys and kinematic models. [Reference 8](#reference-8) [Reference 9](#reference-9)

### D●SgrA* — Distance to Sgr A*

Measured distance from the Sun to the black hole Sagittarius A* at
the Galactic centre, measured by stellar orbit monitoring with the
GRAVITY instrument. [Reference 6](#reference-6) [Reference 7](#reference-7)

### R●SgrA* — Sgr A* Schwarzschild radius

Measured Schwarzschild radius of the black hole Sagittarius A*, derived
from its mass measurement by the Event Horizon Telescope collaboration.
[Reference 7](#reference-7)

### M●SgrA* — Sgr A* black hole mass

Measured mass of the supermassive black hole Sagittarius A* at the
Galactic centre, measured from the orbits of stars in its immediate vicinity.
[Reference 7](#reference-7)

### MNGC224 — Andromeda galaxy total mass

Measure of the total mass of the Andromeda galaxy (M31, NGC 224), including
dark matter, estimated from satellite galaxy kinematics and gravitational
timing arguments. [Reference 11](#reference-11) [Reference 12](#reference-12)

### fDMNGC224 — Andromeda dark matter fraction

Measured fraction of dark matter in the Andromeda galaxy (M31), estimated
from dynamical modelling of its halo. [Reference 11](#reference-11) [Reference 12](#reference-12)

### RNGC224 — Andromeda galaxy disk radius

Measurement of the mean equatorial radius of the Andromeda galaxy (M31),
estimated from stellar population surveys. [Reference 11](#reference-11)

### DNGC224 — Distance to Andromeda galaxy

Measurement of the mean distance from the Milky Way to the Andromeda
galaxy (M31), measured using Cepheid variable stars and tip of the red
giant branch. [Reference 11](#reference-11)

### GM★Vega — Vega gravitational parameter

Measurement of the gravitational parameter of the star Vega (mass times
gravitational constant `G`). Measured by interferometry and spectral
analysis. [Reference 13](#reference-13) [Reference 14](#reference-14)

### L★Vega — Vega luminosity

Measured luminosity of the star Vega, used as a photometric reference
standard in stellar astronomy. [Reference 13](#reference-13) [Reference 14](#reference-14)



## Materials


## A. GASES (at `StdT = 273.15 K = 0°C`, `StdP = 101.325 kPa`)
  Parameters: `ρv`, `n*`, `cp`, `cv`, `γ`, `R`, `ε`, `μ`, `ρ`, `Ed`
   *: where relevant
  `γ = cp/cv` and `R = cp − cv` apply to gases only.
  `γ` is given as a calculation `cp/cv` for diatomic gases.
  `R` is given as a calculation `cp-cv` for all gases.

## Dry Air — Gas

### ρvAir — Dry air density

Measured. Volumetric density of dry air at standard temperature (0°C)
and pressure (101.325 kPa).
[Materials 1](#materials-1)
[Materials 2](#materials-2)

### nAir — Refractive index of dry air

Measured. Refractive index of dry air at 589 nm (sodium D line), 0°C,
1 atm. Varies slightly with humidity and CO2 content.
[Materials 2](#materials-2)

### cpAir — Specific heat cp of dry air

Measured. Specific heat of dry air at constant pressure at 300 K.
[Materials 1](#materials-1)
[Materials 2](#materials-2)

### cvAir — Specific heat cv of dry air

Specific heat of dry air at constant volume, calculated as
`cpAir − RAir`.
[Materials 1](#materials-1)
[Materials 2](#materials-2)

### γAir — Adiabatic index of dry air

Calculated from measurement. Adiabatic index (heat capacity ratio) of
dry air, `γ=cp/cv`, at 300 K.
[Materials 1](#materials-1)
[Materials 2](#materials-2)

### RAir — Specific gas constant of dry air

Exact calculation: `R/M_Air`. Specific gas constant of dry air,
`RAir=R/M_Air`, where `M_Air=28.9647 g/mol`. [Materials 1](#materials-1)

### εAir — Electric permittivity of dry air

Measured. Electric permittivity of dry airat STP. The relative
permittivity `εr≈1.000536` is very close to vacuum permittivity `ε₀`.
It is a measure of how dense an electric field is "permitted" to
form in response to electric charges.
[Materials 1](#materials-1)
[Materials 2](#materials-2)

### μAir — Magnetic permeability of dry air

Measured. Magnetic permeability of dry air at STP. The relative
permeability `μr≈1.0000004`, essentially identical to `μ₀`. It
quantifies the strength of the magnetic field induced by an electric
current.
[Materials 1](#materials-1)
[Materials 2](#materials-2)

### ρAir — Resistivity of dry air

Measured. Electrical resistivity of dry air at STP. Dry air is an
excellent insulator; resistivity ranges from 10¹³ to 10¹⁶ Ω·m
depending on ionisation level. It quantifies how strongly a material
opposes the flow of electric current.
[Materials 1](#materials-1)
[Materials 2](#materials-2)

### EdAir — Dielectric rigidity of dry air

Measured. Dielectric breakdown field strength of dry air at STP.
Above this threshold, dry air undergoes avalanche ionisation
(lightning, spark discharge).
[Materials 1](#materials-1)
[Materials 2](#materials-2)

## N2 — Gas

### ρvN2 — Dinitrogen N2 density

Measured. Volumetric density of dinitrogen (N2) at `0°C`,
`101.325 kPa`.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### cpN2 — Dinitrogen N2 specific heat cp

Measured. Specific heat of N2 at constant pressure.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### cvN2 — Dinitrogen N2 specific heat cv

Measured. Specific heat of N2 at constant volume.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### γN2 — Dinitrogen N2 adiabatic index

Measured. Adiabatic index of N2, γ=cp/cv.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### RN2 — Dinitrogen N2 specific gas constant

Exact calculation: `R/M_N2`. Specific gas constant of N2, `RN2=R/M_N2`,
where `M_N2=28.014 g/mol`. [Materials 2](#materials-2)

### εN2 — Electric permittivity of N2

Measured. Electric permittivity of N2 at STP. Relative permittivity
`εr≈1.000580`; N2 is widely used as an insulating gas in high-voltage
equipment. It is a measure of how dense an electric field is "permitted"
to form in response to electric charges. [Materials 2](#materials-2)

### μN2 — Magnetic permeability of N2

Measured. Magnetic permeability of N2 at STP. Diamagnetic with `μr≈1.0000000`,
essentially equal to `μ₀`. It quantifies the strength of the magnetic field
induced by an electric current. [Materials 2](#materials-2)

### ρN2 — Resistivity of N2

Measured. Electrical resistivity of dry N2 at STP. Excellent insulator
used in purged electrical enclosures. It quantifies how strongly a
material opposes the flow of electric current. [Materials 2](#materials-2)

### EdN2 — Dielectric rigidity of N2

Measured. Dielectric breakdown field of N2 at STP. Slightly higher
than dry air; preferred for high-voltage insulation. It is the electric
field threshold above which an avalanche ionisation occurs. [Materials 2](#materials-2)

## O2 — Gas

### ρvO2 — Dioxygen O2 density

Measured. Volumetric density of dioxygen (O2) at `0°C`,
`101.325 kPa`.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### cpO2 — Dioxygen O2 specific heat cp

Measured. Specific heat of O2 at constant pressure.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### cvO2 — Dioxygen O2 specific heat cv

Measured. Specific heat of O2 at constant volume.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### γO2 — Dioxygen O2 adiabatic index

Measured. Adiabatic index of O2, `γ=cp/cv`, at `300 K`.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### RO2 — Dioxygen O2 specific gas constant

Exact calculation: `R/M_O2`. Specific gas constant of O2, `RO2=R/M_O2`,
where `M_O2=31.999 g/mol`. [Materials 2](#materials-2)

### εO2 — Electric permittivity of O2

Measured. Electric permittivity of O2 at STP. Relative permittivity
`εr≈1.000530`, comparable to N2. It is a measure of how dense an electric
field is "permitted" to form in response to electric charges. [Materials 2](#materials-2)

### μO2 — Magnetic permeability of O2

Measured. Magnetic permeability of O2 at STP. Paramagnetic with
`μr≈1.0000019` due to its unpaired electrons. It quantifies the strength
of the magnetic field induced by an electric current. [Materials 2](#materials-2)

### ρO2 — Resistivity of O2

Measured. Electrical resistivity of dry O2 at STP. Good insulator
under standard conditions. It quantifies how strongly a material
opposes the flow of electric current. [Materials 2](#materials-2)

### EdO2 — Dielectric rigidity of O2

Measured. Dielectric breakdown electric field of O2 at STP.
Slightly lower than N2. [Materials 2](#materials-2)

## Argon — Gas

### ρvAr — Argon density

Measured. Volumetric density of argon (Ar) at `0°C`,
`101.325 kPa`. [Materials 2](#materials-2)

### cpAr — Argon specific heat cp

Exact calculation: `(5/2)·R/M_Ar` (monatomic ideal gas). Specific
heat of argon at constant pressure. [Materials 2](#materials-2)

### cvAr — Argon specific heat cv

Specific heat of argon at constant volume. Exactly `(3/2)·R/M_Ar` for
a monatomic ideal gas. [Materials 2](#materials-2)

### γAr — Argon adiabatic index

Adiabatic index of argon. Exactly `5/3` for any monatomic ideal gas. [Materials 2](#materials-2)

### RAr — Argon specific gas constant

Exact calculation. Specific gas constant of argon, `RAr=R/M_Ar`, where
`M_Ar=39.948 g/mol`. [Materials 2](#materials-2)

### εAr — Electric permittivity of Ar

Measured. Electric permittivity of Ar at STP. Relative permittivity
`εr≈1.000516`; used as an inert dielectric medium. It is a measure of how
dense an electric field is "permitted" to form in response to electric charges.
[Materials 2](#materials-2)

### μAr — Magnetic permeability of Ar

Measured. Magnetic permeability of Ar at STP. Diamagnetic with `μr≈1.0000000`,
essentially equal to `μ₀`. It quantifies the strength of the magnetic field
induced by an electric current. [Materials 2](#materials-2)

### ρAr — Resistivity of Ar

Measured. Electrical resistivity of argon at STP. Excellent insulator; used
in plasma physics and dielectric studies. It quantifies how strongly a
material opposes the flow of electric current. [Materials 2](#materials-2)

### EdAr — Dielectric rigidity of Ar

Measured. Dielectric breakdown electric field of argon at STP. Lower than
air due to absence of polar molecules. [Materials 2](#materials-2)

## CO2 — Gas

### ρvCO2 — Carbon dioxide CO2 density

Measured. Volumetric density of carbon dioxide (CO2) at `0°C`,
`101.325 kPa`.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### cpCO2 — Carbon dioxide CO2 specific heat cp

Measured. Specific heat of CO2 at constant pressure at `300 K`.
Strongly temperature-dependent.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### cvCO2 — Carbon dioxide CO2 specific heat cv

Measured. Specific heat of CO2 at constant volume at `300 K`.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### γCO2 — Carbon dioxide CO2 adiabatic index

Measured. Adiabatic index of CO2 at `300 K`. Lower than diatomic gases
due to its additional vibrational modes.
[Materials 2](#materials-2)
[Materials 3](#materials-3)


### RCO2 — Carbon dioxide CO2 specific gas constant

Exact calculation. Specific gas constant of CO2, `RCO2=R/M_CO2`,
where `M_CO2=44.010 g/mol`. [Materials 2](#materials-2)

### εCO2 — Electric permittivity of CO2

Measured. Electric permittivity of CO2 at STP. Relative permittivity `εr≈1.000921`,
slightly higher than N2 and Ar due to CO2's quadrupole moment. It is a measure
of how dense an electric field is "permitted" to form in response to electric
charges.
[Materials 2](#materials-2)
[Materials 3](#materials-3)

### μCO2 — Magnetic permeability of CO2

Measured. Magnetic permeability of CO2 at STP. Diamagnetic with `μr≈1.0000000`,
essentially equal to `μ₀`. It quantifies the strength of the magnetic field
induced by an electric current. [Materials 2](#materials-2)

### ρCO2 — Resistivity of CO2

Measured. Electrical resistivity of CO2 at STP. Lower than N2 and Ar; sometimes
used as arc-quenching medium in circuit breakers. It quantifies how strongly
a material opposes the flow of electric current. [Materials 2](#materials-2)

### EdCO2 — Dielectric rigidity of CO2

Measured. Dielectric breakdown electric field of CO2 at STP. Comparable to dry
air. [Materials 2](#materials-2)


## B. LIQUIDS (at `T = 293.15 K = 0°C`, `StdP = 101.325 kPa`)
  Parameters: `ρv`, `n`, `cp`, `ρ`
  For liquids: `cv ≈ `cp (difference < 1%), `γ ≈ 1`, `R = cp - cv ≈ 0`.
  These parameters are therefore not listed for liquids.

## H2O — Liquid

### ρvH2O — Liquid water density

Measured (IAPWS-IF97). Volumetric density of liquid water at `20°C`,
`101.325 kPa`. [Materials 4](#materials-4)
[Materials 2](#materials-2)

### nH2O — Refractive index of liquid water

Measured. Refractive index of liquid water at `589 nm` (sodium D line), `20°C`.
[Materials 4](#materials-4)

### cpH2O — Specific heat cp of liquid water

Measured (IAPWS-IF97). Specific heat of liquid water at constant pressure
at `20°C`. Near its minimum value in the `0–100°C` range. [Materials 4](#materials-4)
[Materials 2](#materials-2)

### ρH2O — Resistivity of liquid water

Measured. Electrical resistivity of ultra-pure water at `25°C` . The value
`1.8×10⁵ Ω·m (18 MΩ·cm)` is the theoretical maximum for pure water. It
quantifies how strongly a material opposes the flow of electric current.
[Materials 4](#materials-4) [Materials 2](#materials-2)


## C. SOLIDS (at `T = 293.15 K = 20°C`, `StdP = 101.325 kPa`)
  Parameters: `ρv`, `n`, `cp`, `ε`, `ρ*`, `Ed*`  * when relevant

## C1. Materials for optical fibers and waveguides

## Silicon — Dielectric

### ρvSi — Crystalline silicon density

Measured. Volumetric density of crystalline silicon, the foundation
of modern microelectronics and an important infrared photonic
material. [Materials 10](#materials-10) [Materials 11](#materials-11)

### nSi — Refractive index of crystalline silicon

Measured (Silicon is transparent only for `λ > 1.1 µm`). Refractive
index of crystalline silicon at `1150 nm`, `20°C`. [Materials 11](#materials-11)

### cpSi — Specific heat of silicon

Measured. Specific heat of crystalline silicon at `300 K`. [Materials 10](#materials-10)

### εsi — Dielectric constant of silicon

Exact convention. Dielectric constant of silicon. The relative permittivity
measures how well an applied electric field penetrates the material
compared to vacuum. Has an exact value by convention. [HP50g Manual]

### ρSi — Resistivity of crystalline silicon

Measured. Electrical resistivity of intrinsic crystalline silicon at `300 K`.
Semiconductor; resistivity varies over many orders of magnitude with doping.
It quantifies how strongly a material opposes the flow of electric current.
[Materials 10](#materials-10) [Materials 11](#materials-11)

## SiO2 — Dielectric

### ρvSiO2 — Fused silica density

Measured. Volumetric density of amorphous fused silica (vitreous SiO2),
the primary material for standard optical fibers. [Materials 5](#materials-5) [Materials 7](#materials-7)

### nSiO2 — Refractive index of fused silica

Measured (Malitson 1965 Sellmeier fit). Refractive index of fused silica
at `589.3 nm` (sodium D line), `20°C`. The Sellmeier-calibrated value
from Malitson (1965) is the standard reference for fiber optic design. [Materials 5](#materials-5)

### cpSiO2 — Specific heat of fused silica

Measured. Specific heat of fused silica at `20°C`. [Materials 7](#materials-7)

### εox — Dielectric constant of SiO2

Exact convention. Dielectric constant of SiO2 (silicon dioxide). The relative
permittivity of thermally grown or deposited silicon dioxide, used in MOS
device modelling. Has an exact value by convention. It measures how well
an applied electric field penetrates the material compared to vacuum. [HP50g Manual]

### ρSiO2 — Resistivity of fused silica

Measured. Electrical resistivity of fused silica at `20°C`. Among the best
solid insulators; used in high-voltage isolation and MEMS. It quantifies
how strongly a material opposes the flow of electric current. [Materials 7](#materials-7)

### EdSiO2 — Dielectric rigidity of fused silica

Measured. Dielectric breakdown electric field of fused silica. Very high;
SiO2 is the standard gate dielectric in MOSFET technology. [Materials 7](#materials-7)

## GeO2 — Dielectric

## ρvGeO2 — Germanate glass density

Measured. Volumetric density of germanate glass (GeO2), used as a dopant
in the core of standard single-mode optical fibers to raise the refractive
index. [Materials 8](#materials-8) [Materials 9](#materials-9)

### nGeO2 — Refractive index of germanate glass

Measured. Refractive index of pure GeO2 glass at `589 nm`, `20°C`. The
actual core index of a doped fiber is interpolated between nSiO2
and nGeO2. [Materials 8](#materials-8)

### cpGeO2 — Specific heat of germanate glass

Measured. Specific heat of germanate glass. [Materials 9](#materials-9)

### ρGeO2 — Resistivity of germanate glass

Measured. Electrical resistivity of pure GeO2 glass at `20°C`. Lower
than fused silica but still an excellent insulator. It quantifies how
strongly a material opposes the flow of electric current. [Materials 9](#materials-9)

### EdGeO2 — Dielectric rigidity of germanate glass

Measured. Dielectric breakdown electric field of GeO2 glass. Similar
order of magnitude to SiO2; varies with purity. [Materials 9](#materials-9)

## Si3N4 — Dielectric

### ρvSi3N4 — Silicon nitride density

Measured. Volumetric density of silicon nitride (Si3N4) deposited by
LPCVD, the standard platform for integrated photonic waveguides.
[Materials 18](#materials-18) [Materials 19](#materials-19)

### nSi3N4 — Refractive index of LPCVD Si3N4

Measured; depends on deposition conditions. Refractive index of LPCVD Si3N4
at `633 nm` (He-Ne laser), `20°C`. The value depends on deposition conditions
and stoichiometry. [Materials 18](#materials-18)

### cpSi3N4 — Specific heat of Si3N4

Measured. Specific heat of silicon nitride. [Materials 19](#materials-19)

### ρSi3N4 — Resistivity of Si3N4

Measured. Electrical resistivity of LPCVD Si3N4 at `20°C`. Excellent insulator
used for passivation and etch-stop layers in integrated circuits. It quantifies
how strongly a material opposes the flow of electric current. [Materials 19](#materials-19)

### EdSi3N4 — Dielectric rigidity of Si3N4

Measured. Dielectric breakdown electric field of Si3N4 thin films. Higher than
SiO2; used as storage dielectric in SONOS flash memory. [Materials 19](#materials-19)


## C2. Reference optical glasses

##BK7 Glass — Dielectric

### ρvBK7 — Schott BK7 glass density

Measured. Volumetric density of Schott BK7 borosilicate glass, the
most widely used optical glass for visible-range optics. [Materials 15](#materials-15)

### nBK7 — Refractive index of Schott BK7 glass

Measured (Schott catalogue nominal nd). Refractive index of Schott BK7 at
`589.3 nm` (sodium d line), `20°C`. This is the nominal nd value used in
optical design. [Materials 15](#materials-15)

### cpBK7 — Specific heat of BK7 glass

Measured. Specific heat of BK7 glass. [Materials 15](#materials-15)

### ρBK7 — Resistivity of BK7 glass

Measured. Electrical resistivity of BK7 borosilicate glass at `20°C`. Used
in high-voltage optical isolators. It quantifies how strongly a material
opposes the flow of electric current. [Materials 15](#materials-15)

### EdBK7 — Dielectric rigidity of BK7 glass

Measured. Dielectric breakdown electric field of BK7 glass. Lower than
crystalline dielectrics due to its amorphous microstructure. [Materials 15](#materials-15)


## C3. Optical crystals (UV & IR)

## Al2O3 Sapphire — Dielectric

### ρvAl2O3 — Sapphire density

Measured. Volumetric density of single-crystal sapphire (alpha-Al2O3),
used for UV-to-IR windows, substrates, and high-power laser rods. [Materials 16](#materials-16)

### nAl2O3 — Ordinary refractive index of sapphire

Measured (ordinary axis; birefringent material). Ordinary refractive index
of sapphire at `589 nm`, `20°C`. Sapphire is birefringent; the extraordinary
index is slightly higher. [Materials 16](#materials-16)

### cpAl2O3 — Specific heat of sapphire

Measured. Specific heat of sapphire at `300 K`. [Materials 16](#materials-16) [Materials 2](#materials-2)

### ρAl2O3 — Resistivity of sapphire

Measured. Electrical resistivity of single-crystal sapphire at `20°C`. One
of the best insulating substrates; used in power electronics and RF
substrates. It quantifies how strongly a material opposes the flow of electric
current. [Materials 16](#materials-16)

### EdAl2O3 — Dielectric rigidity of sapphire

Measured. Dielectric breakdown electric field of sapphire. High strength;
enables use in high-field capacitors and microwave devices. [Materials 16](#materials-16)

## CaF2 — Dielectric

### ρvCaF2 — Calcium fluoride density

Measured. Volumetric density of calcium fluoride (CaF2), a key material
for deep ultraviolet optics (`193` nm excimer lithography) and infrared
windows. [Materials 17](#materials-17) [Materials 15](#materials-15)

### nCaF2 — Refractive index of CaF2

Measured. Refractive index of CaF2 at `589 nm`, `20°C`. CaF2 is transparent
from `130 nm` to `10 µm`, making it useful across a very wide spectral range.
[Materials 17](#materials-17)

### cpCaF2 — Specific heat of CaF2

Measured. Specific heat of calcium fluoride. [Materials 15](#materials-15)

### ρCaF2 — Resistivity of CaF2

Measured. Electrical resistivity of single-crystal CaF2 at `20°C` . Excellent
insulator; ionic crystal structure provides high purity. It quantifies
how strongly a material opposes the flow of electric current. [Materials 17](#materials-17)

### EdCaF2 — Dielectric rigidity of CaF2

Measured. Dielectric breakdown electric field of CaF2 crystal. Very high;
consistent with use in high-power UV laser optics. [Materials 17](#materials-17)

## C Diamond — Dielectric

### ρvC — Diamond density

Measured. Volumetric density of cubic (type IIa) diamond. Diamond has
the highest known thermal conductivity of any material at room
temperature. [Materials 13](#materials-13) [Materials 14](#materials-14)

### nC — Refractive index of diamond

Measured (type IIa diamond). Refractive index of diamond at `589 nm`,
`20°C`. [Materials 14](#materials-14)

### cpC — Specific heat of diamond

Measured. Specific heat of diamond at `300 K`. [Materials 13](#materials-13)

### ρC — Resistivity of diamond

Measured (type IIa natural diamond). Electrical resistivity of type IIa diamond
at `20°C`. Resistivity varies strongly with crystal purity. It quantifies how
strongly a material opposes the flow of electric current. [Materials 13](#materials-13) [Materials 14](#materials-14)

### EdC — Dielectric rigidity of diamond

Measured (type IIa diamond). Dielectric breakdown electric field of diamond.
Highest of any known solid; enables use in next-generation high-power
electronics. [Materials 13](#materials-13) [Materials 14](#materials-14)


## C4. Reference metals (thermal and mechanical engineering)
    Parameters: `ρv`, `cp`
    For metals: `n` not listed (complex `n`, strongly wavelength-dependent).
    `cv ≈ cp` for all metals at room temperature (difference < 0.5%).

## Al — Metal

### ρvAl — Aluminium density

Measured. Volumetric density of pure aluminium, a standard reference
material in mechanical and thermal engineering. [Materials 12](#materials-12) [Materials 2](#materials-2)

### cpAl — Specific heat of aluminium

Measured. Specific heat of pure aluminium at `20°C`. [Materials 12](#materials-12) [Materials 2](#materials-2)

## Cu — Metal

### ρvCu — Copper density

Measured. Volumetric density of copper, the reference material for
thermal and electrical conductivity in engineering. [Materials 12](#materials-12) [Materials 2](#materials-2)

### cpCu — Specific heat of copper

Measured. Specific heat of copper at `20°C`. [Materials 12](#materials-12) [Materials 2](#materials-2)

## Fe — Metal

### ρvFe — Iron density (alpha phase)

Measured (alpha phase, BCC). Volumetric density of iron in its alpha
phase (body-centred cubic), stable below `912°C`. [Materials 12](#materials-12) [Materials 2](#materials-2)

### cpFe — Specific heat of iron (alpha phase, BCC)

Measured (alpha phase). Specific heat of iron (alpha phase) at `20°C`.
[Materials 12](#materials-12) [Materials 2](#materials-2)


## Dates Constants

### BastilleDay constant

French national day

### MartinLutherKingDeath constant

Martin Luther King Jr., an African-American clergyman and civil rights movement
leader, was fatally shot at the Lorraine Motel in Memphis, Tennessee, on April
4, 1968, at 6:01 p.m.

### IndependenceDay constant

Independence Day, known colloquially as the Fourth of July, is a federal holiday
in the United States which commemorates the ratification of the Declaration of
Independence by the Second Continental Congress on July 4, 1776, establishing
the United States of America.


## Computing constants

### No constant

The value of "No" is false

### Yes constant

The value of "Yes" is true

### UnixEpoch constant

Date from which all dates are counted in Unix computer systems.

### SinclairZX81RAM constant

Amount of memory in the Sinclair ZX81 personal computer.
This is also the amount of memory used in the video memory for the mouse cursor
at its smallest size on modern computers.

### PageSize constant

Base page size in the virtual memory system of most modern computers. Note that
for performance reason, many modern computers now support multiple page sizes.

### HelloWorld constant

A constant often used in computer programs to denote joy, happiness and the
emergence into the world of some new computer language.


## Primary physics data reference

### CODATA

CODATA 2022: Fundamental Physical Constants. National Institute of Standards and Technology (NIST). [Table](https://physics.nist.gov/cuu/pdf/wall_2022.pdf)

Applies to all unspecified sources for fundamental physical constants.

### Planck 2020

Planck Collaboration (2020). "Planck 2018 results. VI. Cosmological parameters". Astronomy & Astrophysics, 641, A6. arXiv:1807.06209 — DOI: 10.1051/0004-6361/201833910

### Particle Data Group 2024

Particle Data Group (2024). "Review of Particle Physics — Astrophysical Constants". Physical Review D, 110, 030001. DOI: 10.1103/PhysRevD.110.030001

### Particle Data Group 2023

Particle Data Group 2023 Prša, A., et al. (2016). "Nominal values for selected solar and planetary quantities: IAU 2015 Resolution B3". The Astronomical Journal, 152(2), 41. arXiv:1605.09788 — DOI: 10.3847/0004-6256/152/2/41

### Reference 4

Park, R.S., et al. (2021). "The JPL Planetary and Lunar Ephemerides DE440 and DE441". The Astronomical Journal, 161(3), 105. DOI: 10.3847/1538-3881/abd414

### Reference 5

Tiesinga, E., et al. (2021). "CODATA recommended values of the fundamental physical constants: 2018". Reviews of Modern Physics, 93(2), 025010. DOI: 10.1103/RevModPhys.93.025010

## Galactic and extragalactic data

### Reference 6

GRAVITY Collaboration (2021). "Mass distribution in the Galactic Center based on interferometric astrometry of multiple stellar orbits". Astronomy & Astrophysics, 647, A59. arXiv:2012.02169 — DOI: 10.1051/0004-6361/202040208

### Reference 7

Event Horizon Telescope Collaboration (2022). "First Sagittarius A* Event Horizon Telescope Results. I. The Shadow of the Supermassive Black Hole in the Center of the Milky Way". The Astrophysical Journal Letters, 930(2), L12. arXiv:2311.08680 — DOI: 10.3847/2041-8213/ac6674

### Reference 8

Eilers, A.-C., et al. (2019). "The Circular Velocity Curve of the Milky Way from 5 to 25 kpc". The Astrophysical Journal, 871(1), 120. arXiv:1810.09466 — DOI: 10.3847/1538-4357/aaf648

### Reference 9

Watkins, L.L., et al. (2019). "Evidence for an Intermediate-Mass Milky Way from Gaia DR2 Halo Globular Cluster Motions". The Astrophysical Journal, 873(2), 118. arXiv:1804.11348 — DOI: 10.3847/1538-4357/ab089f

### Reference 10

Posti, L., & Helmi, A. (2019). "Mass and shape of the Milky Way's dark matter halo with globular clusters from Gaia and Hubble". Astronomy & Astrophysics, 621, A56. arXiv:1805.01408 — DOI: 10.1051/0004-6361/201833355

### Reference 11

Peñarrubia, J., et al. (2014). "A timing constraint on the (total) mass of the Large Magellanic Cloud". Monthly Notices of the Royal Astronomical Society, 443(3), 2204–2222. arXiv:1405.4662 — DOI: 10.1093/mnras/stu879

### Reference 12

Veljanoski, J., et al. (2014). "The M31 satellite plane: Evidence for a recent flyby?" Monthly Notices of the Royal Astronomical Society: Letters, 442(1), L86–L90. arXiv:1404.5988 — DOI: 10.1093/mnrasl/slu050

### Reference 13

Yoon, J., et al. (2010). "A New View of Vega's Composition, Mass, and Age". The Astrophysical Journal, 708(1), 71–79. arXiv:0910.3124 — DOI: 10.1088/0004-637X/708/1/71

### Reference 14

Aufdenberg, J.P., et al. (2006). "First Results from the CHARA Array. VII. Long-Baseline Interferometric Measurements of Vega Consistent with a Pole-On, Rapidly Rotating Star". The Astrophysical Journal, 645(1), 664–675. arXiv:astro-ph/0603327 — DOI: 10.1086/504149

### Particle physics data

### Reference 15

ATLAS-CONF-2023-037: "ATLAS sets record precision on Higgs boson's mass" (CERN, July 21, 2023).

### Reference 16

CMS Collaboration (2026): "High-precision measurement of the W boson mass with the CMS experiment", Nature (published online April 8, 2026).

### Reference 17

LEP Electroweak Working Group, "A Combination of Preliminary Electroweak Measurements and Constraints on the Standard Model," arXiv:hep-ex/0605011, often summarized in Physics Reports 427 (2006) 257–454.
Solar data

### Reference 18

Haberreiter, M., Schmutz, W., & Kosovichev, A.G. (2008). "Solving the Discrepancy between the Seismic and Photospheric Solar Radius". The Astrophysical Journal Letters, 675(1), L53–L56. DOI: 10.1086/529492

### Reference 19

Mamajek, E.E., et al. (2015). "On the Age and Binarity of Fomalhaut". The Astrophysical Journal, 800(2), L17. arXiv:1412.5018 — DOI: 10.1088/2041-8205/800/2/L17

### Reference 20

Snodgrass, H.B., & Ulrich, R.K. (1990). "Rotation of Doppler features in the solar photosphere". The Astrophysical Journal, 351, 309–316. DOI: 10.1086/168467

### Reference 21

Beck, J.G. (2000). "A comparison of differential rotation measurements". Solar Physics, 191(1), 47–70. DOI: 10.1023/A:1005226402796

## Planetary and lunar data

### Reference 22

Archinal, B.A., et al. (2018). "Report of the IAU Working Group on Cartographic Coordinates and Rotational Elements: 2015". Celestial Mechanics and Dynamical Astronomy, 130(3), 22. DOI: 10.1007/s10569-017-9805-5

### Reference 23

NASA JPL Solar System Dynamics — Horizons On-Line Ephemeris System. URL: https://ssd.jpl.nasa.gov/horizons/ Accessed: February 2026. Epoch: J2000.0 (JD 2451545.0)

### Reference 24

Seidelmann, P.K., & Fukushima, T. (1992). "Why new time scales?" Astronomy & Astrophysics, 265(2), 833–838.

### Reference 25

Williams, J.G., et al. (2014). "Lunar interior properties from the GRAIL mission". Journal of Geophysical Research: Planets, 119(7), 1546–1578. DOI: 10.1002/2013JE004559

### Reference 26


Nimmo, F., et al. (2017). "Mean radius and shape of Pluto and Charon from New Horizons images". Icarus, 287, 12–29. arXiv:1603.00821 — DOI: 10.1016/j.icarus.2016.06.027

## Materials & Fluids data

### Materials 1

Lemmon, E.W., Jacobsen, R.T., Penoncello, S.G., & Friend, D.G. (2000). "Thermodynamic Properties of Air and Mixtures of Nitrogen, Argon, and Oxygen from 60 to 2000 K at Pressures to 2000 MPa". J. Phys. Chem. Ref. Data, 29(3), 331–385. DOI: 10.1063/1.1285884

### Materials 2

NIST WebBook of Chemistry (2024). National Institute of Standards and Technology. [Source](https://webbook.nist.gov/chemistry) — Accessed: March 2026

### Materials 3

Chase, M.W. (1998). NIST-JANAF Thermochemical Tables, 4th ed. J. Phys. Chem. Ref. Data, Monograph 9. American Chemical Society / AIP.

### Materials 4

Wagner, W., & Pruss, A. (2002). "The IAPWS Formulation 1995 for the Thermodynamic Properties of Ordinary Water Substance for General and Scientific Use". J. Phys. Chem. Ref. Data, 31(2), 387–535. DOI: 10.1063/1.1461829

### Materials 5

Malitson, I.H. (1965). "Interspecimen Comparison of the Refractive Index of Fused Silica". J. Opt. Soc. Am., 55(10), 1205–1209. DOI: 10.1364/JOSA.55.001205

### Materials 6

Palik, E.D. (Ed.) (1985). Handbook of Optical Constants of Solids. Academic Press, New York. ISBN: 0-12-544420-6

### Materials 7

Corning Incorporated (2023). Corning HPFS Fused Silica — Standard Grade Optical Properties. Product datasheet. [Source](https://www.corning.com)

### Materials 8

Fleming, J.W. (1984). "Dispersion in GeO2-SiO2 glasses". Applied Optics, 23(24), 4486–4493. DOI: 10.1364/AO.23.004486

### Materials 9

Dianov, E.M., & Guryanov, A.N. (2010). "High-power Raman fiber lasers". Quantum Electronics, 40(7), 579–599. DOI: 10.1070/QE2010v040n07ABEH014211

### Materials 10

Glassbrenner, C.J., & Slack, G.A. (1964). "Thermal Conductivity of Silicon and Germanium from 3°K to the Melting Point". Physical Review, 134(4A), A1058–A1069. DOI: 10.1103/PhysRev.134.A1058

### Materials 11

Hull, R. (Ed.) (1999). Properties of Crystalline Silicon. EMIS Datareviews Series No. 20. IEE, London. ISBN: 0-85296-933-3

### Materials 12

Ho, C.Y., Powell, R.W., & Liley, P.E. (1972). "Thermal Conductivity of the Elements". J. Phys. Chem. Ref. Data, 1(2), 279–421. DOI: 10.1063/1.3253100

### Materials 13

Slack, G.A. (1973). "Nonmetallic crystals with high thermal conductivity". J. Phys. Chem. Solids, 34(2), 321–335. DOI: 10.1016/0022-3697(73)90092-9

### Materials 14

Zaitsev, A.M. (2001). Optical Properties of Diamond: A Data Handbook. Springer, Berlin. ISBN: 3-540-66603-8

### Materials 15

Schott AG (2023). Optical Glass Data Sheets. [Source](https://www.schott.com/en-gb/products/optical-glass) — Accessed: March 2026

### Materials 16
 Dobrovinskaya, E.R., Lytvynov, L.A., & Pishchik, V. (2009). Sapphire: Material, Manufacturing, Applications. Springer, Boston. ISBN: 978-0-387-85694-0

### Materials 17

Burnett, J.H., Levine, Z.H., & Shirley, E.L. (2001). "Intrinsic birefringence in calcium fluoride and barium fluoride". Physical Review B, 64(24), 241102. DOI: 10.1103/PhysRevB.64.241102

### Materials 18

Luke, K., Okawachi, Y., Lamont, M.R.E., Gaeta, A.L., & Lipson, M. (2015). "Broadband mid-infrared frequency comb generation in a Si3N4 microresonator". Optics Letters, 40(21), 4823–4826. DOI: 10.1364/OL.40.004823

### Materials 19

NIST Materials Measurement Laboratory (2024). Materials Properties Database. [Source](https://trc.nist.gov) — Accessed: March 2026

### Materials 20

Espenak, F. (2025–2026). Sky Event Almanacs — Greenwich Mean Time. AstroPixels.com. [Source](https://www.astropixels.com/almanac/almanac21) Accessed: April–May 2026. (Source for perihelion/perigee dates of inner planets and Moon.)

### Materials 21

Park, R.S., et al. (2021). "The JPL Planetary and Lunar Ephemerides DE440 and DE441". The Astronomical Journal, 161(3), 105. DOI: 10.3847/1538-3881/abd414 (Source for perihelion dates of outer planets: Jupiter to Pluto.)

### Materials 22

Meeus, J. (1998). Astronomical Algorithms, 2nd ed. Willmann-Bell Inc., Richmond, Virginia. ISBN: 978-0-943396-61-3. Chapter 50: Perigee and Apogee of the Moon, pp. 355–358. (Formula for lunar perigee — implemented in MPERC.txt.)
