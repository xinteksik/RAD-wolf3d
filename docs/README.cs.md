# RAD-Wolf – český přehled

Zdrojový port Wolfensteinu 3D pro C64 s RAD a Raspberry Pi Zero 2 W. Hra běží
na Pi, obraz zobrazuje VIC-II a zvuk přehrává SID. Podporovaný zvuk vyžaduje
reálný SID; Ultimate/UltiSID audio je experimentální a nemusí znít správně.

Herní data nejsou součástí repozitáře. Použij vlastní zakoupenou kopii,
například z [GOG](https://www.gog.com/en/game/wolfenstein_3d). MD5 a SHA-256
testovaných souborů jsou v GAME_DATA.md a game-data-hashes.json.

Začni hlavním README, potom BUILD.md a SD_CARD.md. Publikovaná verze nemá
úvodní RAD animaci ani závislost na Doom datech, soundfontu či grafice intra.
Výchozí nastavení je mono s automatickou detekcí SID1. Stereo používá SID1
vlevo a SID2 vpravo; nastavuje se v AUDIO.CFG. Projekt byl podle potvrzení
správce otestován na reálném Commodore 64C i Ultimate64 Elite2.

Volitelně lze použít MIDI hudbu se souborem `RADWOLF/soundfont.sf2` a převody
skladeb ve složce `RADWOLF/MIDI/`. Bez SoundFontu hraje původní OPL hudba.
MIDI se zatím vytváří jednorázově nástrojem na PC; samotná hra je při startu
z WL6 negeneruje. Výměna SF2 nevyžaduje nový převod skladeb.
Testovací balík obsahuje anglickou dávku `Prepare-MIDI.bat` a EXE ve složce
`RADWOLF`. Po nahrání WL6 ji spusť ve Windows přímo z SD; vytvoří složku `MIDI`.
Instalace Pythonu ani kompilátoru není nutná. Postup je ve
[WINDOWS-MIDI.md](WINDOWS-MIDI.md).
Příklady bank jsou ve složce
[RJModules/soundfonts od Miserlou / Rich Jonese](https://github.com/Miserlou/RJModules/tree/master/soundfonts);
repozitář uvádí licenci MIT v
[LICENSE.txt](https://github.com/Miserlou/RJModules/blob/master/LICENSE.txt).
Banky zde nepřibalujeme. Ověř si pokrytí nástrojů a bicích General MIDI;
jednotlivé specializované banky nemusí obsahovat celý orchestr. Vybraný SF2
ulož pod názvem `RADWOLF/soundfont.sf2` a restartuj hru. Podrobnosti a stereo
jsou v [MIDI-MENU.md](MIDI-MENU.md).

Složka obsahuje nové adaptéry, celé upravené soubory původních projektů,
odkazy a přesné revize upstreamů, licence a nástroje pro sestavení. Neobsahuje
herní data, uložené hry, binární firmware ani kompilátory. Postup publikování
je v PUBLISHING.md. Ověření hardwaru a omezení jsou v STATUS.md.
