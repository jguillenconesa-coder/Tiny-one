# CWO 246: base matemática contrastada

Se preparan los componentes de CWO para cerrar después su callback completo.
**Este bloque aún no añade CWO al selector ni cambia el instrumento 0.25.0.**
La validación tiene un proyecto CMake independiente de JUCE para comprobar Linux,
Windows y macOS con rapidez, sin recompilar los instaladores del instrumento.

## Datos legibles y cuatro valores protegidos

El inicializador `0x01946228` convierte ocho float32 de `0xff9004b0..0xff9004cc`
en coeficientes de la red de fase. Cuatro posiciones están dentro de los primeros
8 bytes de grupos de 24 del payload CALLBACK/INDIRECT inventariado; las otras
cuatro son legibles. [Registro de direcciones](firmware/cwo246-phase-blocker.json).
El decompilador permite describir el consumidor, pero no descifra esos valores.

Se conservan exactamente las cuatro palabras legibles. Las cuatro posiciones
protegidas se sustituyen **explícitamente** por un modelo propio de cuadratura.
El script de reproducción rechaza cambiar cualquier palabra legible y no copia
los bytes protegidos como si fueran coeficientes. No se han recuperado claves OTP
ni una captura de L1 descifrada.

El productor `0x01945da8` consume constantes legibles de SDRAM y genera:

- 1.025 valores de seno.
- 257 palabras Q31 para frecuencia.
- 257 registros de cuatro coeficientes para Feedback.
- 257 palabras Q31 para Delay.

`0x019463b0` genera las 8.194 entradas de interpolación desde su fuente legible.
Son **10.761 entradas de tabla**, además de 25 palabras globales y ocho
coeficientes de fase. El prefijo contrastado ocupa 22.684 bytes con su padding.
Las tablas de control y de interpolación no se presentan como datos descifrados;
se ejecutan sus productores originales.

## Componentes contrastados juntos

| Rutina original | Reconstrucción propia y comprobación |
| --- | --- |
| `0x01944528` | Interpolación del control de frecuencia, todos los raw 0–32767, dos veces. |
| `0x019445a8` | Interpolación de los cuatro coeficientes de Feedback, mismo dominio exhaustivo. |
| `0x01944660` | Interpolación del control de Delay, mismo dominio exhaustivo. |
| `0x0194444e` | Recortador cúbico, los 65.536 valores int16 y sus dos salidas. |
| `0x01943b2e` | Etapa allpass Q15 con estado, entrada y coeficiente explícitos, 65.536 llamadas independientes. |
| `0x01943bea` | Filtro previo con tres historiales y coeficiente explícito, 65.536 llamadas con estados completos. |
| `0x0194435c` | Suavizado de control con velocidad/posición, coeficiente y destino explícitos, 65.536 llamadas. No establece la cadencia del llamador. |
| `0x01943e0a` | Red de fase de dos ramas y cuatro etapas por rama, con estado de 136 bytes y retardo de una muestra; 65.536 llamadas continuas con 16 cargas de estado. La coincidencia está condicionada a los cuatro coeficientes sustituidos. |

Se comparan todas las salidas y todos los estados. Incluye INT32_MIN/INT32_MAX,
paridad 0/1, extremos signed16, saturación serial, redondeo par y cambios de
coeficientes. El segundo oráculo privado independiente reproduce las fixtures
byte a byte. Los sanitizadores verifican los componentes C++ con esas fixtures.
[Registro de validación](firmware/cwo246-foundation-validation.json).

## Qué significa el modelo de fase

El modelo mantiene las cuatro posiciones legibles y ajusta las otras cuatro
mediante mínimos cuadrados de la respuesta allpass ideal. Se diseña a 44,1 kHz,
entre 30 y 18.000 Hz, con 600 puntos logarítmicos. La desviación máxima de
cuadratura en esa rejilla es aproximadamente **4,257°**. La prueba C++ comprueba
un presupuesto de **4,3°** sobre 10.000 puntos usando los Q31 realmente generados.

Esta métrica describe una red lineal **sin saturación**. No demuestra el sonido,
la respuesta de fase ni el rechazo de banda lateral del OP-1 real; tampoco
identifica los cuatro valores originales. La coincidencia con instrucciones
originales demuestra la aritmética y la evolución de estados **bajo el modelo**.
La calibración acústica deberá conservar explícita esa diferencia.

[Modelo y procedencia](../research/blackfin/cwo246-phase-model.json) ·
[Diseñador propio](../scripts/design-cwo246-phase.py) (NumPy/SciPy opcionales para
rediseñar, no necesarios para compilar ni ejecutar las pruebas).

## Reproducción y siguiente cierre

```bash
cmake -S research/cwo-foundation -B build-cwo -DCMAKE_BUILD_TYPE=Release
cmake --build build-cwo --config Release --parallel 2
ctest --test-dir build-cwo -C Release --output-on-failure
```

El script `scripts/verify-cwo246-foundation.py --help` permite repetir el oráculo
con firmware aportado por el operador, herramientas GNU Blackfin y un directorio
privado nuevo. Firmware, ELF y pseudocódigo permanecen fuera del repositorio.
El script valida el SHA de rev246 y la identidad de las cuatro palabras legibles.

Quedan por contrastar el retardo variable y sus buffers, el generador de fases
por bloque y la composición del callback `0x0194480c`. Después se integrarán los
cuatro controles, Value FX, persistencia y el adaptador del host. No se sustituye
la composición restante por una fórmula supuestamente original. Presets completos
al final. La estimación global permanece en **46 % ±10 puntos**.
