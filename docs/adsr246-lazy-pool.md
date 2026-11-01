# Rev246: arranque diferido ADSR y cadena completa de eventos sin transporte

La función completa `0x0191264c..0x019126d8` devuelve uno de los seis estados
ADSR de 24 bytes en `0xff8000d4`. Antes comprueba la **palabra completa** de
ready en `0x01b6053c`. Si es cero escribe 1 e inicializa **los seis estados**, no
solo el solicitado, mediante el constructor `0x019120ba` y los helpers de array
`0x01aa3862` / `0x01aa37c4`. Con cualquier palabra no nula conserva todos los
estados y el ready exacto; se comprueban también 0x100, 0x10000, 0x80000000 y
0xffffffff. No es un booleano de un solo byte.

La inicialización borra las primeras cinco palabras de cada estado y escribe
32767 únicamente en la mitad baja de ganancia. Conserva su mitad alta, los
vecinos y scratch. El retorno original es `base + indice * 24`. La reconstrucción
independiente está en `Source/Adsr246Pool.h`, sin asignación dinámica.

El ensayo original solo admite índices 0–5. Un índice inválido llega al helper
de assert **después de una posible inicialización**. El adaptador devuelve
nullptr antes de mutar: es una protección propia, no una emulación de ese error.
No se presenta este accessor como asignador de voces o consumidor de pendientes.

## Un bloque de validación de principio a fin

Se integra el trabajo anterior `ec1c182` de notificaciones con metadata original
([contrato](voice246-quiet-note-lifecycle.md)), que no estaba en main. Se reproduce
su oráculo sin regenerar fixtures. Se añade el acceso diferido a los helpers
existentes en el punto donde el original obtiene una envolvente. Sus llamadores
con pool preparado mantienen el comportamiento anterior; pueden suministrar
opcionalmente la palabra ready para contrastar el arranque diferido.

Dos ensayos complementarios ejecutan instrucciones originales sin parchearlas:

| Ensayo | Cobertura y resultado |
| --- | --- |
| Accessor ADSR completo | 1.024 llamadas: 512 independientes + ocho secuencias de 64; 200 arranques y 824 accesos a pool listo; seis índices y 682 mutaciones explícitas entre llamadas para comprobar conservación. |
| Constructor → on/off → notificación → ADSR | 1.536 eventos / 12 secuencias, selectores 1–3, metadata 1 y salida externa explícitamente cero; pool y ready retenidos entre eventos. |

La segunda prueba compara 4.760 bytes de regiones seleccionadas antes/después
del render, argumentos, copia local, guardas y muestras. Hay 432 inserciones,
192 gates, 144 duplicados, 432 retiradas, 336 ausentes y 999 cambios de mapa.
**153.472 posiciones ADSR se escriben y 43.136 se conservan**. La variante previa
con pool listo escribe 156.032: no se sustituyen sus fixtures por los nuevos.

Los perfiles de inicio son ready 0/65536/0xffffffff/0, repetidos para cada selector.
En tres secuencias con gate activo no se alcanza el accessor: ready permanece cero
hasta el final. En las otras tres inicialmente frías cambia a 1 una sola vez.
Los otros seis perfiles conservan su palabra no nula exacta. No se recarga ready
ni se limpia actividad después de cada evento; retirar la última nota sigue sin
hacer libre una voz. Orden, gate por evento, controles, afinación, elección de
voz para render y palabra de salida cero son entradas explícitas del harness.

El ensayo standalone captura dos regiones: ready y vecinos (12 bytes), pool y
vecinos (160 bytes), más el puntero de retorno. Conserva estado entre llamadas
y permite mutar una palabra sintética antes del siguiente acceso. Esto no afirma
observar RAM/CPU completas ni concurrencia del aparato. Los helpers existentes
siguen teniendo sus límites; no se amplía la integración a voces polifónicas 0
ni a todos los contextos iniciales posibles de lista no vacía.

## Evidencia y reproducción

GNU BF524 se reconstruye desde las fuentes verificadas con ambos parches de
writeback ya documentados. Los preflight halfword y las watches PC propias
(0/5/5/5) pasan; las ejecuciones originales evitan asserts, transporte y L1.
Cada nuevo oráculo se reproduce en otro directorio privado sin `--record`, con
coincidencia byte a byte. El oráculo previo de notificación también coincide.

REA 6.8.0 / Node 24.22.1 inspecciona el símbolo sintético `original_adsr_get`
mediante GNU objdump: salida completa, código 0, sin truncamiento y digest del
ELF sin cambios. Ese símbolo solo delimita la función observada. La inspección
estática no descifra L1 ni sustituye la comparación numérica. El informe completo,
ELF, firmware y desensamblado se mantienen fuera del repositorio; se guardan
únicamente resumen de procedencia y hashes en la evidencia numérica.

```bash
source /workspace/tiny-one-blackfin/blackfin-env.sh
python3 scripts/verify-adsr246-pool.py \
  --ldr /privado/OP1_vdk.ldr --assembler "$POCKET_BFIN_AS" \
  --linker "$POCKET_BFIN_LD" --simulator "$POCKET_BFIN_SIM" \
  --testutils "$POCKET_BFIN_TESTUTILS" --output /privado/pool-nuevo
python3 scripts/verify-voice246-quiet-note-lifecycle.py --cold-pool \
  --ldr /privado/OP1_vdk.ldr --assembler "$POCKET_BFIN_AS" \
  --linker "$POCKET_BFIN_LD" --simulator "$POCKET_BFIN_SIM" \
  --testutils "$POCKET_BFIN_TESTUTILS" --output /privado/eventos-frios-nuevo
```

`--record` solo se usa para crear deliberadamente los fixtures numéricos propios;
no se necesita para repetirlos. Cada destino debe ser nuevo y externo al checkout.
[Preparación](new-account-dsp-setup.md) ·
[Evidencia pool](firmware/adsr246-pool-validation.json) ·
[Evidencia cadena](firmware/voice246-cold-note-lifecycle-validation.json).

Pasan **61 suites CTest Linux con plugin**, compilación VST3/standalone y los dos
comparadores con ASan/UBSan; también la variante previa de eventos bajo sanitizadores.
LeakSanitizer desactivado por la restricción ptrace del entorno. La ejecución
nativa Windows/macOS se comprueba por CI, separada de estos resultados Linux.
No se afirma una prueba física en Ableton, ASIO ni hardware.

## Continuación del instrumento

Se elimina la precondición de pool siempre preparado en esta cadena acotada.
Siguen pendientes productor del buffer de pitch, consumidor L1 de los campos
ADSR, cadencia, mezcla/salida e integración Dr. Wave en el instrumento.
No se conecta una fórmula inventada para presentarlo como motor original.
Este bloque no cambia el audio del plugin ni requiere otro instalador; presets,
IDs de automatización, sesiones y bancos se conservan.
