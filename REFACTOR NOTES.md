# REFACTOR NOTES

---

- During Math refactor, I found out Hexen: Edge of Chaos has two new, but commented out functions **Vec45CtrClock( idVec3 vec )* and *Vec45Cloc( idVec3 vec )*. They are not used, but they seem to be intended to produce a vector perpendicular to another vector, rotating it clockwise or counterclockwise on the XY plane. There's also a mistake, their code seems to rotate by 90 degrees not 45. They should belong in Vectors refactor.
- During Math refactor, I found BielBdeLuna added new math utilities in his dhewm3 fork - branch *new_movement*. That fork adds skimming physics to the player. I could not compile it due to me using a newer CMake version but I don't think I need. The math utils contain one generic math utility MinNormalizeMax which has been ported to Dominion, one vector/cone/geometry one and idAI Ballistics extension (moved and changed from idAI to Biel's new math utils.) Worth revisiting during Vector / Geometry / BV / idAI refactor
- I cannot remember if idTech5+ engines have specialized RadixSort, but for reference I found one implementation (not based or trying to reimplement Tech5) in *niTech4*.
- NETLizard forks has code useful to peek at during other idLib refactor. A search for "math." gave me vector code, bounds code, math code, quat, rotation etc.... 
  Note we already reviewd generic Math functions and ported what was necessary!
- Potentially useful, RaynorPat's ioDOOM fork has a new idLib/Format files.
