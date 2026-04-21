# 🎮 FNAF Switch

> *"It's me."*

**Five Nights at Freddy's** — portado de forma nativa a **Nintendo Switch**, escrito en **C** desde cero usando el SDK homebrew de la consola.

---

## 📸 Capturas

> *(Añade aquí tus capturas de pantalla del juego corriendo en la Switch)*

---

## 🕹️ ¿Qué es esto?

Un port homebrew del videojuego original **Five Nights at Freddy's** (2014, Scott Cawthon) para **Nintendo Switch**. El proyecto reimplementa la mecánica completa del juego: las noches de supervivencia, el sistema de cámaras, los animatrónicos con su IA, el sistema de energía y todos los estados del juego, usando directamente la API de libnx y devkitPro.

---

## ✨ Características

- 🤖 **Sistema de animatrónicos** con IA independiente por personaje
- 📷 **Sistema de cámaras** navegable con los controles de la Switch
- ⚡ **Sistema de energía** con consumo dinámico según las acciones del jugador
- 🎵 **Motor de audio** con efectos de sonido y música ambiente
- 🖥️ **Motor gráfico** propio con soporte de sprites y animaciones por frames (~600 frames)
- 💾 **Sistema de guardado** para progresar entre noches
- 📋 **Gestor de estados** (menú, carga, juego, game over, 6AM, advertencias...)
- 🎮 **Controles nativos** de Nintendo Switch (Joy-Con / Pro Controller)

---

## 🗂️ Estructura del proyecto

```
fnaf_switch/
├── source/
│   ├── main.c                  # Punto de entrada
│   ├── engine/
│   │   ├── animatronics.c      # IA de los animatrónicos
│   │   ├── audio.c             # Motor de audio
│   │   ├── graphics.c          # Motor gráfico
│   │   └── input.c             # Gestión de controles
│   └── game/
│       ├── game.c              # Loop principal del juego
│       ├── camera_system.c     # Sistema de cámaras
│       ├── power_system.c      # Sistema de energía
│       ├── save_system.c       # Guardado/carga de partida
│       ├── state_manager.c     # Máquina de estados
│       ├── hud.c               # Interfaz de usuario
│       ├── menu.c              # Menú principal
│       ├── loading.c           # Pantalla de carga
│       ├── gameover.c          # Pantalla de Game Over
│       ├── 6am.c               # Pantalla de victoria (6 AM)
│       ├── warning.c           # Pantalla de advertencia inicial
│       ├── what_day.c          # Selector de noche
│       └── ad.c                # Pantalla de créditos/anuncio
├── include/
│   ├── engine/                 # Headers del motor
│   └── game/                   # Headers del juego
├── romfs/
│   ├── gfx/                    # Sprites y frames (~600 imágenes PNG)
│   └── sfx/                    # Efectos de sonido y música (.wav)
├── build/                      # Archivos objeto compilados
├── Makefile
├── fnaf_switch.nro             # Ejecutable para Nintendo Switch
└── icon.jpg                    # Icono de la aplicación homebrew
```

---

## 🛠️ Requisitos de compilación

- [devkitPro](https://devkitpro.org/) con **devkitA64**
- **libnx** — librería de sistema para Nintendo Switch homebrew
- **switch-tools** — herramientas para generar `.nro` y `.nacp`

### Instalación de dependencias (con pacman de devkitPro)

```bash
dkp-pacman -S switch-dev
```

---

## 🔨 Compilación

```bash
git clone https://github.com/PAMABA261/fnaf_switch.git
cd fnaf_switch
make
```

Esto generará el archivo `fnaf_switch.nro` listo para ejecutar.

---

## 🚀 Instalación y ejecución

1. Asegúrate de tener tu Nintendo Switch con **CFW (Custom Firmware)** instalado (p.ej. Atmosphère).
2. Copia el archivo `fnaf_switch.nro` a la carpeta `/switch/` de tu tarjeta SD.
3. Abre el **Homebrew Launcher** en tu consola.
4. Selecciona **FNAF Switch** y... ¡buena suerte sobreviviendo la noche!

---

## ⚠️ Aviso legal

Este proyecto es un **fan-made homebrew** sin fines comerciales. Five Nights at Freddy's y todos sus personajes son propiedad de **Scott Cawthon / Steel Wool Studios**. Los assets de audio y gráficos originales pertenecen a sus respectivos autores.

Este proyecto es solo para uso educativo y personal.

---

## 👤 Autor

Hecho con 🖤 por **PAMABA261**

---

<div align="center">
  <sub><i>¿Sobrevivirás las cinco noches?</i></sub>
</div>
