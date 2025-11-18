# MPQDraft
MPQDraft allows in-memory patching of game data files, added features through plugins, and creation of self-executing mods.

This fork aims to modernise MPQDraft for use on Win10 APIs and streamline the development process.

## Command Line Usage
```
Usage: MPQDraft.exe --target <exePath> --mpq <mpqFile> --plugin <qdpFile>

Example:
MPQDraft.exe --target "C:\Starcraft\StarCraft.exe" --mpq "C:\Mod\my_mod_1.mpq" --mpq "C:\Mod\my_mod_2.mpq" --plugin "C:\Mod\my_plugin_1.qdp" --plugin "C:\Mod\my_plugin_2.qdp"
```

### CLI Plugin Configuration
MPQDraft plugins can optionally have configuration dialogs. The CLI contains no support for configuring plugins, but if one first runs MPQDraft in GUI mode, the plugins can be configured there, and those changes should persist when running in CLI mode.


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
- [x] GitHub Actions build-pipeline
- [x] Case-sensitive file paths
- [x] Remove obsolete `#if _MSC_VER > 1000  #pragma once  #endif // _MSC_VER > 1000`
- [x] Refactor CLI into its own directory
- [x] Refactor GUI into its own directory
- [x] Remove MFC from CLI tool
- [x] Add `--help`, `-h`, `--version`, `-v` flags to CLI
- [x] Add plugin readiness-check to CLI
- [ ] Replace MFC GUI with a cross-platform GUI framework
- [ ] Correct `_cdecl`
- [ ] Use std namespace for `min` and `max` and setup import
- [ ] Correct casts
- [ ] Remove Windows-specific dead code
- [ ] Conditionally compile Windows-specific code
- [ ] Set up a CMake build system
- [ ] Handle Windows-specific Structured Exception Handling (SEH)
- [ ] Handle inline Assembly code

## Credits
[Quantam, creator of MPQDraft](http://qstuff.blogspot.com/2010/01/bibliography-programming.html).

MPQDraft is licensed under CDDL.
