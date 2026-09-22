# Offline Proxy

This repository contains the source code for a DLL proxy used by the Brave Frontier offline mod. The proxy enables connection to a local server and embeds the offline server itself.

## Overview
The offline proxy is designed to facilitate offline gameplay for Brave Frontier by routing connections to a local server instead of the online service. This project is tailored for Windows users and includes specific compatibility and build requirements.

## Compatibility
- **Operating System**: Windows 8 or later (32-bit only)

## Requirements
To build the offline proxy, you’ll need the following tools:
- **Compiler**: Visual Studio 2022 or MinGW
- **Build System**: CMake 3.20 or later
- **Dependencies**: Ensure `vcpkg` is installed and configured (see Building section for details), with `libcurl` available.

## Building the Offline Proxy
Follow these steps to clone, configure, and build the offline proxy. This will generate `libcurl.dll`, which is necessary for the proxy to function.

1. **Clone the Repository**  
   Use Git to clone the repository:
   ```bash
   git clone https://github.com/decompfrontier/offline-proxy
   ```

2. **Set Up the Build Directory**  
   Create and navigate to a build directory within the cloned repository:
   ```bash
   cd offline-proxy
   mkdir build
   cd build
   ```

3. **Configure the Build with CMake**  
   Use CMake with a preset to configure the project. For testing with Visual Studio 2022, run:
   ```bash
   cmake --preset debug-vs ../
   ```
   - This preset is ideal for testing the Brave Frontier (BF) client with a local server using Visual Studio.

4. **Build the Project**  
   Compile the project to generate `libcurl.dll`:
   ```bash
   cmake --build . --config Debug
   ```
   - After a successful build, you’ll find `libcurl.dll` in the `bin/` directory of your `build` folder.

### Build Presets
CMake presets allow you to configure the build for different purposes and compilers. Choose the preset that matches your needs:

| Preset Name     | Description                                                                      |
|-----------------|----------------------------------------------------------------------------------|
| `deploy-vs`     | Configuration for deploying to the BF client using Visual Studio as the compiler |
| `deploy-mingw`  | Configuration for deploying to the BF client using MinGW as the compiler         |
| `debug-vs`      | Configuration for testing the BF client with a local server using Visual Studio  |
| `debug-mingw`   | Configuration for testing the BF client with a local server using MinGW          |

- **Note**: The `debug-vs` preset (used in the example above) is recommended for initial testing and development. Adjust the preset and configuration (`Debug` or `RelWithDebInfo`) based on your deployment or testing needs.

## Extra cmake variables
You can modify some of the Proxy behaviour by defining the CMake variables defined here:
| Name                      | Description                                                                              | Default value   |
| ------------------------- | ---------------------------------------------------------------------------------------- | --------------- |
| OFFLINEMOD_IP             | Sets the server IP which the proxy would connect                                         | 127.0.0.1       |
| OFFLINEMOD_PORT           | Sets the server port which the proxy would connect                                       | 9960            |
| OFFLINEMOD_DISABLE_HTTPS  | Enables the patches that allows the proxy to connect to an insecure (HTTP) connection    | 1 (true)        |
| OFFLINEMOD_FPS_CAP        | Compile-time fallback render-loop cap when the server probe fails. 0 disables.           | 60              |
| OFFLINEMOD_FPS_CAP_DIAG   | Compile in diagnostic logging for the FPS cap (%TEMP%\offlinemod_fps.log + OutputDebugString) | 0 (off)    |

### FPS cap server probe
The offline-proxy fetches the desired cap at startup via
`GET http://127.0.0.1:9960/offline_mod/fps_cap` (the existing InternetConnect
hook redirects to the configured server). The server returns a bare integer
in the body; the client applies it on the first valid `Render` call. If the
probe fails (server down, timeout, unreachable) the compile-time
`OFFLINEMOD_FPS_CAP` value remains in effect — the cap is never *un-capped*
by a missing server, just stuck at the build default.

To change the cap without rebuilding the DLL, edit `fps_cap` in the server's
`deploy/config.json` (under `plugins[0].config.server`) and relaunch the game.
The probe runs once per game launch; mid-game changes require a restart.

## Next Steps: Building the Game Client
Once you’ve successfully built the offline proxy and obtained `libcurl.dll`, refer to the Brave Frontier offline mod documentation for instructions on integrating the proxy with the game client and setting up the local server. This typically involves:
- Copying `libcurl.dll` to the appropriate directory in your Brave Frontier installation.
- Configuring the game to connect to your local server (details vary by mod version and setup).

For specific guidance, consult the Brave Frontier offline mod’s official repository or community forums.

## License
This project is licensed under the GNU Affero General Public License (AGPL) 3.0 or later. For more details, see the `LICENSE` file in the repository.
