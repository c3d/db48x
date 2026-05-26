# Implementation status

This section documents the implementation status for all HP50 RPL commands as
listed in the HP50G Advanced Reference Manual. This is a strict superset of the
HP48 implementation.

* [Implemented](#implemented-commands)
* [Not implemented](#unimplemented-commands)
* [Unapplicable commands](#unapplicable-commands)


## Implemented commands

The following is an extensive list of implemented commands, some with multiple
spellings.

<!--- DMNONE --->
* `!` (`fact`, `factorial`, `x!`)
* `%` (`Percent`)
* `%Ch` (`%Change`, `PercentChange`)
* `%T` (`%Total`, `PercentTotal`)
* `*` (`multiply`, `·`, `×`)
* `+` (`add`)
* `-` (`subtract`)
* `/` (`divide`, `÷`)
* `<` (`TestLT`)
* `=` (`TestEQ`)
* `==` (`TestSame`)
* `>` (`TestGT`)
* `^` (`pow`, `Power`, `↑`)
* `abs` (`Modulus`)
* `acos` (`arccos`, `cos⁻¹`)
* `acosh` (`cosh⁻¹`)
* `acot` (`arccot`, `cot⁻¹`)
* `acsc` (`arccsc`, `csc⁻¹`)
* `AlarmMenu`
* `AlgebraMenu`
* `AllEquationVariables`
* `AllSolutions`
* `alog` (`exp10`)
* `AlphaForeground`
* `Amort` (`TVMAmort`, `TVMAmortize`)
* `AmortTable` (`AmortizationSchedule`, `TVMAmortizationTable`, `TVMAmortTable`)
* `And`
* `AngleMode`
* `AnglesMenu`
* `Apply`
* `ApplyInverseUnit`
* `ApplyUnit`
* `arg`
* `ArithmeticMenu`
* `Arry→` (`Array→`, `FromArray`)
* `asec` (`arcsec`, `sec⁻¹`)
* `asin` (`arcsin`, `sin⁻¹`)
* `asinh` (`sinh⁻¹`)
* `Asn` (`AssignKey`)
* `ASR` (`ArithmeticShiftRight`)
* `ASRB` (`ArithmeticShiftRightByte`)
* `ASRC` (`ArithmeticShiftRightCount`)
* `atan` (`arctan`, `tan⁻¹`)
* `atanh` (`tanh⁻¹`)
* `Attach`
* `AutomaticTemporariesCleanup`
* `AutoScaleStack`
* `AutoSimplify`
* `Avg` (`Average`, `Mean`, `ΣMean`)
* `Background`
* `Bar` (`BarPlot`)
* `Base`
* `BasedDotOrComma`
* `BasedSeparatorCommand`
* `BasedSpaces`
* `BasedSpacing`
* `BasedTicks`
* `BasedUnderscore`
* `BasesMenu`
* `BatteryBackground`
* `BatteryForeground`
* `BatteryLevelForeground`
* `BatteryRefresh`
* `BatteryVoltage`
* `Beep`
* `BeepOff`
* `BeepOn`
* `BestFit`
* `BigFractions`
* `Bin` (`Binary`)
* `Bins` (`FrequencyBins`)
* `BitwiseLogicForIntegers`
* `Blank` (`BlankGraphic`)
* `BlankBitmap`
* `BlankGrob`
* `BusyIndicatorRefresh`
* `Bytes`
* `B→R` (`BinaryToReal`)
* `CalculationMenu`
* `Capitalized` (`CapitalizedCommands`)
* `CapitalizedNames` (`CapitalizedSymbols`)
* `CarefulEvaluation`
* `Catalog`
* `ceil`
* `Centr` (`Center`, `PlotCenter`)
* `CF` (`ClearFlag`)
* `CharactersMenu`
* `ChargingForeground`
* `Chr` (`Code→Char`, `Code→Text`, `UnicodeToText`)
* `ChronoTime`
* `Circle` (`DrawCircle`)
* `CircularMenu`
* `ClassicExponent`
* `Clear` (`ClearStack`)
* `ClearBit`
* `ClearThingsMenu`
* `Clip`
* `ClLCD` (`ClearLCD`)
* `Clone` (`NewOb`, `NewObj`, `NewObject`)
* `ClΣ` (`ClearData`, `ClearΣ`)
* `CNrm` (`ColumnNorm`)
* `Col+` (`AddColumn`, `Column+`)
* `Col-` (`Column-`, `DeleteColumn`)
* `Collect`
* `Color` (`ColorPattern`)
* `ColΣ` (`DataColumns`, `ΣColumns`)
* `Col→` (`ColumnsToMatrix`, `Columns→`)
* `comb` (`Combinations`)
* `CommandBackground`
* `CommandDisplayMode`
* `CommandForeground`
* `CompareMenu`
* `CompatibleBasedNumbers`
* `CompatibleGROBs`
* `CompatibleTypes`
* `ComplexIAfterImaginary`
* `ComplexIBeforeImaginary`
* `ComplexMenu`
* `ComplexResults`
* `Con` (`ConstantArray`)
* `conj`
* `ConLib` (`ConstantsLibrary`, `ConstantsMenu`)
* `Const` (`LibraryConstant`)
* `ConstantRange`
* `Constants`
* `cont` (`Continue`)
* `Convert`
* `ConvertToUnit`
* `ConvertToUnitPrefix`
* `Corr` (`Correlation`)
* `cos`
* `cosh`
* `cot`
* `CountBits`
* `Cov` (`Covariance`)
* `CrDir` (`CreateDirectory`, `MkDir`)
* `csc`
* `Cst` (`CustomMenu`)
* `CstRU` (`RelativeUncertainty`)
* `CstSU` (`StandardUncertainty`)
* `CSwp` (`ColumnSwap`)
* `CurrentClip`
* `CurrentDirectory`
* `CurrentEquationVariables`
* `CurrentFitSums`
* `CursorAlphaBackground`
* `CursorAlphaBorder`
* `CursorAlphaForeground`
* `CursorBackground`
* `CursorBlinkRate`
* `CursorBorder`
* `CursorFont`
* `CursorForeground`
* `CursorSelBackground`
* `CurveFilling`
* `CustomHeaderRefresh`
* `Cycle`
* `C→R` (`ComplexToReal`, `RectangularToReal`, `ℂ→ℝ`)
* `Date`
* `Date+` (`DateAdd`)
* `Date-` (`DateSub`, `DaysBetweenDates`, `DDays`)
* `DateDash`
* `DateDot`
* `DateForeground`
* `DatesMenu`
* `DateSeparatorCommand`
* `DateSlash`
* `DateSpace`
* `DateTime`
* `DayBeforeMonth`
* `dbug` (`Debug`)
* `DebugMenu`
* `DebugOnError`
* `Dec` (`Decimal`)
* `DecimalComma`
* `DecimalDot`
* `Decr` (`Decrement`)
* `Deg` (`Degrees`)
* `DelKeys` (`DeleteKeys`)
* `Depth`
* `det` (`Determinant`)
* `Detach`
* `DetailedTypes`
* `DFC` (`ContinuedFraction`)
* `DFC2F` (`ContinuedFractionToFraction`)
* `DifferentialSolverMenu`
* `Disp` (`DrawText`)
* `DisplayDigits`
* `DisplayMode`
* `DisplayModesMenu`
* `DispXY` (`DrawStyledText`)
* `DistinguishSymbolCase`
* `Div2` (`IDiv2`, `QuoRem`, `QuotientRemainder`)
* `DMCPDisplayRefresh`
* `DMCPLowBattery`
* `DMS+` (`DMSAdd`)
* `DMS-` (`DMSSub`)
* `DMS→` (`FromDMS`)
* `doerr` (`DoError`)
* `DoList`
* `DoSubs`
* `dot` (`⋅`)
* `Draw` (`DrawPlot`)
* `DrawPlotAxes`
* `Drax` (`DrawAxes`)
* `Drop`
* `Drop2`
* `DropN`
* `DTag` (`DeleteTag`)
* `Dup` (`Duplicate`)
* `Dup2` (`Duplicate2`)
* `DupDup` (`DuplicateTwice`)
* `DupN` (`DuplicateN`)
* `D→R` (`DegreesToRadians`, `Degrees→Radians`)
* `Edit`
* `EditMenu`
* `EditorBackground`
* `EditorBegin`
* `EditorClear`
* `EditorCopy`
* `EditorCut`
* `EditorEnd`
* `EditorFlip`
* `EditorFont`
* `EditorForeground`
* `EditorHistory`
* `EditorHistoryBack`
* `EditorLineForeground`
* `EditorPaste`
* `EditorReplace`
* `EditorSearch`
* `EditorSelect`
* `EditorWordLeft`
* `EditorWordRight`
* `EditorWrapColumn`
* `Ellipse` (`DrawEllipse`)
* `EndSub`
* `Eng` (`EngineeringDisplay`)
* `Eq` (`Equation`)
* `EqnLib` (`EquationsLibrary`, `EquationsMenu`)
* `equiv`
* `erf`
* `erfc`
* `err0` (`ClearError`)
* `errm` (`ErrorMessage`)
* `errn` (`ErrorNumber`)
* `ErrorBackground`
* `ErrorBeepDuration`
* `ErrorBeepFrequency`
* `ErrorBorder`
* `ErrorForeground`
* `Eval` (`Evaluate`)
* `EvalEq` (`EvaluateEquation`)
* `Excludes`
* `ExitClearsMenu`
* `ExitKeepsMenu`
* `exp`
* `exp2` (`alog2`)
* `Expan` (`Expand`)
* `ExpFit` (`ExponentialFit`)
* `ExplicitWildcards`
* `ExpLogIdentitiesMenu`
* `ExpLogMenu` (`PowersMenu`)
* `expm` (`expm1`)
* `Factors`
* `False`
* `FancyExponent`
* `FC?` (`TestFlagClear`)
* `FC?C` (`TestFlagClearThenClear`)
* `FC?S` (`TestFlagClearThenSet`)
* `FF` (`FlipFlag`)
* `FilesMenu`
* `Filter`
* `FinalAlgebraResults`
* `FinanceRounding`
* `FirstBitSet`
* `Fix` (`FixedDisplay`)
* `FixedWidthDigits`
* `FlagsMenu`
* `FlatMenus`
* `FlipBit`
* `floor`
* `FoldConstants`
* `FolderCornerForeground`
* `Foreground`
* `FourDigitYear`
* `FP` (`FracPart`, `FractionalPart`)
* `FractionsMenu`
* `FractionSpacing`
* `FreeMemory`
* `Freeze`
* `FS?` (`TestFlagSet`)
* `FS?C` (`TestFlagSetThenClear`)
* `FS?S` (`TestFlagSetThenSet`)
* `Function` (`FunctionPlot`)
* `GAnd` (`GraphicAnd`)
* `GC` (`GarbageCollect`)
* `GCD` (`GreatestCommonDenominator`)
* `GCIconForeground`
* `GCStats` (`GarbageCollectorStatistics`)
* `GCStatsClearAfterRead`
* `GCStatsKeepAfterRead`
* `GCTemporariesCleanup`
* `Get`
* `GetI` (`GetIteration`)
* `GOr` (`GraphicOr`)
* `Grad` (`Grads`)
* `GraphicAppend`
* `GraphicExponent`
* `GraphicIntegral`
* `GraphicNorm`
* `GraphicParentheses`
* `GraphicProduct`
* `GraphicRatio`
* `GraphicResultDisplay`
* `GraphicRoot`
* `GraphicsMenu`
* `GraphicStackDisplay`
* `GraphicSubscript`
* `GraphicSum`
* `GraphingTimeLimit`
* `GraphRenderingSizeLimit`
* `Gray` (`GrayPattern`, `Grey`, `GreyPattern`)
* `GrobAdd` (`GraphicStack`)
* `GXor` (`GraphicXor`)
* `HalfBatteryForeground`
* `Halt`
* `HaltedIconForeground`
* `Hdr` (`Header`)
* `Head`
* `HeaderBackground`
* `HeaderFont`
* `HeaderForeground`
* `Help`
* `Hex` (`Hexadecimal`)
* `HFP` (`HardFP`, `HardwareFloatingPoint`)
* `HideBuiltinCharacters`
* `HideBuiltinConstants`
* `HideBuiltinEquations`
* `HideBuiltinLibrary`
* `HideBuiltinUnits`
* `HideDate`
* `HideDayOfWeek`
* `HideEmptyMenu`
* `HideEquationBody`
* `HideMonthName`
* `HideSeconds`
* `HideTime`
* `HideVoltage`
* `Histogram` (`HistogramPlot`)
* `HMS+` (`HMSAdd`)
* `HMS-` (`HMSSub`)
* `HMS→` (`FromHMS`)
* `Home` (`HomeDirectory`)
* `HorizontalLists`
* `HorizontalProgramRendering`
* `HorizontalVectors`
* `HSV` (`HSVPattern`)
* `HyperbolicMenu`
* `Idn` (`IdentityMatrix`)
* `IFT` (`IfThen`)
* `IFTE` (`IfThenElse`)
* `IgnorePrecisionLoss`
* `IgnoreSymbolCase`
* `im`
* `ImplicitWildcards`
* `Implies`
* `ImproperFractions`
* `Incr` (`Increment`)
* `IndexDidNotWrap`
* `IndexWrapped`
* `InfiniteResultIndicator`
* `InfinityError`
* `InfinityValue`
* `Input`
* `IntegrationImprecision`
* `IntegrationIterations`
* `IntegrationMenu` (`DifferentiationMenu`)
* `Intercept`
* `inv` (`Invert`, `x⁻¹`)
* `IOMenu`
* `IP` (`IntegerPart`, `IntPart`)
* `Isol` (`Isolate`)
* `IsPrime`
* `JDN` (`JulianDayNumber`)
* `JDN→` (`DateFromJulianDayNumber`)
* `Keys` (`KeyMap`)
* `Kill`
* `KillOnError`
* `LastArg` (`LastArguments`)
* `LastBitSet`
* `LastMenu`
* `LastX`
* `LaxArrayResizing`
* `LazyEvaluation`
* `LCD→` (`FromLCD`, `Screen→`)
* `LCM` (`LeastCommonMultiple`)
* `LeadingZero`
* `LeftShiftBackground`
* `LeftShiftForeground`
* `LibEq` (`LibraryEquation`)
* `Library`
* `Libs` (`Libraries`)
* `Line` (`DrawLine`)
* `LinearFitSums`
* `LinearSolverMenu`
* `LineWidth`
* `LinFit` (`LinearFit`)
* `ListAsData`
* `ListAsProgram`
* `ListMenu`
* `ListRecursionDepth`
* `List→` (`FromList`)
* `ln`
* `ln1p` (`lnp1`)
* `LName` (`ListExpressionNames`)
* `lnΓ` (`lgamma`, `LnGamma`)
* `log` (`log10`)
* `log2`
* `LogFit` (`LogarithmicFit`)
* `LongForm` (`LongFormCommands`)
* `LongFormNames` (`LongFormSymbols`)
* `LoopsMenu`
* `LowBattery`
* `LowBatteryForeground`
* `LowerAlphaForeground`
* `LowerCase` (`LowerCaseCommands`)
* `LowerCaseNames` (`LowerCaseSymbols`)
* `LR` (`LinearRegression`)
* `MainMenu`
* `mant` (`Mantissa`)
* `MantissaSpacing`
* `Map`
* `MatchUp` (`↑Match`)
* `MathMenu`
* `MathModesMenu`
* `MatrixMenu`
* `Max`
* `MaxFactorIterations`
* `MaxFactorsBits`
* `MaxFlags`
* `MaxH` (`MaximumShowHeight`)
* `MaximumDecimalExponent`
* `MaxNumberBits`
* `MaxRewrites`
* `MaxW` (`MaximumShowWidth`)
* `MaxΣ` (`MaxData`, `ΣMax`)
* `Median`
* `Mem` (`AvailableMemory`)
* `MemoryMenu`
* `Menu`
* `MenuAppearance`
* `MenuBackground`
* `Min`
* `MinimumBatteryVoltage`
* `MinimumSignificantDigits`
* `MinΣ` (`MinData`, `ΣMin`)
* `MixedFractions`
* `mod`
* `ModernBasedNumbers`
* `Modes`
* `ModesMenu`
* `MonthBeforeDay`
* `MRoot` (`MultipleEquationsSolver`)
* `MSlv` (`MultipleVariablesSolver`)
* `MSolvr` (`SolverMenu`)
* `MultilineCursorFont`
* `MultilineEditorFont`
* `MultiLineResult`
* `MultiLineStack`
* `MultipleSteps`
* `MultiSolverMenu`
* `NameDisplayMode`
* `nand`
* `NDupN` (`NDuplicateN`)
* `NeedToClearErrors`
* `neg` (`Negate`)
* `NegativeUnderflowIndicator`
* `NextEq` (`NextEquation`)
* `NextPr` (`NextPrime`)
* `Nip`
* `NoAngleUnits`
* `NoAutoSimplify`
* `NoCurveFilling`
* `NoErrorForTooManyDigits`
* `NoInfiniteResult`
* `NoLastArguments`
* `NoLeadingZero`
* `NoNeedToClearErrors`
* `NoNegativeUnderflow`
* `NoNumberedVariables`
* `NoOverflow`
* `NoPlotAxes`
* `NoPositiveUnderflow`
* `NoProgramLastArguments`
* `NOr`
* `norm`
* `NormalPolynomialRender`
* `NoStackAutoScale`
* `NoStackSave`
* `not`
* `NoTrailingDecimal`
* `NoUndefinedResult`
* `NSub`
* `Num` (`CharToUnicode`, `Char→Code`, `Codepoint`)
* `NumberDotOrComma`
* `NumberedVariables`
* `NumberSeparatorCommand`
* `NumbersMenu`
* `NumberSpaces`
* `NumberTicks`
* `NumberUnderscore`
* `NumericalConstants`
* `NumericalIntegration`
* `NumericalResults`
* `NumericalSolver`
* `NumericalSolverMenu`
* `NΣ` (`DataSize`, `ΣSize`)
* `ObjectMenu`
* `Obj→` (`Explode`, `FromObj`, `Object→`)
* `Oct` (`Octal`)
* `Off` (`PowerOff`)
* `OffWithImage` (`PowerOffWithImage`)
* `OnesComplement`
* `or`
* `Over`
* `OverflowError`
* `OverflowIndicator`
* `OverflowValue`
* `PackedBitmaps`
* `Parametric` (`ParametricPlot`)
* `PartsMenu`
* `Path` (`DirectoryPath`)
* `PCov` (`PopulationCovariance`)
* `perm` (`Permutations`)
* `PgAll` (`PurgeAll`)
* `PgDir` (`PurgeDirectory`, `RmDir`)
* `Pi` (`π`)
* `Pick`
* `Pick3`
* `Pict` (`GraphicPicture`)
* `PiRadians` (`πradians`)
* `Pix?` (`PixelGrayLevel`, `PixTest`)
* `PixC?` (`PixColor`, `PixelColor`)
* `PixOff` (`DrawBackgroundPixel`)
* `PixOn` (`DrawPixel`)
* `PlotErrorBackground`
* `PlotMenu`
* `PlotRefreshRate`
* `PMax` (`PlotMax`)
* `PMin` (`PlotMin`)
* `Polar` (`PolarPlot`)
* `PolynomialsMenu`
* `PolynomialSolverMenu`
* `Poly→` (`FromPolynomial`, `Polynomial→`)
* `PositiveUnderflowIndicator`
* `PowerVoltage`
* `PPar` (`PlotParameters`)
* `PrcRnd` (`PrecisionRound`)
* `Prec` (`Precision`)
* `PredX` (`PredictX`)
* `PredY` (`PredictY`)
* `PrefixPolynomialRender`
* `PrevPr` (`PreviousPrime`)
* `PrincipalSolution`
* `PrintingMenu`
* `ProbabilitiesMenu`
* `ProgramLastArguments`
* `ProgramMenu`
* `Prompt`
* `PSDev` (`PopulationStandardDeviation`)
* `Purge`
* `PushEvaluatedAssignment`
* `PushOriginalAssignment`
* `Put`
* `PutI` (`PutIteration`)
* `PVar` (`PopulationVariance`)
* `PwrFit` (`PowerFit`)
* `P→R` (`PolarToReal`, `Polarℂ→ℝ`)
* `QuickSort`
* `Quote`
* `Rad` (`Radians`)
* `rand` (`RandomNumber`)
* `Random`
* `RandomGeneratorBits`
* `RandomGeneratorOrder`
* `RangeMenu`
* `Range→` (`FromRange`)
* `RanM` (`RandomArray`, `RandomMatrix`)
* `RcEq` (`RecallEquation`)
* `Rcl` (`Recall`)
* `Rcl*` (`Rcl×`, `Recall*`, `RecallMul`, `Recall×`)
* `Rcl+` (`Recall+`, `RecallAdd`)
* `Rcl-` (`Recall-`, `RecallSub`)
* `Rcl/` (`Rcl÷`, `Recall/`, `RecallDiv`, `Recall÷`)
* `RclF` (`FlagsToBinary`)
* `RclKeys` (`RecallKeys`)
* `RclMenu` (`RecallMenu`)
* `RclΣ` (`RecallData`, `RecallΣ`)
* `rcws` (`RecallWordSize`)
* `rdz` (`RandomSeed`)
* `re`
* `RealMenu`
* `RealResults`
* `Rect` (`DrawRectangle`)
* `Reduce` (`Stream`)
* `RelRnd` (`RelativeRound`)
* `rem`
* `ReorderTerms`
* `ReportPrecisionLoss`
* `Res` (`Resolution`)
* `ResetModes`
* `ResultBackground`
* `ResultFont`
* `ResultForeground`
* `ResultGraphingTimeLimit`
* `ReverseQuickSort`
* `ReverseSort`
* `RevList` (`ReverseList`)
* `RGB` (`RGBPattern`)
* `RightShiftBackground`
* `RightShiftForeground`
* `Risch` (`Primitive`)
* `RL` (`RotateLeft`)
* `RLB` (`RotateLeftByte`)
* `RLC` (`RotateLeftCount`)
* `Rnd` (`Round`)
* `RNrm` (`RowNorm`)
* `Roll`
* `RollD` (`RollDown`)
* `Root`
* `Rot`
* `RoundedMenus`
* `RoundMenuBackground`
* `RoundMenuForeground`
* `Row+` (`AddRow`)
* `Row-` (`DeleteRow`)
* `Row→` (`RowsToMatrix`, `Rows→`)
* `RR` (`RotateRight`)
* `RRB` (`RotateRightByte`)
* `RRC` (`RotateRightCount`)
* `RRect` (`DrawRoundedRectangle`)
* `RSwp` (`RowSwap`)
* `Run`
* `RunningIconForeground`
* `RunStats` (`RuntimeStatistics`)
* `RunStatsClearAfterRead`
* `RunStatsKeepAfterRead`
* `R→B` (`RealToBinary`)
* `R→C` (`RealToComplex`, `RealToRectangular`, `ℝ→ℂ`)
* `R→D` (`RadiansToDegrees`, `Radians→Degrees`)
* `R→I` (`ToInteger`, `→Int`, `→Integer`)
* `R→P` (`RealToPolar`, `ℝ→Polarℂ`)
* `same`
* `SaveLastArguments`
* `SaveStack`
* `SaveState`
* `Scale` (`PlotScale`)
* `ScaleH` (`PlotScaleX`, `ScaleX`, `XScale`)
* `ScaleW` (`PlotScaleY`, `ScaleY`, `YScale`)
* `Scatter` (`ScatterPlot`)
* `Sci` (`ScientificDisplay`)
* `ScreenCapture`
* `SDev` (`StandardDeviation`)
* `SearchBackground`
* `SearchForeground`
* `sec`
* `SelectedMenuForeground`
* `SelectionBackground`
* `SelectionForeground`
* `SeparatorModesMenu`
* `SetAngleUnits`
* `SetBit`
* `SetDate` (`→Date`)
* `SetTime` (`→Time`)
* `SF` (`SetFlag`)
* `SFP` (`SoftFP`, `SoftwareFloatingPoint`)
* `Show`
* `ShowAsDecimal`
* `ShowBuiltinCharacters`
* `ShowBuiltinConstants`
* `ShowBuiltinEquations`
* `ShowBuiltinLibrary`
* `ShowBuiltinUnits`
* `ShowDate`
* `ShowDayOfWeek`
* `ShowEmptyMenu`
* `ShowEquationBody`
* `ShowIntegersAndFractions`
* `ShowMonthName`
* `ShowSeconds`
* `ShowTime`
* `ShowTimeLimit`
* `ShowVoltage`
* `Sig` (`SignificantDisplay`)
* `SigDig` (`SignificantDigits`)
* `sign`
* `SignalProcessingMenu`
* `SilentBeepOff`
* `SilentBeepOn`
* `Simplify`
* `sin`
* `SingleLineResult`
* `SingleLineStack`
* `SingleRowMenus`
* `sinh`
* `Size`
* `SKey` (`StandardKey`)
* `SkippedMenuBackground`
* `SL` (`ShiftLeft`)
* `SLB` (`ShiftLeftByte`)
* `SLC` (`ShiftLeftCount`)
* `Slope`
* `SmallFractions`
* `SoftwareDisplayRefresh`
* `SolverImprecision`
* `SolverIterations`
* `SolverShuffles`
* `SolvingMenu`
* `Sort`
* `sq` (`Square`, `x²`)
* `SquareMenuBackground`
* `SquareMenuForeground`
* `SquareMenus`
* `SR` (`ShiftRight`)
* `SRB` (`ShiftRightByte`)
* `SRC` (`ShiftRightCount`)
* `sst` (`SingleStep`)
* `StackBackground`
* `StackEditor`
* `StackFont`
* `StackForeground`
* `StackGraphingTimeLimit`
* `StackLevelBackground`
* `StackLevelForeground`
* `StackLineForeground`
* `StackMenu`
* `StandardExponent`
* `StateNameForeground`
* `StatisticsMenu`
* `StatsPlotBins`
* `Std` (`StandardDisplay`)
* `StdRnd` (`StandardRound`)
* `StepByStepAlgebraResults`
* `StepOut`
* `StepOver`
* `SteppingIconForeground`
* `StEq` (`StoreEquation`)
* `Sto` (`Store`)
* `Sto*` (`Store*`, `StoreMul`, `Store×`, `Sto×`)
* `Sto+` (`Store+`, `StoreAdd`)
* `Sto-` (`Store-`, `StoreSub`)
* `Sto/` (`Store/`, `StoreDiv`, `Store÷`, `Sto÷`)
* `StoF` (`BinaryToFlags`)
* `StoKeys` (`StoreKeys`)
* `StoreAtEnd`
* `StoreAtStart`
* `StoVX` (`StoreAlgebraVariable`, `Storeⓧ`)
* `StoΣ` (`StoreData`, `StoreΣ`)
* `StrictArrayResizing`
* `Str→` (`Compile`, `Text→`)
* `stws` (`WordSize`)
* `Sub` (`Extract`)
* `Subst` (`Substitute`)
* `Swap`
* `SymbolicConstants`
* `SymbolicIntegration`
* `SymbolicMenu`
* `SymbolicResults`
* `SymbolicSolver`
* `SymbolicSolverMenu`
* `SystemMemory`
* `SystemSetup`
* `TabWidth`
* `Tag→` (`FromTag`)
* `Tail`
* `tan`
* `tanh`
* `TestsMenu`
* `TEval` (`TimedEval`)
* `TextMenu`
* `TextRenderingSizeLimit`
* `TextResultDisplay`
* `TextStackDisplay`
* `Text→Algebraic` (`CompileToAlgebraic`)
* `Text→Code` (`TextToUnicode`)
* `Text→Expression` (`CompileToExpression`)
* `Text→Integer` (`CompileToInteger`)
* `Text→Number` (`CompileToNumber`)
* `Text→Object` (`CompileToObject`)
* `Text→Positive` (`CompileToPositive`)
* `Text→Real` (`CompileToReal`)
* `ThreeRowsMenus`
* `Ticks`
* `Time`
* `Time12H`
* `Time24H`
* `TimeForeground`
* `TimeMenu`
* `TMenu` (`TemporaryMenu`)
* `ToArray` (`→Array`, `→Arry`)
* `ToBitmap` (`→Bitmap`)
* `ToCylindrical` (`→Cylindrical`)
* `ToDMS` (`→DMS`)
* `ToggleCustomMenu`
* `ToggleUserMode`
* `ToGrads` (`→Grad`, `→Grads`)
* `ToGrob` (`→Grob`)
* `ToHMS` (`→HMS`)
* `ToHPGrob` (`→HPGrob`)
* `ToLCD` (`→LCD`, `→Screen`)
* `ToList` (`→List`)
* `ToolsMenu`
* `TooManyDigitsErrors`
* `ToPolar` (`→Polar`)
* `ToRange` (`→Interval`, `→Range`)
* `ToSpherical` (`→Spherical`)
* `ToStr` (`ToText`, `→Str`, `→Text`)
* `Tot` (`DataTotal`, `ΣTotal`)
* `ToTag` (`→Tag`)
* `ToUnit` (`→Unit`)
* `TrailingDecimal`
* `Tran` (`Transpose`)
* `TrigIdentitiesMenu`
* `TrigSin`
* `Trn` (`TransConjugate`)
* `Trnc` (`Truncate`)
* `True`
* `Truth` (`TruthPlot`)
* `TruthLogicForIntegers`
* `TVars` (`TypedVariables`)
* `TVM` (`FinanceSolverMenu`)
* `TVMBeg` (`TVMPayAtBeginningOfPeriod`)
* `TVMEnd` (`TVMPayAtEndOfPeriod`)
* `TVMEquation`
* `TVMRoot`
* `TwoDigitYear`
* `TwosComplement`
* `Type`
* `TypeName`
* `UBase` (`BaseUnits`)
* `UFact` (`FactorUnit`)
* `UIAlgebraic`
* `UIAlpha`
* `UIBackspace`
* `UIDecimal`
* `UIDelete`
* `UIDown`
* `UIEnter`
* `UIExit`
* `UILeft`
* `UILeftShift`
* `UILowercase`
* `UINoAlpha`
* `UINoShift`
* `UIRight`
* `UIRightShift`
* `UIText`
* `UIUp`
* `UndefinedError`
* `UndefinedResultIndicator`
* `UndefinedValue`
* `UnderflowError`
* `UnderflowValue`
* `Undo`
* `Unimplemented`
* `UnimplementedForeground`
* `UnitsConversionsMenu`
* `UnitsMenu`
* `UnitsSIPrefixCycle`
* `UnPick`
* `UnRot`
* `UpDir` (`UpDirectory`)
* `UpperCase` (`UpperCaseCommands`)
* `UpperCaseNames` (`UpperCaseSymbols`)
* `USBPowered`
* `UseCrossForMultiplication`
* `UseDotForMultiplication`
* `UserCommandBackground`
* `UserCommandBorder`
* `UserCommandForeground`
* `UserInterfaceModesMenu`
* `UserMode`
* `UserModeLock`
* `UserModeMenu`
* `UserModeOff`
* `UserModeOnce`
* `UVal` (`UnitValue`)
* `Var` (`Variance`)
* `VariablesMenu`
* `VariableWidthDigits`
* `Vars` (`Variables`)
* `VectorMenu`
* `Version`
* `VerticalLists`
* `VerticalProgramRendering`
* `VerticalVectors`
* `VoltageForeground`
* `VX` (`AlgebraVariable`, `RclVX`, `ⓧ`)
* `V→` (`FromVector`, `Vector→`)
* `Wait`
* `XCol` (`IndependentColumn`)
* `XLib` (`LibraryItem`)
* `XLibName` (`ConstantName`, `EquationName`)
* `XLibValue` (`ConstantValue`, `EquationValue`)
* `xor`
* `xpon` (`Exponent`)
* `XQ` (`ToQuotient`, `→QPi`, `→Qπ`)
* `XRng` (`PlotXRange`, `XRange`)
* `xroot`
* `XVars` (`ExpressionVariables`)
* `XYPlotBins`
* `x³` (`cubed`)
* `YCol` (`DependentColumn`)
* `YearFirst`
* `YearLast`
* `YRng` (`PlotYRange`, `YRange`)
* `ZeroOverZeroIsError`
* `ZeroOverZeroIsUndefined`
* `ZeroPowerZeroIsOne`
* `ZeroPowerZeroIsUndefined`
* `|` (`Where`)
* `Γ` (`Gamma`, `tgamma`)
* `Σ` (`Sum`)
* `Σ+` (`AddData`)
* `Σ-` (`RemoveData`)
* `ΣDat` (`StatsData`, `ΣData`)
* `ΣLine` (`RegressionFormula`)
* `ΣList` (`ListSum`)
* `ΣPar` (`StatsParameters`, `ΣParameters`)
* `ΣX` (`SumOfX`)
* `ΣX2` (`SumOfXSquares`, `ΣX²`)
* `ΣXY` (`SumOfXY`)
* `ΣY` (`SumOfY`)
* `ΣY2` (`SumOfYSquares`, `ΣY²`)
* `℮` (`EulerianNumber`)
* `→%Range` (`ToPercentRange`)
* `→2D` (`To2DVector`, `→V2`)
* `→3D` (`To3DVector`, `→V3`)
* `→Cartesian` (`ToRectangular`, `→Rectangular`)
* `→Col` (`MatrixToColumns`, `→Columns`)
* `→Deg` (`ToDegrees`, `→Degrees`)
* `→Num` (`ToDecimal`, `→Decimal`, `→Number`)
* `→Poly` (`ToPolynomial`, `→Polynomial`)
* `→Prg` (`ToProgram`, `→Program`)
* `→Q` (`ToFraction`, `→Frac`, `→ℚ`)
* `→QDigits` (`FractionDigits`)
* `→QIterations` (`FractionIterations`)
* `→QπMaxPrime` (`FractionLargestPrime`)
* `→Rad` (`ToRadians`, `→Radians`)
* `→Row` (`MatrixToRows`, `→Rows`)
* `→Un` (`ToUncertain`, `→Uncertain`, `→σRange`)
* `→Ur` (`ToRelativeUncertainty`, `→RelUnc`)
* `→Us` (`ToStandardUncertainty`, `→StdUnc`)
* `→πr` (`ToPiRadians`, `→PiRadians`)
* `→∆Range` (`ToDeltaRange`)
* `↓Match` (`MatchDown`)
* `∂` (`Derivative`)
* `∆List` (`ListDifferences`)
* `∏` (`Product`)
* `∏List` (`ListProduct`)
* `−∞` (`NegativeInfinity`)
* `√` (`sqrt`, `SquareRoot`)
* `∛` (`cbrt`, `CubeRoot`)
* `∞` (`Infinity`)
* `∠` (`atan2`)
* `∩` (`Intersect`, `RangeIntersect`)
* `∪` (`RangeUnion`, `Union`)
* `∫` (`Integrate`)
* `≠` (`TestNE`)
* `≤` (`TestLE`)
* `≥` (`TestGE`)
* `⊿` (`hypot`, `Hypothenuse`)
* `Ⓓ` (`AlgebraConfiguration`, `CASDir`)
* `▶` (`Copy`)
* `⨯` (`cross`)
<!--- !DMNONE --->
<!--- DM42 --->
* `!`
* `%`
* `%Ch`
* `%T`
* `*`
* `+`
* `-`
* `/`
* `<`
* `=`
* `==`
* `>`
* `^`
* `abs`
* `acos`
* `acosh`
* `acot`
* `acsc`
* `AlarmMenu`
* `AlgebraMenu`
* `AllEquationVariables`
* `AllSolutions`
* `alog`
* `AlphaForeground`
* `Amort`
* `AmortTable`
* `And`
* `AngleMode`
* `AnglesMenu`
* `Apply`
* `ApplyInverseUnit`
* `ApplyUnit`
* `arg`
* `ArithmeticMenu`
* `Arry→`
* `asec`
* `asin`
* `asinh`
* `Asn`
* `ASR`
* `ASRB`
* `ASRC`
* `atan`
* `atanh`
* `Attach`
* `AutomaticTemporariesCleanup`
* `AutoScaleStack`
* `AutoSimplify`
* `Avg`
* `Background`
* `Bar`
* `Base`
* `BasedDotOrComma`
* `BasedSeparatorCommand`
* `BasedSpaces`
* `BasedSpacing`
* `BasedTicks`
* `BasedUnderscore`
* `BasesMenu`
* `BatteryBackground`
* `BatteryForeground`
* `BatteryLevelForeground`
* `BatteryRefresh`
* `BatteryVoltage`
* `Beep`
* `BeepOff`
* `BeepOn`
* `BestFit`
* `BigFractions`
* `Bin`
* `Bins`
* `BitwiseLogicForIntegers`
* `Blank`
* `BlankBitmap`
* `BlankGrob`
* `BusyIndicatorRefresh`
* `Bytes`
* `B→R`
* `CalculationMenu`
* `Capitalized`
* `CapitalizedNames`
* `CarefulEvaluation`
* `Catalog`
* `ceil`
* `Centr`
* `CF`
* `CharactersMenu`
* `ChargingForeground`
* `Chr`
* `ChronoTime`
* `Circle`
* `CircularMenu`
* `ClassicExponent`
* `Clear`
* `ClearBit`
* `ClearThingsMenu`
* `Clip`
* `ClLCD`
* `Clone`
* `ClΣ`
* `CNrm`
* `Col+`
* `Col-`
* `Collect`
* `Color`
* `ColΣ`
* `Col→`
* `comb`
* `CommandBackground`
* `CommandDisplayMode`
* `CommandForeground`
* `CompareMenu`
* `CompatibleBasedNumbers`
* `CompatibleGROBs`
* `CompatibleTypes`
* `ComplexIAfterImaginary`
* `ComplexIBeforeImaginary`
* `ComplexMenu`
* `ComplexResults`
* `Con`
* `conj`
* `ConLib`
* `Const`
* `ConstantRange`
* `Constants`
* `cont`
* `Convert`
* `ConvertToUnit`
* `ConvertToUnitPrefix`
* `Corr`
* `cos`
* `cosh`
* `cot`
* `CountBits`
* `Cov`
* `CrDir`
* `csc`
* `Cst`
* `CstRU`
* `CstSU`
* `CSwp`
* `CurrentClip`
* `CurrentDirectory`
* `CurrentEquationVariables`
* `CurrentFitSums`
* `CursorAlphaBackground`
* `CursorAlphaBorder`
* `CursorAlphaForeground`
* `CursorBackground`
* `CursorBlinkRate`
* `CursorBorder`
* `CursorFont`
* `CursorForeground`
* `CursorSelBackground`
* `CurveFilling`
* `CustomHeaderRefresh`
* `Cycle`
* `C→R`
* `Date`
* `Date+`
* `Date-`
* `DateDash`
* `DateDot`
* `DateForeground`
* `DatesMenu`
* `DateSeparatorCommand`
* `DateSlash`
* `DateSpace`
* `DateTime`
* `DayBeforeMonth`
* `dbug`
* `DebugMenu`
* `DebugOnError`
* `Dec`
* `DecimalComma`
* `DecimalDot`
* `Decr`
* `Deg`
* `DelKeys`
* `Depth`
* `det`
* `Detach`
* `DetailedTypes`
* `DFC`
* `DFC2F`
* `DifferentialSolverMenu`
* `Disp`
* `DisplayDigits`
* `DisplayMode`
* `DisplayModesMenu`
* `DispXY`
* `DistinguishSymbolCase`
* `Div2`
* `DMCPDisplayRefresh`
* `DMCPLowBattery`
* `DMS+`
* `DMS-`
* `DMS→`
* `doerr`
* `DoList`
* `DoSubs`
* `dot`
* `Draw`
* `DrawPlotAxes`
* `Drax`
* `Drop`
* `Drop2`
* `DropN`
* `DTag`
* `Dup`
* `Dup2`
* `DupDup`
* `DupN`
* `D→R`
* `Edit`
* `EditMenu`
* `EditorBackground`
* `EditorBegin`
* `EditorClear`
* `EditorCopy`
* `EditorCut`
* `EditorEnd`
* `EditorFlip`
* `EditorFont`
* `EditorForeground`
* `EditorHistory`
* `EditorHistoryBack`
* `EditorLineForeground`
* `EditorPaste`
* `EditorReplace`
* `EditorSearch`
* `EditorSelect`
* `EditorWordLeft`
* `EditorWordRight`
* `EditorWrapColumn`
* `Ellipse`
* `EndSub`
* `Eng`
* `Eq`
* `EqnLib`
* `equiv`
* `erf`
* `erfc`
* `err0`
* `errm`
* `errn`
* `ErrorBackground`
* `ErrorBeepDuration`
* `ErrorBeepFrequency`
* `ErrorBorder`
* `ErrorForeground`
* `Eval`
* `EvalEq`
* `Excludes`
* `ExitClearsMenu`
* `ExitKeepsMenu`
* `exp`
* `exp2`
* `Expan`
* `ExpFit`
* `ExplicitWildcards`
* `ExpLogIdentitiesMenu`
* `ExpLogMenu`
* `expm`
* `Factors`
* `False`
* `FancyExponent`
* `FC?`
* `FC?C`
* `FC?S`
* `FF`
* `FilesMenu`
* `Filter`
* `FinalAlgebraResults`
* `FinanceRounding`
* `FirstBitSet`
* `Fix`
* `FixedWidthDigits`
* `FlagsMenu`
* `FlatMenus`
* `FlipBit`
* `floor`
* `FoldConstants`
* `FolderCornerForeground`
* `Foreground`
* `FourDigitYear`
* `FP`
* `FractionsMenu`
* `FractionSpacing`
* `FreeMemory`
* `Freeze`
* `FS?`
* `FS?C`
* `FS?S`
* `Function`
* `GAnd`
* `GC`
* `GCD`
* `GCIconForeground`
* `GCStats`
* `GCStatsClearAfterRead`
* `GCStatsKeepAfterRead`
* `GCTemporariesCleanup`
* `Get`
* `GetI`
* `GOr`
* `Grad`
* `GraphicAppend`
* `GraphicExponent`
* `GraphicIntegral`
* `GraphicNorm`
* `GraphicParentheses`
* `GraphicProduct`
* `GraphicRatio`
* `GraphicResultDisplay`
* `GraphicRoot`
* `GraphicsMenu`
* `GraphicStackDisplay`
* `GraphicSubscript`
* `GraphicSum`
* `GraphingTimeLimit`
* `GraphRenderingSizeLimit`
* `Gray`
* `GrobAdd`
* `GXor`
* `HalfBatteryForeground`
* `Halt`
* `HaltedIconForeground`
* `Hdr`
* `Head`
* `HeaderBackground`
* `HeaderFont`
* `HeaderForeground`
* `Help`
* `Hex`
* `HFP`
* `HideBuiltinCharacters`
* `HideBuiltinConstants`
* `HideBuiltinEquations`
* `HideBuiltinLibrary`
* `HideBuiltinUnits`
* `HideDate`
* `HideDayOfWeek`
* `HideEmptyMenu`
* `HideEquationBody`
* `HideMonthName`
* `HideSeconds`
* `HideTime`
* `HideVoltage`
* `Histogram`
* `HMS+`
* `HMS-`
* `HMS→`
* `Home`
* `HorizontalLists`
* `HorizontalProgramRendering`
* `HorizontalVectors`
* `HSV`
* `HyperbolicMenu`
* `Idn`
* `IFT`
* `IFTE`
* `IgnorePrecisionLoss`
* `IgnoreSymbolCase`
* `im`
* `ImplicitWildcards`
* `Implies`
* `ImproperFractions`
* `Incr`
* `IndexDidNotWrap`
* `IndexWrapped`
* `InfiniteResultIndicator`
* `InfinityError`
* `InfinityValue`
* `Input`
* `IntegrationImprecision`
* `IntegrationIterations`
* `IntegrationMenu`
* `Intercept`
* `inv`
* `IOMenu`
* `IP`
* `Isol`
* `IsPrime`
* `JDN`
* `JDN→`
* `Keys`
* `Kill`
* `KillOnError`
* `LastArg`
* `LastBitSet`
* `LastMenu`
* `LastX`
* `LaxArrayResizing`
* `LazyEvaluation`
* `LCD→`
* `LCM`
* `LeadingZero`
* `LeftShiftBackground`
* `LeftShiftForeground`
* `LibEq`
* `Library`
* `Libs`
* `Line`
* `LinearFitSums`
* `LinearSolverMenu`
* `LineWidth`
* `LinFit`
* `ListAsData`
* `ListAsProgram`
* `ListMenu`
* `ListRecursionDepth`
* `List→`
* `ln`
* `ln1p`
* `LName`
* `lnΓ`
* `log`
* `log2`
* `LogFit`
* `LongForm`
* `LongFormNames`
* `LoopsMenu`
* `LowBattery`
* `LowBatteryForeground`
* `LowerAlphaForeground`
* `LowerCase`
* `LowerCaseNames`
* `LR`
* `MainMenu`
* `mant`
* `MantissaSpacing`
* `Map`
* `MatchUp`
* `MathMenu`
* `MathModesMenu`
* `MatrixMenu`
* `Max`
* `MaxFactorIterations`
* `MaxFactorsBits`
* `MaxFlags`
* `MaxH`
* `MaximumDecimalExponent`
* `MaxNumberBits`
* `MaxRewrites`
* `MaxW`
* `MaxΣ`
* `Median`
* `Mem`
* `MemoryMenu`
* `Menu`
* `MenuAppearance`
* `MenuBackground`
* `Min`
* `MinimumBatteryVoltage`
* `MinimumSignificantDigits`
* `MinΣ`
* `MixedFractions`
* `mod`
* `ModernBasedNumbers`
* `Modes`
* `ModesMenu`
* `MonthBeforeDay`
* `MRoot`
* `MSlv`
* `MSolvr`
* `MultilineCursorFont`
* `MultilineEditorFont`
* `MultiLineResult`
* `MultiLineStack`
* `MultipleSteps`
* `MultiSolverMenu`
* `NameDisplayMode`
* `nand`
* `NDupN`
* `NeedToClearErrors`
* `neg`
* `NegativeUnderflowIndicator`
* `NextEq`
* `NextPr`
* `Nip`
* `NoAngleUnits`
* `NoAutoSimplify`
* `NoCurveFilling`
* `NoErrorForTooManyDigits`
* `NoInfiniteResult`
* `NoLastArguments`
* `NoLeadingZero`
* `NoNeedToClearErrors`
* `NoNegativeUnderflow`
* `NoNumberedVariables`
* `NoOverflow`
* `NoPlotAxes`
* `NoPositiveUnderflow`
* `NoProgramLastArguments`
* `NOr`
* `norm`
* `NormalPolynomialRender`
* `NoStackAutoScale`
* `NoStackSave`
* `not`
* `NoTrailingDecimal`
* `NoUndefinedResult`
* `NSub`
* `Num`
* `NumberDotOrComma`
* `NumberedVariables`
* `NumberSeparatorCommand`
* `NumbersMenu`
* `NumberSpaces`
* `NumberTicks`
* `NumberUnderscore`
* `NumericalConstants`
* `NumericalIntegration`
* `NumericalResults`
* `NumericalSolver`
* `NumericalSolverMenu`
* `NΣ`
* `ObjectMenu`
* `Obj→`
* `Oct`
* `Off`
* `OffWithImage`
* `OnesComplement`
* `or`
* `Over`
* `OverflowError`
* `OverflowIndicator`
* `OverflowValue`
* `PackedBitmaps`
* `Parametric`
* `PartsMenu`
* `Path`
* `PCov`
* `perm`
* `PgAll`
* `PgDir`
* `Pi`
* `Pick`
* `Pick3`
* `Pict`
* `PiRadians`
* `Pix?`
* `PixC?`
* `PixOff`
* `PixOn`
* `PlotErrorBackground`
* `PlotMenu`
* `PlotRefreshRate`
* `PMax`
* `PMin`
* `Polar`
* `PolynomialsMenu`
* `PolynomialSolverMenu`
* `Poly→`
* `PositiveUnderflowIndicator`
* `PowerVoltage`
* `PPar`
* `PrcRnd`
* `Prec`
* `PredX`
* `PredY`
* `PrefixPolynomialRender`
* `PrevPr`
* `PrincipalSolution`
* `PrintingMenu`
* `ProbabilitiesMenu`
* `ProgramLastArguments`
* `ProgramMenu`
* `Prompt`
* `PSDev`
* `Purge`
* `PushEvaluatedAssignment`
* `PushOriginalAssignment`
* `Put`
* `PutI`
* `PVar`
* `PwrFit`
* `P→R`
* `QuickSort`
* `Quote`
* `Rad`
* `rand`
* `Random`
* `RandomGeneratorBits`
* `RandomGeneratorOrder`
* `RangeMenu`
* `Range→`
* `RanM`
* `RcEq`
* `Rcl`
* `Rcl*`
* `Rcl+`
* `Rcl-`
* `Rcl/`
* `RclF`
* `RclKeys`
* `RclMenu`
* `RclΣ`
* `rcws`
* `rdz`
* `re`
* `RealMenu`
* `RealResults`
* `Rect`
* `Reduce`
* `RelRnd`
* `rem`
* `ReorderTerms`
* `ReportPrecisionLoss`
* `Res`
* `ResetModes`
* `ResultBackground`
* `ResultFont`
* `ResultForeground`
* `ResultGraphingTimeLimit`
* `ReverseQuickSort`
* `ReverseSort`
* `RevList`
* `RGB`
* `RightShiftBackground`
* `RightShiftForeground`
* `Risch`
* `RL`
* `RLB`
* `RLC`
* `Rnd`
* `RNrm`
* `Roll`
* `RollD`
* `Root`
* `Rot`
* `RoundedMenus`
* `RoundMenuBackground`
* `RoundMenuForeground`
* `Row+`
* `Row-`
* `Row→`
* `RR`
* `RRB`
* `RRC`
* `RRect`
* `RSwp`
* `Run`
* `RunningIconForeground`
* `RunStats`
* `RunStatsClearAfterRead`
* `RunStatsKeepAfterRead`
* `R→B`
* `R→C`
* `R→D`
* `R→I`
* `R→P`
* `same`
* `SaveLastArguments`
* `SaveStack`
* `SaveState`
* `Scale`
* `ScaleH`
* `ScaleW`
* `Scatter`
* `Sci`
* `ScreenCapture`
* `SDev`
* `SearchBackground`
* `SearchForeground`
* `sec`
* `SelectedMenuForeground`
* `SelectionBackground`
* `SelectionForeground`
* `SeparatorModesMenu`
* `SetAngleUnits`
* `SetBit`
* `SetDate`
* `SetTime`
* `SF`
* `SFP`
* `Show`
* `ShowAsDecimal`
* `ShowBuiltinCharacters`
* `ShowBuiltinConstants`
* `ShowBuiltinEquations`
* `ShowBuiltinLibrary`
* `ShowBuiltinUnits`
* `ShowDate`
* `ShowDayOfWeek`
* `ShowEmptyMenu`
* `ShowEquationBody`
* `ShowIntegersAndFractions`
* `ShowMonthName`
* `ShowSeconds`
* `ShowTime`
* `ShowTimeLimit`
* `ShowVoltage`
* `Sig`
* `SigDig`
* `sign`
* `SignalProcessingMenu`
* `SilentBeepOff`
* `SilentBeepOn`
* `Simplify`
* `sin`
* `SingleLineResult`
* `SingleLineStack`
* `SingleRowMenus`
* `sinh`
* `Size`
* `SKey`
* `SkippedMenuBackground`
* `SL`
* `SLB`
* `SLC`
* `Slope`
* `SmallFractions`
* `SoftwareDisplayRefresh`
* `SolverImprecision`
* `SolverIterations`
* `SolverShuffles`
* `SolvingMenu`
* `Sort`
* `sq`
* `SquareMenuBackground`
* `SquareMenuForeground`
* `SquareMenus`
* `SR`
* `SRB`
* `SRC`
* `sst`
* `StackBackground`
* `StackEditor`
* `StackFont`
* `StackForeground`
* `StackGraphingTimeLimit`
* `StackLevelBackground`
* `StackLevelForeground`
* `StackLineForeground`
* `StackMenu`
* `StandardExponent`
* `StateNameForeground`
* `StatisticsMenu`
* `StatsPlotBins`
* `Std`
* `StdRnd`
* `StepByStepAlgebraResults`
* `StepOut`
* `StepOver`
* `SteppingIconForeground`
* `StEq`
* `Sto`
* `Sto*`
* `Sto+`
* `Sto-`
* `Sto/`
* `StoF`
* `StoKeys`
* `StoreAtEnd`
* `StoreAtStart`
* `StoVX`
* `StoΣ`
* `StrictArrayResizing`
* `Str→`
* `stws`
* `Sub`
* `Subst`
* `Swap`
* `SymbolicConstants`
* `SymbolicIntegration`
* `SymbolicMenu`
* `SymbolicResults`
* `SymbolicSolver`
* `SymbolicSolverMenu`
* `SystemMemory`
* `SystemSetup`
* `TabWidth`
* `Tag→`
* `Tail`
* `tan`
* `tanh`
* `TestsMenu`
* `TEval`
* `TextMenu`
* `TextRenderingSizeLimit`
* `TextResultDisplay`
* `TextStackDisplay`
* `Text→Algebraic`
* `Text→Code`
* `Text→Expression`
* `Text→Integer`
* `Text→Number`
* `Text→Object`
* `Text→Positive`
* `Text→Real`
* `ThreeRowsMenus`
* `Ticks`
* `Time`
* `Time12H`
* `Time24H`
* `TimeForeground`
* `TimeMenu`
* `TMenu`
* `ToArray`
* `ToBitmap`
* `ToCylindrical`
* `ToDMS`
* `ToggleCustomMenu`
* `ToggleUserMode`
* `ToGrads`
* `ToGrob`
* `ToHMS`
* `ToHPGrob`
* `ToLCD`
* `ToList`
* `ToolsMenu`
* `TooManyDigitsErrors`
* `ToPolar`
* `ToRange`
* `ToSpherical`
* `ToStr`
* `Tot`
* `ToTag`
* `ToUnit`
* `TrailingDecimal`
* `Tran`
* `TrigIdentitiesMenu`
* `TrigSin`
* `Trn`
* `Trnc`
* `True`
* `Truth`
* `TruthLogicForIntegers`
* `TVars`
* `TVM`
* `TVMBeg`
* `TVMEnd`
* `TVMEquation`
* `TVMRoot`
* `TwoDigitYear`
* `TwosComplement`
* `Type`
* `TypeName`
* `UBase`
* `UFact`
* `UIAlgebraic`
* `UIAlpha`
* `UIBackspace`
* `UIDecimal`
* `UIDelete`
* `UIDown`
* `UIEnter`
* `UIExit`
* `UILeft`
* `UILeftShift`
* `UILowercase`
* `UINoAlpha`
* `UINoShift`
* `UIRight`
* `UIRightShift`
* `UIText`
* `UIUp`
* `UndefinedError`
* `UndefinedResultIndicator`
* `UndefinedValue`
* `UnderflowError`
* `UnderflowValue`
* `Undo`
* `Unimplemented`
* `UnimplementedForeground`
* `UnitsConversionsMenu`
* `UnitsMenu`
* `UnitsSIPrefixCycle`
* `UnPick`
* `UnRot`
* `UpDir`
* `UpperCase`
* `UpperCaseNames`
* `USBPowered`
* `UseCrossForMultiplication`
* `UseDotForMultiplication`
* `UserCommandBackground`
* `UserCommandBorder`
* `UserCommandForeground`
* `UserInterfaceModesMenu`
* `UserMode`
* `UserModeLock`
* `UserModeMenu`
* `UserModeOff`
* `UserModeOnce`
* `UVal`
* `Var`
* `VariablesMenu`
* `VariableWidthDigits`
* `Vars`
* `VectorMenu`
* `Version`
* `VerticalLists`
* `VerticalProgramRendering`
* `VerticalVectors`
* `VoltageForeground`
* `VX`
* `V→`
* `Wait`
* `XCol`
* `XLib`
* `XLibName`
* `XLibValue`
* `xor`
* `xpon`
* `XQ`
* `XRng`
* `xroot`
* `XVars`
* `XYPlotBins`
* `x³`
* `YCol`
* `YearFirst`
* `YearLast`
* `YRng`
* `ZeroOverZeroIsError`
* `ZeroOverZeroIsUndefined`
* `ZeroPowerZeroIsOne`
* `ZeroPowerZeroIsUndefined`
* `|`
* `Γ`
* `Σ`
* `Σ+`
* `Σ-`
* `ΣDat`
* `ΣLine`
* `ΣList`
* `ΣPar`
* `ΣX`
* `ΣX2`
* `ΣXY`
* `ΣY`
* `ΣY2`
* `℮`
* `→%Range`
* `→2D`
* `→3D`
* `→Cartesian`
* `→Col`
* `→Deg`
* `→Num`
* `→Poly`
* `→Prg`
* `→Q`
* `→QDigits`
* `→QIterations`
* `→QπMaxPrime`
* `→Rad`
* `→Row`
* `→Un`
* `→Ur`
* `→Us`
* `→πr`
* `→∆Range`
* `↓Match`
* `∂`
* `∆List`
* `∏`
* `∏List`
* `−∞`
* `√`
* `∛`
* `∞`
* `∠`
* `∩`
* `∪`
* `∫`
* `≠`
* `≤`
* `≥`
* `⊿`
* `Ⓓ`
* `▶`
* `⨯`
<!--- !DM42 --->
<!--- DM32 --->
* `!`
* `%`
* `%Ch`
* `%T`
* `*`
* `+`
* `-`
* `/`
* `<`
* `=`
* `==`
* `>`
* `^`
* `abs`
* `acos`
* `acosh`
* `acot`
* `acsc`
* `AlarmMenu`
* `AlgebraMenu`
* `AllEquationVariables`
* `AllSolutions`
* `alog`
* `AlphaForeground`
* `Amort`
* `AmortTable`
* `And`
* `AngleMode`
* `AnglesMenu`
* `Apply`
* `ApplyInverseUnit`
* `ApplyUnit`
* `arg`
* `ArithmeticMenu`
* `Arry→`
* `asec`
* `asin`
* `asinh`
* `Asn`
* `ASR`
* `ASRB`
* `ASRC`
* `atan`
* `atanh`
* `Attach`
* `AutomaticTemporariesCleanup`
* `AutoScaleStack`
* `AutoSimplify`
* `Avg`
* `Background`
* `Bar`
* `Base`
* `BasedDotOrComma`
* `BasedSeparatorCommand`
* `BasedSpaces`
* `BasedSpacing`
* `BasedTicks`
* `BasedUnderscore`
* `BasesMenu`
* `BatteryBackground`
* `BatteryForeground`
* `BatteryLevelForeground`
* `BatteryRefresh`
* `BatteryVoltage`
* `Beep`
* `BeepOff`
* `BeepOn`
* `BestFit`
* `BigFractions`
* `Bin`
* `Bins`
* `BitwiseLogicForIntegers`
* `Blank`
* `BlankBitmap`
* `BlankGrob`
* `BusyIndicatorRefresh`
* `Bytes`
* `B→R`
* `CalculationMenu`
* `Capitalized`
* `CapitalizedNames`
* `CarefulEvaluation`
* `Catalog`
* `ceil`
* `Centr`
* `CF`
* `CharactersMenu`
* `ChargingForeground`
* `Chr`
* `ChronoTime`
* `Circle`
* `CircularMenu`
* `ClassicExponent`
* `Clear`
* `ClearBit`
* `ClearThingsMenu`
* `Clip`
* `ClLCD`
* `Clone`
* `ClΣ`
* `CNrm`
* `Col+`
* `Col-`
* `Collect`
* `Color`
* `ColΣ`
* `Col→`
* `comb`
* `CommandBackground`
* `CommandDisplayMode`
* `CommandForeground`
* `CompareMenu`
* `CompatibleBasedNumbers`
* `CompatibleGROBs`
* `CompatibleTypes`
* `ComplexIAfterImaginary`
* `ComplexIBeforeImaginary`
* `ComplexMenu`
* `ComplexResults`
* `Con`
* `conj`
* `ConLib`
* `Const`
* `ConstantRange`
* `Constants`
* `cont`
* `Convert`
* `ConvertToUnit`
* `ConvertToUnitPrefix`
* `Corr`
* `cos`
* `cosh`
* `cot`
* `CountBits`
* `Cov`
* `CrDir`
* `csc`
* `Cst`
* `CstRU`
* `CstSU`
* `CSwp`
* `CurrentClip`
* `CurrentDirectory`
* `CurrentEquationVariables`
* `CurrentFitSums`
* `CursorAlphaBackground`
* `CursorAlphaBorder`
* `CursorAlphaForeground`
* `CursorBackground`
* `CursorBlinkRate`
* `CursorBorder`
* `CursorFont`
* `CursorForeground`
* `CursorSelBackground`
* `CurveFilling`
* `CustomHeaderRefresh`
* `Cycle`
* `C→R`
* `Date`
* `Date+`
* `Date-`
* `DateDash`
* `DateDot`
* `DateForeground`
* `DatesMenu`
* `DateSeparatorCommand`
* `DateSlash`
* `DateSpace`
* `DateTime`
* `DayBeforeMonth`
* `dbug`
* `DebugMenu`
* `DebugOnError`
* `Dec`
* `DecimalComma`
* `DecimalDot`
* `Decr`
* `Deg`
* `DelKeys`
* `Depth`
* `det`
* `Detach`
* `DetailedTypes`
* `DFC`
* `DFC2F`
* `DifferentialSolverMenu`
* `Disp`
* `DisplayDigits`
* `DisplayMode`
* `DisplayModesMenu`
* `DispXY`
* `DistinguishSymbolCase`
* `Div2`
* `DMCPDisplayRefresh`
* `DMCPLowBattery`
* `DMS+`
* `DMS-`
* `DMS→`
* `doerr`
* `DoList`
* `DoSubs`
* `dot`
* `Draw`
* `DrawPlotAxes`
* `Drax`
* `Drop`
* `Drop2`
* `DropN`
* `DTag`
* `Dup`
* `Dup2`
* `DupDup`
* `DupN`
* `D→R`
* `Edit`
* `EditMenu`
* `EditorBackground`
* `EditorBegin`
* `EditorClear`
* `EditorCopy`
* `EditorCut`
* `EditorEnd`
* `EditorFlip`
* `EditorFont`
* `EditorForeground`
* `EditorHistory`
* `EditorHistoryBack`
* `EditorLineForeground`
* `EditorPaste`
* `EditorReplace`
* `EditorSearch`
* `EditorSelect`
* `EditorWordLeft`
* `EditorWordRight`
* `EditorWrapColumn`
* `Ellipse`
* `EndSub`
* `Eng`
* `Eq`
* `EqnLib`
* `equiv`
* `erf`
* `erfc`
* `err0`
* `errm`
* `errn`
* `ErrorBackground`
* `ErrorBeepDuration`
* `ErrorBeepFrequency`
* `ErrorBorder`
* `ErrorForeground`
* `Eval`
* `EvalEq`
* `Excludes`
* `ExitClearsMenu`
* `ExitKeepsMenu`
* `exp`
* `exp2`
* `Expan`
* `ExpFit`
* `ExplicitWildcards`
* `ExpLogIdentitiesMenu`
* `ExpLogMenu`
* `expm`
* `Factors`
* `False`
* `FancyExponent`
* `FC?`
* `FC?C`
* `FC?S`
* `FF`
* `FilesMenu`
* `Filter`
* `FinalAlgebraResults`
* `FinanceRounding`
* `FirstBitSet`
* `Fix`
* `FixedWidthDigits`
* `FlagsMenu`
* `FlatMenus`
* `FlipBit`
* `floor`
* `FoldConstants`
* `FolderCornerForeground`
* `Foreground`
* `FourDigitYear`
* `FP`
* `FractionsMenu`
* `FractionSpacing`
* `FreeMemory`
* `Freeze`
* `FS?`
* `FS?C`
* `FS?S`
* `Function`
* `GAnd`
* `GC`
* `GCD`
* `GCIconForeground`
* `GCStats`
* `GCStatsClearAfterRead`
* `GCStatsKeepAfterRead`
* `GCTemporariesCleanup`
* `Get`
* `GetI`
* `GOr`
* `Grad`
* `GraphicAppend`
* `GraphicExponent`
* `GraphicIntegral`
* `GraphicNorm`
* `GraphicParentheses`
* `GraphicProduct`
* `GraphicRatio`
* `GraphicResultDisplay`
* `GraphicRoot`
* `GraphicsMenu`
* `GraphicStackDisplay`
* `GraphicSubscript`
* `GraphicSum`
* `GraphingTimeLimit`
* `GraphRenderingSizeLimit`
* `Gray`
* `GrobAdd`
* `GXor`
* `HalfBatteryForeground`
* `Halt`
* `HaltedIconForeground`
* `Hdr`
* `Head`
* `HeaderBackground`
* `HeaderFont`
* `HeaderForeground`
* `Help`
* `Hex`
* `HFP`
* `HideBuiltinCharacters`
* `HideBuiltinConstants`
* `HideBuiltinEquations`
* `HideBuiltinLibrary`
* `HideBuiltinUnits`
* `HideDate`
* `HideDayOfWeek`
* `HideEmptyMenu`
* `HideEquationBody`
* `HideMonthName`
* `HideSeconds`
* `HideTime`
* `HideVoltage`
* `Histogram`
* `HMS+`
* `HMS-`
* `HMS→`
* `Home`
* `HorizontalLists`
* `HorizontalProgramRendering`
* `HorizontalVectors`
* `HSV`
* `HyperbolicMenu`
* `Idn`
* `IFT`
* `IFTE`
* `IgnorePrecisionLoss`
* `IgnoreSymbolCase`
* `im`
* `ImplicitWildcards`
* `Implies`
* `ImproperFractions`
* `Incr`
* `IndexDidNotWrap`
* `IndexWrapped`
* `InfiniteResultIndicator`
* `InfinityError`
* `InfinityValue`
* `Input`
* `IntegrationImprecision`
* `IntegrationIterations`
* `IntegrationMenu`
* `Intercept`
* `inv`
* `IOMenu`
* `IP`
* `Isol`
* `IsPrime`
* `JDN`
* `JDN→`
* `Keys`
* `Kill`
* `KillOnError`
* `LastArg`
* `LastBitSet`
* `LastMenu`
* `LastX`
* `LaxArrayResizing`
* `LazyEvaluation`
* `LCD→`
* `LCM`
* `LeadingZero`
* `LeftShiftBackground`
* `LeftShiftForeground`
* `LibEq`
* `Library`
* `Libs`
* `Line`
* `LinearFitSums`
* `LinearSolverMenu`
* `LineWidth`
* `LinFit`
* `ListAsData`
* `ListAsProgram`
* `ListMenu`
* `ListRecursionDepth`
* `List→`
* `ln`
* `ln1p`
* `LName`
* `lnΓ`
* `log`
* `log2`
* `LogFit`
* `LongForm`
* `LongFormNames`
* `LoopsMenu`
* `LowBattery`
* `LowBatteryForeground`
* `LowerAlphaForeground`
* `LowerCase`
* `LowerCaseNames`
* `LR`
* `MainMenu`
* `mant`
* `MantissaSpacing`
* `Map`
* `MatchUp`
* `MathMenu`
* `MathModesMenu`
* `MatrixMenu`
* `Max`
* `MaxFactorIterations`
* `MaxFactorsBits`
* `MaxFlags`
* `MaxH`
* `MaximumDecimalExponent`
* `MaxNumberBits`
* `MaxRewrites`
* `MaxW`
* `MaxΣ`
* `Median`
* `Mem`
* `MemoryMenu`
* `Menu`
* `MenuAppearance`
* `MenuBackground`
* `Min`
* `MinimumBatteryVoltage`
* `MinimumSignificantDigits`
* `MinΣ`
* `MixedFractions`
* `mod`
* `ModernBasedNumbers`
* `Modes`
* `ModesMenu`
* `MonthBeforeDay`
* `MRoot`
* `MSlv`
* `MSolvr`
* `MultilineCursorFont`
* `MultilineEditorFont`
* `MultiLineResult`
* `MultiLineStack`
* `MultipleSteps`
* `MultiSolverMenu`
* `NameDisplayMode`
* `nand`
* `NDupN`
* `NeedToClearErrors`
* `neg`
* `NegativeUnderflowIndicator`
* `NextEq`
* `NextPr`
* `Nip`
* `NoAngleUnits`
* `NoAutoSimplify`
* `NoCurveFilling`
* `NoErrorForTooManyDigits`
* `NoInfiniteResult`
* `NoLastArguments`
* `NoLeadingZero`
* `NoNeedToClearErrors`
* `NoNegativeUnderflow`
* `NoNumberedVariables`
* `NoOverflow`
* `NoPlotAxes`
* `NoPositiveUnderflow`
* `NoProgramLastArguments`
* `NOr`
* `norm`
* `NormalPolynomialRender`
* `NoStackAutoScale`
* `NoStackSave`
* `not`
* `NoTrailingDecimal`
* `NoUndefinedResult`
* `NSub`
* `Num`
* `NumberDotOrComma`
* `NumberedVariables`
* `NumberSeparatorCommand`
* `NumbersMenu`
* `NumberSpaces`
* `NumberTicks`
* `NumberUnderscore`
* `NumericalConstants`
* `NumericalIntegration`
* `NumericalResults`
* `NumericalSolver`
* `NumericalSolverMenu`
* `NΣ`
* `ObjectMenu`
* `Obj→`
* `Oct`
* `Off`
* `OffWithImage`
* `OnesComplement`
* `or`
* `Over`
* `OverflowError`
* `OverflowIndicator`
* `OverflowValue`
* `PackedBitmaps`
* `Parametric`
* `PartsMenu`
* `Path`
* `PCov`
* `perm`
* `PgAll`
* `PgDir`
* `Pi`
* `Pick`
* `Pick3`
* `Pict`
* `PiRadians`
* `Pix?`
* `PixC?`
* `PixOff`
* `PixOn`
* `PlotErrorBackground`
* `PlotMenu`
* `PlotRefreshRate`
* `PMax`
* `PMin`
* `Polar`
* `PolynomialsMenu`
* `PolynomialSolverMenu`
* `Poly→`
* `PositiveUnderflowIndicator`
* `PowerVoltage`
* `PPar`
* `PrcRnd`
* `Prec`
* `PredX`
* `PredY`
* `PrefixPolynomialRender`
* `PrevPr`
* `PrincipalSolution`
* `PrintingMenu`
* `ProbabilitiesMenu`
* `ProgramLastArguments`
* `ProgramMenu`
* `Prompt`
* `PSDev`
* `Purge`
* `PushEvaluatedAssignment`
* `PushOriginalAssignment`
* `Put`
* `PutI`
* `PVar`
* `PwrFit`
* `P→R`
* `QuickSort`
* `Quote`
* `Rad`
* `rand`
* `Random`
* `RandomGeneratorBits`
* `RandomGeneratorOrder`
* `RangeMenu`
* `Range→`
* `RanM`
* `RcEq`
* `Rcl`
* `Rcl*`
* `Rcl+`
* `Rcl-`
* `Rcl/`
* `RclF`
* `RclKeys`
* `RclMenu`
* `RclΣ`
* `rcws`
* `rdz`
* `re`
* `RealMenu`
* `RealResults`
* `Rect`
* `Reduce`
* `RelRnd`
* `rem`
* `ReorderTerms`
* `ReportPrecisionLoss`
* `Res`
* `ResetModes`
* `ResultBackground`
* `ResultFont`
* `ResultForeground`
* `ResultGraphingTimeLimit`
* `ReverseQuickSort`
* `ReverseSort`
* `RevList`
* `RGB`
* `RightShiftBackground`
* `RightShiftForeground`
* `Risch`
* `RL`
* `RLB`
* `RLC`
* `Rnd`
* `RNrm`
* `Roll`
* `RollD`
* `Root`
* `Rot`
* `RoundedMenus`
* `RoundMenuBackground`
* `RoundMenuForeground`
* `Row+`
* `Row-`
* `Row→`
* `RR`
* `RRB`
* `RRC`
* `RRect`
* `RSwp`
* `Run`
* `RunningIconForeground`
* `RunStats`
* `RunStatsClearAfterRead`
* `RunStatsKeepAfterRead`
* `R→B`
* `R→C`
* `R→D`
* `R→I`
* `R→P`
* `same`
* `SaveLastArguments`
* `SaveStack`
* `SaveState`
* `Scale`
* `ScaleH`
* `ScaleW`
* `Scatter`
* `Sci`
* `ScreenCapture`
* `SDev`
* `SearchBackground`
* `SearchForeground`
* `sec`
* `SelectedMenuForeground`
* `SelectionBackground`
* `SelectionForeground`
* `SeparatorModesMenu`
* `SetAngleUnits`
* `SetBit`
* `SetDate`
* `SetTime`
* `SF`
* `SFP`
* `Show`
* `ShowAsDecimal`
* `ShowBuiltinCharacters`
* `ShowBuiltinConstants`
* `ShowBuiltinEquations`
* `ShowBuiltinLibrary`
* `ShowBuiltinUnits`
* `ShowDate`
* `ShowDayOfWeek`
* `ShowEmptyMenu`
* `ShowEquationBody`
* `ShowIntegersAndFractions`
* `ShowMonthName`
* `ShowSeconds`
* `ShowTime`
* `ShowTimeLimit`
* `ShowVoltage`
* `Sig`
* `SigDig`
* `sign`
* `SignalProcessingMenu`
* `SilentBeepOff`
* `SilentBeepOn`
* `Simplify`
* `sin`
* `SingleLineResult`
* `SingleLineStack`
* `SingleRowMenus`
* `sinh`
* `Size`
* `SKey`
* `SkippedMenuBackground`
* `SL`
* `SLB`
* `SLC`
* `Slope`
* `SmallFractions`
* `SoftwareDisplayRefresh`
* `SolverImprecision`
* `SolverIterations`
* `SolverShuffles`
* `SolvingMenu`
* `Sort`
* `sq`
* `SquareMenuBackground`
* `SquareMenuForeground`
* `SquareMenus`
* `SR`
* `SRB`
* `SRC`
* `sst`
* `StackBackground`
* `StackEditor`
* `StackFont`
* `StackForeground`
* `StackGraphingTimeLimit`
* `StackLevelBackground`
* `StackLevelForeground`
* `StackLineForeground`
* `StackMenu`
* `StandardExponent`
* `StateNameForeground`
* `StatisticsMenu`
* `StatsPlotBins`
* `Std`
* `StdRnd`
* `StepByStepAlgebraResults`
* `StepOut`
* `StepOver`
* `SteppingIconForeground`
* `StEq`
* `Sto`
* `Sto*`
* `Sto+`
* `Sto-`
* `Sto/`
* `StoF`
* `StoKeys`
* `StoreAtEnd`
* `StoreAtStart`
* `StoVX`
* `StoΣ`
* `StrictArrayResizing`
* `Str→`
* `stws`
* `Sub`
* `Subst`
* `Swap`
* `SymbolicConstants`
* `SymbolicIntegration`
* `SymbolicMenu`
* `SymbolicResults`
* `SymbolicSolver`
* `SymbolicSolverMenu`
* `SystemMemory`
* `SystemSetup`
* `TabWidth`
* `Tag→`
* `Tail`
* `tan`
* `tanh`
* `TestsMenu`
* `TEval`
* `TextMenu`
* `TextRenderingSizeLimit`
* `TextResultDisplay`
* `TextStackDisplay`
* `Text→Algebraic`
* `Text→Code`
* `Text→Expression`
* `Text→Integer`
* `Text→Number`
* `Text→Object`
* `Text→Positive`
* `Text→Real`
* `ThreeRowsMenus`
* `Ticks`
* `Time`
* `Time12H`
* `Time24H`
* `TimeForeground`
* `TimeMenu`
* `TMenu`
* `ToArray`
* `ToBitmap`
* `ToCylindrical`
* `ToDMS`
* `ToggleCustomMenu`
* `ToggleUserMode`
* `ToGrads`
* `ToGrob`
* `ToHMS`
* `ToHPGrob`
* `ToLCD`
* `ToList`
* `ToolsMenu`
* `TooManyDigitsErrors`
* `ToPolar`
* `ToRange`
* `ToSpherical`
* `ToStr`
* `Tot`
* `ToTag`
* `ToUnit`
* `TrailingDecimal`
* `Tran`
* `TrigIdentitiesMenu`
* `TrigSin`
* `Trn`
* `Trnc`
* `True`
* `Truth`
* `TruthLogicForIntegers`
* `TVars`
* `TVM`
* `TVMBeg`
* `TVMEnd`
* `TVMEquation`
* `TVMRoot`
* `TwoDigitYear`
* `TwosComplement`
* `Type`
* `TypeName`
* `UBase`
* `UFact`
* `UIAlgebraic`
* `UIAlpha`
* `UIBackspace`
* `UIDecimal`
* `UIDelete`
* `UIDown`
* `UIEnter`
* `UIExit`
* `UILeft`
* `UILeftShift`
* `UILowercase`
* `UINoAlpha`
* `UINoShift`
* `UIRight`
* `UIRightShift`
* `UIText`
* `UIUp`
* `UndefinedError`
* `UndefinedResultIndicator`
* `UndefinedValue`
* `UnderflowError`
* `UnderflowValue`
* `Undo`
* `Unimplemented`
* `UnimplementedForeground`
* `UnitsConversionsMenu`
* `UnitsMenu`
* `UnitsSIPrefixCycle`
* `UnPick`
* `UnRot`
* `UpDir`
* `UpperCase`
* `UpperCaseNames`
* `USBPowered`
* `UseCrossForMultiplication`
* `UseDotForMultiplication`
* `UserCommandBackground`
* `UserCommandBorder`
* `UserCommandForeground`
* `UserInterfaceModesMenu`
* `UserMode`
* `UserModeLock`
* `UserModeMenu`
* `UserModeOff`
* `UserModeOnce`
* `UVal`
* `Var`
* `VariablesMenu`
* `VariableWidthDigits`
* `Vars`
* `VectorMenu`
* `Version`
* `VerticalLists`
* `VerticalProgramRendering`
* `VerticalVectors`
* `VoltageForeground`
* `VX`
* `V→`
* `Wait`
* `XCol`
* `XLib`
* `XLibName`
* `XLibValue`
* `xor`
* `xpon`
* `XQ`
* `XRng`
* `xroot`
* `XVars`
* `XYPlotBins`
* `x³`
* `YCol`
* `YearFirst`
* `YearLast`
* `YRng`
* `ZeroOverZeroIsError`
* `ZeroOverZeroIsUndefined`
* `ZeroPowerZeroIsOne`
* `ZeroPowerZeroIsUndefined`
* `|`
* `Γ`
* `Σ`
* `Σ+`
* `Σ-`
* `ΣDat`
* `ΣLine`
* `ΣList`
* `ΣPar`
* `ΣX`
* `ΣX2`
* `ΣXY`
* `ΣY`
* `ΣY2`
* `℮`
* `→%Range`
* `→2D`
* `→3D`
* `→Cartesian`
* `→Col`
* `→Deg`
* `→Num`
* `→Poly`
* `→Prg`
* `→Q`
* `→QDigits`
* `→QIterations`
* `→QπMaxPrime`
* `→Rad`
* `→Row`
* `→Un`
* `→Ur`
* `→Us`
* `→πr`
* `→∆Range`
* `↓Match`
* `∂`
* `∆List`
* `∏`
* `∏List`
* `−∞`
* `√`
* `∛`
* `∞`
* `∠`
* `∩`
* `∪`
* `∫`
* `≠`
* `≤`
* `≥`
* `⊿`
* `Ⓓ`
* `▶`
* `⨯`
<!--- !DM32 --->

## Unimplemented commands

The following is a list of unimplemented HP50G RPL commands which should be
implemented by the time the project reaches version 1.0.

* `ABCUV`
* `ACK`
* `ACKALL`
* `ACOS2S`
* `ADDTMOD`
* `ADDTOREAL`
* `ALGB`
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
* `BOX`
* `C2P`
* `CASCFG`
* `CASCMD`
* `CHINREM`
* `CHOLESKY`
* `CHOOSE`
* `CIRC`
* `CLUSR`
* `CLVAR`
* `CMPLX`
* `COLCT`
* `COND`
* `CONIC`
* `CORR`
* `CURL`
* `CYCLOTOMIC`
* `CYLIN`
* `C→PX`
* `DARCY`
* `DEF`
* `DEFINE`
* `DEGREE`
* `DELALARM`
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
* `INT`
* `INTEGER`
* `INTVX`
* `INVMOD`
* `IQUOT`
* `IREMAINDER`
* `ISOM`
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
* `LCM`
* `LCXM`
* `LDEC`
* `LEGENDRE`
* `LGCD`
* `lim`
* `LIMIT`
* `LIN`
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
* `PREVAL`
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
* `TRIGTAN`
* `TRN`
* `TRUNC`
* `TRUTH`
* `TSIMP`
* `TSTR`
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

* `BAUD`
* `BUFLEN`
* `C$`
* `CKSM`
* `CLKADJ`
* `CLOSEIO`
* `CR`
* `DELAY`
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
