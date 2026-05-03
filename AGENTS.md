# DB48X Project Guide

## General Guidelines for AI Assistants

- **Do not stack unrelated code changes.** If the user is in the middle of
  developing feature A, avoid introducing unrelated changes (B, C) into the same
  edits; they can be hard to deinterlace for clean commits. If the repository
  is clean and a git operation would make it easy to restore the earlier state,
  then making extra changes is OK. Otherwise, add ideas to the todo list to
  be done later in a separate commit.
- **Avoid overwriting the user's edits.** When making changes, the user may have
  adjusted files themselves. Read the current file state before editing; prefer
  minimal, targeted edits that accomplish the goal without clobbering unrelated
  changes the user has made.

## Build System

- `make -j8` builds the DM42 ARM firmware. A QSPI CRC change requires a
  **second `make`** run (the first will say "Run build once more").
- `make -j sim` builds the macOS Qt simulator (used for testing).
- `help/db48x.md` and `help/db50x.md` are **auto-generated** by the build.
  Never edit them directly; edit `doc/commands/*.md` instead.
- **Documentation internal links**: use only the anchor part, e.g. `(#Anchor)`,
  not `(settings.md#Anchor)`. The shipped help (`db48x.md`, `db50x.md`) is built
  by concatenating and sed-editing the input markdown files into a single file,
  so cross-file links break.
- **Command cross-references in docs**: to reference another command, simply use
  backtick syntax, e.g. `` `ToRectangular` ``. The help rendering engine
  automatically links backtick-quoted command names to their documentation.
  Any spelling or alias works; the engine displays the current style.
  Do not use explicit markdown links for command cross-references.
- **No duplicate entries for alternate spellings**: the help system automatically
  displays all alternate spellings for a command. Do not create separate `##`
  entries for aliases or compatibility spellings. Document each command once
  under its primary name.

## Adding a New Command

1. **`src/ids.tbl`**: Add `NAMED(Foo, "→Foo")` or `CMD(Foo)`. Add `ALIAS` for
   alternate spellings. Update any `ID_RANGE` lines whose boundary the new
   command changes (e.g. `is_command`, `is_algebraic_fn`).
2. **Header** (e.g. `src/functions.h`): Declare with `FUNCTION(Foo)`,
   `COMMAND_DECLARE(Foo, nargs)`, or `STANDARD_FUNCTION(Foo)` as appropriate.
3. **Implementation** (e.g. `src/functions.cc`): Implement with
   `FUNCTION_BODY(Foo)` or `COMMAND_BODY(Foo)`.
4. **`src/menu.cc`**: Wire up menu entries (replace `ID_Unimplemented` if a
   placeholder exists).

### Menu layout

- Menus display **6×3 = 18 entries** when there are fewer than 18; otherwise
  **5×3 + next/previous** keys (15 visible, paginated).
- Group entries logically (vertically or horizontally). Alignment matters for
  usability.
- When adding a menu entry, ensure the count stays ≤18 or that the grouping
  still makes sense with pagination. If adding one entry would exceed 18,
  consider removing another that is available elsewhere (e.g. via shortcuts or
  other menus).

## Running Tests

- Build the simulator first: `make -j sim`
- **Before submitting changes, run the full test suite** and fix any failures.
  From the repository root (after building the simulator):
  ```
  db48x -H -Tall
  ```
  The `-H` flag runs in headless mode (no Qt window). Exit code 0 means all
  tests passed; non-zero means there are failures (see console output or
  `failures.log` if you capture it).
- To run a single test group: `db48x -H -Tname` (e.g. `-Tsectrig`).
- Enable recorder traces during tests with `-tchannel` (see Debugging section):
  ```
  db48x -H -Ttoqpi -talgebraic
  ```
- **All new features and bug fixes must include tests.** Build and run the
  full suite (`db48x -H -Tall`) before considering work complete.

## Adding Tests

- Test functions go in `src/tests.cc`, declared in `src/tests.h`.
- Every test function using `BEGIN(name)` **must** have a corresponding
  `TESTS(name, "description")` macro near the top of `tests.cc` (around
  lines 89-160). Without it, the `check_<name>` function won't exist and the
  sim build will fail.
- Register the function call in the test runner (around line 237).
- The calculator works at 24-digit precision by default. Test inputs typed as
  literal decimals must have enough digits, or use computed values (e.g.
  `"pi →Num 2 /"` instead of `"1.5707963268"`).
- Default fraction display uses superscript/subscript (`¹/₄`). Use
  `"BigFractions ImproperFractions"` in tests for plain `1/4` format, and
  restore with `"SmallFractions MixedFractions"` at the end.
- **Restoring settings**: when a test changes a setting (e.g. `10000
  FractionLargestPrime`), restore the default at the end using `Purge`:
  `{ FractionLargestPrime } Purge Std`. The `Std` reapplies standard
  defaults. See `MaxFactorIterations` and `FractionLargestPrime` in
  `tests.cc` for examples.
- **Test assertions** (`.expect`, `.want`, `.match`):
  - Use `.expect()` for **exact match**.
  - Use `.want()` when you only need to ignore/allow variation in spaces
    (e.g. matrices and vectors, which may include newlines or extra spaces).
  - Use `.match()` when the expected value is easier to express with a
    **regexp**.
- **Vectors and matrices** cannot contain non-algebraic expressions directly.
  Use `[ 0.25 '√2' ] →Num` instead of `[ 0.25 2 √ →Num ]` when you need a
  vector or matrix of decimals for testing.
- **Lists are not evaluated** in RPL. `{ 1.5 neg 2.3 }` builds a list
  containing the literal objects 1.5, `neg` (the operator), and 2.3 — not
  -1.5. For a list with negative numbers, use `{ -1.5 2.3 }` (literal minus).
  Using `neg` inside a list leads to "Bad argument type" when applying →Q or
  similar commands that expect numeric elements.

## Architecture Notes

- **DB48X uses C++ in a very special way.** Object values are not stored in the
  object itself, but as LEB128 encoding in the object. RPL objects can be moved
  in memory, garbage collected, etc. They are byte-aligned and very compact.
  **It is NOT ALLOWED EVER EVER EVER to use a virtual function**, because that
  would add
  at least one word to the object for the vptr, which can't be copied or moved
  willy-nilly. Dispatch is done using `obj->type()` and a switch statement (or
  the handler table), not virtual functions.

- `src/ids.tbl` is the single source of truth for all object/command IDs. It is
  `#include`d with varying macro definitions to generate enums, spelling tables,
  and dispatch tables.
- Dispatch: `object::handler[ID_Foo].evaluate` calls `Foo::do_evaluate`.
- For `FUNCTION`-style commands: the evaluate chain is
  `do_evaluate` -> `evaluate()` (static) -> `evaluate(algebraic_r x)`.
- **Smart pointer conventions** (`_p`, `_g`, `_r`):
  - Prefer **`_p`** return types unless there is a type-related need for `_g`
    (e.g. the caller must extend lifetime / adopt a new allocation).
  - Use **`_r`** for arguments unless the callee would need to construct a `_g`
    inside (e.g. to hold a temporary).
  - `_g` = GC-safe owning pointer; `_p` = non-owning raw pointer; `_r` = reference.
- Arithmetic operators (`+`, `-`, `*`, `/`) are overloaded for `algebraic_r`.
  Multiplying a numeric type by a symbolic type (e.g. `constant`) will typically
  try numeric evaluation, **not** create a symbolic expression. Use
  `expression::make(ID_multiply, x, y)` to build symbolic products explicitly.
- `constant::lookup("π", false)` returns a symbolic constant (the `false`
  suppresses errors if not found). `algebraic::pi()` returns a numeric value.

### Type ranges (`ID_RANGE`) — critical gotcha

The `ID_RANGE(is_integer, ...)` range covers IDs from `based_integer` through
`neg_integer`. Because of the ordering in `ids.tbl`, **bignums fall inside this
range** (`bignum` and `neg_bignum` have IDs between `based_integer` and
`neg_integer`). Therefore `object::is_integer(ID_bignum)` returns **true**.

When extracting numeric values, always check `is_bignum` **before** `is_integer`
and use `bignum_p(…)->value<ularge>()`. Casting a bignum via `integer_p` gives
garbage because `integer` uses LEB128 encoding while `bignum` inherits from
`text` (length-prefixed raw bytes). Similarly, `is_fraction` includes
`big_fraction`, so `fraction_p(…)->numerator_value()` works for both, but be
aware of the overlap.

The general rule: **narrow type checks (`is_bignum`, `is_big_fraction`) before
wide ones (`is_integer`, `is_fraction`)** when the code path depends on the
memory layout.

`decimal::to_fraction()` returns a `bignum` (not `integer`) for whole-number
results, because it works with `bignum_g` internally.

## Debugging with the Recorder

Use the project's `recorder` library instead of `fprintf(stderr, ...)`.

1. Each source file declares recorder channels near the top:
   `RECORDER(algebraic, 16, "RPL Algebraics");`
2. Log with `record(channel, "format %+s %u %p", ...)`:
   - `%+s` for `utf8` / `object::name(ty)` strings
   - `%t` for **readable object dumps** (objects rendered like the stack display)
   - `%u`, `%d` for integers, `%p` for pointers
3. Recorder output is ring-buffered and low-overhead; safe to leave in
   production code (unlike `fprintf`). **Never use `fprintf` for debugging.**
4. **Enabling traces at runtime**: use the **`-t` flag** on the simulator
   command line. The flag takes a comma-separated list of recorder channel names:
   ```
   ./db48x.app/Contents/MacOS/db48x -Ttoqpi -talgebraic
   ./db48x.app/Contents/MacOS/db48x -Tall -talgebraic,fraction,expression
   ```
   You can also use the `RECORDER_TRACES` environment variable:
   `RECORDER_TRACES=algebraic ./db48x ...`
5. Trace output is interleaved with test results on stderr, formatted as:
   `[sequence timestamp] channel: message`
6. For new functionality, add `record()` calls at key decision points
   (type dispatch, computed values, best-candidate selection) so future
   debugging doesn't require adding and removing `fprintf` statements.

## Code Style

- **Section comments** (grouping multiple functions/structs): use `// ====` on 5 lines:
  ```
  // ============================================================================
  //
  //   Section title
  //
  // ============================================================================
  ```
- **Function/struct headers**: `// ---...---` separator lines around bodies.
- **One-line header only**: The comment between the first `// ----` lines after a
  function name must be strictly **one line**. If it can't be summarized in one
  line, it's too verbose. Put details below the second `// ----` line.
- No narrating comments; comments explain non-obvious intent only.
- Use `object::ID_foo` (not bare `ID_foo`) in static/non-member functions where
  the `id` typedef is not in scope.
- **No hanging statements**: every `for`, `while`, `if`, etc. that controls more
  than a trivial single statement must use braces. Even when the body is a single
  sub-statement (e.g. a `while` inside a `for`), wrap the outer loop body in
  braces. Allman-style brace placement (opening brace on its own line).
- **Line length**: if a statement fits within 80 columns, keep it on one line;
  avoid gratuitous line breaks.
- When unsure how to format a section of code, run it through `clang-format`.
