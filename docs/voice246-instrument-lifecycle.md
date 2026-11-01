# Rev246: controlador compartido de eventos de instrumento

Se compone y contrasta la cadena **entrada de nota → despacho exterior on/off →
selección/reinicio de voz → notificación → render ADSR** en los selectores por
sonido **0–4**, con pools fríos/listos y cambios del selector entre eventos.
No se conecta todavía Dr. Wave al plugin: pitch por bloque, consumo de pendientes,
reloj y salida dependen de tramos L1 que siguen sin imagen descifrada validada.

Esta entrega mantiene el frente Dr. Wave y agrupa las ramas ya reconstruidas.
No reabre la auditoría de referencias cifradas ni sustituye ese relevo por una
fórmula propia. La cadena es un paso hacia una voz completa, no esa voz completa.

## Un único dueño del despacho

`Source/Voice246InstrumentDispatch.h` contiene el despacho de instrumento con
**discriminador y modo exterior cero**, Core Dr. Wave válido, módulos auxiliares
nulos y palabra de salida externa explícitamente cero. Conserva los contratos de
los helpers previos de primera voz libre, ocupada, pitch explícito, lista vacía,
notas retenidas y retirada. Esos helpers siguen teniendo sus pruebas aisladas.

Las interfaces anteriores `Voice246NoteDispatch.h` y
`Voice246QuietNoteDispatch.h` ahora delimitan sus contextos estrechos y delegan
en este mismo controlador. Se eliminan las copias de preparación exterior y
notificación; sus fixtures históricos siguen pasando sin regenerarlos.

Con entrada válida, on escribe el indicador byte, normaliza **solo la palabra
completa** de velocidad `0xffffffff` a 127 y copia los campos del argumento local,
conservando padding 9–11. El pitch cero consulta la tabla; el no cero sustituye
esa fuente **antes** de la afinación Q31. No es una frecuencia ya afinada.

| Selector | Rama de on incluida |
| --- | --- |
| 0 | Primera actividad cero o máximo signed32 de edad entre seis ocupadas; inicio o solicitud de reinicio y primera desvinculación de dueño en lista. |
| 1 y 3 | Lista vacía: inicio/reinicio según actividad. Lista retenida: reinicio pendiente. El 1 procesa una voz; el 3, seis. |
| 2 | Lista vacía: inicio/reinicio según actividad. Con notas retenidas actualiza pitch directamente sin solicitar el reinicio de 1/3. |
| 4 | No añade la nota ni obtiene una envolvente; la preparación exterior y la notificación aplicable sí suceden. |

Gate y duplicados preceden a esas ramas. Off compone retirada polifónica o los
selectores 1–4, incluyendo compactación con padding conservado, dueños negativos,
recuperación de pitch y release ADSR. No limpia actividad al vaciar la lista.
La notificación se ejecuta después, incluso con gate/duplicado/nota ausente, si
metadata es no nula. Velocidad cero afecta al mapa; el on interior no se convierte
en off. La palabra de salida cero excluye preparación de paquetes/transporte.

### El arranque frío cambia la selección ocupada

El escaneo polifónico original llama al accessor ADSR **por cada voz ocupada
antes de leer su edad**, incluso si encuentra una voz libre más adelante.
Si ready es cero, el primer acceso inicializa los seis estados y borra sus edades.
No se puede elegir una víctima usando las edades anteriores y después inicializar.
En un pool frío con las seis edades negativas el original puede elegir tras esa
inicialización; un pool listo con las seis edades negativas entra en un assert
excluido. El adaptador rechaza ese segundo contexto antes de mutar.

La ampliación de acceso diferido se conserva en los helpers de pitch explícito
con lista vacía y retirada polifónica. Los llamadores que no suministran ready
siguen usando su contrato previo de pool preparado.

Los guards/enums son política propia, no retornos semánticos de R0 ni emulación
de asserts originales. Se excluyen capacidad desbordada, índices inválidos,
discriminadores alternativos, modo exterior no cero, módulos auxiliares presentes
y transporte habilitado. No se atribuyen nombres comerciales a todos los selectores.

## Prueba conjunta contra las instrucciones originales

**3.072 eventos: 24 secuencias de 128**, con lista, actividad, edades, pitch,
velocidad, seis estados ADSR, seis Dr. Wave, RNG, mapa y ready retenidos entre
eventos. No se inyecta una liberación de actividad ni commit de pendientes.

- Veinte secuencias cruzan cinco selectores con cuatro perfiles de entrada.
- Cuatro cambian el selector a lo largo de cada ciclo, sin recargar lista o voces:
  on A(0), on A(4), on B(3), off A(2), on C(4), off C(0), off B(3), off X(4).
- Dos perfiles ejecutan el constructor original: metadata 1 y pitch cero sin
  sobrescribirlos después. Otros dos reciben argumentos raw desde la entrada,
  con seis pitches no cero, metadata 0/1/128/255 y padding independiente.
- Ready inicial 0/65536/0/0xffffffff; actividad inicialmente libre, mixta o
  totalmente ocupada. Se cubren accesos fríos antes de una voz libre posterior y
  selección ocupada fría con edades negativas. Hay **10 inicializaciones**.
- Las 128 notas se presentan en on de cada selector. Gates, velocidad cero,
  sentinel completo, aceptación/rechazo Core y seis voces de render incluidos.

| Observación de la cadena | Cantidad |
| --- | ---: |
| On / off | 1.536 / 1.536 |
| Inserciones / retiradas | 888 / 888 |
| Gates / duplicados | 48 / 280 |
| On selector 4 deshabilitado / off ausente | 320 / 648 |
| Inserciones polifónicas libres / ocupadas | 26 / 222 |
| Cambios de byte del mapa | 1.728 |
| Posiciones ADSR escritas / conservadas | 313.984 / 79.232 |

Las 222 inserciones ocupadas producen campos pendientes y reasignan un dueño en
lista; **no demuestran que el consumidor active la nueva nota en el renderer**.
Se incluyen 64 on nuevos deshabilitados con lista retenida y 56 duplicados
en selector 4, conservando lista/pool y comprobando la notificación posterior.
Los cuatro cruces libre/ocupada × lookup/explicit se alcanzan. El RNG no avanza
en los eventos polifónicos; sí puede hacerlo el render ADSR posterior explícito.

Después de cada evento y render se comparan imágenes completas de **4.760 bytes
de regiones seleccionadas**, argumentos/copia local, padding, muestras, retorno
ADSR y guardas. El reloj, gate, selección, controles, afinación, voz/volumen para
render y orden de eventos pertenecen al harness. No es el dispatcher MIDI ni
el scheduler del aparato. Los monitores de transporte se conservan.

## Validación y reproducción

- **62/62 suites CTest Linux** con JUCE/plugin; VST3 y standalone compilados.
- Comparador nuevo con **ASan/UBSan**, sin errores; `detect_leaks=0` por ptrace.
- Repetición original en directorio privado independiente, sin `--record`,
  idéntica byte a byte. Las dos cadenas anteriores, fría y preparada, también
  reproducidas sin cambiar sus fixtures; todas sus regresiones C++ pasan.
- Preflight halfword, ambos parches GNU y watches PC propias 0/5/5/5. Se permiten
  ahora las ramas polifónicas verificadas; se mantienen excluidos assert,
  discriminadores ajenos, preparación/transporte y L1.
- REA 6.8.0 / Node 24.22.1 / GNU objdump inspecciona el llamador completo
  `0x01a84604..0x01a84ba0` bajo símbolo sintético: completo, salida 0, sin
  truncamiento ni cambio del ELF. No descifra ni aporta el contraste numérico.

El lote anterior `bdcda2b` completó CI nativo Linux/macOS/Windows, incluido arranque,
instalación/desinstalación Windows. Este nuevo lote requiere su propio CI; esos
resultados no se confunden con la ejecución Linux actual ni con Ableton físico.

```bash
source /workspace/tiny-one-blackfin/blackfin-env.sh
python3 scripts/verify-voice246-quiet-note-lifecycle.py --instrument \
  --ldr /privado/OP1_vdk.ldr --assembler "$POCKET_BFIN_AS" \
  --linker "$POCKET_BFIN_LD" --simulator "$POCKET_BFIN_SIM" \
  --testutils "$POCKET_BFIN_TESTUTILS" --output /privado/instrumento-nuevo
```

Destino nuevo y externo al checkout. `--record` crea solo los fixtures del perfil
seleccionado; no se necesita para la reproducción. Los perfiles sin `--instrument`
conservan las dimensiones/datos anteriores. [Preparación](new-account-dsp-setup.md) ·
[Evidencia](firmware/voice246-instrument-lifecycle-validation.json).

Formato: entrada 3.072 × **5.072 bytes**; salida tabla de notas 512 bytes,
3.072 × **9.900**, tabla final 512. Conserva el formato quiet/cold anterior.
Cabecera +12: bit 0 off (0 on), bit 1 argumento raw (0 constructor). La copia
local se captura antes de reutilizar la pila para render. No se cambian los bytes
de pitch/metadata después del constructor. El esquema no incluye instrucciones
originales; firmware, PDF, ELF y desensamblados permanecen privados.

La integración audible de Dr. Wave, productor de pitch, consumidor L1, reloj y
mezcla/salida siguen pendientes. No cambian MIDI/automatización/sesiones del
plugin ni se añaden presets para presentar este avance como un motor completo.
