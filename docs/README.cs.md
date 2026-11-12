# RAD-Wolf – český přehled

Zdrojový port Wolfensteinu 3D pro C64 s RAD a Raspberry Pi Zero 2 W. Hra běží
na Pi, obraz zobrazuje VIC-II a zvuk přehrává SID. Podporovaný zvuk vyžaduje
reálný SID; Ultimate/UltiSID audio je experimentální a nemusí znít správně.

Herní data nejsou součástí repozitáře. Použij vlastní zakoupenou kopii,
například z [GOG](https://www.gog.com/en/game/wolfenstein_3d). MD5 a SHA-256
testovaných souborů jsou v GAME_DATA.md a game-data-hashes.json.

Začni hlavním README, potom BUILD.md a SD_CARD.md. Publikovaná verze nemá
úvodní RAD animaci ani závislost na Doom datech, soundfontu či grafice intra.
Mono používá SID1; stereo SID1 vlevo a SID2 vpravo. Nastavení je v AUDIO.CFG.

Složka obsahuje nové adaptéry, celé upravené soubory původních projektů,
odkazy a přesné revize upstreamů, licence a nástroje pro sestavení. Neobsahuje
herní data, uložené hry, binární firmware ani kompilátory. Postup publikování
je v PUBLISHING.md. Ověření hardwaru a omezení jsou v STATUS.md.
