# v0.3.4-rc1 validation

Release preparation on 2026-11-15 ran:

- make test test-opl test-audio-startup test-flash-engine: passed.
- Actual Wolf save/load: new saves, legacy saves and corrupt optional history.
- Actual map adapter: keys, locks, pickup/save/load, elevator, no player halo.
- Eight-direction marker, adjacent wall/path preservation and C64 color tags.
- Six Windows-only tests in the portable suite are skipped under Linux;
  the final extracted SD Windows launcher is checked separately in the
  delivered release VALIDATION.json. Prior Windows-kit validation is historical.

The exact maintainer-accepted kernel is preserved, not rebuilt for packaging:
ce00a903b8aa9698e123c2df496aa033b8c76da05de8d152a664857a91994222.
Maintainer acceptance on 2026-11-14 covers the deployed candidate. Detailed
prior traversal/save-load and legacy-save reports are in STATUS.md.
A 30-minute stability run, level-transition reset and exhaustive hardware
matrix are not claimed. The new private captures are not published.

Publication audit, archive checksums, dependency/override matching, unchanged
boot configuration and final Windows-kit smoke results are recorded in the
release VALIDATION.md/VALIDATION.json supplied beside the archives.
