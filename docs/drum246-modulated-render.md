# Batería rev246: nota, modulación retenida, release y render

Se compone la cadena original legible bajo una cadencia explícita: note-on
completo, dispatcher Core de reset, reset del modulador y del sampler, callbacks
de modulación por bloque, note-off completo y lector FIR. Es investigación;
no cambia el instrumento 0.54.0 ni genera instaladores.

## Cobertura y resultado

288 sesiones de 16 bloques cubren 12 voces × 8 bancos × 3 perfiles. Los 24 pads
aparecen en el corpus, sin afirmar una rejilla completa voz/banco/perfil/pad.
Cada sesión mantiene sus objetos, fases, RNG, displays, posición, modo y contador
entre bloques; solo el inicio de sesión vuelve a cargar las semillas.

| Perfil | Callbacks aceptados | Rechazados | Renders con salida no nula |
| --- | ---: | ---: | ---: |
| Value drum | 1316 | 220 | 1009 |
| Random drum | 1317 | 219 | 904 |
| Tremolo común | 1316 | 220 | 789 |

Son 4608 callbacks, 288 note-on y 288 note-off completos. Se ejecutan 4464
renders de 128 muestras: **571392 muestras de salida**, sin contar los buffers
de entrada otra vez. Otros 144 bloques avanzan la modulación pero omiten render;
conservan el audio envenenado y la ganancia derivada anterior. Los rechazos
comprueban también la conservación de estados/buffers según el contrato.

Se suministran ocho fuentes PCM de 529200 frames, con guardas de 16 frames en
ambos lados. Son señales sintéticas de 12 segundos nominales a 44,1 kHz, no
capturas del aparato. La fuente completa evita atribuir al firmware el relleno
con cero de un PCM corto al desplazar regiones. Todo el PCM/guardas permanece
intacto. Los coeficientes FIR, seno, decaimiento y descriptor de rangos originales
se comprueban antes de comparar cada byte de cada bloque.

## Composición que se ejecuta

- Factory de audio tipo 8: `0x01967984`, con referencias reales del ensayo.
- Note-on `0x01a84ba0` → dispatcher activo `0x0196735e` → reset original Value
  drum `0x019cff16`, Random drum `0x019cef66` o Tremolo `0x019cbdca` → reset del
  lector. Los vtables auxiliares son propios, sin spies ni sustitutos del reset.
- Paso Value drum `0x019cfdac`, Random drum `0x019cedc0` o Tremolo `0x019cbe48`.
  El workspace se renueva desde los controles base antes de cada paso.
- Note-off `0x01a84dac` al inicio del bloque 12; release consulta los controles
  base del banco, mientras render consume los controles del workspace.
- Lector `0x019789ec` con pitch/gain explícitos; Tremolo puede modificar ambos
  antes de render. El lector conserva la referencia enlazada por la factory.

El comparador reutiliza DrumVoice246, CoreModulatorReset246, los tres modelos de
modulación y Drum246, sin cambios en ellos. Compara pool, globals, lista retenida,
controles, punteros, estados/displays, RNG, retorno, pitch/gain, ganancia derivada
y audio, incluidos los datos que deberían permanecer intactos.

## Validación y frontera pendiente

Dos ejecuciones originales independientes coinciden; la segunda reproduce los
fixtures congelados sin `--record`. Release y ASan/UBSan aprobados. También
pasan las regresiones de note-on activo, VoiceCtl, Core de sampler, bancos,
palabras de evento, inicialización Core y las ocho suites de lector/adaptador.
[Formato/reproducción](../research/drum-modulated-render/README.md) y
[evidencia](firmware/drum-modulated-render246-validation.json).

La cadencia, PCM, selección de perfil, controles y buffers iniciales son entradas
del ensayo. No se ha recuperado el scheduler del aparato, los productores de
pitch/ganancia, la factory que asigna Tremolo a esa ruta ni el consumidor L1
protegido. Modificar el control 0 con Value/Random no demuestra su conversión a
pitch por muestra. Continuar llamando al lector tras release, incluso en modo
cero, prueba este llamador explícito. No demuestra que el scheduler real lo haga.

Siguiente frente: productores y consumidores activos de pitch/ganancia y ruteo
protegido; conservar esta composición como regresión al integrar cualquier
sustitución híbrida. Acústica y Ableton real siguen sin medirse. Planificación
**≈70 % ±10**, sin subir por número de muestras. Presets al final.
