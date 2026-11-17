# Rage Chorus

A JUCE 8 chorus audio effect with a dark red-accented interface. The CMake project is configured for a universal macOS build (Apple Silicon and Intel), targeting macOS 11 or later, and produces VST3, Audio Unit, and Standalone formats.

## Requirements

- macOS 11 or later
- Xcode Command Line Tools (including Apple Clang)
- CMake 3.22 or later
- An internet connection on the first configure/build; CMake fetches JUCE 8.0.4

## Build

From the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The plugin formats are generated under `build/RageChorus_artefacts/Release/`. With `COPY_PLUGIN_AFTER_BUILD` enabled, JUCE also copies built plugins to the standard macOS plugin locations where supported.

To build only the VST3 target:

```sh
cmake --build build --config Release --target RageChorus_VST3
```

To build only the Audio Unit target:

```sh
cmake --build build --config Release --target RageChorus_AU
```

## Controls

- **Rate** — modulation speed
- **Depth** — modulation depth
- **Delay** — chorus centre delay
- **Fdbk** — feedback
- **Mix** — dry/wet blend

The project fetches JUCE at configure time; JUCE is not vendored in this repository.
