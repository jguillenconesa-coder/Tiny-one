# Value por voz en Dr. Wave y Cluster — 0.16.0

Con **07 / DR. WAVE** o **08 / CLUSTER**, abre **set 4 → VALUE 246 VOICE**.
Los cuatro mandos MIDI existentes controlan velocidad, cantidad bipolar, destino
Instrument/Envelope ADSR y color objetivo. No se añaden parámetros ni presets.

En Instrument, el color selecciona uno de los cuatro controles del motor. En
Envelope ADSR selecciona Attack, Decay, Sustain o Release; ambos motores híbridos
usan ya su ADSR reconstruida. Los valores base se conservan: la modulación actúa
solo en la copia de cada voz y no escribe automatización en el host.

Cada voz conserva su estado de Value. **FREE PHASE** mantiene la fase al robar o
reutilizar esa voz; apagado, note-on la reinicia. Las voces inactivas pausan su
reloj. Voice tiene prioridad sobre Value FX si el host activa ambos. Phase y
Digital conservan sus rutas y pruebas anteriores. El LFO genérico del plugin
sigue disponible al apagar Voice.

## Evidencia nueva y límites

Se ejecutó la ruta original `0x019cfaf8` para los tipos 2 y 4 y su inicializador
de rangos. Las 4.096 llamadas —2.048 independientes y 2.048 continuadas— coinciden
en los 12 bytes de estado, 32 de display, ocho palabras destino, retorno y tempo.
Se repitió el oráculo en otro directorio privado con salida idéntica. Las 4.096
llamadas Digital/Phase y las pruebas FX anteriores siguen siendo regresiones.
[Procedencia y hashes](firmware/value-hybrid246-validation.json).

Los tres primeros colores de Dr. Wave usan el wrap observado del rango, con
saturación Q15 previa; naranja usa clamp. Cluster y los cuatro ADSR usan clamp.
Los rangos y escalas de modulación vienen del inicializador original, no de una
normalización común inventada. El rango de host de Dr. Wave se corrige a
**0–24575, 0–16383, 0–16383 y 0–32767**: verde/blanco antes exponían todo el dominio
seguro del núcleo, más amplio que el rango observado. No se priorizan migraciones
de sesiones históricas, según la decisión del propietario.

El reloj de control de 44.100/64 Hz, la asignación de voces y la conexión a los
adaptadores siguen siendo propios. Dr. Wave actualiza su onda cada seis bloques
internos; Cluster procesa controles por bloque. Estas cadencias pueden escalonar
la modulación y no se presentan como la alineación completa del hardware. La
memoria L1 sigue protegida y las aproximaciones de normalización de Cluster no
cambian. No se afirma equivalencia acústica completa ni se recuperan otros LFO,
destinos drum, cuatro campos ADSR auxiliares o los modos originales de voz.

## Validación integrada

- 14 comprobaciones de reloj/retrigger/FREE a siete tasas para ambos tipos.
- 48 renders con modulación de todos los colores y ADSR, cantidades negativas,
  cero y positivas, seis voces/robo, release y pánico.
- Editor cerrado, proxies MIDI, GUI habilitada, sesiones, slots y clipboard.
- Audio idéntico con buffers host de 64 y 257 muestras, con Voice apagado,
  Instrument y Envelope; las bases permanecen intactas.
- Cola comunicada al host ampliada cuando Release puede modularse.
- 132 cargas de callback, incluidas cuatro variantes nuevas Instrument/Envelope
  de los dos motores. Primer disparo y callback continuo sin heap rastreado.

El CI nativo sigue construyendo VST3/standalone para Windows y macOS, más el
paquete universal del lote anterior. Scanner/audio/automatización en Ableton real
requieren su prueba específica; el número de tests no mide parecido sonoro.

## Reproducir el oráculo

```bash
python3 scripts/verify-value-voice246.py --hybrid \
  --ldr /privado/OP1_vdk.ldr \
  --assembler /herramientas/bfin-as --linker /herramientas/bfin-ld \
  --simulator /herramientas/run-parallel \
  --testutils /fuentes-gdb/sim/testsuite/bfin \
  --output /privado/value-hybrid-nuevo
```

El directorio debe ser nuevo y quedar fuera del checkout. Sin `--hybrid`, el
script conserva el oráculo Digital/Phase previo. Firmware e instrucciones
originales no se distribuyen; los fixtures nuevos contienen entradas y resultados
numéricos del ensayo aislado.
