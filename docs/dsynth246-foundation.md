# D-synth 246: evidencia histórica de la base

Este documento describe el commit `2a33075`, anterior al renderer C++ completo.
La versión actual es [D-synth integrado en 0.32.0](dsynth246-native.md), con un
corpus corregido y la comparación completa de audio/estado.

Este bloque prepara la reconstrucción de D-synth. **Todavía no añade D-synth al
selector ni implementa su renderer de audio C++.** El callback original se ejecuta
y registra como referencia; sus muestras no se presentan como coincidencia con
un motor ya portado. Pocket One conserva la versión funcional 0.31.0.

## Dependencias localizadas

La vtable `0x0190fbbc + 0x14` apunta a `0x0197e1d8`, referencia candidata del
op1-sdk ya acreditado y comprobada en el LDR rev246 disponible. El callback combina
dos osciladores, preparación de envolventes/filtros, cuatro configuraciones de
oscilador y ocho rutas de mezcla. Consume ocho controles, 128 palabras de pitch
y 128 valores de ganancia suministrados por el llamador. Los cuatro pequeños
helpers llamados durante el tramo final copian/leen/escriben valores temporales.

Las dos tablas contiguas en `0x0106b3cc`/`0x0106b5cc` corresponden a las 256
conversiones legibles de `0x018c13c4`, ya contrastadas en el ADSR común. La tabla
de filtro `0xff801f50` deriva de 129 floats legibles en `0x018be744`; coincide
con la tabla de frecuencia ya usada por Filter/Punch. Se comprueban todas estas
conversiones contra las instrucciones originales. El seno se prepara con el
loader contrastado. No hace falta una identidad OTP ni RAM exterior arbitraria
para ejecutar esta ruta con sus dependencias explícitas. Esto no descifra L1 ni
recupera el scheduler o el ruteo completo del instrumento.

La descompilación privada Ghidra 12.2/Blackfin 1.2.0 elimina como inalcanzables
ramas de los bucles de hardware. Además presenta como escritura de 32 bits una
inicialización que escribe solo 16 y conserva el resto; el contraste numérico
identifica y corrige esa diferencia en la reconstrucción propia. No se toma ese pseudocódigo como
portado correcto; se conserva únicamente en el directorio privado. El siguiente
paso sigue requiriendo reconstruir y contrastar la aritmética completa del audio.

## Reconstrucción implementada

`Dsynth246Lifecycle.h` implementa independientemente estados de **72 bytes**,
seis slots, dos osciladores por estado y un contexto compartido de **824 bytes**:
una constante de 32 bits y otra de 16 con padding conservado, seis pares de indicadores, seis históricos de 64 muestras y
seis contadores. No contiene código ni instrucciones originales.

| Operación | Dirección original | Efecto comprobado |
| --- | --- | --- |
| Factoría/constructor | `0x0197ee60` / `0x0197e058` | Inicializa campos; conserva slot y padding hasta el binding explícito |
| Binding | `0x0197e0b0` | Asocia uno de seis slots válidos |
| Reset | `0x0197e0ee` | Reinicia dos osciladores/envolventes/filtros; toma el cuarto control y borra histórico/contador del slot |
| Release | `0x0197e168` | Pone a cero los dos campos de excitación; no equivale a un ADSR externo |
| Inicialización compartida | `0x0197eeb0` | Constantes (32 y 16 bits), indicadores e históricos; conserva padding y contadores |
| Decaimiento de indicadores | `0x0197e17c` | Multiplica ambas mitades por 28672 en Q15 |
| Reinicio de captura | `0x0197dfcc` | Pone a cero solo el contador del slot |
| Captura posterior al render | tramo `0x0197ea6c` | Guarda muestras 0/32/64/96 cuando el contador firmado es menor que 60 |

Los límites defensivos del C++ rechazan slots no válidos. El contraste original
se limita a los slots válidos: no se presenta esa defensa como el assert original.

## Ocho destinos reales de modulación

Todos los descriptores de tipo **12**, `0x01de5b90`, son
`{0,32767,10485,3}`. El kind 3 tiene un ruteo diferente:

- **Value Instrument** selecciona entre ocho controles con `colour >> 11`.
  ENV conserva cuatro colores con `colour >> 12`.
- **Random Instrument** aplica los ocho valores a ocho destinos directamente,
  sin volver a los cuatro primeros ni atenuar la segunda mitad por 1/16.
- Los tipos ya integrados conservan sus rutas anteriores.

Se contrastan **4096 Value + 4096 Random callbacks**, estados completos, RNG,
indicadores, ocho destinos y retornos. Esto implementa la dependencia de modulación;
los controles secundarios y su interfaz de host se integrarán con el renderer.

## Referencia original y límites de la validación

El harness propio ejecuta **4096 bloques / 524288 muestras originales** con
controles/estados/pitch/ganancia sintéticos, ocho mezclas, eventos de ciclo y
secuencias retenidas. Registra pools completos antes/después, contexto compartido,
audio, pitch derivado, segunda señal, ganancia y guardas/RNG.

El test C++ compara el ciclo reconstruido contra el estado anterior al render.
Después **recibe explícitamente el estado posterior del renderer original** para
el siguiente evento. Compara también su captura del histórico y caracteriza la
preservación de voces vecinas, constantes, históricos, pitch, ganancia y guardas.
No calcula ese audio ni ese estado posterior mediante una réplica C++ del render.

Faltan portar el callback completo (incluidas todas sus ramas de mezcla y
envolventes internas), integrar ocho controles/estado/interfaz/modulación en el
plugin y calibrar acústicamente. El ADSR común no completa por sí solo las
envolventes específicas de D-synth. Presets originales completos al final.

Prueba pública:

```sh
cmake -S research/dsynth-render -B build-dsynth -DCMAKE_BUILD_TYPE=Release
cmake --build build-dsynth --config Release --parallel 2
ctest --test-dir build-dsynth -C Release --output-on-failure
```

El generador `scripts/verify-dsynth246.py` y los verificadores Value/Random con
`--dsynth` usan el LDR privado de hash comprobado y GNU BF524 con los dos parches
documentados. ELF, firmware, descompilación y SDRAM quedan fuera del repositorio.
Solo se versionan código propio y observaciones numéricas sintéticas.

Validación Linux: **103 suites aprobadas**, con repetición de la suite de ciclo
tras corregir esa preservación de 16 bits; **468 cargas existentes sin heap
rastreado**, ASan/UBSan de ciclo/histórico y modulación aprobados. Las cargas
de host siguen sin ejecutar D-synth como motor audible.

[Formato de los vectores](../research/dsynth-render/vectors/README.md) ·
[Metadatos y validación](firmware/dsynth246-foundation-validation.json).


[CI específico](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41498098182)
aprobado en Linux, Windows/MSVC y macOS Intel. El
[CI completo de plugins/instaladores de este commit](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41498098231)
sigue en curso al registrar esta evidencia; el CI completo anterior de Voltage
terminó con éxito en Linux, Windows y macOS universal (runtime Intel).
