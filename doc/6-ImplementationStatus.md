# Implementation status

This section documents the implementation status for all HP50 RPL commands as
listed in the HP50G Advanced Reference Manual. This is a strict superset of the
HP48 implementation.

* [Implemented](#implemented-commands)
* [Not implemented](#unimplemented-commands)
* [Unapplicable commands](#unapplicable-commands)
* [Additional](#additional-commands)


# Implemented commands

The following is a list of the HP50 RPL commands which are implemented in DB48X.

* `!` Factorial
* `+` Addition
* `<` Less than
* `==` Object equality
* `=` Mathematical equality
* `>` Greater than
* `ABS` Absolute value
* `ACOSH` Inverse hyperbolic cosine
* `ACOS` Arc cosine
* `ADD` Addition (differs from the HP Add command)
* `ALOG`
* `AND`
* `APPLY`
* `ARG`
* `ARRY→`
* `→ARRY`
* `ASINH`
* `ASIN`
* `ASN`
* `ASR`
* `ASRC`
* `ATANH`
* `ATAN`
* `AXES`
* `BARPLOT`
* `BAR`
* `BEEP`
* `BESTFIT`
* `BIN`
* `BYTES`
* `B→R`
* `CASE`
* `CEIL`
* `CF`
* `%CH`
* `CHR`
* `CLEAR`
* `CLLCD`
* `CLΣ`
* `COLLECT`
* `COLΣ`
* `COMB`
* `CON`
* `CONJ`
* `CONLIB`
* `CONST`
* `CONSTANTS`
* `CONT`
* `CONVERT`
* `COSH`
* `COS`
* `COV`
* `CRDIR`
* `CROSS`
* `C→R`
* `DATE`
* `→DATE`
* `DATE+`
* `DDAYS`
* `DBUG`
* `DEC`
* `DECR`
* `DEDICACE`
* `DEG`
* `DEPTH`
* `DET`
* `DIR`
* `DISP`
* `DISPXY`
* `DIV2`
* `DO`
* `DOERR`
* `DOLIST`
* `DOSUBS`
* `DOT`
* `DRAW`
* `DRAX`
* `DROP2`
* `DROPN`
* `DROP`
* `DTAG`
* `DUP2`
* `DUPDUP`
* `DUPN`
* `DUP`
* `D→R`
* `e`
* `EDIT`
* `ELSE`
* `END`
* `ENDSUB`
* `ENG`
* `EQNLIB`
* `ERR0`
* `ERRM`
* `ERRN`
* `EVAL`
* `EXPAND`
* `EXPAN`
* `EXPFIT`
* `EXPM`
* `EXP`
* `FACT`
* `FC?C`
* `FC?`
* `FIX`
* `FLOOR`
* `FOR`
* `FP`
* `FREEZE`
* `FS?C`
* `FS?`
* `FUNCTION`
* `GAMMA`
* `GET`
* `GETI`
* `GOR`
* `GROB`
* `→GROB`
* `GROBADD`
* `GXOR`
* `HALT`
* `HEAD`
* `HELP` (Different meaning)
* `HEX`
* `HOME`
* `HMS–`
* `HMS+`
* `HMS→`
* `→HMS`
* `IDN`
* `IDIV2`
* `IFERR`
* `IFTE`
* `IFT`
* `IF`
* `IM`
* `INCR`
* `INV`
* `IP`
* `ISOL`
* `KILL`
* `LASTARG`
* `LINE`
* `LINFIT`
* `LIST→`
* `∆LIST`
* `ΠLIST`
* `ΣLIST`
* `LNP1`
* `LN`
* `LNAME`
* `LOG`
* `LOGFIT`
* `LR`
* `MAP`
* `↓MATCH`
* `↑MATCH`
* `MAX`
* `MAXΣ`
* `MEAN`
* `MEM`
* `MENU`
* `MIN`
* `MINΣ`
* `MOD`
* `MROOT`
* `NDUPN`
* `NEG`
* `NEWOB`
* `NEXT`
* `NIP`
* `NOT`
* `NSUB`
* `NΣ`
* `NUM`
* `OBJ→`
* `OCT`
* `OFF`
* `OR`
* `OVER`
* `PARAMETRIC`
* `PATH`
* `PCOV`
* `PERM`
* `PGDIR`
* `PICK`
* `PICK3`
* `PICT`
* `PIX?`
* `PIXOFF`
* `PIXON`
* `POLAR`
* `PMAX`
* `PMIN`
* `PURGE`
* `PUT`
* `PUTI`
* `PVAR`
* `PWRFIT`
* `RAD`
* `RAND`
* `RANM`
* `RCEQ`
* `RCL`
* `RCLKEYS`
* `RCLMENU`
* `RCLΣ`
* `RCWS`
* `RDZ`
* `RE`
* `RECT` (Different meaning: draws a rectangle)
* `REPEAT`
* `REVLIST`
* `REWRITE` (Different meaning: performs a rewrite)
* `RISCH`
* `RND`
* `RL`
* `RLB`
* `RLC`
* `ROLLD`
* `ROLL`
* `ROOT`
* `ROT`
* `RR`
* `RRB`
* `RRC`
* `R→B`
* `R→C`
* `R→D`
* `SAME`
* `SCALE`
* `SCALEH`
* `SCALEW`
* `SCI`
* `SF`
* `SIGN`
* `SINH`
* `SIN`
* `SL`
* `SLB`
* `SLC`
* `SORT`
* `SQ`
* `SR`
* `SRB`
* `SST`
* `SST↓`
* `START`
* `STD`
* `STEP`
* `STEQ`
* `STOKEYS`
* `STORE` (Different meaning: long form of STO)
* `STO`
* `STO+`
* `STO–`
* `STO*`
* `STO/`
* `STOΣ`
* `STR→`
* `STWS`
* `SUBST`
* `SWAP`
* `%T`
* `TAIL`
* `TANH`
* `TAN`
* `TEVAL`
* `THEN`
* `TICKS`
* `TIME`
* `→TIME`
* `TMENU`
* `TOT`
* `TRNC`
* `TVARS`
* `TYPE`
* `UBASE`
* `UFACT`
* `UNPICK`
* `UNROT`
* `UNTIL`
* `UPDIR`
* `UVAL`
* `V→`
* `→V2`
* `→V3`
* `VAR`
* `VARS`
* `VERSION`
* `WAIT`
* `WHILE`
* `XCOL`
* `XOR`
* `XPON`
* `XRNG`
* `XROOT`
* `YRNG`
* `YCOL`
* `%`
* `^` (Power)
* `i`
* `«»` (Program delimiters)
* `×` (Multiply)
* `÷` (Divide)
* `Σ+`
* `ΣXY`
* `ΣX`
* `ΣX²`
* `ΣY`
* `ΣY²`
* `Σ–`
* `π` (Pi)
* `–` (Subtract)
* `→LIST`
* `→NUM`
* `→Q`
* `→STR`
* `→TAG`
* `→UNIT`
* `?`
* `|` (Where)
* `Σ`
* `→` (Create Local)
* `∂`
* `√` (Square root)
* `∞`
* `∫` (Integrate)
* `≠` (Not equal)
* `≤` (Less than or equal)
* `≥` (Greater than or Equal)
* `▶` (Store)
* `＿` (Unit attachment)



# Unimplemented commands

The following is a list of unimplemented HP50 RPL commands which should be implemented by the time the project reaches version 1.0.

* `ABCUV`
* `ACK`
* `ACKALL`
* `ACOS2S`
* `ADDTMOD`
* `ADDTOREAL`
* `ALGB`
* `AMORT`
* `ANIMATE`
* `ANS`
* `ARC`
* `ARCHIVE`
* `ARIT`
* `ASIN2C`
* `ASIN2T`
* `ASSUME`
* `ATAN2S`
* `ATICK`
* `AUGMENT`
* `AUTO`
* `AXL`
* `AXM`
* `AXQ`
* `BASIS`
* `BINS`
* `BLANK`
* `BOX`
* `C2P`
* `CASCFG`
* `CASCMD`
* `CENTR`
* `CHINREM`
* `CHOLESKY`
* `CHOOSE`
* `CIRC`
* `CLUSR`
* `CLVAR`
* `CMPLX`
* `CNRM`
* `→COL`
* `COL→`
* `COL–`
* `COL+`
* `COLCT`
* `COND`
* `CONIC`
* `CORR`
* `CSWP`
* `CURL`
* `CYCLOTOMIC`
* `CYLIN`
* `C→PX`
* `DARCY`
* `DEF`
* `DEFINE`
* `DEGREE`
* `DELALARM`
* `DELKEYS`
* `DEPND`
* `DERIV`
* `DERVX`
* `DESOLVE`
* `DIAG→`
* `→DIAG`
* `DIAGMAP`
* `DIFF`
* `DIFFEQ`
* `DISTRIB`
* `DIV`
* `DIV2MOD`
* `DIVIS`
* `DIVMOD`
* `DIVPC`
* `dn`
* `DOMAIN`
* `DRAW3DMATRIX`
* `DROITE`
* `EDITB`
* `EGCD`
* `EGV`
* `EGVL`
* `ENDSUB`
* `EPSX0`
* `EQW`
* `EQ→`
* `ERASE`
* `EULER`
* `EXLR`
* `EXP&LN`
* `EXP2HYP`
* `EXP2POW`
* `EXPANDMOD`
* `EXPLN`
* `EYEPT`
* `F0λ`
* `FACTOR`
* `FACTORMOD`
* `FACTORS`
* `FANNING`
* `FAST3D`
* `FCOEF`
* `FDISTRIB`
* `FFT`
* `FINDALARM`
* `FONT6`
* `FONT7`
* `FONT8`
* `FONT→`
* `→FONT`
* `FOURIER`
* `FROOTS`
* `FXND`
* `GAUSS`
* `GBASIS`
* `GCD`
* `GCDMOD`
* `GRAD`
* `GRAMSCHMIDT`
* `GRAPH`
* `GREDUCE`
* `GRIDMAP`
* `*H`
* `HADAMARD`
* `HALFTAN`
* `HEADER→`
* `→HEADER`
* `HERMITE`
* `HESS`
* `HILBERT`
* `HISTOGRAM`
* `HISTPLOT`
* `HORNER`
* `IABCUV`
* `IBASIS`
* `IBERNOULLI`
* `IBP`
* `ICHINREM`
* `IEGCD`
* `IFFT`
* `ILAP`
* `IMAGE`
* `INDEP`
* `INFORM`
* `INPUT`
* `INT`
* `INTEGER`
* `INTVX`
* `INVMOD`
* `IQUOT`
* `IREMAINDER`
* `ISOM`
* `ISPRIME?`
* `I→R`
* `JORDAN`
* `KER`
* `KEY`
* `KEYEVAL`
* `LABEL`
* `LAGRANGE`
* `LAP`
* `LAPL`
* `LAST`
* `LCD→`
* `→LCD`
* `LCM`
* `LCXM`
* `LDEC`
* `LEGENDRE`
* `LGCD`
* `lim`
* `LIMIT`
* `LIN`
* `ΣLINE`
* `LININ`
* `LINSOLVE`
* `LNCOLLECT`
* `LOCAL`
* `LQ`
* `LSQ`
* `LU`
* `LVAR`
* `MAD`
* `MAIN`
* `MANT`
* `MATHS`
* `MATR`
* `MAXR`
* `MCALC`
* `MENUXY`
* `MINIFONT→`
* `→MINIFONT`
* `MINIT`
* `MINR`
* `MITM`
* `MKISOM`
* `MODSTO`
* `MODULAR`
* `MOLWT`
* `MSGBOX`
* `MSLV`
* `MSOLVR`
* `MULTMOD`
* `MUSER`
* `NDIST`
* `NEXTPRIME`
* `NOVAL`
* `NUMX`
* `NUMY`
* `ORDER`
* `P2C`
* `PA2B2`
* `PARSURFACE`
* `PARTFRAC`
* `PCAR`
* `PCOEF`
* `PCONTOUR`
* `PDIM`
* `PERINFO`
* `PERTBL`
* `PEVAL`
* `PICTURE`
* `PLOT`
* `PLOTADD`
* `PMINI`
* `POLYNOMIAL`
* `POP`
* `POS`
* `POTENTIAL`
* `POWEXPAND`
* `POWMOD`
* `PREDV`
* `PREDX`
* `PREDY`
* `PREVAL`
* `PREVPRIME`
* `PROMPT`
* `PROMPTSTO`
* `PROOT`
* `PROPFRAC`
* `PSDEV`
* `PSI`
* `Psi`
* `PTAYL`
* `PTPROP`
* `PUSH`
* `PVIEW`
* `PX→C`
* `→Qπ`
* `qr`
* `QR`
* `QUAD`
* `QUOT`
* `QUOTE`
* `QXA`
* `RANK`
* `RATIO`
* `RCI`
* `RCIJ`
* `RCLALARM`
* `RCLF`
* `RCLVX`
* `RDM`
* `REF`
* `REMAINDER`
* `RENAME`
* `REORDER`
* `REPL`
* `RES`
* `RESULTANT`
* `RKF`
* `RKFERR`
* `RKFSTEP`
* `RNRM`
* `ROW–`
* `ROW+`
* `ROW→`
* `→ROW`
* `rref`
* `RREF`
* `RREFMOD`
* `RRK`
* `RRKSTEP`
* `RSBERR`
* `RSD`
* `RSWP`
* `RULES`
* `R→I`
* `SCATRPLOT`
* `SCATTER`
* `SCHUR`
* `SCLΣ`
* `SCONJ`
* `SCROLL`
* `SDEV`
* `SEQ`
* `SERIES`
* `SEVAL`
* `SHOW`
* `SIDENS`
* `SIGMA`
* `SIGMAVX`
* `SIGNTAB`
* `SIMP2`
* `SIMPLIFY`
* `SINCOS`
* `SINV`
* `SIZE`
* `SLOPEFIELD`
* `SNEG`
* `SNRM`
* `SOLVE`
* `SOLVEQN`
* `SOLVER`
* `SOLVEVX`
* `SPHERE`
* `SRAD`
* `SREPL`
* `STOALARM`
* `STOF`
* `STOVX`
* `STREAM`
* `STURM`
* `STURMAB`
* `SUB`
* `SUBTMOD`
* `SVD`
* `SVL`
* `SYLVESTER`
* `SYST2MAT`
* `TABVAL`
* `TABVAR`
* `TAN2CS2`
* `TAN2SC`
* `TAN2SC2`
* `TAYLOR0`
* `TAYLR`
* `TCHEBYCHEFF`
* `TCOLLECT`
* `TDELTA`
* `TESTS`
* `TEXPAND`
* `TEXT`
* `TINC`
* `TLIN`
* `TLINE`
* `TRACE`
* `TRAN`
* `TRIG`
* `TRIGCOS`
* `TRIGO`
* `TRIGSIN`
* `TRIGTAN`
* `TRN`
* `TRUNC`
* `TRUTH`
* `TSIMP`
* `TSTR`
* `TVM`
* `TVMBEG`
* `TVMEND`
* `TVMROOT`
* `UNASSIGN`
* `UNASSUME`
* `UNBIND`
* `UTPC`
* `UTPF`
* `UTPN`
* `UTPT`
* `VANDERMONDE`
* `VER`
* `VISIT`
* `VISITB`
* `VPOTENTIAL`
* `VTYPE`
* `*W`
* `WIREFRAME`
* `XNUM`
* `XQ`
* `XVOL`
* `XXRNG`
* `YSLICE`
* `YVOL`
* `YYRNG`
* `ZEROS`
* `ZFACTOR`
* `ZVOL`
* `;` (Semicolon)

## Unapplicable commands

The following commands are not applicable to the DB48X implementation of RPL,
for example because they are dealing with hardware details or system-level
features that have no equivalent on the hardware DB48X runs on.
As a result, they behave like normal names on DB48X.

* `ATTACH`
* `BAUD`
* `BUFLEN`
* `C$`
* `CKSM`
* `CLKADJ`
* `CLOSEIO`
* `CR`
* `DELAY`
* `DETACH`
* `FILER`
* `FINISH`
* `FLASHEVAL`
* `FREE`
* `KERRM`
* `→KEYTIME`
* `KEYTIME→`
* `KGET`
* `LANGUAGE→`
* `→LANGUAGE`
* `LIBEVAL`
* `LIBS`
* `MERGE`
* `MINEHUNT`
* `→NDISP`
* `OLDPRT`
* `OPENIO`
* `PARITY`
* `PINIT`
* `PKT`
* `PR1`
* `PRLCD`
* `PRST`
* `PRSTC`
* `PRVAR`
* `PVARS`
* `RECN`
* `RECV`
* `RESTORE`
* `ROMUPLOAD`
* `RPL>`
* `SBRK`
* `SEND`
* `SERVER`
* `SRECV`
* `STIME`
* `STRM`
* `SYSEVAL`
* `TRANSIO`
* `UFL1→MINIF`
* `WSLOG`
* `XGET`
* `XMIT`
* `XPUT`
* `XRECV`
* `XSEND`
* `XSERV`


## Additional commands

The following commands are unique to DB48X and are not found in any
Hewlett-Packard RPL implementation.

* `atan2` Arc-tangent from two arguments
* `AngleUnitsMenu`
* `AnglesMenu`
* `ApplyInverseUnit`
* `ApplyUnit`
* `AreaUnitsMenu`
* `AutoSimplify` Automatically simplify expressions
* `BASE` Select an arbitrary base for based numbers
* `Background` Select background pattern for graphic operations
* `BasedDotOrComma` Use dot or comma as based number digit separator
* `BasedSpaces` Use thin spaces as based number digit separator
* `BasedSpacing` Grouping of digits for based numbers
* `BasedTicks` Use tick marsk `'` as based number digit separator
* `BasedUnderscore` Use underscore `_` as based number digit separator
* `BasesMenu`
* `CBRT` Cube root
* `CYCLE` Cycle between object representations
* `Capitalized` Show commands capitalized
* `Catalog` Present catalog of all functions with auto-completion
* `CharsMenu`
* `CircularMenu`
* `ClassicExponent` Use E as exponent marker, e.g. 1.3E128
* `ClearThingsMenu`
* `CompareMenu`
* `ComplexMenu`
* `ComputerUnitsMenu`
* `ConstantsMenu`
* `ConvertToUnitPrefix`
* `ConvertToUnit`
* `CursorBlinkRate` Select cursor blink rate in milliseconds
* `DebugMenu`
* `DecimalComma` Select comma as decimal separator
* `DecimalDot` Select dot as decimal separator
* `DifferentialSolverMenu`
* `DisplayModesMenu`
* `EQUIV` Logical equivalence
* `ERFC` Complementary error function
* `ERF` Error function
* `EXCLUDES` Logical exclusion
* `EditMenu`
* `EditorBegin`
* `EditorClear`
* `EditorCopy`
* `EditorCut`
* `EditorEnd`
* `EditorFlip`
* `EditorFontSize` Select font size for text editor
* `EditorMultilineFontSize` Select font size for multi-line text editor
* `EditorPaste`
* `EditorReplace`
* `EditorSearch`
* `EditorSelect`
* `EditorWordLeft`
* `EditorWordRight`
* `ElectricityUnitsMenu`
* `EnergyUnitsMenu`
* `EquationsMenu`
* `ExpLogMenu`
* `FancyExponent` Use power-of-ten rendering, e.g. 1.3×₁₀¹²⁸
* `FilesMenu`
* `FinanceSolverMenu`
* `FlagsMenu`
* `FlatMenus` Flatten menus (no use of shift)
* `ForceUnitsMenu`
* `Foreground` Select foreground pattern for graphic operations
* `FractionSpacing` Grouping of digits for fractional part of numbers
* `FractionsMenu`
* `GAND` Graphical And
* `GarbageCollect`
* `GraphicsMenu`
* `GraphicsStackDisplay` Select graphic display of the stack
* `HYPOT` Hypothenuse
* `HideBuiltinUnits`
* `HyperbolicMenu`
* `IMPLIES` Logical implication
* `IOMenu`
* `IntegrationMenu`
* `LastMenu` Select last menu
* `LastX` Return last X argument (for easier translation of RPN programs)
* `LengthUnitsMenu`
* `LibrariesMenu`
* `LightUnitsMenu`
* `LineWidth` Select line width for line drawing operations
* `LinearSolverMenu`
* `ListMenu`
* `LongForm` Show commands in long form
* `LoopsMenu`
* `LowerCase` Show commands in lowercase
* `MainMenu`
* `MantissaSpacing` Grouping of digits for whole part of numbers
* `MassUnitsMenu`
* `MathMenu`
* `MathModesMenu`
* `MatrixMenu`
* `MaxNumberBits` Maximum number of bits used by a number
* `MaxRewrites` Maximum number of equation rewrites
* `MemoryMenu`
* `MenuFirstPage`
* `MenuNextPage`
* `MenuPreviousPage`
* `MinimumSignificantDigits` adjustment of FIX mode switch to SCI
* `ModesMenu`
* `MultiSolverMenu`
* `NAND` Not And
* `NOR` Not Or
* `NoAutoSimplify` Do not automatically simplify expressions
* `NoTrailingDecimal` display 1.0 as 1
* `NumberDotOrComma` Use dot or comma as digit group separator
* `NumberSpaces` Use thin spaces as digit group separator
* `NumberTicks` Use tick marks `'` as digit group separator
* `NumberUnderscore` Use underscore `_` as digit group separator
* `NumbersMenu`
* `NumericResults` Produce numeric (decimal) results
* `NumericalSolverMenu`
* `ObjectMenu`
* `PIRADIANS` Angle mode with multiples of pi
* `PartsMenu`
* `PlotMenu`
* `PolynomialSolverMenu`
* `PolynomialsMenu`
* `PowerUnitsMenu`
* `PowersMenu`
* `Precision` Select decimal computing precision
* `PressureUnitsMenu`
* `PrintingMenu`
* `ProbabilitiesMenu`
* `ProgramMenu`
* `REM` remainder
* `RadiationUnitsMenu`
* `RealMenu`
* `ResultFontSize` Select font size to display level 1 of stack
* `RoundedMenus` Select round menu style
* `SIG` Significant digits mode
* `SaveState` Save system state to current state file
* `SelfInsert`
* `SeparatorModesMenu`
* `ShowBuiltinUnits`
* `SignalProcessingMenu`
* `SingleRowMenus` Display menus on single row
* `SolverMenu`
* `SpeedUnitsMenu`
* `SquareMenus` Select square (C47-like) menu style
* `StackFontSize` Select font size to display levels above 1 of stack
* `StackMenu`
* `StandardExponent` Display with standard exponent mode
* `StatisticsMenu`
* `SymbolicMenu`
* `SymbolicResults` Produce symbolic results
* `SymbolicSolverMenu`
* `SystemMemory`
* `SystemSetup` Enter DMCP system setup menu
* `Tag→`
* `TemperatureUnitsMenu`
* `TestsMenu`
* `TextMenu`
* `TextStackDisplay` Select text-only display of the stack
* `ThreeRowsMenus` Display menus on up to three rows
* `TimeMenu`
* `TimeUnitsMenu`
* `ToFractionDigits` Required digits of precision for →Q
* `ToFractionIterations` Max number of iterations for →Q
* `ToolsMenu` Automatically select a menu based on context
* `TrailingDecimal` display 1.0 with trailing decimal separator
* `TypeName`
* `Undo` Restore stack to state before command
* `UnitsConversionsMenu`
* `UnitsMenu`
* `UpperCase` Show commands in uppercase
* `UserInterfaceModesMenu`
* `VariablesMenuExecute`
* `VariablesMenuRecall`
* `VariablesMenuStore`
* `VariablesMenu`
* `VectorMenu`
* `ViscosityUnitsMenu`
* `VolumeUnitsMenu`
