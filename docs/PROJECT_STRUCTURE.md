# Project Structure

## Arduino IDE Style (Branch: `main`)

Configuration is defined inline in your sketch file:

```
MyProject/
├── MyProject.ino           ← Main sketch
│   ├── Sound configuration (enum, table)
│   ├── #include <PicoSound_AudioCore.h>
│   ├── #include <PicoSound_DualCore.h>
│   └── setup(), loop(), setup1(), loop1()
│
└── (optional) Additional .h/.cpp files

Arduino/libraries/
└── PicoSound/
    ├── README.md
    ├── LICENSE
    ├── library.properties
    │
    ├── src/
    │   ├── PicoSound.h
    │   ├── PicoSound.cpp
    │   ├── PicoSound_AudioCore.h
    │   ├── PicoSound_AudioCore.cpp
    │   ├── PicoSound_DualCore.h
    │   ├── PicoSound_DualCore.cpp
    │   └── templates/
    │       ├── picosound_user_cfg_template.h
    │       ├── picosound_melodies_template.h
    │       └── picosound_samples_template.h
    │
    ├── examples/
    │   └── 01_BasicGame/
    │       └── 01_BasicGame.ino
    │
    └── docs/
        ├── API.md
        └── HARDWARE.md
```

### Workflow

1. **Create new sketch** in Arduino IDE
2. **Define sounds** at top of `.ino` file:
   ```cpp
   // Sound configuration
   #define USER_SND_OUT OUT_I2S
   #define USER_PIN_BCK 14
   // ... etc
   
   typedef enum { SND_BEEP, SND_LASER } SoundID;
   const SoundDefinition PICOSOUND_TABLE[] = { ... };
   ```

3. **Include PicoSound**:
   ```cpp
   #include <PicoSound_AudioCore.h>
   #include <PicoSound_DualCore.h>
   ```

4. **Write your program**

---

## PlatformIO Style (Branch: `platformio-style`)

Configuration in separate files for cleaner organization:

```
MyProject/
├── platformio.ini
│
├── include/
│   ├── picosound_user_cfg.h      ← Main config
│   ├── picosound_melodies.h      ← Melodies (optional)
│   └── picosound_samples.h       ← WAV samples (optional)
│
├── src/
│   └── main.cpp
│       ├── #include <PicoSound_AudioCore.h>
│       ├── #include <PicoSound_DualCore.h>
│       └── setup(), loop(), setup1(), loop1()
│
└── lib/
    └── PicoSound/
        ├── library.properties
        │
        ├── src/
        │   ├── PicoSound.h
        │   ├── PicoSound.cpp
        │   ├── PicoSound_AudioCore.h
        │   ├── PicoSound_AudioCore.cpp
        │   ├── PicoSound_DualCore.h
        │   ├── PicoSound_DualCore.cpp
        │   └── templates/
        │       ├── picosound_user_cfg_template.h
        │       ├── picosound_melodies_template.h
        │       └── picosound_samples_template.h
        │
        └── examples/
            └── ...
```

### Workflow

1. **Copy templates** from `lib/PicoSound/src/templates/` to `include/`:
   ```bash
   cp lib/PicoSound/src/templates/picosound_user_cfg_template.h include/picosound_user_cfg.h
   ```

2. **Edit** `include/picosound_user_cfg.h`:
   - Set hardware pins
   - Define sound IDs
   - Configure sound table

3. **In main.cpp**:
   ```cpp
   #include <PicoSound_AudioCore.h>
   #include <PicoSound_DualCore.h>
   // Config is auto-loaded from include/
   ```

4. **Build and upload**

---

## Key Differences

| Aspect | Arduino Style | PlatformIO Style |
|--------|---------------|------------------|
| **Config location** | Top of `.ino` file | `include/*.h` files |
| **Organization** | All in one file | Separated by concern |
| **Per-project config** | ✅ Yes (in sketch) | ✅ Yes (in include/) |
| **Library updates** | ✅ Safe | ✅ Safe |
| **Code clarity** | ⚠️ Large file | ✅ Clean separation |
| **Portability** | Arduino IDE only | PlatformIO/VSCode |

---

## Which to Use?

- **Arduino IDE** → Use `main` branch (inline config)
- **PlatformIO/VSCode** → Use `platformio-style` branch (separate files)

Both approaches keep your configuration safe during library updates.
