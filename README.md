# Argentum Online

Trabajo Práctico Final de **Taller de Programación** (FIUBA): un juego
multijugador en red basado en *Argentum Online*. Varios jugadores se conectan a
un mismo servidor y comparten el mundo en tiempo real. El proyecto incluye el
cliente gráfico (SDL2 con launcher en Qt5), el servidor de juego y un editor de
mapas.

<img width="960" alt="image" src="https://github.com/resources/logos/gameLogo.png"/>

## Integrantes

- Aylén Bartomeo — 111738
- Franco Bustos — 110759
- Alexander Coronado — 111256
- Bruno Pezman — 110457

## Estructura del proyecto

```
client/      Cliente gráfico (SDL2 + launcher Qt5)
server/      Servidor de juego
editor/      Editor de mapas
common/      Código compartido (protocolo, sockets, DTOs, etc.)
tests/       Tests (GoogleTest)
config/      Configuración del juego (.toml: personajes, items, monstruos, inventario)
maps/        Mapas en JSON (mapa canónico: maps/defaultMap.json)
resources/   Assets gráficos del cliente y el editor
```

## Dependencias

Requiere **CMake ≥ 3.24** y un compilador con **C++20** (gcc/g++).

Antes de compilar hay que instalar a mano algunos paquetes del sistema. En
Ubuntu / Xubuntu 24.04:

```bash
sudo apt-get install qtbase5-dev qtbase5-dev-tools \
  libopus-dev libopusfile-dev libxmp-dev libfluidsynth-dev fluidsynth \
  libwavpack1 libwavpack-dev libfreetype-dev wavpack
```

- `qtbase5-dev` / `qtbase5-dev-tools`: requeridos por el launcher del cliente (Qt5).
- El resto son dependencias de audio/fuentes de SDL2.

SDL2, SDL2pp, SDL2_image, SDL2_mixer, SDL2_ttf, toml++, nlohmann_json, bcrypt y
GoogleTest se descargan y compilan automáticamente vía FetchContent; **no** hace
falta instalarlos a mano.

## Compilación

Todo se ejecuta **parado en la raíz del proyecto**. El `Makefile` resuelve los
casos más comunes:

| Comando | Qué hace |
|---|---|
| `make compile-debug` | Compila el proyecto (con símbolos de depuración y sin optimizaciones, para poder debuggear) |
| `make run-tests` | Compila y corre los tests |
| `make valgrind-tests` | Compila y corre los tests bajo Valgrind (reporte en `build/valgrind/`) |
| `make all` | Recompila todo desde cero y corre los tests |
| `make clean` | Borra `build/` |

---

## Guía paso a paso: ejecución y pruebas manuales

### Ingreso al juego

#### Paso 1: Limpiar la base de datos (opcional pero recomendado)

Si venís de pruebas anteriores y querés arrancar con un entorno limpio, borrá los
datos de usuarios y progresos previos. Parado en la raíz del proyecto:

| Comando | Qué borra |
|---|---|
| `make clean-db-auth` | Solo las cuentas registradas y los personajes creados (`auth_data`, `users_data`) |
| `make clean-db-world` | Solo los mundos guardados (carpeta `worlds`) |
| `make clean-db` | Todo junto de una sola vez |

> Nota: los mapas se borran manualmente por seguridad.

#### Paso 2: Iniciar el servidor

Para levantar el juego, primero hay que encender el servidor indicando en qué
puerto va a escuchar las conexiones. Parado en la raíz del proyecto:

**Crear un mundo nuevo (personalizado):**

```bash
make run-server-create WORLD="MiMundo" PORT=8080 MAP="maps/defaultMap.json"
```

o más rápido, con parámetros por default:

```bash
make run-server-create
```

**Cargar un mundo existente (personalizado):**

```bash
make run-server-load WORLD="MiMundo" PORT=8080
```

o más rápido, cargando el mundo por default:

```bash
make run-server-load
```

> Nota: si se omiten las variables, el `Makefile` usa automáticamente
> `DefaultWorld`, puerto `8080` y `maps/defaultMap.json` como valores
> predeterminados. Así se puede levantar un servidor rápido con solo tipear
> `make run-server-create` o `make run-server-load`.

#### Paso 3: Abrir el cliente

En una nueva pestaña de la terminal, parado en la raíz del proyecto:

```bash
make run-client
```

#### Paso 4: Conexión al host

Al abrirse la ventana del juego, la primera pantalla solicita la IP y el puerto
del servidor.

- **IP:** `127.0.0.1` (o `localhost` si están en la misma máquina).
- **Puerto:** `8080` (o el mismo que se usó en el Paso 2).

Validaciones en esta etapa:

- Si ingresás un puerto o IP donde no hay ningún servidor corriendo, el juego
  rechaza la conexión (no te deja avanzar a la siguiente pantalla).
- Solo avanza cuando logra establecer el *handshake* inicial con el servidor.

#### Paso 5: Autenticación (login / registro)

Una vez conectados al servidor, aparece la pantalla de autenticación, con dos
opciones:

**Opción A — Registrar un nuevo jugador:**

1. Ingresá un nombre de usuario y contraseña inventados.
2. Hacé clic en **Sign Up (Register)**.
3. Validaciones: si el nombre de usuario ya existe en `auth_data/`, el servidor
   rechaza la solicitud y hay que elegir otro nombre. Si es nuevo, crea la cuenta
   y entrás al juego directamente.

**Opción B — Iniciar sesión (login):**

1. Ingresá tu nombre de usuario y contraseña existentes.
2. Hacé clic en **Sign In (Login)**.
3. Validaciones:
   - El servidor verifica los datos. Si el usuario no existe, o la contraseña es
     incorrecta, la solicitud es rechazada.
   - **Sesión múltiple:** si los datos son correctos pero ese usuario *ya está
     conectado dentro del juego*, el servidor bloquea el acceso para evitar
     jugadores duplicados.
   - Si todo es correcto y la cuenta no está en uso, ingresás al mundo.

### Funcionalidades pasivas del juego

Comportamiento que funciona de fondo, sin que el jugador apriete botones.

**Persistencia y guardado de posición:**

- **Ingreso inicial:** la primera vez que jugás con tu usuario, el personaje
  aparece en las coordenadas iniciales por defecto (el *spawn point* definido en
  el mapa).
- **Autoguardado:** mientras te movés por el mundo, el servidor hace un barrido
  periódico de todos los jugadores conectados y guarda automáticamente sus
  posiciones cada 30 segundos.
- **Desconexión segura:** al cerrar el juego (cerrando la ventana del cliente),
  el servidor detecta la salida y guarda inmediatamente la última posición exacta
  en la carpeta `game_data/`.
- **Reconexión:** para comprobar la persistencia, cerrá el cliente y volvé a
  ingresar con login (Paso 5). El personaje aparece exactamente en la misma
  baldosa donde lo dejaste, persistiendo también así todo su progreso y estado al momento de desconectarse.
- **Progreso por cuenta, por mundo:** note que cada jugador tendrá su propio progreso asociado a su cuenta POR CADA MUNDO en el que haya jugado, es decir un mismo usuario puede tener varios progresos (estados) en distintos mundos, cada uno persistido por separado.

---

## Editor de mapas

Una vez compilado el proyecto (`make compile-debug`), parado en la raíz del repo:

```bash
make run-editor
```

Se abre una ventana con el mapa y, a la derecha, la paleta de tiles. Se puede:

- **Pintar el piso:** elegir un tile de la paleta de la derecha y pintar con clic
  izquierdo sobre el mapa.
- **Elegir herramienta** (arriba a la derecha): *pincel* para pintar terreno, o
  *spawn* para definir dónde aparece el jugador. La opción activa queda con borde
  amarillo.
- Con *spawn*, el clic pone el muñequito en la celda donde arranca el jugador.
- **Moverse por el mapa** con las flechas del teclado, o arrastrando con el botón
  derecho del mouse.
- **Guardar** con el botón verde o la tecla `S`.

Para saber si guardó: el título de la ventana dice "guardado" y "* sin guardar"
cuando hay cambios, el botón verde parpadea, y en la terminal aparece
`Mapa guardado en maps/defaultMap.json`.

El mapa se guarda en `maps/defaultMap.json`, que es el mapa del juego: el cliente lee ese mismo archivo.

## Documentacion
Dentro de la carpeta ***documentation*** se encuentran los 3 manuales del juego y el enunciado del mismo.

- [Enunciado Argentum Online](documents/enunciado.pdf).
- 'Manual de Usuario'.
- 'Informe Técnico'.
- 'Informe del Proyecto'.

## Créditos
Se han usado las siguiente bibliotecas desarrolladas por [Di Paola, Martin](https://www.github.com/eldipa):

- 'Socket'.
- 'Resolver'.
- 'ResolverError'.
- 'LibError'.
- 'Queue'.
- 'Thread'.