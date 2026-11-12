# License boundaries and publication scope

This source package retains component licenses; it does not claim ownership
of upstream code or of Wolfenstein's game data. RAD-Wolf's original glue and
tools are provided under GPL-3.0-or-later (root LICENSE). Original source
headers, changed-file notices, the exact bases and complete modified files
are included. Names and trademarks remain their owners' property.

| Component | Basis retained here |
|---|---|
| RAD/RAD-Doom, Circle and circle-stdlib | GPL v3 notices; source file grants apply |
| Wolf4SDL/id engine | Upstream README explicitly offers the GPL option; its GPL v2 text and original alternative id notice are preserved |
| DOSBox DBOPL | GPL-2.0-or-later notices in upstream source |
| Measured UltiSID table | MIT, copyright 2026 Kelly Fox; full notice in third_party |
| Newlib | Multiple per-file licenses; COPYING.NEWLIB reproduced for reference |
| MAME OPL | Implementation is not linked; alternative license retained as an upstream notice, not selected for this build |

Wolf's upstream `README.TXT` explains the alternative licensing choice; its
licensing section is reproduced in `licenses/Wolf4SDL-licensing-choice.txt`. This
build uses that GPL route and DOSBox DBOPL, not the restrictive id alternative
or the MAME OPL implementation. The C interface is declared locally in
`src/radwolf_opl_api.h`; neither MAME's implementation nor its header is used
by the firmware build. Synthesis is supplied by `src/radwolf_opl.cpp` and DBOPL.
Unchanged upstream files are fetched from exact public revisions rather than
silently copied into this publication folder. See NOTICE.md and the lock file.

Commercial WL6 data, DOS executables, GOG installers, saves,
soundfonts, demo animation/music and boot firmware are excluded. Buying a
game does not grant a right to redistribute its data. Checksums identify
locally supplied files only; a matching hash is neither a license grant nor
proof of purchase. Users obtain data under their own purchase/license terms.

The two maintainer-supplied screenshots in `docs/images/` illustrate the port.
They show copyrighted game artwork and are excluded from the source-code
license grant; see their image notices. Their inclusion does not grant a
license to reuse or extract the underlying game graphics.

This delivery contains no kernel binaries. Before offering a compiled release,
provide the complete corresponding source and build/configuration material
required by the licenses of all linked components, including dependency
notices and any runtime exceptions. A list of upstream URLs alone should not
be presented as a substitute for the complete source obligation of a binary
release. See [GNU GPL distribution guidance](https://www.gnu.org/licenses/gpl-faq.html#UnchangedJustBinary)
and [GPLv3 section 6](https://www.gnu.org/licenses/gpl-3.0.html#section6).

This is a documented source/licensing inventory, not a guarantee about every
possible later redistribution or jurisdiction. Changes of dependencies,
bundled assets or binary distribution require another review of their grants.
