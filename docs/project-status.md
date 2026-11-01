# Estado y entregas del proyecto — 1 de noviembre de 2026

Objetivo: instrumento inspirado en OP-1 original, standalone macOS/Windows y
VST3 para Ableton Live; avanzar hacia la reconstrucción verificable de motores,
efectos y flujo. La réplica de todos los presets originales queda al final.
Esta cuenta es la coordinadora principal según la instrucción del propietario.
Las notas de coordinación entre cuentas anteriores se conservan como historial,
no como reservas indefinidas de tareas ya entregadas o sin PR abierto.

## Avance estimado: aproximadamente 38 % del objetivo completo

Es una estimación de planificación, **no una medición de equivalencia sonora**,
ni un cociente de muestras, pruebas o commits. La incertidumbre es de unos
±10 puntos: las dependencias L1 protegidas impiden estimar con precisión el
trabajo de fidelidad que falta. Se usa esta ponderación para no confundir un
prototipo tocable con una réplica completa:

| Área | Peso del objetivo | Avance estimado del área | Aportación |
| --- | ---: | ---: | ---: |
| Motores originales y sus filtros | 30 % | 35 % | 10,5 puntos |
| Efectos, envolventes y moduladores | 20 % | 40 % | 8 puntos |
| Voz, modos de ejecución y ruteo original | 15 % | 30 % | 4,5 puntos |
| Interfaz, cinta, mezcla y flujo creativo | 15 % | 60 % | 9 puntos |
| Distribución macOS/Windows y validación Live | 10 % | 55 % | 5,5 puntos |
| Todos los presets originales fieles | 10 % | 0 % | 0 puntos |
| Total | 100 % | | **37,5 ≈ 38 %** |

La base funcional de software está bastante más adelantada (orientativamente
70–75 %): VST3/standalone, MIDI, estados, interfaz, cinta y motores propios ya
funcionan. Ese porcentaje no se suma al anterior. Phase/Digital/Punch/Tremolo y partes
de Value/ADSR tienen reconstrucciones contrastadas; Dr. Wave y Cluster incorporan motores híbridos tocables con
dependencias propias explícitas. El FM actual y
varios efectos son propios. La [matriz técnica](original-op1-coverage.md) es la
fuente del estado por componente y prevalece sobre este resumen orientativo.

Hay instalador Windows y builds multiplataforma en CI. Se añade empaquetado
macOS universal; la validación nativa del paquete corresponde a su propio CI.
Falta validación actual en Ableton real de ambos sistemas; el
CI nativo o Wine no demuestra por sí solo aceptación del scanner de Live,
latencia física o ausencia de dropouts. No se afirma que el 100 % esté cerca.

## Entrega agrupada actual: Tremolo reconstruido e integrado (0.18.0)

Tremolo 246 añade en set 4 pitch, amplitud, envolvente del LFO y cinco formas a
Phase/Digital 246 y Dr. Wave/Cluster. 2048 llamadas originales con 262144 palabras
de pitch y otras tantas muestras de amplitud, estado/display completos, reset
y tabla de 24 coeficientes. Repetición independiente idéntica. Los adaptadores
por muestra y buffer conservan esa coincidencia.
[Guía, ruteo y límites](tremolo246-native.md).

La estimación sube a ~38 % por un modulador adicional contrastado e integrado,
no por una comparación acústica con hardware. La política de voz, reloj,
mezcla/salida y relevo L1 pendientes siguen documentados como adaptadores.
Los presets originales completos continúan aplazados.
68/68 suites Linux, 180 cargas de callback sin heap rastreado y sanitizadores
aprobados. [Validación](releases/v0.18.0-validation.json). El CI de este commit
valida los binarios/paquetes de Windows y macOS; Live real sigue pendiente.

## Entrega anterior: Mono / Legato / Glide híbridos (0.17.0)

Dr. Wave y Cluster añaden en set 2 modos Poly/Mono/Legato y portamento de
0–2000 ms. Prioridad de última nota, fallback con velocidad/canal, sustain/bend
por canal, pánico y cambios de modo completos; controles automatizables,
sesiones/slots/clipboard y callback con almacenamiento fijo.
[Comportamiento, procedencia y comprobaciones](hybrid-performance.md).
66/66 suites Linux, 156 cargas de callback sin heap rastreado y sanitizadores
aprobados. [Evidencia](releases/v0.17.0-validation.json). El CI 41343540488 pasó
Windows y Linux. Su job macOS quedó cancelado sin pasos ejecutados; no se da por
validada esa plataforma para 0.17. Live real sigue pendiente.

Son adaptadores propios y no recuperan los modos originales L1. Los núcleos y
sus oráculos se mantienen intactos. La estimación permanece ~37 %: mejora la
interpretación funcional, sin nueva evidencia de fidelidad acústica.

## Entrega anterior: Value por voz en los motores híbridos (0.16.0)

Dr. Wave y Cluster reciben Value sobre sus cuatro controles o su ADSR. Se añaden
4.096 llamadas contrastadas y repetidas, con rangos originales, wrap de Dr. Wave,
clamp de Cluster y sus estados/display completos. Se conservan los 4.096 casos
Digital/Phase. Los máximos verde/blanco de Dr. Wave se corrigen a 16383.
[Guía y evidencia](value-hybrid246-native.md).

Sin nuevos presets ni parámetros, la integración conserva bases y usa las mismas
identidades MIDI con editor cerrado, sesiones, slots/clipboard y cola Release.
65 suites Linux, 132 cargas de callback y sanitizadores. La estimación pasa a
~37 % por la ampliación funcional y numérica, no por parecido acústico medido.
El CI propio valida binarios/paquetes nativos; Live real sigue pendiente.
El CI 0.16 (41339832399) ya terminó correctamente en Linux, Windows y macOS,
incluido instalador Windows y paquete macOS universal. El reempaquetado 0.15
sin compilar (41339868716) también terminó correctamente; el fallo previo de
argumentos de `lipo` queda cerrado. Resultados consultados el 1 de noviembre;
no se extrapolan a los binarios de la siguiente entrega.

## Entrega anterior: Cluster híbrido y macOS universal (0.15.0)

Cluster usa el núcleo contrastado con cinco normalizaciones observadas y tres
aproximaciones documentadas. Integra seis voces, controles, ADSR, MIDI, guardado,
slots/clipboard e interfaz. Se verifica el relevo entre los dos motores híbridos.
[Guía](cluster246-hybrid.md). La estimación pasa a ~36 % por esta integración;
no mide fidelidad acústica ni da por resueltos los coeficientes originales.

El CI añade un `.pkg` y ZIP portable universales para macOS con inspección de
arquitecturas, firmas locales y payload. No son builds notarizados y queda
pendiente probarlos en Live real. [Empaquetado](macos-packaging.md). Se amplía la
regresión a 64 suites Linux y 108 cargas de callback. El estado de los runners
nativos pertenece a cada ejecución; no se extrapola del resultado Linux.

## Entrega anterior: Dr. Wave híbrido tocable (0.14.0)

Se adopta la decisión del propietario: núcleos contrastados y adaptadores propios
para las dependencias L1 pendientes, buscando máxima semejanza sin esperar una
captura descifrada. No se prioriza compatibilidad con sesiones anteriores.
[Guía, adaptadores y límites](drwave246-hybrid.md).

Nuevo motor con seis voces, cuatro controles originales del núcleo, ADSR,
bend/sustain, robo de voz, conversión a la tasa del host, interfaz, MIDI y estado.
Un init propio permite empezar sin adelantar la réplica final de presets. Se
amplían las pruebas de audio, integración y heap. La estimación sube de ~33 % a
~35 % por la integración funcional; no cuantifica cuánto se parece acústicamente.

El lote anterior unificó el despacho 0–4 y conservó 3.072 eventos contrastados;
62 suites y CI Linux/macOS/Windows pasaron en `8612850`. Esa evidencia sigue
siendo del controlador aislado; el adaptador híbrido no se rebautiza como L1.

## Entrega anterior: arranque diferido ADSR

- Recuperación/reproducción del avance de notificación `ec1c182`, pendiente de
  main: constructor, metadata original, on/off, mapa y ADSR con salida cero.
- Reconstrucción del accessor y arranque diferido de las seis ADSR, 1.024 casos.
- Composición con 1.536 eventos y estado de inicialización retenido, incluyendo
  gates que no deben inicializar y palabras ready no canónicas que se conservan.
- Validación conjunta: 61 suites Linux con plugin, repetición de oráculos y
  sanitizadores. [Contrato y evidencia](adsr246-lazy-pool.md).
- Corrección de la ruta antigua del script cloud: deriva su raíz del archivo,
  sin modificar configuración publicada ni volver a guardar el borrador.

Firmware y PDF aportados se usan como referencias privadas; su contenido no
constituye instrucciones de trabajo ni se copia al producto. El LDR coincide
con el perfil rev246 inventariado. Los presets continúan aplazados.

## Investigación del bloqueo cifrado

[Ghidra Blackfin ya está preparado y contrastado](boot246-decompiler.md).
Se ha localizado el callback de carga y la fuente OTP de sus 16 bytes de clave,
con seis funciones descompiladas y 2.048 llamadas XTEA originales contrastadas.
No se ha recuperado la clave ni una imagen L1 descifrada; el bloqueo es ahora
más concreto y no se presenta como resuelto. La estimación global no cambia.

## Próximas entregas amplias

1. **Calibración Dr. Wave y reconstrucción híbrida.** Mantener los oráculos del
   núcleo. Contrastar el instrumento completo con capturas conocidas cuando estén
   disponibles: pitch, espectro, ganancia, transitorios, controles y tiempos ADSR.
   Sin capturas, continuar con otros bloques legibles y documentar qué
   coeficientes o rutas siguen siendo aproximaciones. La memoria descifrada ayuda,
   pero deja de ser un requisito para construir una versión funcional.
2. **Integración por bloques completos.** Cada motor incluye parámetros, interfaz,
   MIDI, guardado, tasas, pánico y callback sin heap. Una validación conjunta por
   lote antes de generar binarios. Mantener la persistencia actual sin dedicar
   este tramo a migraciones históricas que el usuario no necesita.
3. **macOS/Windows y Live.** Empaquetar artefactos nativos, completar instalación
   macOS y comprobar scanner, audio, estado, automatización y reapertura de un
   proyecto en Live en ambos sistemas. Solicitar pruebas externas solo para
   capacidades que el entorno no pueda ejecutar.
4. **Motores/FX restantes y flujo completo.** Aplicar el mismo método a FM,
   String y demás motores; filtros, efectos, modos de voz, secuenciadores y cinta
   según la matriz. Agrupar dependencias relacionadas, sin rebautizar modelos
   propios como motores originales.
5. **Etapa final de presets.** Recrear y comparar los 169 patches synth y 24 kits
   inventariados cuando sus motores/rutas estén disponibles; después recetas y
   comparación acústica. No ajustar DSP incompleto para hacer coincidir presets.

## Forma de continuar con menos intervenciones

Trabajar por resultados completos: inspección → implementación → oráculo →
composición → regresión → documentación. Reutilizar la compilación incremental y
hacer una validación conjunta por lote; regenerar instaladores solo si cambia
el instrumento ejecutable. No pedir confirmación para decisiones rutinarias ya
autorizadas. Mantener actualizaciones de progreso y preguntar solo por una
credencial, entrada o decisión imprescindible que no pueda inferirse.

El propietario autoriza en esta sesión push directo de los cambios verificados
a main; no equivale a publicar releases o reconfigurar/publicar el entorno.
Antes de cada push comprobar main remoto, trabajo concurrente, cambios propios y
resultados; conservar las contribuciones y nunca forzar el historial.
