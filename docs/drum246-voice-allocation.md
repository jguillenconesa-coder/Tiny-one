# Batería 0.53.0: asignador contrastado dentro del instrumento híbrido

Entrega del 6 de noviembre de 2026. El kit con lector 246 incorpora el selector
original de doce voces, supresión de note-on retenidos y comprobación del
propietario al liberar. El método híbrido acordado permite integrar ese contrato
sin esperar a recuperar toda la cadencia de audio. Las fronteras con el host
quedan explícitas; no se atribuyen al firmware sus decisiones de retirada.

## Comportamiento audible

Una nota ya retenida en el mismo canal no reinicia el lector ni su LFO por voz.
Tras su note-off puede volver a dispararse. Las otras notas y canales pueden
seguir sonando, con un máximo de doce lectores. Si una voz ha sido reutilizada,
el note-off de la nota anterior retira su pertenencia sin liberar al nuevo dueño.

Se toma la primera voz que el host marca libre. Si las doce están ocupadas, se
consulta su actividad en orden y se toma el mayor contador **firmado** de bloques;
el primer índice gana los empates. Por ejemplo, con contadores 5/120/120, se
elige la segunda voz. El orden cronológico de creación ya no decide el robo.
La consulta del lector con modo cero limpia su contador, como el original.
No se consultan voces posteriores a la primera libre.

En una ráfaga antes de renderizar, varios contadores pueden valer cero: los
empates pueden reutilizar la misma voz varias veces. Es el contrato contrastado,
no un round-robin inventado. El ordinal del host se conserva para visualización.

## Qué se ha portado

[Drum246Allocator.h](../Source/Drum246Allocator.h) contiene la selección de
`0x01a84ba0`, extraída de [VoiceCtl](drum246-voice-lifecycle.md). Ese mismo código
Source se usa ahora en los verificadores de VoiceCtl y bancos: **12.288 registros
originales** siguen coincidiendo, incluidos los efectos de las consultas sobre
los estados. Son 4.416 selecciones aceptadas dentro de esos registros; no se
presentan todas las filas como nuevas llamadas de selección o nuevas muestras.
Se reutilizan fixtures ya extraídos dos veces y sus hashes, sin volver a contar
la evidencia ni incluir firmware en el repositorio.

[Drum246Voice.h](../Source/Drum246Voice.h) conecta el selector al banco de doce
hits. Un registro fijo por nota/canal conserva la pertenencia hasta note-off,
aunque la nota haya perdido el lector o haya terminado naturalmente. La ruta de
liberación comprueba nota y canal contra el dueño actual antes de cambiar su
gate. Reset, reemplazo de kit y cambios de motor limpian ese registro junto con
el banco, sin asignaciones dinámicas nuevas.

## Adaptaciones que siguen siendo propias

- Ocupación basada en `Hit.active`, retirada al terminar el lector y su cola de
  SRC/filtros, reloj de render, fuentes/caché y compensación FIR de ocho frames.
- Registro directo de 17 × 128 entradas en lugar de la lista original de notas
  53–76. Conserva los 16 canales MIDI más el canal interno del secuenciador y el
  mapeo de teclado/aliases existente. Dos notas distintas del mismo pad son
  identidades distintas; dos canales tampoco se fusionan.
- Sustain, liberaciones diferidas, pánico y fundido de robo de 3 ms. El pánico
  libera audio: no se sustituye por la [limpieza original de registro](drum246-bank-lifecycle.md),
  que conserva los lectores.
- Si todos los contadores consultados son negativos, el original no encuentra
  candidato y hace assertion. El host elige la primera voz como recuperación
  acotada propia; se prueba ese caso sin ejecutar la assertion del firmware.
- Velocidad, productores de pitch/gain, curva dinámica, salida y mezcla propios.
  Los disparos de FX globales siguen respondiendo al MIDI entrante del frontend,
  incluso si una nota retenida se ignora en el lector. No se recupera por ello el
  reset/ruteo original de esos FX.

No se añade ningún parámetro ni formato de proyecto. El registro describe solo
la ejecución actual; al cargar un proyecto el banco arranca vacío. No se integra
el resto del modelo de investigación de Core/bancos ni se modifica D-box o el kit
legacy. La decisión anterior de esperar a toda la composición se acota: se usa
el selector probado dentro de un adaptador identificado como propio, conforme
al método híbrido autorizado, sin presentar el conjunto como la voz original.

## Verificación

[Pruebas del adaptador](../Tests/Drum246AllocationTests.cpp): mayor contador,
empates, efectos de consulta y parada en primera libre, duplicados sin consultas,
propietarios antiguos, fallback firmado propio, sustain y pánico por canal,
las 2.176 teclas soportadas retenidas y 84 composiciones de lector (siete tasas ×
tres modos × cuatro configuraciones: sin LFO, Value, Random, Tremolo).

[Prueba de plugin](../Tests/Drum246AllocationPluginSupport.h): igualdad exacta
con/sin seis duplicados timestamped, note-off y cola, a 48/96 kHz y buffers
64/257. Se refuerzan 36 cargas de batería ya existentes con ráfagas de 24 notas,
duplicados, offs y sustain dentro de las **1.446 cargas** de tiempo real; no se
incrementa su contador por reutilizarlas. Los límites del rastreo de heap y del
CI compartido se mantienen: no certifican dropouts o latencia física en Live.

[Evidencia de entrega](releases/v0.53.0-validation.json). La equivalencia del
selector se limita a sus entradas; el llamador de render y los productores de
buffers siguen pendientes. No se recupera la envolvente dinámica original ni se
mide fidelidad acústica. Estimación de planificación **≈70 % ±10 puntos**, con
el área de voz/ruteo ajustada de 37 % a 40 % por la integración funcional; no se
incrementa el avance por acumular fixtures. Live real, firma/notarización,
ARM64 nativo y presets originales completos siguen pendientes.
