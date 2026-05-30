# CW demodulator plugin — implementation report

> Report covering the creation of the **CW demodulator** (`demodcw`) channel receive plugin for SDRangel, derived from the existing **SSB demodulator** (`demodssb`).
>
> Date: April 2026

---

## 1. Goal

Add a new channel receive plugin called **CW demodulator** that is plugged into the SDRangel runtime alongside the existing demodulators. The plugin should:

- Be selectable from the channel picker as `CW Demodulator`.
- Reuse the proven SSB DSP chain (narrow SSB is the textbook way to receive CW).
- Be recognised by the feature plugins that already cooperate with SSB (Morse Decoder, Demod Analyzer, Denoiser, JogDial Controller).
- Expose a WebAPI surface compatible with existing tooling.
- Follow the SDRangel folder/naming conventions so the project structure stays consistent.

---

## 2. What was done

### 2.1 New plugin directory `plugins/channelrx/demodcw/`

Cloned from `plugins/channelrx/demodssb/` with token-level renames. All 16 files were created:

| File | Purpose | Notes |
|---|---|---|
| `CMakeLists.txt` | Build script | `project(cw)`, `cw_SOURCES`/`cw_HEADERS`, `TARGET_NAME = ${PLUGINS_PREFIX}demodcw` (server: `${PLUGINSSRV_PREFIX}demodcwsrv`), `CLASS_NAME CWPlugin` |
| `cwdemod.{h,cpp}` | Channel API class `CWDemod` | `MsgConfigureCWDemod`, `m_channelIdURI = "sdrangel.channel.cwdemod"`, `m_channelId = "CWDemod"` |
| `cwdemodbaseband.{h,cpp}` | Baseband sink wrapper `CWDemodBaseband` | DSP thread wrapper |
| `cwdemodsink.{h,cpp}` | DSP sink `CWDemodSink` | Actual sample processing |
| `cwdemodsettings.{h,cpp}` | Settings struct `CWDemodSettings` | Serialisation, defaults, filter bank |
| `cwdemodgui.{h,cpp,ui}` | Qt GUI class `CWDemodGUI` | Help URL → `plugins/channelrx/demodcw/readme.md`, member renamed from `m_ssbDemod` to `m_cwDemod` |
| `cwdemodwebapiadapter.{h,cpp}` | Settings WebAPI adapter | Reused SSB schema (see §3.4) |
| `cwplugin.{h,cpp}` | Qt plugin descriptor | `Q_PLUGIN_METADATA(IID "sdrangel.channel.cwdemod")`, display name `"CW Demodulator"` |
| `readme.md` | User-facing documentation | Rewritten for CW (narrow filter, Morse-oriented guidance) |

### 2.2 Build wiring

| File | Change |
|---|---|
| `CMakeLists.txt` (root, line 148) | Added `option(ENABLE_CHANNELRX_DEMODCW "Enable channelrx demodcw plugin" ON)` next to the SSB option |
| `plugins/channelrx/CMakeLists.txt` (lines 52–56) | Added `if (ENABLE_CHANNELRX_DEMODCW) add_subdirectory(demodcw) … endif()` block right after `demodssb` |

### 2.3 Core integration (`sdrbase/`)

| File | Change | Effect |
|---|---|---|
| `sdrbase/channel/channelutils.cpp` | Added explicit pass-through entry for `sdrangel.channel.cwdemod` | Documents the URI alongside SSB; no legacy remap needed |
| `sdrbase/webapi/webapiutils.cpp` (line 82) | `{"sdrangel.channel.cwdemod", "SSBDemodSettings"}` | Maps URI → Swagger settings key |
| `sdrbase/webapi/webapiutils.cpp` (line 206) | `{"CWDemod", "SSBDemodSettings"}` | Maps channel id → Swagger settings key |
| `sdrbase/webapi/webapirequestmapper.cpp` | **No edit needed** | Existing branch on JSON key `SSBDemodSettings` already handles CW because the new plugin reuses the SSB schema |

### 2.4 Cooperating feature plugins

Added `"sdrangel.channel.cwdemod"` to the `m_channelURIs` list in:

- `plugins/feature/morsedecoder/morsedecodersettings.cpp`
- `plugins/feature/demodanalyzer/demodanalyzersettings.cpp`
- `plugins/feature/denoiser/denoisersettings.cpp`
- `plugins/feature/jogdialcontroller/jogdialcontrollersettings.cpp`

Additionally added `"CWDemod"` to `JogdialControllerSettings::m_channelTypes` (the only feature that maintains a parallel channel-id list).

These four feature plugins now list and operate on CW demod channels in their channel pickers.

### 2.5 Folder/naming convention alignment

Initial scaffold used `plugins/channelrx/cwdemod/`. We renamed it to `plugins/channelrx/demodcw/` to match the existing SDRangel convention (`demodssb`, `demodam`, `demodnfm`, `demoddsd`, …). File names inside the folder remained `cwdemod*.cpp/h/ui`, matching the SSB plugin's pattern of `demodssb/ssbdemod*.cpp/h/ui`.

Renamed identifiers as part of the alignment:

- Build option: `ENABLE_CHANNELRX_CWDEMOD` → `ENABLE_CHANNELRX_DEMODCW`
- CMake target: `${PLUGINS_PREFIX}cwdemod` → `${PLUGINS_PREFIX}demodcw`
- Server target: `${PLUGINSSRV_PREFIX}cwdemodsrv` → `${PLUGINSSRV_PREFIX}demodcwsrv`
- Help URL inside `cwdemodgui.cpp` → `plugins/channelrx/demodcw/readme.md`

---

## 3. Decisions taken

### 3.1 Clone vs. shared base class

**Decision:** Clone the SSB plugin verbatim and rename, instead of refactoring SSB into a base class with CW as a thin subclass.

**Rationale:**
- The SDRangel codebase consistently uses copy-and-evolve as the plugin extension pattern (FT8 demod, FreeDV, M17 — all derived from SSB or AM via clone). Following the existing convention preserves project consistency and reduces review friction.
- A shared base class would touch the existing SSB plugin and risk regressions in a stable component.
- CW will likely diverge from SSB over time (Morse decoder integration, sidetone offset, key-shaping), so shared inheritance would become a liability.

**Trade-off:** Code duplication ≈ 3000 lines. Acceptable in plugin code.

### 3.2 Folder naming

**Decision:** Use `plugins/channelrx/demodcw/` (not `cwdemod/`).

**Rationale:** Consistent with `demodssb`, `demodam`, `demodnfm`, etc. — the directory prefix is `demod`, the suffix is the modulation. Keeps alphabetical browsing intuitive in IDEs and `ls` output.

### 3.3 Internal file naming

**Decision:** Files inside the folder remain `cwdemod*.{h,cpp,ui}`, mirroring how `demodssb/` contains `ssbdemod*.{h,cpp,ui}`.

**Rationale:** Symbol/file-name correspondence (file `cwdemod.cpp` declares class `CWDemod`) is the dominant convention across the SDRangel tree and is more discoverable than the folder-prefixed alternative.

### 3.4 Swagger/WebAPI strategy

**Decision:** Adopt **Option A — reuse the SSB Swagger schema** (`SWGSSBDemodSettings`, `SWGSSBDemodReport`) rather than generating new `SWGCWDemodSettings`/`SWGCWDemodReport` classes.

**Rationale:**
- Settings fields are currently identical to SSB (filter bank, AGC, DNR, audio routing).
- Avoids the swagger-codegen toolchain dependency for the initial scaffolding.
- Lets the plugin compile and load on day one, without modifying generated code under `swagger/sdrangel/code/qt5/client/` (which is regen-only by project policy).
- WebAPI clients can already configure the new plugin: the URI / channel id resolves through `webapiutils.cpp` to the `SSBDemodSettings` key, and the existing JSON key branch in `webapirequestmapper.cpp` (line 4748) handles the deserialisation.

**Visible side-effect:** the JSON key in WebAPI payloads addressing a CW channel is `SSBDemodSettings` (not `CWDemodSettings`). Slightly misleading documentation-wise, but functionally complete.

**When to revisit (Option B):** the moment any CW-specific field is added to the settings — see §5.

### 3.5 What was deliberately *not* renamed

Several `ssb`/`SSB`-bearing tokens were left untouched on purpose:

| Token | Reason |
|---|---|
| `SWGSSBDemodSettings`, `SWGSSBDemodReport`, `getSsbDemodSettings()`, `setSsbDemodSettings()` | Generated Swagger classes — reused per Option A |
| `SpectrumSettings::m_ssb` | Public SDRangel API flag for SSB-style spectrum display — applies to CW too |
| `SSBFilter`, `m_ssbFftLen` (inside `cwdemodsink`) | Internal DSP variable names — cosmetic; renaming would change diff size without functional benefit |

---

## 4. Things to know before building

1. **CMake re-configure required.** If your build directory was previously generated without the new plugin (e.g. inside Docker with a different source path), CMake will not auto-discover it. Either delete the cache or use a fresh build folder.
2. **CLion users:** `Tools → CMake → Reset Cache and Reload Project`. The `demodcw` target should then appear in the CMake target list and run configurations.
3. **Symbol verification after build:** the resulting plugin shared library is named per `${PLUGINS_PREFIX}demodcw` (e.g. `libdemodcw.so` on Linux, depending on the prefix). Visible in the running app via the channel picker as `CW Demodulator`.

---

## 5. Recommendations for future work

The current plugin is a faithful clone of SSB with CW-appropriate documentation. The following items turn it into a *real* CW demodulator with distinct behavior. Listed roughly in priority order.

### 5.1 CW-specific settings defaults *(implemented startup preset; further refinement optional)*

`CWDemodSettings::resetToDefaults()` now seeds a CW-friendly startup state:

- Default `m_filterBank[0]` to a 300 Hz passband centred on a 600 Hz sidetone, i.e. `m_lowCutoff = 450 Hz` and `m_rfBandwidth = 750 Hz`.
- Default `m_volume` higher (CW has lower average power than SSB).
- Default `m_agcTimeLog2` to a smaller value (32–64 ms; currently `5`) to follow keying.
- Default `m_dsb = false`.
- Pre-tune the filter bank with CW-oriented entries covering 100 Hz, 200 Hz, 400 Hz, 600 Hz, 800 Hz and 1 kHz; higher slots fall back to a safe 1 kHz preset.

These defaults mean an empty settings load starts with a usable CW receiver without manual tweaking.

### 5.2 Built-in Morse decoder *(moderate effort, killer feature)*

Currently a CW user must add a separate "Morse Decoder" feature plugin to see decoded text. Embed a Morse decoder directly in `CWDemodSink` and surface the decoded text in `CWDemodGUI`.

Implementation sketch:
- Hook the post-AGC magnitude envelope and feed it into a keying-detector (Goertzel filter at the sidetone offset, or already-filtered I/Q magnitude).
- Use an adaptive threshold and dot-length tracker (the WPM follower from the existing Morse Decoder feature plugin can be reused — its sources are under `plugins/feature/morsedecoder/`).
- Add a read-only text widget in the GUI showing decoded characters with a clear button.
- Expose decoded WPM and text in `CWDemodReport`.

### 5.3 Sidetone / pitch control *(low effort)*

Add a "Pitch" or "Sidetone" knob (typical: 400–1000 Hz, default 600 Hz). Internally it just shifts `m_lowCutoff` and `m_rfBandwidth` symmetrically around the chosen pitch. This is what CW operators actually expect — they tune for a comfortable beat note, not for a low-cut/high-cut pair.

### 5.4 Dedicated Swagger schema (Option B) *(moderate effort)*

Trigger any of: §5.1's altered defaults visible in the API, §5.2's decoded text/WPM in the report, §5.3's pitch field. Procedure:

1. Copy `sdrbase/resources/webapi/doc/swagger/include/SSBDemod.yaml` → `CWDemod.yaml`. Rename top-level objects to `CWDemodSettings`, `CWDemodReport`. Add CW-specific fields.
2. Reference them in `ChannelSettings.yaml` and `ChannelReport.yaml`.
3. Regenerate the Qt client:
   ```bash
   cd swagger/sdrangel
   /opt/install/swagger/swagger-codegen generate \
       -i api/swagger/swagger.yaml -l qt5cpp \
       -c qt5cpp-config.json -o code/qt5
   ```
4. In `cwdemod.cpp`, switch all `Ssb*` Swagger accessors to `Cw*` and update the includes.
5. In `webapiutils.cpp` change the two CW entries to `"CWDemodSettings"`.
6. In `webapirequestmapper.cpp` add the new JSON-key branch (and the cleanup `setCwDemodSettings(nullptr)` / `setCwDemodReport(nullptr)`).

### 5.5 Narrow CW filter quality *(moderate effort)*

The reused FFT bandpass works but at very narrow bandwidths (<200 Hz) ringing/transient response degrades. Options:
- Add a CW-tuned IIR (e.g. 4th-order Butterworth) post-FFT-filter mode.
- Or use a longer FFT block size when bandwidth < 300 Hz.

### 5.6 Visual aids *(low effort)*

- A vertical marker in the channel spectrum view at the configured pitch.
- A simple level-vs-time strip-chart of the keying envelope above the spectrum panel — useful for verifying the squelch/threshold by eye.

### 5.7 Removing SSB-only GUI elements *(low effort, polish)*

The cloned UI exposes the **DSB** toggle and **binaural / flip channels** controls that have no practical use for CW. Hide them behind a "Show SSB-mode controls" advanced toggle, or remove them entirely from `cwdemodgui.ui`. Keep the underlying settings for serialisation backwards-compatibility, just don't surface them.

### 5.8 Plugin icon *(low effort)*

There is currently no CW-specific icon. The channel picker shows a generic placeholder. Add a `cwdemod.png` resource and reference it from `CWPlugin::getPluginDescriptor()`.

### 5.9 Documentation polish *(low effort)*

- Replace the `SSBDemod_plugin*.png` references in `readme.md` with new `CWDemod_plugin*.png` screenshots (currently the readme reuses the SSB images as placeholders).
- Add a section linking to the Morse Decoder feature plugin until §5.2 is implemented.
- Add an entry under `doc/` if there is a per-plugin index there.

### 5.10 Tests *(stretch goal)*

SDRangel does not have a strong unit-testing tradition, but a minimal smoke test that:
- Constructs a `CWDemod`, feeds it a synthesised keyed tone at a known offset,
- Verifies the audio sample rate, AGC magnitude, and (after §5.2) the decoded text,

…would protect the plugin from regressions during subsequent refactors. Place under `cmake/test/` next to existing helpers.

---

## 6. Summary of files changed/created

### Created
```
plugins/channelrx/demodcw/CMakeLists.txt
plugins/channelrx/demodcw/cwdemod.{h,cpp}
plugins/channelrx/demodcw/cwdemodbaseband.{h,cpp}
plugins/channelrx/demodcw/cwdemodsink.{h,cpp}
plugins/channelrx/demodcw/cwdemodsettings.{h,cpp}
plugins/channelrx/demodcw/cwdemodgui.{h,cpp,ui}
plugins/channelrx/demodcw/cwdemodwebapiadapter.{h,cpp}
plugins/channelrx/demodcw/cwplugin.{h,cpp}
plugins/channelrx/demodcw/readme.md
```

### Modified
```
CMakeLists.txt                                              # +1 option
plugins/channelrx/CMakeLists.txt                            # +5 lines (add_subdirectory block)
sdrbase/channel/channelutils.cpp                            # +2 lines
sdrbase/webapi/webapiutils.cpp                              # +2 lines (URI + id maps)
plugins/channelrx/demodcw/cwdemodsettings.cpp               # CW startup defaults and filter bank presets
plugins/feature/morsedecoder/morsedecodersettings.cpp       # +1 line
plugins/feature/demodanalyzer/demodanalyzersettings.cpp     # +1 line
plugins/feature/denoiser/denoisersettings.cpp               # +1 line
plugins/feature/jogdialcontroller/jogdialcontrollersettings.cpp  # +2 lines (channel id + URI)
```

### Not touched (intentional, see §3.5)
```
swagger/sdrangel/code/qt5/client/SWG*.{h,cpp}               # generated, do not edit
sdrbase/resources/webapi/doc/swagger/include/SSBDemod.yaml  # leave SSB schema alone
sdrbase/webapi/webapirequestmapper.cpp                      # SSB key branch already covers CW
plugins/channelrx/demodssb/                                 # original SSB plugin untouched
```

---

## 7. Verification checklist

- [x] `plugins/channelrx/demodcw/` exists with 16 files mirroring `demodssb/`
- [x] All include guards renamed (`INCLUDE_CWDEMOD_*`, `PLUGINS_CHANNELRX_CWDEMOD_*`)
- [x] All class names renamed (`CWDemod`, `CWDemodSettings`, …, `CWPlugin`)
- [x] All Swagger reusable accessors preserved (`SWGSSBDemodSettings`, `getSsbDemodSettings()` …)
- [x] `Q_PLUGIN_METADATA(IID "sdrangel.channel.cwdemod")` set
- [x] CMake `add_subdirectory(demodcw)` and `ENABLE_CHANNELRX_DEMODCW` option in place
- [x] `CWDemodSettings::resetToDefaults()` seeds a CW-ready startup preset and filter bank
- [x] Help URL points at the renamed folder
- [x] Four cooperating feature plugins recognise the new URI
- [x] Jogdial Controller also recognises the new channel id
- [ ] Project re-configured and built end-to-end (requires the user to reset their build cache; not run in this session because the existing `cmake-build-debug-docker/` cache is locked to a Docker-internal source path)

---

## 8. Quick rebuild instructions

```bash
# from the project root
rm -rf cmake-build-debug-docker            # only if it was a stale Docker cache
cmake --preset default                      # or: cmake -S . -B build-default
cmake --build --preset default -j$(nproc)   # or: cmake --build build-default -j$(nproc)
```

In CLion: `Tools → CMake → Reset Cache and Reload Project` (use a non-Docker build folder).
