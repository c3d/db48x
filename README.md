# DB48X: A modern implementation of RPL

The DB48X project intends to rebuild and improve the user experience of the
HP48 family of calculators, notably their "Reverse Polish Lisp" (RPL)
language with its rich set of data types and built-in functions, and
[Lisp-level programming power](http://www.paulgraham.com/avg.html).

The project in this repository is presently targeting the
[SwissMicro DM42](https://www.swissmicros.com/product/dm42) and
[DM32](https://www.swissmicros.com/product/dm42) calculators.
You can [order keyboard overlays](https://www.hpmuseum.org/forum/thread-20113.html) to adjust the calculator's faceplates to DB48X.
DB48X leverages SwissMicros's built-in software platform, known as
[DMCP](https://technical.swissmicros.com/dmcp/doc/DMCP-ifc-html/).
There is also a simulator that is tested on macOS or Linux, and you can even
[try it in your browser](http://c3d.github.com/db48x)

In the long-term, the vision is to be able to port DB48X on a number of
[different physical calculator platforms](https://www.youtube.com/watch?v=34pPycq8ia8),
like the ARM-based
[HP50 and related machines (HP49, HP48Gii, etc)](https://en.wikipedia.org/wiki/HP_49/50_series),
and the [HP Prime](https://en.wikipedia.org/wiki/HP_Prime)
(at least the G1, since the G2 seems a bit more locked down), maybe others.
The basis for that work can be found in the [DB48X](https://github.com/c3d/db48x) project.

This project was presented at [FOSDEM 2023][fosdem23] and [FOSDEM 2024][fosdem24]

[fosdem23]: https://fosdem.org/2023/schedule/event/reversepolishlisp/
[fosdem24]: https://fosdem.org/2024/schedule/event/fosdem-2024-1990-how-much-math-can-you-fit-in-700k-/

[![FOSDEM 2024 video](http://img.youtube.com/vi/WU5Dg1mPkl8/0.jpg)](https://www.youtube.com/watch?v=WU5Dg1mPkl8&list=PLz1qkflzABy-Cs1R07zGB8A9K5Yjolmlf "FOSDEM 2024 video")

## Why name the project DB48X?

DB stands for "Dave and Bill", who are more commonly known as Hewlett and
Packard. The order is reversed compared to HP, since they reportedly chose the
order at random, and it's about time Dave Packard was given preeminence.

Part of Dave and Bill's great legacy (beyond giving birth to the Silicon Valley)
is a [legendary series of calculators](https://www.hpmuseum.org).
The [HP48](https://en.wikipedia.org/wiki/HP_48_series) remains one of my
favorites, notably for its rich built-in programming language, known as [Reverse
Polish Lisp (RPL)](https://en.wikipedia.org/wiki/RPL_(programming_language)).
This project aims at recreating a decent successor to the HP48, at least in
spirit.


## State of the project

This is currently **UNSTABLE** and **INCOMPLETE** software. Please only consider
installing this if you are a developer and interested in contributing. Or else,
have a paperclip at hand just in case you need to reset your calculator.

The detailed current status is described in the [STATUS file](STATUS.md).

[![Self-test in the simulator](http://img.youtube.com/vi/vT-I3UlROtA/0.jpg)](https://www.youtube.com/watch?v=vT-I3UlROtA "Self-test demo")


## How to build this project

There is a separate document explaining [how to build this project](BUILD.md).
The simulator includes a test suite, which you should run before submitting
patches. To run these tests, pass the `-T` option to the simulator, or hit the
**F12** key in the simulator.


## Built-in documentation

The calculator features an extensive [built-in documentation](help/db48x.md)
that uses a restricted version of [Markdown](https://www.markdownguide.org).
You access that built-in help by [holding a key down](help/db48x.md#help), or
using the [`Help` command](doc/commands/system.md#help)

* [Design overview](help/db48x.md#design-overview)
* [Keyboard interaction](help/db48x.md#keyboard-interaction)
* [Soft menus](help/db48x.md#soft-menus)


## Other documentation

There is DMCP interface doc in progress see [DMCP IFC doc](http://technical.swissmicros.com/dmcp/doc/DMCP-ifc-html/)
(or you can download html zip from [doc directory](http://technical.swissmicros.com/dmcp/doc/)).

The [source code of the `DM42PGM` program](https://github.com/swissmicros/DM42PGM)
is also quite informative about the capabilities of the DMCP.
