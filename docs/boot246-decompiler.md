# Rev246: Ghidra Blackfin y procedencia de la clave de carga

**Ghidra ayuda a analizar el descifrado; no convierte ciphertext en código sin
su clave.** Se ha preparado y usado Ghidra 12.2 con el módulo Blackfin 1.2.0 de
[geepot](https://github.com/geepot/ghidra-blackfin). El módulo aporta la arquitectura
que Ghidra no incluye de serie. JDK Temurin 25.0.5+8, Ghidra y la extensión se
obtuvieron de sus releases oficiales y se comprobaron sus SHA256 antes de usarlos.
No se cambia la configuración publicada del entorno ni la del cliente.

## Resultado nuevo: el callback de carga y su fuente de clave

El `te-boot.ldr` del contenedor rev246 contiene **dos fases superpuestas**. Se
han reconstruido en proyectos distintos para no reemplazar las instrucciones de
inicialización por la imagen runtime que se carga después en las mismas direcciones.
La entrada privada tiene SHA256
`f6233b974bdb6c92f39e73401f206721924ef3ce52abd66b642a5350cebcad93`.

| Rutina | Observación |
| --- | --- |
| Inicializador `0xffa00000..0xffa00194` de la primera fase | Pone a cero el estado de carga, llama al lector con índice **0xd0** y dos destinos en offsets 0x10/0x18 del contexto. Intercambia las palabras 0↔1 y 2↔3 dentro de esos 16 bytes. Puede registrar el callback `0xffa1007e`, según las condiciones originales. |
| Lector `0xffa00514` | Llama a ROM `0xef000018` para preparar/finalizar, y a `0xef00001a` con el índice y flags 0/1 para llenar las dos mitades de salida. Comprueba bits de error. |
| Callback `0xffa1007e` | Obtiene la clave de contexto+0x10, alcanzado mediante el puntero en `0xff804004`. Procesa ventanas de ocho bytes con paso de 24, conserva estado entre buffers y tiene retornos deshabilitado/sin buffer. |
| Primitiva optimizada `0xffa10000..0xffa1007e` | XTEA con bucle hardware; el callback suministra 32 rondas. |
| Primitiva runtime `0xffa0bee6..0xffa0bf8a` | Otra implementación de XTEA en la fase final de te-boot, alcanzada por el importador de datos de licencia de `0x000001da`. No se confunde ese importador con el callback de aplicación. |

Las entradas ROM están identificadas por las cabeceras públicas de U-Boot para
Blackfin: `_BOOTROM_OTP_COMMAND = 0xef000018` y
`_BOOTROM_OTP_READ = 0xef00001a`. [Referencia](https://github.com/u-boot/u-boot/blob/v2014.10/arch/blackfin/include/asm/mach-common/bits/bootrom.h).
Por tanto, la ruta observada **lee OTP del dispositivo**, no una tabla de cuatro
constantes del fichero. No se ha ejecutado esa lectura ni recuperado sus valores.
Esto no demuestra una búsqueda exhaustiva de toda posible copia externa de la clave.

La dependencia ahora es concreta: disponer legítimamente de esos 16 bytes de
OTP/clave preparada, o de una captura de la RAM de aplicación ya descifrada que
pueda validarse. El descompilador por sí solo no proporciona ese dato. No se hace
un ataque de fuerza bruta, no se flashea ni se modifica un dispositivo.

La imagen L1 runtime de te-boot tiene el mismo tamaño que la de aplicación, pero
**no es su plaintext intercambiable**: solo coinciden 929 de 32.724 posiciones
clasificadas como claras bajo el perfil comunitario de grupos completos. No se
copia esa otra imagen para rellenar instrucciones protegidas. Los detalles de
fragmentos/tails del callback tampoco se extrapolan como una implementación
completa del cargador ROM sin validarlos.

## Descompilación contrastada, no solo pseudocódigo plausible

Se producen seis descompilaciones privadas: inicializador, lector ROM, primitiva
optimizada, callback, primitiva runtime e importador de licencia. En los epílogos
de primera fase que restauran P0, hacen UNLINK y JUMP(P0), se anotan retornos en
la base de análisis. La anotación exige que las instrucciones coincidan con el
patrón observado; **no cambia los bytes del ELF**. Corrige la interpretación de
retornos indirectos del proveedor, no las instrucciones originales.

Se ejecutan ambas primitivas originales aisladas en GNU BF524:

- **1.024 entradas sintéticas / 2.048 llamadas originales** con 0/1/2/16/32/64
  rondas, palabras de 32 bits, claves sintéticas y guardas.
- Ambas coinciden con una reconstrucción aritmética independiente de XTEA;
  se compara el registro entero, incluidos clave, guardas y campos no escritos.
- Las dos funciones aritméticas descompiladas se compilan **solo privadamente**
  para el host y se comparan con el original: otras **2.048 comparaciones**.
  Se adapta la anchura del puntero de clave y se define wrap signed mediante
  `-fwrapv`. No se introduce ese pseudocódigo en el producto.
- La reproducción completa vuelve a extraer desde el contenedor verificado,
  crea proyectos Ghidra nuevos, comprueba las seis salidas y repite esos contrastes.

No se ejecuta el boot completo, MMIO, OTP, el callback con datos reales ni código
L1 cifrado. Las claves de prueba no permiten descifrar el firmware. Los modelos
GNU/p-code no certifican todo el procesador ni temporización física.

**Resultado: herramientas de descompilación útiles y ruta de clave localizada;
L1 de aplicación todavía sin descifrado validado.** Dr. Wave/FM/Cluster no se
marcan integrados por este resultado. No cambia el audio del plugin ni los presets.

## REA y soporte efectivo

REA 6.8.0 sigue sirviendo para Evidence con GNU objdump. Su operación nativa
`decompile` rechazó este ELF Blackfin con `unsupported ELF architecture` antes
de abrir Ghidra, incluso seleccionando su lenguaje explícitamente. No se falsea
el encabezado ELF ni se cambia la arquitectura para superar esa admisión.
Se usa directamente Ghidra headless con el módulo Blackfin; no se presenta el
intento REA rechazado como una descompilación suya completada.

## Reproducir

Requisitos: [herramientas GNU](new-account-dsp-setup.md), `cc`, Ghidra 12.2,
Blackfin 1.2.0 y Temurin JDK 25.0.5+8. Se conservan fuera del checkout. Estas son
las descargas verificadas en esta sesión:

| Artefacto oficial | SHA256 |
| --- | --- |
| [Ghidra 12.2 PUBLIC](https://github.com/NationalSecurityAgency/ghidra/releases/download/Ghidra_12.2_build/ghidra_12.2_PUBLIC_20261016.zip) | `2c2cd776db3c1f6a097ba418922378ac3649cadd5e844a6f5f088ae628a7be67` |
| [Blackfin 1.2.0 para 12.2](https://github.com/geepot/ghidra-blackfin/releases/download/v1.2.0/ghidra_12.2_PUBLIC_Blackfin.zip) | `28fffb3f7f2c34ba09794865e58f832af546954cfacd1ee801a36443ef1be98a` |
| [Temurin 25.0.5+8 Linux x64](https://github.com/adoptium/temurin25-binaries/releases/download/jdk-25.0.5%2B8/OpenJDK25U-jdk_x64_linux_hotspot_25.0.5_8.tar.gz) | `68982529869257d0ee6b824ec5aa270ff8b118bb438670da0b97ebdab178d7fb` |

Antes de extraer cualquier descarga, comprueba su hash. La extensión verificada
se instala en `Ghidra/Extensions/Blackfin` del directorio Ghidra local. No se
registran plugins/MCP ni se instala software en el sistema. La prueba usa un
home Java privado y dos CPU, sin cambiar HOME ni el JDK del proyecto.

```bash
source /workspace/tiny-one-blackfin/blackfin-env.sh
python3 scripts/inspect-op1-boot-decompiler.py \
  --firmware /privado/op1_246.op1 --output /privado/decompiler-nuevo \
  --ghidra /workspace/ghidra-tools/ghidra_12.2_PUBLIC \
  --jdk /workspace/ghidra-tools/jdk/jdk-25.0.5+8 \
  --assembler "$POCKET_BFIN_AS" --linker "$POCKET_BFIN_LD" \
  --simulator "$POCKET_BFIN_SIM" --testutils "$POCKET_BFIN_TESTUTILS"
```

Salida nueva, privada y externa al repositorio. El script valida contenedor,
CRC, LZMA, miembro boot y estructura LDR. Solo extrae el boot necesario y mantiene
sus fases separadas. Exige marcadores de éxito de cada descompilación, además del
exit code. Guarda pseudocódigo, ELF, imágenes y resultados originales únicamente
bajo ese destino privado. [Resumen numérico y procedencia](firmware/boot246-decompiler-validation.json).

No se publican firmware, pseudocódigo original, claves, datos OTP ni desensamblados.
Ghidra es Apache-2.0; la extensión Blackfin, GPL-3.0-or-later; Temurin, GPLv2 con
Classpath Exception. Las herramientas no se enlazan con el instrumento.
