# DB48X Menu Tree

Soft-menu hierarchy of the DB48X calculator.
Buttons are shown as they appear on the calculator screen:
**6 columns** (F1–F6) and **3 rows** per page.

**Legend**

| Style | Meaning |
|:------|:--------|
| **[Menu]** | Opens a sub-menu (defined in this document) |
| ***[Ext]*** | Opens an external menu (Constants, Units, …) |
| [Command] | Executes a command — link leads to the reference doc |
| Command | Command with no documentation entry found |
| *Unimplemented* | Not yet implemented |
| `text` | Inserts literal text in the command line |

---

## Contents

- [AlarmMenu](#alarmmenu)
- [AlgebraMenu](#algebramenu)
- [AnglesMenu](#anglesmenu)
- [ArithmeticMenu](#arithmeticmenu)
- [BasesMenu](#basesmenu)
- [CalculationMenu](#calculationmenu)
- [CircularMenu](#circularmenu)
- [ClearThingsMenu](#clearthingsmenu)
- [CompareMenu](#comparemenu)
- [ComplexMenu](#complexmenu)
- [DateMenu](#datemenu)
- [DebugMenu](#debugmenu)
- [DifferentialSolverMenu](#differentialsolvermenu)
- [DisplayModesMenu](#displaymodesmenu)
- [EditMenu](#editmenu)
- [ExpLogIdentitiesMenu](#explogidentitiesmenu)
- [FilesMenu](#filesmenu)
- [FlagsMenu](#flagsmenu)
- [FractionsMenu](#fractionsmenu)
- [GraphicsMenu](#graphicsmenu)
- [HyperbolicMenu](#hyperbolicmenu)
- [IntegrationMenu](#integrationmenu)
- [IOMenu](#iomenu)
- [LinearSolverMenu](#linearsolvermenu)
- [ListMenu](#listmenu)
- [LoopsMenu](#loopsmenu)
- [MainMenu](#mainmenu)
- [MathMenu](#mathmenu)
- [MathModesMenu](#mathmodesmenu)
- [MatrixMenu](#matrixmenu)
- [MemoryMenu](#memorymenu)
- [ModesMenu](#modesmenu)
- [MultiSolverMenu](#multisolvermenu)
- [NumbersMenu](#numbersmenu)
- [NumericalSolverMenu](#numericalsolvermenu)
- [ObjectMenu](#objectmenu)
- [PartsMenu](#partsmenu)
- [PlotMenu](#plotmenu)
- [PolynomialsMenu](#polynomialsmenu)
- [PolynomialSolverMenu](#polynomialsolvermenu)
- [PowersMenu](#powersmenu)
- [PrintingMenu](#printingmenu)
- [ProbabilitiesMenu](#probabilitiesmenu)
- [ProgramMenu](#programmenu)
- [RangeMenu](#rangemenu)
- [RealMenu](#realmenu)
- [SeparatorModesMenu](#separatormodesmenu)
- [SignalProcessingMenu](#signalprocessingmenu)
- [SolverMenu](#solvermenu)
- [StackMenu](#stackmenu)
- [StatisticsMenu](#statisticsmenu)
- [SymbolicMenu](#symbolicmenu)
- [SymbolicSolverMenu](#symbolicsolvermenu)
- [TestsMenu](#testsmenu)
- [TextMenu](#textmenu)
- [TimeMenu](#timemenu)
- [TrigIdentitiesMenu](#trigidentitiesmenu)
- [UnitsConversionsMenu](#unitsconversionsmenu)
- [UserInterfaceModesMenu](#userinterfacemodesmenu)
- [UserModeMenu](#usermodemenu)
- [VectorMenu](#vectormenu)

---

## AlarmMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| *AckAll* | **[Time](#timemenu)** | **[Date](#datemenu)** | &nbsp; | &nbsp; | &nbsp; |
| *Alarm* | *Ack* | *→Alarm* | *Alarm→* | *FindAlm* | *DelAlm* |

## AlgebraMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Ⓓ](commands/symbolic.md#algebraconfiguration) | [ⓧ](commands/symbolic.md#algebravariable) | [Stoⓧ](commands/symbolic.md#storealgebravariable) | [Final](commands/symbolic.md#finalalgebraresults) | &Wild | **[Symb](#symbolicmenu)** |
| [∂](commands/symbolic.md#derivative) | ∫ | ∑ | ∏ | *∆* | *→Qπ* |
| [↓Match](commands/symbolic.md#match-1) | [↑Match](commands/symbolic.md#match) | [Isolate](commands/symbolic.md#isolate) | Apply | Subst | [\|](commands/symbolic.md#where) |

## AnglesMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [→DMS](commands/system.md#dms) | [DMS→](commands/system.md#dms-1) | DMS+ | DMS- | **[Math](#mathmodesmenu)** | **[Modes](#modesmenu)** |
| →Deg | →Rad | →Grad | →πr | [→Polar](commands/angles.md#polar) | [→Rect](commands/complex.md#torectangular) |
| Deg | Rad | Grad | [πr](commands/angles.md#piradians) | [D→R](commands/angles.md#dr) | [R→D](commands/angles.md#rd) |

## ArithmeticMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| *Show* | *Quote* | [\|](commands/symbolic.md#where) | `=` | *Rules* | **[Symb](#symbolicmenu)** |
| [∂](commands/symbolic.md#derivative) | ∫ | ∑ | ∏ | *∆* | *Taylor* |

## BasesMenu

*3 pages · 45 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [NOr](commands/binary.md#nor) | [Implies](commands/binary.md#implies) | [Excludes](commands/binary.md#excludes) | [SL](commands/binary.md#shiftleft) | [SR](commands/binary.md#shiftright) | [ASR](commands/binary.md#arithmeticshiftright) |
| Bin | Oct | Dec | Hex | [WordSize](commands/settings.md#wordsize) | [NAnd](commands/binary.md#nand) |
| `#` | [And](commands/binary.md#and) | [Or](commands/binary.md#or) | [Xor](commands/binary.md#xor) | [Not](commands/binary.md#not) | [Base](commands/settings.md#base) |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| `#` | R→B | B→R | [Base](commands/settings.md#base) | [WordSize](commands/settings.md#wordsize) | SetBit |
| [RRB](commands/binary.md#rotaterightbyte) | [SLC](commands/binary.md#shiftleftcount) | [SRC](commands/binary.md#shiftrightcount) | [ASRC](commands/binary.md#arithmeticshiftrightcount) | [RLC](commands/binary.md#rotateleftcount) | [RRC](commands/binary.md#rotaterightcount) |
| [RL](commands/binary.md#rotateleft) | [RR](commands/binary.md#rotateright) | [SLB](commands/binary.md#shiftleftbyte) | [SRB](commands/binary.md#shiftrightbyte) | [ASRB](commands/binary.md#arithmeticshiftrightbyte) | [RLB](commands/binary.md#rotateleftbyte) |

**Page 3**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| 2-comp | [Modern](commands/settings.md#modernbasednumbers) | [TruthLogicForIntegers](commands/settings.md#truthlogicforintegers) | &nbsp; | &nbsp; | &nbsp; |
| ClrBit | FlipBit | FstSet | LstSet | CntBits | 1-comp |

## CalculationMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| *Limit* | *Serie* | *Taylor* | **[Symb](#symbolicmenu)** | &nbsp; | &nbsp; |
| [LName](commands/symbolic.md#listexpressionnames) | [XVars](commands/symbolic.md#expressionvariables) | *Deriv* | *DerivX* | *IBF* | *IntVX* |

## CircularMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| *sec* | *csc* | *cot* | *sec⁻¹* | *csc⁻¹* | *cot⁻¹* |
| [sin](commands/transcendentals.md#sin) | [cos](commands/transcendentals.md#cos) | [tan](commands/transcendentals.md#tan) | [sin⁻¹](commands/transcendentals.md#asin) | [cos⁻¹](commands/transcendentals.md#acos) | [tan⁻¹](commands/transcendentals.md#atan) |

## ClearThingsMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| [Stack](commands/stack.md#clearstack) | [Purge](commands/dirs.md#purge) | Stats | *Mem* | Error | [LCD](commands/graphics.md#clearlcd) |

## CompareMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [true](commands/comparisons.md#true) | [false](commands/comparisons.md#false) | **[Tests](#testsmenu)** | **[Loops](#loopsmenu)** | **[Prog](#programmenu)** | &nbsp; |
| [and](commands/binary.md#and) | [or](commands/binary.md#or) | [xor](commands/binary.md#xor) | [not](commands/binary.md#not) | [==](commands/comparisons.md#-2) | *Unimplemented* |
| [<](commands/comparisons.md#-3) | [=](commands/comparisons.md#) | [>](commands/comparisons.md#-5) | [≤](commands/comparisons.md#-4) | [≠](commands/comparisons.md#-1) | [≥](commands/comparisons.md#-6) |

## ComplexMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| 2+i3 | [ℝ∡ℝ→ℂ](commands/complex.md#realtopolar) | [ℂ→ℝ∡ℝ](commands/complex.md#polartoreal) | Auto ℂ | **[Angles](#anglesmenu)** | &nbsp; |
| [→Rect](commands/complex.md#torectangular) | [→Polar](commands/angles.md#polar) | [conj](commands/complex.md#conj) | [sign](commands/arithmetic.md#sign) | [\|z\|](commands/arithmetic.md#abs) | [arg](commands/complex.md#arg) |
| `ⅈ` | `∡` | [ℝ→ℂ](commands/complex.md#realtorectangular) | [ℂ→ℝ](commands/complex.md#rectangulartoreal) | [re](commands/complex.md#re) | [im](commands/complex.md#im) |

## DateMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| [→Time](commands/system.md#settime) | [→Date](commands/system.md#setdate) | [JDN](commands/system.md#juliandaynumber) | [JDN→](commands/system.md#datefromjuliandaynumber) | **[Time](#timemenu)** | **[Alarms](#alarmmenu)** |
| `_date` | `_d` | [Date](commands/system.md#date-1) | [Dt+Tm](commands/system.md#datetime) | ∆Date | [Date+](commands/system.md#date) |

## DebugMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Run](commands/debug.md#run) | [ErrDbg](commands/settings.md#debugonerror) | **[Prog](#programmenu)** | &nbsp; | &nbsp; | &nbsp; |
| [Halt](commands/debug.md#halt) | [Step↑](commands/debug.md#stepout) | DoErr | ErrMsg | ErrNum | ClrErr |
| [Debug](commands/debug.md#debug) | [Step](commands/docol2.md#step) | [Over](commands/stack.md#over) | [Steps](commands/debug.md#multiplesteps) | [Continue](commands/debug.md#continue) | [Kill](commands/debug.md#kill) |

## DifferentialSolverMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| [Eq](commands/symbolic.md#equation) | *Indep* | *Root* | **[SolverMenu](#solvermenu)** | &nbsp; | &nbsp; |

## DisplayModesMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [1 1/2](commands/settings.md#mixedfractions) | [3/2](commands/settings.md#improperfractions) | [1/3](commands/settings.md#bigfractions) | [¹/₃](commands/settings.md#smallfractions) | **[UI](#userinterfacemodesmenu)** | **[Math](#mathmodesmenu)** |
| [MantissaSpacing](commands/settings.md#mantissaspacing) | [FractionSpacing](commands/settings.md#fractionspacing) | [BasedSpacing](commands/settings.md#basedspacing) | [StandardExponent](commands/settings.md#standardexponent) | [MinimumSignificantDigits](commands/settings.md#minimumsignificantdigits) | **[Seps](#separatormodesmenu)** |
| Std | Fix | Sci | Eng | Sig | [Precision](commands/settings.md#precision) |

## EditMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| Stack | Hist↑ | Hist↓ | &nbsp; | &nbsp; | &nbsp; |
| Csr⇄Sel | \|← | →\| | Replace | [Copy](commands/dirs.md#copy) | Clear |
| Select | ←Word | Word→ | [Search](commands/ui.md#editorsearch) | [Cut](commands/ui.md#editorcut) | [Paste](commands/ui.md#editorpaste) |

## ExpLogIdentitiesMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| *ExpLn* | *Lin* | *LnCollect* | *SinCos* | *TExpand* | **[Symb](#symbolicmenu)** |

## FilesMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| *Seek* | *Dir* | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| *Save* | *Load* | *Open* | *Close* | *Read* | *Write* |
| [Libs](commands/library.md#libs) | [Attach](commands/library.md#attach) | [Detach](commands/library.md#detach) | [Voltage](commands/system.md#batteryvoltage) | [USB?](commands/system.md#usbpowered) | [Low?](commands/system.md#lowbattery) |

## FlagsMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| **[Prog](#programmenu)** | **[Loops](#loopsmenu)** | **[Modes](#modesmenu)** | &nbsp; | &nbsp; | &nbsp; |
| [F→Bin](commands/flags.md#flagstobinary) | [Bin→F](commands/flags.md#binarytoflags) | **[Tests](#testsmenu)** | [Flip](commands/flags.md#flipflag) | [Set?Set](commands/flags.md#testflagsetthenset) | [Clr?Set](commands/flags.md#testflagclearthenset) |
| [Set](commands/flags.md#setflag) | [Clear](commands/flags.md#clearflag) | [Set?](commands/flags.md#testflagset) | [Clear?](commands/flags.md#testflagclear) | [Set?Clr](commands/flags.md#testflagsetthenclear) | [Clr?Clr](commands/flags.md#testflagclearthenclear) |

## FractionsMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Frac→](commands/objects.md#explode) | [Cycle](commands/objects.md#cycle) | FractionIterations | FractionDigits | [1 1/2](commands/settings.md#mixedfractions) | [¹/₃](commands/settings.md#smallfractions) |
| [%Total](commands/arithmetic.md#percenttotal) | [%Chg](commands/arithmetic.md#percentchange) | DMS+ | DMS- | [→HMS](commands/system.md#hms) | [HMS→](commands/system.md#hms-1) |
| `/` | [%](commands/arithmetic.md#percent) | [→DMS](commands/system.md#dms) | [DMS→](commands/system.md#dms-1) | [→Num](commands/numerics.md#num) | [→Frac](commands/text.md#tofraction) |

## GraphicsMenu

*4 pages · 59 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Pix?](commands/graphics.md#pixtest) | [PixCol?](commands/graphics.md#pixcolor) | *Arc* | [ClLCD](commands/graphics.md#clearlcd) | [Disp](commands/graphics.md#drawtext) | [DispXY](commands/graphics.md#drawstyledtext) |
| [Gray](commands/graphics.md#gray) | Foregnd | Bckgnd | LnWidth | PixOn | PixOff |
| [Line](commands/graphics.md#drawline) | Rect | RndRect | Ellipse | Circle | [RGB](commands/graphics.md#rgb) |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [GOr](commands/graphics.md#gor) | [GXor](commands/graphics.md#gxor) | [GAnd](commands/graphics.md#gand) | [Extract](commands/composites.md#extract) | [Append](commands/graphics.md#graphicappend) | [(.)](commands/graphics.md#graphicparentheses) |
| [MaximumShowHeight](commands/settings.md#maximumshowheight) | [→LCD](commands/graphics.md#tolcd) | [LCD→](commands/graphics.md#fromlcd) | [Pict](commands/graphics.md#pict) | Clip | CurClip |
| [Input](commands/ui.md#input) | [Prompt](commands/ui.md#prompt) | Freeze | [Show](commands/graphics.md#show) | [→Grob](commands/graphics.md#togrob) | [MaximumShowWidth](commands/settings.md#maximumshowwidth) |

**Page 3**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [→HPGrob](commands/graphics.md#tohpgrob) | [→Bitmap](commands/graphics.md#tobitmap) | *Unimplemented* | [→Grob](commands/graphics.md#togrob) | [Blank](commands/graphics.md#blankgraphic) | [BlBitmap](commands/graphics.md#blankbitmap) |
| [∫](commands/graphics.md#graphicintegral) | [Subscript](commands/graphics.md#graphicsubscript) | [Exponent](commands/graphics.md#graphicexponent) | [LCD→](commands/graphics.md#fromlcd) | [→LCD](commands/graphics.md#tolcd) | [→Bitmap](commands/graphics.md#tobitmap) |
| [\|.\|](commands/graphics.md#graphicnorm) | [÷](commands/graphics.md#graphicratio) | [√.](commands/graphics.md#graphicroot) | [Stack](commands/graphics.md#graphicstack) | [Σ](commands/graphics.md#graphicsum) | [∏](commands/graphics.md#graphicproduct) |

**Page 4**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| [BlGrob](commands/graphics.md#blankgrob) | [BlankBitmap](commands/graphics.md#blankbitmap) | *Unimplemented* | [GraphicPicture](commands/graphics.md#pict) | **[Plot](#plotmenu)** | &nbsp; |

## HyperbolicMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| **[Powers](#powersmenu)** | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| [sinh](commands/transcendentals.md#sinh) | [cosh](commands/transcendentals.md#cosh) | [tanh](commands/transcendentals.md#tanh) | [sinh⁻¹](commands/transcendentals.md#asinh) | [cosh⁻¹](commands/transcendentals.md#acosh) | [tanh⁻¹](commands/transcendentals.md#atanh) |

## IntegrationMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| Σ | ∏ | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| [∂](commands/symbolic.md#derivative) | ∫ | [Num ∫](commands/numerics.md#integrate) | Symb ∫ | [Eq](commands/symbolic.md#equation) | *Indep* |

## IOMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| *Save* | [MinimumBatteryVoltage](commands/system.md#minimumbatteryvoltage) | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| *Save* | *Load* | *Print* | [Voltage](commands/system.md#batteryvoltage) | [USB?](commands/system.md#usbpowered) | [Low?](commands/system.md#lowbattery) |

## LinearSolverMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| [Eq](commands/symbolic.md#equation) | *Indep* | *Root* | **[SolverMenu](#solvermenu)** | &nbsp; | &nbsp; |

## ListMenu

*2 pages · 30 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Map](commands/lists.md#map) | [Reduce](commands/lists.md#reduce) | [Filter](commands/lists.md#filter) | [Get](commands/composites.md#get) | [Put](commands/composites.md#put) | [GetI](commands/composites.md#geti) |
| [RQSort](commands/lists.md#reversequicksort) | [∑List](commands/lists.md#listsum) | [∏List](commands/lists.md#listproduct) | [∆List](commands/lists.md#listdifferences) | [Sort](commands/lists.md#sort) | [RSort](commands/lists.md#reversesort) |
| [→List](commands/lists.md#list) | [List→](commands/lists.md#list-1) | [Size](commands/composites.md#size) | [Head](commands/composites.md#head) | [Tail](commands/composites.md#tail) | [QSort](commands/lists.md#quicksort) |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| [Extract](commands/composites.md#extract) | [Obj→](commands/objects.md#explode) | *Find* | **[Objects](#objectmenu)** | **[Matrix](#matrixmenu)** | **[Vector](#vectormenu)** |
| [PutI](commands/composites.md#puti) | [Reverse](commands/lists.md#reverselist) | DoList | DoSubs | NSub | EndSub |

## LoopsMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| **[Compare](#testsmenu)** | **[Prog](#programmenu)** | *Label* | *Goto* | *Gosub* | *Return* |
| [Start](commands/docol2.md#start) | StStep | [For](commands/docol2.md#for) | ForStep | [Until](commands/docol2.md#until) | [While](commands/docol2.md#while) |

## MainMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| ***[Lib](#library)*** | ***[Eqns](#equationsmenu)*** | ***[Const](#constantsmenu)*** | **[Time](#timemenu)** | **[I/O](#iomenu)** | ***[Chars](#charactersmenu)*** |
| [Cat](commands/menus.md#catalog) | **[Real](#realmenu)** | **[Matrix](#matrixmenu)** | **[Symb](#symbolicmenu)** | **[Stack](#stackmenu)** | **[UI](#userinterfacemodesmenu)** |
| Help | **[Math](#mathmenu)** | **[Prog](#programmenu)** | **[Plot](#plotmenu)** | **[Solve](#solvermenu)** | **[Modes](#modesmenu)** |

## MathMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| **[Signal](#signalprocessingmenu)** | **[Bases](#basesmenu)** | **[Angles](#anglesmenu)** | **[Poly](#polynomialsmenu)** | **[Symb](#symbolicmenu)** | **[Frac](#fractionsmenu)** |
| **[Hyper](#hyperbolicmenu)** | **[Proba](#probabilitiesmenu)** | **[Stats](#statisticsmenu)** | **[Solver](#solvermenu)** | ***[Const](#constantsmenu)*** | ***[Eqns](#equationsmenu)*** |
| **[Real](#realmenu)** | **[Cmplx](#complexmenu)** | **[Trig](#circularmenu)** | **[Powers](#powersmenu)** | **[Matrix](#matrixmenu)** | **[Ranges](#rangemenu)** |

## MathModesMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| Lazy | Lossy | [LinFitΣ](commands/settings.md#linearfitsums) | x·y | [Angles](commands/settings.md#setangleunits) | **[Disp](#displaymodesmenu)** |
| [MaxNumberBits](commands/settings.md#maxnumberbits) | [MaxRewrites](commands/settings.md#maxrewrites) | FractionIterations | FractionDigits | [1 1/2](commands/settings.md#mixedfractions) | [¹/₃](commands/settings.md#smallfractions) |
| [Sym](commands/symbolic.md#symbolicresults) | [Simpl](commands/symbolic.md#autosimplify) | 0^0=1 | HwFP | Auto ℂ | Princ |

## MatrixMenu

*2 pages · 35 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| *CondNum* | [Size](commands/composites.md#size) | **[Vector](#vectormenu)** | [Col+](commands/matrix.md#col) | [Col-](commands/matrix.md#col-) | →Col |
| [Norm](commands/arithmetic.md#abs) | [→Array](commands/matrix.md#array) | [Array→](commands/matrix.md#array-1) | [Random](commands/statistics.md#random) | [RowNrm](commands/matrix.md#rownorm) | [ColNrm](commands/matrix.md#columnnorm) |
| `[]` | [Idnty](commands/matrix.md#identitymatrix) | [Const](commands/matrix.md#constantarray) | Transp | TrConj | [Det](commands/matrix.md#determinant) |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Row+](commands/matrix.md#row) | [Row-](commands/matrix.md#row-) | →Row | Row→ | RowSwp | &nbsp; |
| *Cholesky* | *SVD* | *SVL* | *Diag→* | *→Diag* | *SpecRad* |
| Col→ | ColSwp | *LU* | *LQ* | *QR* | *Schur* |

## MemoryMenu

*3 pages · 37 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [PgAll](commands/dirs.md#purgeall) | [RunStats](commands/system.md#runtimestatistics) | [GCStats](commands/system.md#garbagecollectorstatistics) | [Store](commands/dirs.md#store) | Store+ | Store- |
| [Vars](commands/dirs.md#variables) | [Home](commands/dirs.md#homedirectory) | [Path](commands/dirs.md#directorypath) | [GC](commands/system.md#garbagecollect) | [Free](commands/system.md#freememory) | [TVars](commands/dirs.md#typedvariables) |
| [Store](commands/dirs.md#store) | [Recall](commands/dirs.md#recall) | [Purge](commands/dirs.md#purge) | [CrDir](commands/dirs.md#createdirectory) | [UpDir](commands/dirs.md#updirectory) | [Avail](commands/system.md#availablememory) |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [GCStats](commands/system.md#garbagecollectorstatistics) | [RunStats](commands/system.md#runtimestatistics) | [Avail](commands/system.md#availablememory) | System | [Bytes](commands/system.md#bytes) | GC Clr |
| Recall÷ | [▶](commands/dirs.md#copy) | Clone | [Incr](commands/dirs.md#increment) | [Decr](commands/dirs.md#decrement) | CurDir |
| Store× | Store÷ | [Recall](commands/dirs.md#recall) | Recall+ | Recall- | Recall× |

**Page 3**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| RT Clr | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |

## ModesMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Modes](commands/settings.md#modes) | [Reset](commands/settings.md#resetmodes) | [System](commands/system.md#systemsetup) | &nbsp; | &nbsp; | &nbsp; |
| Grad | **[Angles](#anglesmenu)** | Beep | Flash | **[Display](#displaymodesmenu)** | **[Seps](#separatormodesmenu)** |
| Deg | Rad | [n×π](commands/angles.md#piradians) | **[Math](#mathmodesmenu)** | **[User](#usermodemenu)** | **[UI](#userinterfacemodesmenu)** |

## MultiSolverMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| *Eqs* | *Indeps* | *MRoot* | **[SolverMenu](#solvermenu)** | &nbsp; | &nbsp; |

## NumbersMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| →Int | [IsPrime](commands/arithmetic.md#isprime) | [NextPr](commands/arithmetic.md#nextpr) | [PrevPr](commands/arithmetic.md#prevpr) | &nbsp; | &nbsp; |
| [→Num](commands/numerics.md#num) | [→Q](commands/numerics.md#q) | *→Qπ* | R#Seed | [RandomGeneratorBits](commands/settings.md#randomgeneratorbits) | [RandomGeneratorOrder](commands/settings.md#randomgeneratororder) |
| Σ | ∏ | QuoRem | [Factors](commands/arithmetic.md#factors) | Ran# | [Random](commands/statistics.md#random) |

## NumericalSolverMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| [Eq](commands/symbolic.md#equation) | *Indep* | *Root* | **[SolverMenu](#solvermenu)** | &nbsp; | &nbsp; |

## ObjectMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Eval](commands/base.md#evaluate) | [Run](commands/debug.md#run) | Clone | [DTag](commands/tags.md#deletetag) | [Tag→](commands/tags.md#tag-1) | **[Tools](#toolsmenu)** |
| [→List](commands/lists.md#list) | [→Text](commands/text.md#totext) | [→Tag](commands/tags.md#tag) | [→Graph](commands/graphics.md#togrob) | [→Prog](commands/objects.md#program) | [→Array](commands/matrix.md#array) |
| [Bytes](commands/system.md#bytes) | [Type](commands/system.md#type) | [TypeName](commands/system.md#typename) | [Obj→](commands/objects.md#explode) | [→Num](commands/numerics.md#num) | [→Frac](commands/text.md#tofraction) |

## PartsMenu

*2 pages · 30 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [SigDig](commands/arithmetic.md#sigdig) | [Get](commands/composites.md#get) | [GetIteration](commands/composites.md#geti) | [re](commands/complex.md#re) | [im](commands/complex.md#im) | [arg](commands/complex.md#arg) |
| [Mant](commands/arithmetic.md#mant) | [Xpon](commands/arithmetic.md#xpon) | [Ceil](commands/arithmetic.md#ceil) | [Floor](commands/arithmetic.md#floor) | →Int | [→Q](commands/numerics.md#q) |
| [abs](commands/arithmetic.md#abs) | [sign](commands/arithmetic.md#sign) | [IntegerPart](commands/arithmetic.md#intpart) | [FractionalPart](commands/arithmetic.md#fracpart) | Round | Trunc |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| [Tag→](commands/tags.md#tag-1) | [StdRnd](commands/constants.md#standardround) | [RelRnd](commands/constants.md#relativeround) | [→StdUnc](commands/constants.md#us) | [→RelRnd](commands/constants.md#ur) | [PrcRnd](commands/constants.md#precisionround) |
| [Size](commands/composites.md#size) | [Obj→](commands/objects.md#explode) | CstName | CstValue | StdUnc | RelUnc |

## PlotMenu

*2 pages · 20 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [StatsPlotBins](commands/settings.md#statsplotbins) | [XYPlotBins](commands/settings.md#xyplotbins) | PlotRefreshRate | [Bar](commands/plot.md#barplot) | [Histogrm](commands/plot.md#histogramplot) | [Clear](commands/graphics.md#clearlcd) |
| Backgnd | LineWdth | [Xrange](commands/plot.md#xrange) | [Yrange](commands/plot.md#yrange) | Lines | Axes |
| [Function](commands/plot.md#functionplot) | [Polar](commands/angles.md#polar) | [Param](commands/plot.md#parametricplot) | [Scatter](commands/plot.md#scatterplot) | [Truth](commands/plot.md#truthplot) | Foregnd |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| Freeze | DrAxes | &nbsp; | &nbsp; | &nbsp; | &nbsp; |

## PolynomialsMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| *Solve* | *TVMRoot* | [XRoot](commands/arithmetic.md#xroot) | *Zeros* | *FCoef* | &nbsp; |
| *FRoots* | *MRoot* | *MSolvr* | *PCoef* | *PRoot* | [Root](commands/solvers.md#root) |
| `Ⓟ''` | →Poly | Poly→ | [Obj→](commands/objects.md#explode) | Display | QuoRem |

## PolynomialSolverMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| [Eq](commands/symbolic.md#equation) | *Indep* | *Root* | **[SolverMenu](#solvermenu)** | &nbsp; | &nbsp; |

## PowersMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [pow](commands/arithmetic.md#pow) | [xroot](commands/arithmetic.md#xroot) | *FstSet* | *LstSet* | *popcnt* | **[Hyper](#hyperbolicmenu)** |
| [exp2](commands/transcendentals.md#exp2) | log2 | [expm1](commands/transcendentals.md#expm1) | [ln1p](commands/transcendentals.md#ln1p) | x³ | [∛](commands/transcendentals.md#cbrt) |
| [exp](commands/transcendentals.md#exp) | [ln](commands/transcendentals.md#ln) | [exp10](commands/transcendentals.md#exp10) | [log10](commands/transcendentals.md#log10) | [x²](commands/arithmetic.md#sq) | [√](commands/transcendentals.md#sqrt) |

## PrintingMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| *Print* | *Screen* | *Disk* | *IR* | &nbsp; | &nbsp; |

## ProbabilitiesMenu

*2 pages · 22 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| *Chi²* | *F-Distr* | *FFT* | *Normal⁻¹* | *Studnt⁻¹* | *Chi²⁻¹* |
| ln(Γ) | erf | erfc | RSeed | *Normal* | *Student* |
| Comb | Perm | x! | Ran# | [Random](commands/statistics.md#random) | Γ |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| *F-Dist⁻¹* | *FFT⁻¹* | [RandomGeneratorBits](commands/settings.md#randomgeneratorbits) | [RandomGeneratorOrder](commands/settings.md#randomgeneratororder) | &nbsp; | &nbsp; |

## ProgramMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| **[Stack](#stackmenu)** | **[Debug](#debugmenu)** | **[Objects](#objectmenu)** | **[List](#listmenu)** | **[Flag](#flagsmenu)** | [Version](commands/system.md#version) |
| **[Mem](#memorymenu)** | **[Test](#testsmenu)** | **[Cmp](#comparemenu)** | **[Loop](#loopsmenu)** | **[Base](#basesmenu)** | [Eval](commands/base.md#evaluate) |
| `«»` | `{}` | `[]` | `→  «»` | `→  ''` | [Run](commands/debug.md#run) |

## RangeMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Size](commands/composites.md#size) | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| [→Range](commands/ranges.md#range) | [→∆Range](commands/ranges.md#range-1) | [→%Range](commands/ranges.md#range-2) | [→σRange](commands/ranges.md#range-3) | [∪](commands/ranges.md#rangeunion) | [∩](commands/ranges.md#rangeintersect) |
| `…` | `±` | `±%` | `±σ` | Range→ | `ρ` |

## RealMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| **[Trig](#circularmenu)** | **[Hyper](#hyperbolicmenu)** | **[Powers](#powersmenu)** | **[Prob](#probabilitiesmenu)** | **[Angles](#anglesmenu)** | **[Parts](#partsmenu)** |
| [Ceil](commands/arithmetic.md#ceil) | [Floor](commands/arithmetic.md#floor) | rem | [%](commands/arithmetic.md#percent) | [%Chg](commands/arithmetic.md#percentchange) | [%Total](commands/arithmetic.md#percenttotal) |
| [Min](commands/arithmetic.md#min) | [Max](commands/arithmetic.md#max) | [mod](commands/arithmetic.md#mod) | [abs](commands/arithmetic.md#abs) | [→Num](commands/numerics.md#num) | [→Frac](commands/text.md#tofraction) |

## SeparatorModesMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [1.2x10³²](commands/settings.md#fancyexponent) | [1.2E32](commands/settings.md#classicexponent) | [1.0→1.](commands/settings.md#trailingdecimal) | [1→1.0](commands/settings.md#showasdecimal) | Fixed0 | &nbsp; |
| #1 000 | #1.000 | #1'000 | #1_000 | **[Disp](#displaymodesmenu)** | **[Modes](#modesmenu)** |
| [1 000](commands/settings.md#numberspaces) | [1.000,](commands/settings.md#numberdotorcomma) | [1'000](commands/settings.md#numberticks) | [1_000](commands/settings.md#numberunderscore) | [2.3](commands/settings.md#decimaldot) | [2,3](commands/settings.md#decimalcomma) |

## SignalProcessingMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| *FFT* | *InvFFT* | &nbsp; | &nbsp; | &nbsp; | &nbsp; |

## SolverMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| **[Multi](#multisolvermenu)** | **[Finance](#financesolvermenu)** | **[Plot](#plotmenu)** | ***[Eqns](#equationsmenu)*** | [SolverImprecision](commands/settings.md#solverimprecision) | SolverIterations |
| [▶Eq](commands/symbolic.md#steq) | [Stoⓧ](commands/symbolic.md#storealgebravariable) | **[Symb](#symbolicsolvermenu)** | **[Diff](#differentialsolvermenu)** | **[Poly](#polynomialsolvermenu)** | **[Linear](#linearsolvermenu)** |
| [Eq▶](commands/symbolic.md#rceq) | [ⓧ](commands/symbolic.md#algebravariable) | [Root](commands/solvers.md#root) | [EvalEq](commands/symbolic.md#evaleq) | [NxtEq](commands/symbolic.md#nexteq) | **[Solve](#solvingmenu)** |

## StackMenu

*2 pages · 22 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Depth](commands/stack.md#depth) | [Nip](commands/stack.md#nip) | [Pick3](commands/stack.md#pick3) | [Swap](commands/stack.md#swap) | [LastArg](commands/stack.md#lastarguments) | [LastX](commands/stack.md#lastx) |
| [Drop2](commands/stack.md#drop2) | [Rot↓](commands/stack.md#unrot) | [Roll↓](commands/stack.md#rolld) | [Pick](commands/stack.md#pick) | [DupN](commands/stack.md#duplicaten) | [DropN](commands/stack.md#dropn) |
| [Dup](commands/stack.md#duplicate) | [Drop](commands/stack.md#drop) | [Rot↑](commands/stack.md#rot) | [Roll↑](commands/stack.md#roll) | [Over](commands/stack.md#over) | [Dup2](commands/stack.md#duplicate2) |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| [Undo](commands/stack.md#undo) | [ClrStk](commands/stack.md#clearstack) | [NDupN](commands/stack.md#ndupn) | [DupDup](commands/stack.md#duplicatetwice) | &nbsp; | &nbsp; |

## StatisticsMenu

*2 pages · 35 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| ClrΣ | **[Proba](#probabilitiesmenu)** | **[Plot](#plotmenu)** | [LR](commands/statistics.md#linearregression) | ΣLine | PredX |
| [YCol](commands/statistics.md#dependentcolumn) | [MinΣ](commands/statistics.md#min) | [MaxΣ](commands/statistics.md#max) | [ΣSize](commands/statistics.md#size) | StoΣ | RclΣ |
| [Σ+](commands/statistics.md#) | [Σ-](commands/statistics.md#-) | Total | [Mean](commands/statistics.md#average) | [StdDev](commands/statistics.md#standarddeviation) | [XCol](commands/statistics.md#independentcolumn) |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Median](commands/statistics.md#median) | [Bins](commands/statistics.md#frequencybins) | [PopVar](commands/statistics.md#populationvariance) | [PopSDev](commands/statistics.md#populationstandarddeviation) | [PCovar](commands/statistics.md#populationcovariance) | &nbsp; |
| [PwrFit](commands/statistics.md#powerfit) | [ΣX](commands/statistics.md#x) | [ΣY](commands/statistics.md#y) | [ΣXY](commands/statistics.md#xy) | [ΣX²](commands/statistics.md#x-1) | [ΣY²](commands/statistics.md#y-1) |
| PredY | [Corr](commands/statistics.md#correlation) | [BestFit](commands/statistics.md#bestfit) | [LinFit](commands/statistics.md#linearfit) | [ExpFit](commands/statistics.md#exponentialfit) | [LogFit](commands/statistics.md#logarithmicfit) |

## SymbolicMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| **[Integ](#integrationmenu)** | **[DSolve](#differentialsolvermenu)** | [Simplify](commands/symbolic.md#autosimplify) | [KeepAll](commands/symbolic.md#noautosimplify) | &nbsp; | &nbsp; |
| **[Arith](#arithmeticmenu)** | **[Calc](#calculationmenu)** | **[Trig](#trigidentitiesmenu)** | **[Exp/Ln](#explogidentitiesmenu)** | **[Poly](#polynomialsmenu)** | **[Graph](#plotmenu)** |
| Collect | Expand | Simplify | →Poly | [→Prog](commands/objects.md#program) | **[Algbra](#algebramenu)** |

## SymbolicSolverMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| [Eq](commands/symbolic.md#equation) | *Indep* | *Root* | [Isolate](commands/symbolic.md#isolate) | **[SolverMenu](#solvermenu)** | &nbsp; |

## TestsMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Case](commands/docol2.md#case) | Then | When | **[Compare](#comparemenu)** | **[Loops](#loopsmenu)** | **[Prog](#programmenu)** |
| IfThen | IfElse | [IfErr](commands/docol2.md#iferr) | IfErrElse | [IFT](commands/stack.md#ift) | [IFTE](commands/stack.md#ifte) |
| [<](commands/comparisons.md#-3) | [=](commands/comparisons.md#) | [>](commands/comparisons.md#-5) | [≤](commands/comparisons.md#-4) | [≠](commands/comparisons.md#-1) | [≥](commands/comparisons.md#-6) |

## TextMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [T→Real](commands/text.md#compiletoreal) | [T→Expr](commands/text.md#compiletoexpression) | [T→Int](commands/text.md#compiletointeger) | [T→Pos](commands/text.md#compiletopositive) | &nbsp; | &nbsp; |
| [T→Code](commands/text.md#textcode) | [Code→T](commands/text.md#codetext) | [Extract](commands/composites.md#extract) | [T→Obj](commands/text.md#compiletoobject) | [T→Alg](commands/text.md#compiletoalgebraic) | [T→Num](commands/text.md#compiletonumber) |
| [→Text](commands/text.md#totext) | [Text→](commands/text.md#compile) | [Length](commands/composites.md#size) | [Append](commands/arithmetic.md#add) | [Repeat](commands/docol2.md#repeat) | [C→Code](commands/text.md#charcode) |

## TimeMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [→Time](commands/system.md#settime) | [→Date](commands/system.md#setdate) | *ClkAdj* | **[Dates](#datemenu)** | **[Alarms](#alarmmenu)** | &nbsp; |
| Chrono | [Ticks](commands/system.md#ticks) | [Dt+Tm](commands/system.md#datetime) | [T→Str](commands/text.md#totext) | [Wait](commands/ui.md#wait) | [TEval](commands/system.md#teval) |
| _hms | [Time](commands/system.md#time) | [→HMS](commands/system.md#hms) | [HMS→](commands/system.md#hms-1) | [HMS+](commands/system.md#hms-2) | [HMS-](commands/system.md#hms-) |

## TrigIdentitiesMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| **[Symb](#symbolicmenu)** | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| *HalfTan* | *Tan→SinCos* | *Tan→SinCos²* | *TExpand* | *TLin* | [TrigSin](commands/symbolic.md#trigsin) |

## UnitsConversionsMenu

*2 pages · 36 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| p (-12) | T (+12) | P (+15) | f (-15) | d (-1) | da (+1) |
| c (-2) | k (+3) | M (+6) | G (+9) | µ (-6) | n (-9) |
| [Convert](commands/units.md#convert) | [Base](commands/settings.md#base) | [Value](commands/units.md#unitvalue) | [Factor](commands/units.md#factorunit) | [→Unit](commands/units.md#unit) | m (-3) |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| Ei | Zi | Yi | Ri | Qi | [SIPfx](commands/units.md#unitssiprefixcycle) |
| Y (+24) | Ki | Mi | Gi | Ti | [Pi](commands/constants.md#pi) |
| h (+2) | E (+18) | y (-24) | z (-21) | a (-18) | Z (+21) |

## UserInterfaceModesMenu

*3 pages · 47 items total*

**Page 1**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Flat](commands/settings.md#flatmenus) | Round | **[Hide](#hideemptymenu)** | [cmd](commands/settings.md#lowercase) | [CMD](commands/settings.md#uppercase) | [Cmd](commands/settings.md#capitalized) |
| StackFont | EditorFont | MultilineEditorFont | [CursorBlinkRate](commands/settings.md#cursorblinkrate) | [3-lines](commands/settings.md#threerowsmenus) | [1-line](commands/settings.md#singlerowmenus) |
| [GrRes](commands/settings.md#graphicresultdisplay) | [GrStk](commands/settings.md#graphicstackdisplay) | Beep | Flash | **[User](#usermodemenu)** | ResultFont |

**Page 2**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| [Units](commands/settings.md#showbuiltinunits) | Const | Eqns | [Libs](commands/library.md#libs) | Chars | [ResultGraphingTimeLimit](commands/settings.md#resultgraphingtimelimit) |
| ErrorBeepFrequency | Fixed0 | VProg | BusyIndicatorRefresh | **[ExitMenu](#exitclearsmenu)** | [ListEval](commands/settings.md#listasprogram) |
| [Command](commands/settings.md#longform) | ErrorBeepDuration | [EditorWrapColumn](commands/settings.md#editorwrapcolumn) | [TabWidth](commands/settings.md#tabwidth) | [MaximumShowWidth](commands/settings.md#maximumshowwidth) | [MaximumShowHeight](commands/settings.md#maximumshowheight) |

**Page 3**

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| PlotRefreshRate | AllVars | [SIPrefixCycle](commands/units.md#unitssiprefixcycle) | [Header](commands/graphics.md#header) | [CustomHeaderRefresh](commands/settings.md#customheaderrefresh) | &nbsp; |
| [StackGraphingTimeLimit](commands/settings.md#stackgraphingtimelimit) | [GraphingTimeLimit](commands/settings.md#graphingtimelimit) | [ShowTimeLimit](commands/settings.md#showtimelimit) | [MinimumBatteryVoltage](commands/system.md#minimumbatteryvoltage) | [TextRenderingSizeLimit](commands/settings.md#textrenderingsizelimit) | [GraphRenderingSizeLimit](commands/settings.md#graphrenderingsizelimit) |

## UserModeMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| [DelKeys](commands/user-mode.md#deletekeys) | [KeyMap](commands/user-mode.md#keymap) | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| [Toggle](commands/user-mode.md#toggleusermode) | [User](commands/user-mode.md#usermode) | [Lock](commands/user-mode.md#usermodelock) | [RclKeys](commands/user-mode.md#recallkeys) | [StoKeys](commands/user-mode.md#storekeys) | [Assign](commands/user-mode.md#assignkey) |

## VectorMenu

| F1 | F2 | F3 | F4 | F5 | F6 |
|:--:|:--:|:--:|:--:|:--:|:--:|
| &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; | &nbsp; |
| [→Vec2](commands/complex.md#to2dvector) | [→Vec3](commands/complex.md#to3dvector) | [Vec→](commands/complex.md#fromvector) | [→Cylind](commands/complex.md#tocylindrical) | **[Complex](#complexmenu)** | **[Matrix](#matrixmenu)** |
| [Norm](commands/arithmetic.md#abs) | [Dot](commands/matrix.md#dot) | [Cross](commands/matrix.md#cross) | [→Rect](commands/complex.md#torectangular) | [→Polar](commands/angles.md#polar) | [→Spher](commands/complex.md#tospherical) |

