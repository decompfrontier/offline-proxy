# Offline Proxy
This repository contains the source code of the DLL proxy that is used by the Brave Frontier offline mod to allow
connection to a local server, as well as embedding the offline server itself.

## Compatibility
The proxy only works for Windows 8+ 32-bit

## Requirements
- Visual Studio 2022 or MinGW
- CMake 3.20 or later

## Building
1. Clone this repository
2. Setup an environment variable called "VCPKG_ROOT" that points to your vcpkg home directory
3. Create a build directory
4. Go to the build directory with your favioute shell
5. Run cmake `cmake --preset (selected preset) ../`

You will find libcurl.dll inside the bin/ directory of your build directory.

### Presets

| Name | Description |
| ---- | ----------- |
| deploy-vs | Configuration for deployment to the BF client using Visual Studio as a compiler |
| deploy-mingw | Configuration for deployment to the BF client using MinGW as a compiler |
| debug-vs | Configuration for testing the BF client with a local server using Visual Studio as a compiler |
| debug-mingw | Configuration for testing the BF client with a local server using MinGW as a compiler |

## License
GNU Affero General Public License 3.0 or greater.
