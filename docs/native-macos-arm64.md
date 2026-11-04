# Validación nativa en Apple Silicon — 0.43.0

La revisión `b382b23` pasa en un runner **macOS 26 ARM64**, sin Rosetta:
**126/126 suites del producto y 2/2 suites de investigación de Envelope**.
[CI completo](https://github.com/jguillenconesa-coder/Tiny-one/actions/runs/41831123606) · [Evidencia estructurada](native-macos-arm64-validation.json).

Se carga el bundle VST3 real mediante CoreFoundation y se verifican
`bundleEntry`, `GetPluginFactory`, la información de sus tres clases y
`bundleExit`. La aplicación standalone real permanece activa y crea cinco
ventanas de aplicación observables durante la comprobación de ocho segundos.
El probe termina después su propio proceso hijo. Estas comprobaciones complementan
los ensayos de procesador, editor, estado y audio de la suite completa.

## Arquitectura y alcance

El [workflow dedicado](../.github/workflows/native-macos-arm64.yml) exige
`uname -m` y Python nativos ARM64, rechaza una traducción Rosetta declarada y
comprueba que los ejecutables Mach-O del VST3 y standalone contienen únicamente
la arquitectura arm64. Una compilación universal sin ejecución no basta para
superar estas comprobaciones.

La suite realtime también pasa sus 894 cargas y las guardas existentes de
reservas/liberaciones C++. La intercepción de malloc/calloc/realloc/free es
específica de Linux; no se atribuye esa cobertura a macOS. Los tiempos en CI
compartido no certifican latencia física ni ausencia de dropouts.

No se certifican Ableton Live, dispositivos de audio, fidelidad acústica,
instalación nativa del paquete, firma/notarización ni todas las versiones
anteriores de macOS. La inspección de los instaladores universales sobre Intel
permanece como evidencia separada. El producto sigue en **0.43.0**, sin cambios
de DSP, nuevas releases ni configuración del entorno en la nube.

## Repetición y procedencia

El workflow compila una vez el producto y las dos suites de Envelope, ejecuta
CTest y los probes propios de [research/native-macos](../research/native-macos).
Los resultados JUnit, observaciones de arquitectura y métricas realtime quedan
como artefactos del CI. El JSON enlazado identifica el commit, artefacto y hash.

Los cambios al workflow o sus probes activan esta comprobación dedicada. Se
puede ejecutar manualmente para otra revisión mediante workflow_dispatch; no se
repite automáticamente con cada cambio de DSP. La evidencia corresponde a la
revisión indicada y debe renovarse para validar código posterior.

El intento inicial con macOS 15 se canceló mientras esperaba runner, sin iniciar
una compilación. La imagen macOS 26 sí obtuvo capacidad y completó las pruebas.
El rastreo de alias de Element no localizó un productor activo adicional:
el llamador, controles y cadencia de Envelope siguen pendientes y el módulo de
investigación continúa fuera del plugin.
