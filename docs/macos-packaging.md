# Paquete macOS universal

El CI configura `CMAKE_OSX_ARCHITECTURES=arm64;x86_64` y ejecuta las suites en el
runner macOS. Después `packaging/macos/build-package.sh` comprueba que tanto la
aplicación como el VST3 contienen las dos arquitecturas, prepara firmas ad-hoc
locales y verifica sus bundles. Genera:

- `Pocket-One-<version>-macOS-universal.pkg`: instala la aplicación en
  `/Applications` y el VST3 en `/Library/Audio/Plug-Ins/VST3`.
- `Pocket-One-<version>-macOS-universal-portable.zip`: ambos bundles, instrucciones,
  licencia y avisos. Permite instalar el VST3 por usuario en
  `~/Library/Audio/Plug-Ins/VST3`.

El script inspecciona el payload del `.pkg` y falla si faltan los dos ejecutables.
Los archivos se adjuntan al artefacto `Pocket-One-macOS` de la ejecución de CI;
no se publica una release automáticamente. Reproducción en un Mac:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64'
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
bash packaging/macos/build-package.sh build dist
```

El instalador requiere permisos de administrador. No tiene firma Developer ID ni
notarización de Apple; macOS puede requerir autorizarlo en Privacidad y seguridad.
La inspección del paquete no sustituye una instalación y prueba de scanner,
audio, automatización y reapertura en Ableton Live real de Apple Silicon e Intel.
No se afirma haber hecho esas comprobaciones desde Linux ni ejecutado x86_64 por
haber compilado ambas arquitecturas.
