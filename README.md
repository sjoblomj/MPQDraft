# MPQDraft
MPQDraft allows in-memory patching of game data files, added features through plugins, and creation of self-executing mods.
This fork aims to update MPQDraft up to Win10 APIs and streamline the development process when used in combination with GPTP plugin development.

## Command Line Usage
```
Usage: MPQDraft.exe --target <exePath> --mpq <mpqFile> --plugin <qdpFile>

Example:
MPQDraft.exe --target "C:\Starcraft\StarCraft.exe" --mpq "C:\Mod\my_mod_1.mpq" --mpq "C:\Mod\my_mod_2.mpq" --plugin "C:Mod\my_plugin_1.qdp" --plugin "C:\Mod\my_plugin_2.qdp"
```

## Development
Required Visual Studio components:

* C++ MFC for v142 build tools
* C++ ATL for v141 build tools
* C++ MFC for v141 build tools

### Building in Visual Studio
- Open MPQDraft\MPQDraft.sln in Visual Studio 2019
- Build->Build Solution
- Open Debug\MPQDraftD.exe

### Building under Linux
Creating a workable Windows binary under Linux using MinGW-w64 is a goal that is not yet complete.

Todo:
- [ ] Case sensitive file paths
- [ ] Correct `_cdecl`
- [ ] Use std namespace for `min` and `max` and setup import
- [ ] Correct casts
- [ ] Remove Windows-specific dead code
- [ ] Conditionally compile Windows-specific code
- [ ] Set up a CMake build system
- [ ] Handle Windows-specific Structured Exception Handling (SEH)
- [ ] Remove MFC from CLI tool
- [ ] Replace MFC GUI with a cross-platform GUI framework

## Credits
[Quantam, creator of MPQDraft](http://qstuff.blogspot.com/2010/01/bibliography-programming.html).

MPQDraft is licensed under CDDL.
