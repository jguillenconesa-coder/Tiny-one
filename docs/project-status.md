# Estado y entregas del proyecto — 1 de noviembre de 2026

Objetivo: instrumento inspirado en OP-1 original, standalone macOS/Windows y
VST3 para Ableton Live; avanzar hacia la reconstrucción verificable de motores,
efectos y flujo. La réplica de todos los presets originales queda al final.
Esta cuenta es la coordinadora principal según la instrucción del propietario.
Las notas de coordinación entre cuentas anteriores se conservan como historial,
no como reservas indefinidas de tareas ya entregadas o sin PR abierto.

## Avance estimado: aproximadamente 33 % del objetivo completo

Es una estimación de planificación, **no una medición de equivalencia sonora**,
ni un cociente de muestras, pruebas o commits. La incertidumbre es de unos
±10 puntos: las dependencias L1 protegidas impiden estimar con precisión el
trabajo de fidelidad que falta. Se usa esta ponderación para no confundir un
prototipo tocable con una réplica completa:

| Área | Peso del objetivo | Avance estimado del área | Aportación |
| --- | ---: | ---: | ---: |
| Motores originales y sus filtros | 30 % | 25 % | 7,5 puntos |
| Efectos, envolventes y moduladores | 20 % | 30 % | 6 puntos |
| Voz, modos de ejecución y ruteo original | 15 % | 30 % | 4,5 puntos |
| Interfaz, cinta, mezcla y flujo creativo | 15 % | 60 % | 9 puntos |
| Distribución macOS/Windows y validación Live | 10 % | 55 % | 5,5 puntos |
| Todos los presets originales fieles | 10 % | 0 % | 0 puntos |
| Total | 100 % | | **32,5 ≈ 33 %** |

La base funcional de software está bastante más adelantada (orientativamente
70–75 %): VST3/standalone, MIDI, estados, interfaz, cinta y motores propios ya
funcionan. Ese porcentaje no se suma al anterior. Phase/Digital/Punch y partes
de Value/ADSR tienen reconstrucciones contrastadas; Dr. Wave/Cluster conservan
núcleos y dependencias verificadas sin integración completa. El FM actual y
varios efectos son propios. La [matriz técnica](original-op1-coverage.md) es la
fuente del estado por componente y prevalece sobre este resumen orientativo.

Hay instalador Windows y builds multiplataforma en CI. Faltan un paquete macOS
para usuario final y validación actual en Ableton real de ambos sistemas; el
CI nativo o Wine no demuestra por sí solo aceptación del scanner de Live,
latencia física o ausencia de dropouts. No se afirma que el 100 % esté cerca.

## Entrega agrupada actual

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

## Próximas entregas amplias

1. **Voz Dr. Wave de principio a fin.** Resolver conjuntamente el productor de
   pitch, relevo de pendientes ADSR y reloj; contrastar on/off, robo/reutilización,
   cambios de control, mezcla y salida antes de exponer el motor en el plugin.
   Los candidatos L1 protegidos requieren evidencia adicional; conservar los
   bloqueos ya inventariados y no repetir microauditorías sin nueva información.
   Si no se puede resolver ese tramo, completar un bloque DSP legible con salida
   y dependencias bien definidas, documentando el cambio de orden.
2. **Integración tocable y regresión.** Un solo bloque de integración por motor:
   parámetros/estados existentes, MIDI, tasas, automatización, pánico y callback
   sin heap. Una validación conjunta antes de generar binarios para usuario.
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
