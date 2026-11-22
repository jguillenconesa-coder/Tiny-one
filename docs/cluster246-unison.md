# Cluster: Unison de seis voces — 0.60.0

En **08 / CLUSTER → Set 2 / ADSR**, **UNISON** activa seis voces sobre la última
nota retenida. Tiene un parámetro propio **Cluster: Unison** (`cluster246Unison`),
desactivado por defecto, independiente del de Dr. Wave. Usa Glide y sustituye
temporalmente Poly/Mono/Legato. Al cambiarlo se vacía el grupo; vuelve a tocar.
Se guarda en sesión, slots y clipboard, y los estados anteriores lo cargan apagado.
El botón se conecta al parámetro del motor seleccionado, también al abrir el editor.

Cada voz conserva sus osciladores Cluster (1–8 por voz), ADSR, contexto/RNG y
filtros propios. No deben confundirse las seis voces con el conteo de osciladores:
pueden sonar hasta 48 osciladores. Las seis contribuciones se promedian (`1/6`),
una mezcla propia que no garantiza el mismo RMS que Mono ni replica la mezcla
original. La salida MIDI del host sigue siendo una nota, sin duplicación por voz.

## Composición y frontera con el original

Se reutilizan las operaciones de [desviación Q31](drwave246-unison.md) contrastadas
en el contexto Dr. Wave: RNG u32, clasificación libre/activa, multiplicación Q31
y saturación. Su uso en este grupo Cluster es una **composición del host**. No se
ha ejecutado ni contrastado aquí el dispatcher original completo de seis voces
Cluster, su scheduler, consumidor pendiente o identidad comercial del selector.

El núcleo `Cluster246.h`, reset y release mantienen sus fixtures originales. La
[activación Cluster](cluster246-note-activation.md) tiene 1.024 registros y
131.072 muestras originales con pitch suministrado. Esa evidencia no se convierte
en prueba de nuestro orden de seis callbacks ni de su mezcla.

El grupo conserva la gestión híbrida de Cluster: reset de cada voz al cambiar
la nota prioritaria, relevo de su última contribución durante 3 ms, ADSR aplicada
a la salida y release del núcleo una vez. **No se atribuye a Cluster el consumidor
con rampa pendiente de Dr. Wave.** La velocidad flotante externa, contexto por voz,
semillas derivadas del orden de notas, reloj 88,2 kHz, filtros/SRC y retirada siguen
siendo propios. El RNG de desviación usa la semilla aislada del modo Unison.

Sustain y bend siguen al canal de la nota prioritaria, incluido el canal interno
17. Soltar una nota antigua no reinicia el grupo; al soltar la prioritaria se
recupera la anterior retenida. Panic, cambio de modo y cambio de motor lo vacían.
Los parámetros Unison de motores inactivos permanecen guardados, pero no actúan.

Se conservan las normalizaciones conocidas para 1, 4, 5, 6 y 7 osciladores. Los
conteos **2, 3 y 8** siguen usando `floor(16384/count)`, una aproximación propia,
sin declarar recuperados esos coeficientes L1. Todos los coeficientes de damping
contrastados permanecen sin cambios.

## Comparación de seis voces

La nueva prueba compara seis adaptadores con llamadas independientes al núcleo
C++ ya verificado, bajo controles, tablas, pitch y semillas explícitos:

- Ocho conteos × seis voces × bloque de inicio y bloque de release: **96 bloques,
  12.288 muestras**, con estado completo de 108 bytes y RNG comparados.
- 60 bloques usan los cinco conteos de normalización conocidos; 36 usan las
  tres aproximaciones suministradas. Esto no valida sus valores en el hardware.
- Los contextos son independientes, como en el producto; no reproduce el RNG
  global compartido ni una captura de seis voces del aparato.
- A seis tasas se comprueban prioridad, fallback, sustain, canal 17, Panic,
  retirada y recuperación de Poly. El procesador compara nueve pares 64/257,
  además de estado, migración, slots, clipboard e interfaz.
- Seis cargas nuevas con buffers preparados ejercitan 1.536 callbacks con
  ráfagas y automatización, rastreando heap del callback.

[Evidencia de entrega](releases/v0.60.0-validation.json) registra resultados,
sanitizadores y CI. No hay extracciones originales nuevas ni instaladores.
La fidelidad acústica y Ableton real siguen pendientes. Siguiente frente: integrar el [contexto/RNG compartido ya contrastado](cluster246-core-routing.md)
antes de sustituir los contextos independientes del adaptador. Ese oráculo tiene
1.024 registros/131.072 muestras intercalando seis slots bajo orden suministrado;
no es una mezcla completa ni prueba del reloj original. No hace falta regenerar
sus fixtures. La integración deberá definir el orden de reset/render/decay y
verificar su consumo de RNG, sin asumir que sea el scheduler del aparato.

## Comprobación de interfaz en macOS

El CI de 0.59.0 aprobó Linux/Windows y falló únicamente en la aserción de interfaz
Unison de macOS (152/153 suites aprobadas allí). La prueba miraba el estado a los
500 ms, sin observar si se había procesado el clic. No dejó datos suficientes
para atribuir el fallo a una condición concreta de visibilidad/toggle/parámetro.

Ahora la prueba observa el clic real, el parámetro y la reacción de selector/Glide,
con plazo máximo de 5 s y diagnóstico separado. En AppKit crea un peer nativo,
en lugar de probar un editor sin ventana. Conserva las aserciones funcionales;
la validación nativa de esta corrección queda registrada en el CI de 0.60.0.
