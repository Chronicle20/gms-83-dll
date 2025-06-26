# MS Client Edit Collection

## Overview
A collection of MS Client Edits which can be loaded through the provided ijl15 proxy. The collection itself builds off of the fundamentals provided in the MapleClientCollection framework by MinimumDelta. The goal is that each DLL loaded provides clear and concise behavior. Opting in or out of functionality is handled through DLL deployment (or lack thereof).

![GitHub license](https://img.shields.io/badge/License-AGPL%203.0-blue)

## Usage

1. Clone and Build the repository to produce the proxy and client edit DLLs.
2. Backup the original ijl15.dll in the root MapleStory directory. ie. ijl15.dll.bak
3. Place the proxy ijl15.dll in the root MapleStory directory.
4. Create an "edits" folder in the root MapleStory directory to house your client edits.
5. Place (any) DLLs and corresponding INI configurations within the "edits" folder which you would like to apply to the client.

### Compatability

#### Windows 10

Using the artifacts of this project may require the installation of [Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170) on Windows 10, due to it not coming prepackaged with the operating system. This may present as a Themida error "Cannot find 'ijl15.dll', Please, re-install this application"

## Edits
### bypass
Exactly what you think it is.

### enable-minimize
Allow the minimize button to be used in the windowed application.

### no-ad-balloon
Removes the Ad window on client termination.

### no-beginner-party-block
Removes restriction on Beginners from joining / creating parties. There may be server-side checks that need to be removed / configured depending on source.

### no-enter-mts-map-restriction
Removes restriction on entering MTS on certain maps.

### no-patcher
Removes the display of the MapleStory patcher window on launch.

### redirect
Redirect IP the game uses for socket connections. Provided configuration produces a "localhost".

## Notes

### Build Environment

- I use CLion with a VisualStudio(2022 x86) Toolchain. Other IDEs are possible.

#### Recommended Minimum Viable Build Environment Setup (IDE)
1. Download and Install [Git](https://git-scm.com/) to clone the and manage the repository.
2. Download and Install [CLion](https://www.jetbrains.com/clion/) IDE
3. Download and Install [Build Tools for Visual Studio 2022](https://visualstudio.microsoft.com/downloads/). Ensure the following components are selected via Individual components
   - C++ Modules for v143 build tools (x86 & x64)
   - C++ ATL for latest v143 build tools (x86 & x64)
   - C++ MFC for latest v143 build tools (x86 & x64)
   - MSVC v143 - VS 2022 C++ x64/x86 build tools
   - C++ CMake tools for Windows
   - Windows 10 SDK
4. Open CLion and add project to CLion. 
5. Project Wizard will start, ensure the Toolchains->CMake include an entry for Visual Studio. It should prepopulate with the Visual Studio Build Tools which you previously installed. The following changes should be made.
   - Architecture - amd64_x86
5. Debug Profile Wizard will start, ensure the Visual Studio Toolchain is selected. The following CMake options are necessary to debug build the parameterized build.
   - BUILD_REGION = (GMS, JMS, etc)
   - BUILD_MAJOR_VERSION = (83, 87, 95, etc)
   - BUILD_MINOR_VERSION = (0, 1, etc)

#### Minimum Viable Build Environment Setup (CLI)
1. Download and Install [Git](https://git-scm.com/) to clone the and manage the repository.
2. Download and Install [Build Tools for Visual Studio 2022](https://visualstudio.microsoft.com/downloads/). Ensure the following components are selected via Individual components
    - C++ Modules for v143 build tools (x86 & x64)
    - C++ ATL for latest v143 build tools (x86 & x64)
    - C++ MFC for latest v143 build tools (x86 & x64)
    - MSVC v143 - VS 2022 C++ x64/x86 build tools
    - C++ CMake tools for Windows
    - Windows 10 SDK
3. Download and Install [CMake](https://cmake.org/download/). Ensure this is available in your PATH.
4. Download and Install [Ninja](https://github.com/ninja-build/ninja/releases) Ensure this is available in your PATH.
5. Open Developer PowerShell for VS 2022 - This may not be necessary, but will ensure VS build tools are available in PATH.
6. Use the following command to establish cmake build
    - cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM="ninja.exe" -DBUILD_REGION=[REGION] -DBUILD_MAJOR_VERSION=[MAJOR_VERSION] -DBUILD_MINOR_VERSION=[MINOR_VERSION] -G Ninja -S [PATH TO SOURCE] -B [PATH TO CMAKE BUILD FOLDER]
7. Use the following command to build
    - cmake --build [PATH TO CMAKE BUILD FOLDER]

### Adding a new version

1. Copy cmake of an existing version to v(major_version)_(minor_version).cmake located in the expected region. 
2. Clear all hex codes contained in the copied file. You will need to locate the functions in your version. Start with WinMain / CWVsApp related functions.
3. Adjust .github/workflows to include the new version in the config matrix.
4. Review header definitions of the following files. It is minimally important that these are accurate. CWvsApp, CLogin (CMapLoadable, CStage), CClientSocket (unlikely)
5. Review .cpp files with defined(REGION_GMS) blocks. The version thresholds may or may not be completely accurate, and are only based off of existing reviewed versions.

## Credits
The proxy and client edits provided in this collection were all made possible through efforts by the following individuals, as well as my RE efforts. If you feel someone is improperly credited, please reach out.
- Hendi - [Localhost Workshop](https://forum.ragezone.com/threads/localhost-workshop.1202021/) - Foundation for minimum client edits to produce a localhost. 
- MinimumDelta - MapleClientCollection - A wonderful framework for enabling DLL based client edits.
- izarooni - [MapleEzorsia](https://github.com/izarooni/MapleEzorsia) - Foundation for HD client edits.

## License
The code in this repository is under either the AGPL 3.0 license, or a license compatible with the Apache 2 license. See the [License Page](https://www.gnu.org/licenses/agpl-3.0-standalone.html) for more information.
