# 🎮 FNAF Switch

> *"It's me."*

**Five Nights at Freddy's** — portado de forma nativa a **Nintendo Switch**, escrito en **C** desde cero usando el SDK homebrew de la consola.

---

## 📸 Capturas

<img width="1647" height="911" alt="Captura de pantalla (24)" src="https://github.com/user-attachments/assets/e9fc9284-cc0a-4599-84d0-e70f91e5d0fe" />

<img width="1638" height="918" alt="Captura de pantalla 2026-04-21 192814" src="https://github.com/user-attachments/assets/f3ebed5c-bd8c-490b-8d94-55ec04c3acc7" />

<img width="1643" height="920" alt="Captura de pantalla 2026-04-21 192836" src="https://github.com/user-attachments/assets/7bcf24ff-8205-4ae2-ab18-121e7b719161" />

<img width="1639" height="921" alt="Captura de pantalla 2026-04-21 192855" src="https://github.com/user-attachments/assets/6d6adbc4-570e-495f-bd3f-cf89d4cf82e7" />



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
│       ├── game.c              # Pantalla de Oficina
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
│       └── ad.c                # Pantalla de anuncio
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

Este proyecto es un **fan-made homebrew** sin fines comerciales. Five Nights at Freddy's y todos sus personajes son propiedad de **Scott Cawthon**. Los assets de audio y gráficos originales pertenecen a sus respectivos autores.

Este proyecto es solo para uso educativo y personal.

---

## 👤 Autor

Hecho con 🖤 por **PAMABA261**

---

<div align="center">
  <sub><i>¿Sobrevivirás las cinco noches?</i></sub>
</div>
