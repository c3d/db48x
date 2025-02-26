# Constants library

The DB48X calculator features a library of constants covering mathematics,
physics, chemistry and computer science. The built-in constants can be extended
using the `config/constants.csv` configuration file.

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


## Chemistry constant

### NA constant

Avogradro constant is the number of constituent particles per mole.

Since the redefinition of the mole in 2019, the `NA` constant is therefore
defined as an exact value.

### k constant

The Boltzmann constant is the proportionality factor that relates the average
relative thermal energy of particles in a gas with the thermodynamic temperature
of the gas.

Since the 2019 redefinition of the SI units, the `k` constant is therefore
defined as an exact value.

### Vm constant

Molar volume of an ideal gas at 1 atmosphere of pressure and 0°C.

By convention, it is chosen as to be the exact result of `R*StdT/StdP`.

### R constant

The universal gas constant is the molar equivalent to the Boltzmann constant,
expressed in units of energy per temperature increment per amount of substance,
rather than energy per temperature increment per particle.

Since the 2019 redefinition of the SI units, the `R` constant is therefore
defined as an exact quantity.

### σ constant

Stefan-Boltzmann constant, the factor of proportionality in the Stefan-Boltzmann
law describing the intensity of the thermal radiation emitted by matter in terms
of that matter's temperature.

For an ideal absorber/emitter or black body, the Stefan–Boltzmann law states
that the total energy radiated per unit surface area per unit time (also known
as the radiant exitance) is directly proportional to the fourth power of the
black body's temperature, T:`M°=σ·T⁴`

Since the 2019 redefinition of the SI units, the `σ` constant is defined as an
exact result.

### StdT constant

Standard temperature as defined by IUPAC in 1982.

Standard temperature and pressure (STP) or Standard conditions for temperature
and pressure are various standard sets of conditions for experimental
measurements used to allow comparisons to be made between different sets of
data. The most used standards are those of the International Union of Pure and
Applied Chemistry (IUPAC, used by DB48X) and the National Institute of Standards
and Technology (NIST). Other organizations have established a variety of other
definitions. By convention, it is chosen as the exact value StdT=273.15_K`.

### StdP constant

Standard pressure as defined by IUPAC in 1982, corresponding to 1 atm

Standard temperature and pressure (STP) or Standard conditions for temperature
and pressure are various standard sets of conditions for experimental
measurements used to allow comparisons to be made between different sets of
data. The most used standards are those of the International Union of Pure and
Applied Chemistry (IUPAC, used by DB48X) and the National Institute of Standards
and Technology (NIST). Other organizations have established a variety of other
definitions. By convention, it is chosen as the exact value `StdP=101.325_kPa`.

### F constant

Faraday constant. In physical chemistry, the Faraday constant is a
physical constant defined as the quotient of the total electric charge
(`q`) by the amount (`n`) of elementary charge carriers in any given
sample of matter. It's an exact constant.

### Mu constant

Molar mass constant, defined as one twelfth of the molar mass of carbon-12.

The molar mass of an element or compound is its relative atomic mass (or atomic
weight, noted `Ar`) or relative molecular mass multiplied by the molar mass
constant.

Following the 2019 revision of the SI system, the
[Avogadro constant](#NA-constant) became exact. As a result the molar mass
constant is no longer exactly `1_g/mol`. For internal consistency this value
depends therefore on the mass unit `u` [u-constant](#u-constant) and is
calculated by the following expression: `NA·u`.

### MC12 constant

Molar mass of carbon-12. Since 1960, mole is the amount of substance of
a system which contains as many elementary entities as there are atoms in
12 gram of carbon-12. Since 2019, the SI definition of mole changed such
that the molar mass of carbone-12 remains nearly but no longer exactly
12 g/mol. For internal consistency this value depends therefore on the
mass unit `u` [u-constant](#u-constant) and is calculated by the following
expression: `12·Mu`.

### n0 constant

Loschmidt constant or Loschmidt's number is the number of particles
(atoms or molecules) of an ideal gas per volume (the number density),
and usually quoted at standard temperature and pressure. Since 2019 with
the redifinition of the mole, it is calculated exactly as: `NA·Vm`.

### SoR constant

Sakur-Tetrode constant gives the absolute entropy at an absolute temperature
of `T=1_K` and standard atmospheric pressure `StdP` for one mole of an ideal
monoatomic gas composed of particles of mass equal to the atomic mass constant.

This constant is used in the Sakur-Tetrode equation expression the entropy of a
monoatomic ideal gas in terms of its thermodynamic state.

### Da constant

The Dalton constant is the unit mass defined as one twelfth the mass of an
unbound neutral atom of carbon-12 in its nuclear and electronic ground state and
at rest.

### kq constant

This constant expresses the ratio of Boltzmann constant [k](#k-constant) to the
elementary charge [qe](#qe-constant). It has the exact value `kq=k/qe`.


## Physics constants

### ⅉ constant

Notation often used in physics for the imaginary verifying ⅉ²=-1.

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
directed along the line connecting their centres of mass:

![Newton Gravitation](img/NewtonGravitation.bmp)

### g constant

Acceleration of Earth gravity, equivalent to the free-fall acceleration.
Its value is exactly defined by convention as `g=9.80665_m/s²`.

### Z₀ constant

Vaccuum characteristic impedance, also called impedance of free space.

This constant relates the magnitudes of the electric and magnetic fields of
electromagnetic radiation travelling through free space: `Zo=|E|/|H|`. Its
value depends on the vacuum permittivity `μ0`.

### ε₀ constant

Vacuum permittivity, commonly denoted ε₀ (pronounced "epsilon nought" or
"epsilon zero"), is the value of the absolute dielectric permittivity of
classical vacuum. It may also be referred to as the permittivity of free space,
the electric constant, or the distributed capacitance of the vacuum. It is an
ideal (baseline) physical constant. It is a measure of how dense of an electric
field is "permitted" to form in response to electric charges and relates the
units for electric charge to mechanical quantities such as length and force.
Its value depends on the vaccuum permeability constant `μ0`.

### μ₀ constant

The vacuum magnetic permeability (variously vacuum permeability, permeability of
free space, permeability of vacuum, magnetic constant) is the magnetic
permeability in a classical vacuum. It is a physical constant, conventionally
written as μ₀ (pronounced "mu nought" or "mu zero"). It quantifies the strength
of the magnetic field induced by an electric current. Its value depends on the
fine structure constant measurement `α`.

### ke constant

Coulomb constant as it appears in the expression of the Coulomb force:
`Fe=ke·q1·q2/r^2`. Its value depends on the vacuum permittivity `ε0`.


## Mass constants

### me constant

Electron mass. In particle physics, the electron mass is the mass of a
stationary electron, also known as the invariant mass of the electron and it is
one of the fundamental constants of physics. Its value is closely related to
the unit mass measurement through the electron relative atomic mass:
`me=u·Ar(e)` where `Ar(e)`, i.e. [Are](#Are-constant), is determined
iteratively by frequency measurements using Penning trap. Its value can also be
calculated with the fine structure constant `α` and the Rysberg constant `R∞`.

### mn constant

Neutron mass measurement. Its value is determined experimentally by mass
spectrometry.

### mp constant

Proton mass measurement which is the hydrogen H-1 nucleus. Its value is
determined by particle drag race experiments.

### mH constant

Hydrogen mass measurement. The electrically neutral hydrogen H-1
atom contains a single positively charged proton in the nucleus, and
a single negatively charged electron bound to the nucleus by the Coulomb
force. Its value is measured by spectrometry.

### u constant

Unified atomic mass unit.

A unit of mass defined as one twelfth of the mass of an unbound neutral atom of
carbon-12 in its nuclear and electronic ground state and at rest. It is a non-SI
unit accepted for use with SI. It is identical to the
[Dalton](#dalton constant).

Its value can be determined from the calculation of the electron rest mass `me`
and the measurement of the electron relative atomic mass `Ar(e)`
[Are-constant](#Are-constant) (that is, the mass of electron divided by the
atomic mass constant).

### mD constant

Mass of the neutral deuterium atom which is a stable isotope of hydrogen.
The electrically neutral deuterium atom H-2 (also known as heavy hydrogen)
contains a single positively charged proton and a neutron in the nucleus,
and a single negatively charged electron bound to the nucleus by the
Coulomb force. Precise measurements of deuterium is obtained by
spectrometry.

### mT constant

Mass of the neutral tritium atom which is an unstable isotope of hydrogen
H-3 . Its nucleus contains a single positively charged proton and two
neutrons, surrounded by a single negatively charged electron bound to
the nucleus by the Coulomb force. Its mass is measured by spectrometry.

### mHe constant

Mass of the neutral helium atom. The electrically neutral helium atom
He-4 contains two positively charged protons and two neutrons, and two
negatively charged electrons bound to the nucleus by the Coulomb force.
Its mass is measured by spectrometry.

### mμ constant

Mass of the muon which is an unstable elementary particle similar
to the electron (both are classified as leptons), with an electric
charge of `−qe` and spin -1/2, but with a much greater mass. Its mass
is evaluated from energy conservation budget in pair creation reaction.

### mτ constant

Mass of the tau which is an elementary particle similar to the
electron (both are classidief as leptons), with an electric charge
of `−qe` and spin -1/2, but it is heaviest of leptons. Its mass is
evaluated from energy conservation budget in pair creation reaction.

### mpme constant

Dimensionless ratio between the mass of the proton `mp` and the mass
of the electron `me`. Currently, the most precise measurements of the
charge-to-mass ratio of a proton still use a magnetic field like Thompson
did, but rely on measuring (cyclotron) frequencies rather than deflection.

### Are constant

Electron relative atomic mass. The experimental value of the electron
relative atomic mass is an important constant (usually noted as
`Ar(e)`) which is needed to calculate the unit mass value `u`
[u-constant](#u-constant). According to COTATA2022, it is a dimensionless
quantity which is determined iteratively by frequency measurements using
Penning trap.


## Quantum constants

### h constant

The Planck constant is a fundamental physical constant that appears in quantum
mechanics. A photon's energy is equal to its frequency multiplied by the Planck
constant (`E=h·ν`), and the wavelength of a matter wave equals the Planck
constant divided by the associated particle momentum (`λ=h/p`). Since the 2019
SI redefinition, it has an exact value.

### ℏ constant

The reduced Planck constant, ℏ, also known as the Dirac Constant, is
exactly defined as `ℏ=h/2π`.

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
the electromagnetic field.

### ΔfCs constant

Caesium (Cs) hyperfine transition. It is the transition between the two
hyperfine ground states of the caesium atom. The frequency `ΔfCs` of this
transition is used to define the second as the official time unit of the
International System of Units (SI): one second is therefore the duration
of exactly 9192631770 cycles of this radiation. It represents the "tick"
of an extremely accurate atomic clock based on the properties of the
Caesium atom. By convention this constant is exact.

### θw constant

The weak mixing angle or Weinberg angle. It is a parameter in the
Weinberg–Salam theory of the electroweak interaction, part of the
Standard Model of particle physics. It is the angle by which spontaneous
symmetry breaking rotates the original W0 and B0 vector boson plane,
producing as a result the Z0 boson, and the photon. Its value is
calculated from the mesurement of the following dimensionless parameter
for the W and Z bosons: `(sinθw)^2 = 1 - (mW/mZ)^2 = 0.22305 ± 0.00023`.

### Lpl constant

Planck length unit. As an attempt to devise a universal and natural units
system, the Planck units are combinations of basic universal constants. It
is the smallest distance that can be measured, and it represents the
scale at which quantum gravity effects become dominant. It is the distance
travelled by light during one Planck time `Tpl`. Its value depends on the
measured value of the gravitational constant `G`.

### Tpl constant

Planck time unit. As an attempt to devise a universal and natural units
system, the Planck units are combinations of basic universal constants.
It is the shortest time interval that can be measured and it is fundamental
in the study of the universe beginning. It is the time required for light
to travel one Planck length `Lpl`. Its value depends on the measured value
of the gravitational constant `G`.

### Mpl constant

Planck mass unit. As an attempt to devise a universal and natural units
system, the Planck units are combinations of basic universal constants.
It can be viewed as the mass of a black hole with a Swarzhchild radius
of 2 Planck lengths (`rs=2·Lpl`) or, the minimum mass of a black hole
is one half of the Planck mass, the latter having a Planck Length radius.
Its value depends on the measured value of the gravitational constant
`G`.

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
value of the gravitational constant `G`.

### T°pl constant

Planck temperature unit. As an attempt to devise a universal and natural
units system, the Planck units are combinations of basic universal constants.
It is the highest temperature that conventional physics can describe. It's a
fundamental limit of quantum mechanics and is considered the temperature of
the universe during the Big Bang when quantum gravity effects became
dominant. Its value depends on the measured value of the gravitational
constant `G`.

### Eh constant

Hartree energy constant. It is a unit of energy used in atomic physics
and computational chemistry, which is also used in molecular orbital
calculations. It is approximately the negative electric potential energy
of an electron in a hydrogen atom's ground state, and also approximately
twice the ionization energy of a hydrogen atom. Its value depends on the
measured value of the Rydberg constant `R∞`.


## Electromagnetism constants

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
with value `1.60217733⁳⁻¹⁹` coulombs.

### λ0 constant

Photon wavelength. Photon energy can be expressed using any unit of energy
such as the electronvolt (eV) or the Joule (J). For short wavelength sources,
researchers often discuss photon energies in units of eV (or keV for hard
X-rays) out of convenience.  The SI definition for 1 eV derives from the
[definitional value of the electron charge](#me-constant). Photon energy `E`
in eV can be computed from wavelength `λ` in nm as: `E=λ0/λ`. This is an
exact constant.

### f0 constant

Photon frequency. This is the exact frequency associated to the
[photon wavelength λ0][#λ0-constant].

### ge constant

Electron g-factor. It is a dimensionless quantity that characterizes
the magnetic moment and angular momentum of an electron. It is the
ratio of the magnetic moment (or, equivalently, the gyromagnetic ratio)
of the electron to that expected of a classical particle of the same
charge and angular momentum. The electron g-factor is one of the most
precisely measured values in physics.

### qme constant

Ratio between the electron charge `qe` and its mass `me`. The uncertainty
of `qme` dépends on the one of `me`.

### μe constant

Electron magnetic moment. The electron magnetic moment, or more
specifically the electron magnetic dipole moment, is the magnetic
moment of an electron resulting from its intrinsic properties of spin
and electric charge. Its angular momentum comes from two types of
rotation: spin and orbital motion. Therefore an external magnetic field
exerts a torque on the electron magnetic moment revealing its existence.
It's a mearured quantity.

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
neutron magnetic moment revealing its existence. It's a measured quantity.

### μμ constant

Muon magnetic moment. It is the magnetic dipole moment of the meutron
resulting from its intrinsic properties of spin and electric charge.
Its angular momentum comes from two types of rotation: spin and orbital
motion. Therefore an external magnetic field exerts a torque on the muon
magnetic moment revealing its existence. It's a measured quantity.


## Size constants

### re constant

The classical electron radius. Through the Bohr radius `a0`, it
depends on fine structure constant `α`.

### rp constant

Proton charge radius. A direct measure of the proton radius. Since 2010,
the measure was done using either spectroscopy method with muonic hydrogen,
and then with deuterium atom, or either using a more recent electron-proton
scattering experiment.

### a0 constant

Bohr radius. The Bohr radius is a physical constant, approximately equal to
the most probable distance between the nucleus and the electron in a hydrogen
atom in its ground state. Its value depends on the vacuum electric
permittivity `ε0`.

### σe constant

The Thomson cross-section. This type of scattering is valid when the field
energy `h·ν` is much less than the rest mass of the electron `m0·c^2`, the
electric field of the incident wave accelerates the charged target-particle,
causing it, in turn, to emit radiation at the same frequency `ν` as the
incident wave, hence the scattering of the wave. Through the classical
electron radius `re`, its value depends on fine structure constant `α`.


## Magnetism constants

### μB constant

Bohr magneton. In atomic physics, the Bohr magneton is a physical constant
and the natural unit for expressing the magnetic moment of an electron caused
by its orbital or spin angular momentum. In SI units, the Bohr magneton
depends on the electron mass `me`.

### μN constant

The nuclear magneton is a physical constant of magnetic moment. It
is the standard unit used to measure the magnetic dipole moment of
atomic nuclei and nucleons (protons and neutrons), essentially acting
as a scale to quantify their magnetic strength. Defined in SI units,
it depends on the measured value of the proton mass `mp`.

### γe constant

Electron gyromagnetic ratio. It is the ratio of the electron's magnetic
moment to its angular momentum. It can be used to determine the direction
of precession and the resonance frequency of an electron in a magnetic
field. Its value depends on the electron magnetic moment `μe`.

### γp constant

Proton gyromagnetic ratio. It is the ratio of the proton's magnetic
moment to its angular momentum. It can be used to determine the direction
of precession and the resonance frequency of a proton in a magnetic field.
Its value depends on the proton magnetic moment `μp`.

### γn constant

Neutron gyromagnetic ratio. It is the ratio of the Neutron's magnetic
moment to its angular momentum. It is a characteristic of the neutron's
nuclear spin and its sign determines the direction of precession. Its
value depends on the neutron magnetic moment `μn`.

### R∞ constant

Rydberg constant. In spectroscopy, the Rydberg constant is a physical
constant relating to the electromagnetic spectra of an atom. The constant
first arose as an empirical fitting parameter in the Rydberg formula for
the hydrogen spectral series, but Niels Bohr later showed that its value
is related to more fundamental constants according to his model of the
atom. The Rydberg constant value is inferred from measurements of atomic
transition frequencies in three different atoms (hydrogen, deuterium,
and antiprotonic helium).

### Rk constant

Von Klitzing constant. It appears in the expression of the Hall
resistance `Rxy=Rk/ν` (`ν` being either an integer or a fraction)
of the quantum Hall effect, a quantized version of the Hall effect
which is observed in two-dimensional electron systems subjected to
low temperatures and strong magnetic fields. It's an exact constant.

### G0 constant

Conductance quantum constant. It is the quantized unit of electrical
conductance. It is required when measuring the conductance of a quantum
point contact, and also, it appears explicitly in the Landauer formula:
`G(μ)=G0·ΣTn(μ) sum over n` which relates the electrical conductance of
a quantum conductor to its quantum properties. It's an exact constant.

### G0F constant

Fermi reduced coupling constant. It is a fundamental physical constant
that represents the strength of the weak nuclear interaction, essentially
indicating how readily particles can interact via the weak force; a
larger value signifies a stronger interaction, and it is a key parameter
in the Standard Model of particle physics, primarily used to calculate
the decay rates of particles involved in weak interactions like beta
decay. It's a measured quantity.

### c1 constant

First radiation constant. This constant appears in the Radiance
expression of the Planck's law: `Bλ(λ;T)=c1/λ^5/EXPM1(c2/λT)`.
It's an exact constant.

### c2 constant

Second radiation constant. This constant appears in the Radiance
expression of the Planck's law: `Bλ(λ;T)=c1/λ^5/EXPM1(c2/λT)`.
It's an exact constant.

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
expression (using Lambert W function), it's an exact constant.

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
expression (using Lambert W function), it's an exact constant.

### ø constant

Magnetic flux quantum. The (superconducting) magnetic flux quantum is a
combination of fundamental physical constants: the Planck constant `h`
and the electron charge `qe`. Its value is, therefore, the same for
any superconductor. It's an exact constant.

### KJ constant

Josephson constant. The Josephson constant is a constant of
proportionality that relates the potential difference across a
Josephson junction to the frequency of irradiation. It's also
the inverse of the magnetic flux quantum `ø`. It's an exact
constant.

### Kc constant

Quantum of circulation constant. It represents the discrete unit
of circulation in a superfluid, meaning that such circulation around
a vortex can only occur in multiples of this value. The existence
of quantum vortices was first predicted by Lars Onsager in 1949 in
connection with superfluid helium. It is defined as the ratio of
Planck's constant `h` to the mass of the relevant particle `m`
chosen here as the electron.


## Scattering constants

### λc constant

Electron Compton wavelength. The Compton wavelength is a quantum
mechanical property of a particle, defined as the wavelength of a
photon whose energy is the same as the rest energy of that particle
(based on the mass–energy equivalence). The standard Compton wavelength
`λ` of a particle of mass `m` is given by `λ=h/(m·c)`. Since it is
defined here for the electron, it depends on the value of the electron
mass `me`.

### λcp constant

Proton Compton wavelength. The Compton wavelength is a quantum
mechanical property of a particle, defined as the wavelength of
a photon whose energy is the same as the rest energy of that particle
(based on the mass–energy equivalence). The standard Compton wavelength
`λ` of a particle of mass `m` is given by `λ=h/(m·c)`. Since it is
defined here for the proton, it depends on the measured value of the
proton mass `mp`.

### λcn constant

Neutron Compton wavelength. The Compton wavelength is a quantum
mechanical property of a particle, defined as the wavelength of
a photon whose energy is the same as the rest energy of that particle
(based on the mass–energy equivalence). The standard Compton wavelength
`λ` of a particle of mass `m` is given by `λ=h/(m·c)`. Since it is
defined here for the neutron, it depends on the measured value of the
neutron mass `mn`.

### λcμ constant

Muon Compton wavelength. The Compton wavelength is a quantum
mechanical property of a particle, defined as the wavelength of
a photon whose energy is the same as the rest energy of that particle
(based on the mass–energy equivalence). The standard Compton wavelength
`λ` of a particle of mass `m` is given by `λ=h/(m·c)`. Since it is
defined here for the muon particle, it depends on the measured value of
the muon mass `mμ`.

### λcτ constant

Tau Compton wavelength. The Compton wavelength is a quantum
mechanical property of a particle, defined as the wavelength of
a photon whose energy is the same as the rest energy of that particle
(based on the mass–energy equivalence). The standard Compton wavelength
`λ` of a particle of mass `m` is given by `λ=h/(m·c)`. Since it is
defined here for the tau particle, it depends on the measured value of
the tau mass `mτ`.


## Materials constants

### ε₀q constant

Ratio of the vacuum permittivity [vaccum permittivity](#ε₀-constant) to
the elementary charge [elementary charge](#qe-constant): `ε₀q=ε₀/qe`.

### qε₀ constant

Product of the vaccum permittivity [vaccum permittivity](#ε₀-constant) by
the elementary charge [elementary charge](#qe-constant): `qε₀=ε₀·qe`.

### εsi constant

Dielectric constant of silicon. The dielectric constant (or relative
permittivity), is a material property that measures how well an applied electric
field can penetrate a dielectric medium compared to a vacuum. A higher value
corresponds to a lower penetration. It has an exact value by convention.

### εox constant

SiO2 dielectric constant. The dielectric constant (or relative
permittivity), is a material property that measures how well an applied electric
field can penetrate a dielectric medium compared to a vacuum. A higher value
corresponds to a lower penetration. By convention it has an exact value.

### I₀ constant

Reference sound intensity. Sound intensity level or acoustic intensity level is
the level of the intensity of a sound relative to a reference value. It is a
logarithmic quantity, most often expressed in decibels dB. I₀ is the sound
intensity used as a reference, corresponding to `β=0_dB` due to the definition
of the sound pressure level `β=10·LOG10(I/I₀)` where `I=I₀`. By convention it
has an exact value.


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
