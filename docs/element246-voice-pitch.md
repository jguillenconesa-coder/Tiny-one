# Element sobre pitch de voz — 0.52.0

En LFO → Element, elige **Voice / FX Input** (AUDIO en la pantalla) y target
**Pitch**, con el mando naranja. Funciona en FM, Phase, Digital, String, Sampler,
Dr. Wave, Cluster, DNA, Voltage y D-synth. Drum/D-box y legacy lo rechazan.
Las cuatro fuentes, cantidad bipolar, automatización, mandos sin editor y
proyectos/slots/clipboard están integrados. Gain de 0.51.0 se conserva.

## Aritmética contrastada

El callback synth `0x019cda14`, kind 0/destino 3/color 1, transforma 128 words:

- Sensor: negativo saturado del sensor, Q15 con amount, expansión a Q31 y suma
  del producto relativo sobre el pitch firmado, con saturación.
- Level: misma ley relativa con la palabra de nivel suministrada. Procesa el
  buffer y devuelve cero; el host separa despacho y retorno.
- Env: suma `Q15(gain[i],amount)*16384` a cada pitch firmado, saturando.
- Audio: cada palabra externa se utiliza dos veces. La conversión original
  producto→double→float→Q31 satura el delta a un extremo firmado para cada
  producto integral no nulo en los casos contrastados. Se conserva esa respuesta,
  sin sustituirla por un offset en Hz o una curva de FM más suave.

8.192 llamadas originales comparan tag, controles, targets, fuentes y buffers
completos de pitch/gain/audio y retorno: 5.780 procesadas, 2.412 rechazadas y
1.444 Level procesadas con retorno cero. Diez tipos synth, cuatro fuentes,
cantidades y datos signed extremos; rechazos drum, kind/destino/color. Una
repetición privada genera datos idénticos. [Oráculo](firmware/element-pitch246-validation.json).

El puente de captura stream/bloque coincide con **5.175 llamadas / 662.400 words
por variante** de esos fixtures. Env→Pitch usa solamente los originales con gain
constante suministrado para comprobar el puente; el núcleo completo también
compara gain variable. No se presentan esos datos condicionados como recuperación
del productor de gains original ni como una medición acústica del instrumento.

## Composición del host

Se reutiliza la entrada externa de 0.51.0: remuestreo causal propio a 44,1 kHz,
antialias/interpolación, cuantización Q15 firmada e historia circular de 64 words.
Cada voz captura historia/amount al iniciar 128 muestras internas a 88,2 kHz.
Cada palabra externa dirige dos muestras. La historia pasada puede introducir
fase/retardo de modulación frente al productor original desconocido; no se añade
latencia al audio ni cambia el PDC. No requiere monitorización directa.

Env→Pitch retiene el contorno anterior de cada voz en Q14 como gain constante
para el bloque siguiente; con ADSR opcional apagada utiliza 16384. Es un productor
causal propio. FM/String/DNA conservan la lectura escalar del primer word del
buffer; los demás motores consumen sus buffers/streams existentes. String/DNA
mantienen sus límites positivos ya documentados, antes de los divisores del núcleo.

La captura reutiliza la infraestructura Bend/Crank/Element Gain, con propietario
distinto para Pitch. Cambiar Gain↔Pitch reinicia la captura, sin reutilizar datos
de otro propietario. Prepare/pánico limpian historia/capturas; notas y polifonía
comparten frontend y capturan bloques por voz. Legato conserva la voz. El estado
vivo no se guarda como parámetros. Sensor, nivel, clock, ownership, lifecycle,
ubicación y frontend son adaptaciones; no se recuperan allocator/scheduler/L1/OTP.

Además se corrige la declaración de cola de D-synth: Gain de Element con
Sensor/Audio/Level y cantidad no nula puede seguir excitando el núcleo tras su
ADSR exterior. El host declara cola conservadora ilimitada en esas rutas; pánico
sigue deteniendo el sonido. Env→Gain no añade señal cuando su gain vale cero.

[Validación](releases/v0.52.0-validation.json): 364 casos motor/fuente/tasa,
Mono/Legato/glide, propietarios/historia, audio exacto 64/257, automatización,
estado, PDC, pánico y sanitizadores. Envelope auxiliar sigue fuera del plugin
sin contrato de llamada activo. Acústica/Live real, ARM64 actual, productores/ruteo,
dinámica drum y presets originales completos permanecen pendientes.
