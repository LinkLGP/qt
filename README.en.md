# Contribution Instructions

The Qt source tree contains multiple submodules. Following the OpenHarmony community workflow, contributions to Qt for OpenHarmony are managed by providing code patches and independent adaptation modules.

## Generating Code Patches

Use `git diff` to create patch files and place them under the appropriate `patch/` directory.

Convention: name each patch file after the submodule it targets, e.g.:

```bash
git diff v5.15.17 > qtbase.patch
```

After generating the patch, follow the OpenHarmony contribution guidelines when submitting it to the target repository/branch.

# Repository Layout

```
OpenHarmony - Qt
│
└───patch - Source code patches
|____v5.15.12
│   	│ qtbase - Patches for the qtbase submodule
│   	│ qtsensors - Patches for the qtsensors submodule
│   	│ qtmultimedia - Patches for the qtmultimedia submodule
│   	│ qtdeclarative - Patches for the qtdeclarative submodule
│   	│ qtconnectivity - Patches for the qtconnectivity submodule
│   	│ qtquickcontrols - Patches for the qtquickcontrols submodule
|____v5.15.17
│   	│ qtbase - Patches for the qtbase submodule
│   	│ qtsensors - Patches for the qtsensors submodule
│   	│ qtmultimedia - Patches for the qtmultimedia submodule
│   	│ qtdeclarative - Patches for the qtdeclarative submodule
│   	│ qtconnectivity - Patches for the qtconnectivity submodule
│   	│ qtquickcontrols - Patches for the qtquickcontrols submodule
└───LICENSE - Apache License
└───LICENSE.FDL - GNU Free Documentation License
└───LICENSE.GPLv2 - GNU GENERAL PUBLIC LICENSE Version 2
└───LICENSE.GPLv3 - GNU GENERAL PUBLIC LICENSE Version 3
└───LICENSE.LGPLv3 - GNU LESSER GENERAL PUBLIC LICENSE Version 3
└───LICENSE.LGPLv21 - GNU LESSER GENERAL PUBLIC LICENSE Version 2.1
└───LICENSE.QT-LICENSE-AGREEMENT - QT LICENSE AGREEMENT
└───OAT.xml - OAT configuration file, automated open source review tool for the OpenHarmony community
└───README.md - Chinese version of the README
└───README.en.md - English version of the README
```

# Scripts

This repository contains a set of scripts for automating the build of Qt for OpenHarmony located in the `script/` directory. The command-line entry point is `script/build-qt-ohos.py`; core logic lives in the `script/build_qt/` package.

Main capabilities provided by the scripts:

- Clone and initialize the Qt source tree and the OHOS patch repository (`QtRepo`).
- Read `configure.json` / `configure.json.user` for configuration and prepare the build environment (including optional automatic download of missing dependencies).
- Support building both Qt5 and Qt6: configure, host build (for Qt6), cross build, install and packaging.
- Download OHOS SDK components via a remote SDK list API (`OhosSdkDownloader`).
- Provide robust download/extract/pack utilities and a `ziptools/` helper that preserves symlinks, permissions and supports long Windows paths.

Key modules in `script/build_qt/`:

- `config.py`              - Loads configuration, handles user prompts, environment checks and assembles Qt configure/CMake options.
- `qt_repo.py`            - Manages Git clone, submodules, patch application, reset and clean.
- `qt5_build.py`         - Implements Qt5 configure/make/install/pack/clean flow.
- `qt6_build.py`         - Implements Qt6 host build + cross-compile flow (CMake based); applies patches after host build where appropriate.
- `ohos_sdk_downloader.py` - High-level helper to query available SDK versions and download components.
- `utils.py`             - Download, checksum, extract and archive helpers.
- `ziptools/`            - Third-party utilities used for packaging.

## Quick start (PowerShell)

```powershell
cd script
python -m pip install -r requirements.txt

# initialize repositories and apply patches (Qt5 applies patches during init; Qt6 applies patches during build)
python build-qt-ohos.py --init

# check and prepare development environment (may download missing components)
python build-qt-ohos.py --env_check

# perform configure, build, install and pack (use --exe_stage to run specific phases)
python build-qt-ohos.py --exe_stage all --with_pack
```

See  [Instructions for Using the Compilation Script](https://gitcode.com/qtforohos/UserManual/blob/main/reference/%E7%BC%96%E8%AF%91%E8%84%9A%E6%9C%AC%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8E.md) for full usage details and configuration options.

# Using Qt For OpenHarmony

Refer to the project wiki for broader guidance and community information:

https://gitcode.com/openharmony-sig/qt/wiki/Home.md
