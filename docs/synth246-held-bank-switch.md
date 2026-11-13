# Cambio synth con notas retenidas: Dr. Wave 246

El ensayo ejecuta el cambiador original completo `0x01a86420..0x01a86660` en
contexto synth, incluyendo traducción inmediata, helpers de estado MIDI y
reactivación `0x01a854cc..0x01a856bc`. El grafo suministrado usa Dr. Wave (tipo 2),
seis ADSR inicializados y módulos auxiliares ausentes. Se reutiliza el modelo
[de refresco pitch](drwave246-pitch-refresh.md), sin modificar Source ni el plugin.

## Traducción y estado MIDI

Se conserva el retorno inmediato por banco ya seleccionado. Un cambio inicializa
el estado de traducción si procede y libera rápidamente las seis envolventes,
como en el [bloque vacío](synth246-bank-switch.md). El delta de tabla es una
diferencia de palabras. Con flag diferido no nulo se suma ese delta al acumulador;
con flag cero, cada fila cuyo byte de metadata no sea cero cambia su nota en
**delta × 12**. Las filas con metadata cero conservan su nota.

En el recorrido inmediato cada fila traducida llama al helper original note-off
`0x019e2dc0`, escribe la nota nueva y llama al helper `0x019e2e64` con su palabra
completa de velocidad. La salida está explícitamente deshabilitada mediante
`0x01dfe03c = 0`: los helpers originales actualizan el bitmap de actividad en
`0x01aafd60`, pero no se ejecuta transporte ni se envían paquetes. Una velocidad
u32 exactamente cero apaga; un valor como `0x00010000` cuenta como encendido
para ese helper aunque el halfword de velocidad de voz sea cero. Los bytes
activos no canónicos se conservan cuando el helper evita un encendido duplicado.
Los cambios se aplican en orden, incluidas colisiones entre notas de varias filas.

La tabla de ajustes suministrada está en −4…4. Todos los índices traducidos
permanecen en 0…127; la rama que elimina una nota fuera de rango queda pendiente.
El registro puede contener notas repetidas, metadata 0/1/165 e índices de voz
−2/−1/0…5. El contador y el resto de cada fila se conservan.

## Reactivación original compuesta

Tras la espera del sincronizador, el flag diferido cero entra en el refresco:

| Selector común del banco destino | Recorrido |
| --- | --- |
| 0 | Recorre las filas; voces negativas se omiten. Para cada voz válida: ADSR, ganancia y activación Core Dr. Wave. |
| 1/2 | Usa la última fila y reactiva la voz 0. |
| 3 | Usa la última fila y reactiva las seis voces. |
| 4 | Retorna sin reactivar. |

Los bancos tienen selectores distintos para comprobar que se usa el destino
recién seleccionado. Con registro vacío tampoco hay activación. Se comparan fases del pool Dr. Wave,
ADSR, registros de pitch/velocidad/actividad y RNG compartido. No se copia pitch
al buffer de render ni se renderiza PCM. La tabla de notas se suministra desde
el fixture numérico ya contrastado; el índice de afinación es 1200 y su coeficiente
es una entrada explícita, incluidos extremos. No se infiere calibración original.

## Frontera L1 y corpus

Ambas entradas L1 son trampolines propios que, tras 1–4 llamadas, copian el banco
seleccionado al activo. No llaman a factorías ni reconstruyen el servicio protegido.
El pool Dr. Wave y los punteros válidos se suministran antes de cada entrada;
no se atribuyen al servicio L1. El presupuesto finito del harness no es un timeout
original. No hay concurrencia ni temporización física/host.

- 2.048 entradas independientes, todas las 64 parejas de bancos y longitudes
  de registro 0–128; selectores comunes 0–4 e índices de voz −2…5.
- 1.792 cambios, 256 retornos por mismo banco, 4.480 llamadas L1 propias.
- 18.928 pares de llamadas a helpers MIDI en la traducción, 560 entradas al
  refresco, 7.675 resets de Dr. Wave y 10.752 liberaciones rápidas ADSR.
- 896 inicializaciones de traducción y 986 actualizaciones del acumulador diferido.
- Se comparan los 4.156 bytes de ventanas, 96 de trace propio y contador:
  4.256 bytes por salida. Todas las ventanas se recargan en cada fila.

Los contadores de helpers/resets siguen el recorrido del modelo contrastado y
la estructura del bucle original; no son paquetes MIDI ni muestras de audio.
Dos extracciones originales independientes coinciden. Release, ASan/UBSan y siete
suites de regresión aprobados. [CI nativo](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/43468582281): Linux y Windows/MSVC aprobados; macOS 15 cancelado sin ejecutar pruebas.
[Formato y reproducción](../research/synth-held-bank-switch/README.md),
[evidencia](firmware/synth-held-bank-switch246-validation.json).

Quedan las traducciones fuera de rango, módulos auxiliares, tipo de módulo 4,
servicio activo real, transporte MIDI, scheduler y productores pitch/gain.
Este bloque es investigación aislada. Plugin 0.54.0, sin instaladores; avance
≈70 % ±10 de planificación. Fidelidad acústica y Ableton real pendientes.


Actualización: [eliminación fuera de rango y cola MIDI con bomba ocupada](synth246-range-bank-switch.md)
contrastadas en el bloque posterior. Transporte físico y servicio L1 siguen pendientes.


[CI de regresión aprobado](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/43504787882)
en Linux x64, Windows/MSVC x64 y macOS ARM64 sobre `cc79266`. La ejecución anterior
con macOS cancelado permanece en la evidencia; este resultado valida la composición
actual y conserva los mismos fixtures del perfil anterior.
