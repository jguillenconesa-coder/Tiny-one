# Estado y entregas del proyecto — 1 de noviembre de 2026

Objetivo: instrumento inspirado en OP-1 original, standalone macOS/Windows y
VST3 para Ableton Live; avanzar hacia la reconstrucción verificable de motores,
efectos y flujo. La réplica de todos los presets originales queda al final.
Esta cuenta es la coordinadora principal según la instrucción del propietario.
Las notas de coordinación entre cuentas anteriores se conservan como historial,
no como reservas indefinidas de tareas ya entregadas o sin PR abierto.

## Avance estimado: aproximadamente 35 % del objetivo completo

Es una estimación de planificación, **no una medición de equivalencia sonora**,
ni un cociente de muestras, pruebas o commits. La incertidumbre es de unos
±10 puntos: las dependencias L1 protegidas impiden estimar con precisión el
trabajo de fidelidad que falta. Se usa esta ponderación para no confundir un
prototipo tocable con una réplica completa:

| Área | Peso del objetivo | Avance estimado del área | Aportación |
| --- | ---: | ---: | ---: |
| Motores originales y sus filtros | 30 % | 30 % | 9 puntos |
| Efectos, envolventes y moduladores | 20 % | 30 % | 6 puntos |
| Voz, modos de ejecución y ruteo original | 15 % | 30 % | 4,5 puntos |
| Interfaz, cinta, mezcla y flujo creativo | 15 % | 60 % | 9 puntos |
| Distribución macOS/Windows y validación Live | 10 % | 55 % | 5,5 puntos |
| Todos los presets originales fieles | 10 % | 0 % | 0 puntos |
| Total | 100 % | | **34 ≈ 35 %** |

La base funcional de software está bastante más adelantada (orientativamente
70–75 %): VST3/standalone, MIDI, estados, interfaz, cinta y motores propios ya
funcionan. Ese porcentaje no se suma al anterior. Phase/Digital/Punch y partes
de Value/ADSR tienen reconstrucciones contrastadas; Dr. Wave incorpora un motor híbrido tocable; Cluster conserva
núcleos y dependencias verificadas sin integración completa. El FM actual y
varios efectos son propios. La [matriz técnica](original-op1-coverage.md) es la
fuente del estado por componente y prevalece sobre este resumen orientativo.

Hay instalador Windows y builds multiplataforma en CI. Faltan un paquete macOS
para usuario final y validación actual en Ableton real de ambos sistemas; el
CI nativo o Wine no demuestra por sí solo aceptación del scanner de Live,
latencia física o ausencia de dropouts. No se afirma que el 100 % esté cerca.

## Entrega agrupada actual: Dr. Wave híbrido tocable (0.14.0)

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
   Sin capturas, continuar con los núcleos legibles de Cluster y documentar qué
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
