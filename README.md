# GMS v83 Client Edit Collection

## Overview
A collection of GMS v83 Client Edits which can be loaded through the provided ijl15 proxy. The collection itself builds off of the fundamentals provided in the MapleClientCollection framework by MinimumDelta. The goal is that each DLL loaded provides clear and concise behavior. Opting in or out of functionality is handled through DLL deployment (or lack thereof).

## Usage

1. Clone and Build the repository to produce the proxy and client edit DLLs.
2. Backup the original ijl15.dll in the root MapleStory directory. ie. ijl15.dll.bak
3. Place the proxy ijl15.dll in the root MapleStory directory.
4. Create an "edits" folder in the root MapleStory directory to house your client edits.
5. Place (any) DLLs and corresponding INI configurations within the "edits" folder which you would like to apply to the client.

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

- I use CLion with a VisualStudio(2022 x86) Toolchain. Other IDEs are possible. 

## Credits
The proxy and client edits provided in this collection were all made possible through efforts by the following individuals, as well as my RE efforts. If you feel someone is improperly credited, please reach out.
- Hendi - [Localhost Workshop](https://forum.ragezone.com/threads/localhost-workshop.1202021/) - Foundation for minimum client edits to produce a localhost. 
- MinimumDelta - MapleClientCollection - A wonderful framework for enabling DLL based client edits.
- izarooni - [MapleEzorsia](https://github.com/izarooni/MapleEzorsia) - Foundation for HD client edits.
