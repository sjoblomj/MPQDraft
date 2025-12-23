# Risky commits

Significant refactoring and modernisation has been done in November-December 2025 by Johan Sjöblom. I have tried to test everything, but have had limited time to be thorough. If you have issues or experience bugs or crashes, the following are a list of commits that are most likely to have introduced issues.

- `985a8cdc97d7e41d35f89b29aa3522c73157030c`: Extracting SEMPQ creation code from gui
- `a85f19c8cec4dbb9ad1cbcc1a1d8bee73c1b3b77`: Changing SEMPQCreator interface to be platform-agnostic
- `5448c91bb5046740bed6558e5bf3c83a262f52d8`: Introducing new PluginManager
- `a73fa75e78dab3dbea99619ec7941751537db954`: Using the correct address
- `6597c820fc10d57805a450f7623d65d5c4e8c26b`: Making validation improvements
- `40c3ab7381367aec88242b7b06a17195d09764e8`: More validation improvements
- `4b3475bde4a59fb1a276581f38cfee35783829e6`: Creating GCC version of assembly
