# Bend: ganancia y pitch de voz — 0.47.0

En la página LFO selecciona **Bend** y cambia Green a **Voice**. Blue elige
**Level** o **Pitch**; White conserva Amount bipolar y Orange mueve la entrada
manual. Reverse conserva su función. Son rutas de voz para los diez motores
melódicos nativos, incluido Sampler 246. Drum, D-box y motores heredados muestran
OFF. No es el pitch wheel MIDI ni una ganancia maestra.

El destino, target, cantidad, entrada y Reverse son automatizables y se guardan
con proyectos, slots y clipboard. El coeficiente vivo y la captura de bloque se
reinician con el lifecycle del host; no se serializan como parámetros. La fase
manual sigue siendo global y no se reinicia por nota.

## Operaciones originales contrastadas

Se amplía el estudio del callback synth `0x019c9d44`: kind 0, destino 3, colores
0/1. Hay **6.144 llamadas originales adicionales**, con 4.096 synth y 2.048 de
rechazo drum, tipos/kinds/targets válidos y rechazados, estados cargados y
retenidos, prepare/reset y una repetición independiente idéntica. No se cambia
el oráculo previo de parámetros de Bend. Hay 696 llamadas aceptadas (356 Level y
340 Pitch), con 89.088 pares de pitch/ganancia; las otras 5.448 se rechazan.

Se comparan tag, ocho palabras del estado compartido, dos palabras de historial,
display/flag, ocho controles, ocho targets, retorno, **128 pitch words, 128 gain
halfwords y 128 FX audio halfwords**. Los buffers se inicializan explícitamente
con patrones numéricos por llamada; no son productores de voz recuperados. El
puente por bloque también se compara con todos los bloques originales aceptados.

Para Level, con nivel manual `l` entre 0 y 127:

1. `delta = Q15(l × 128, Amount)`.
2. Se suma 16384 con saturación y se filtra: `Q15(target, 4096) + Q15(previous, 28672)`.
3. Se conserva el coeficiente en el halfword bajo de `0x01dd591c`, sin borrar
   flags del halfword alto.
4. Cada gain se multiplica por el coeficiente y por dos con saturación.

Amount cero conduce hacia ganancia unidad, con una rampa si el coeficiente inicial
es cero. El aumento de gain puede saturar un buffer ya a fondo de escala; no se
presenta como un amplificador ilimitado del audio final.

Para Pitch, el factor toma `l × 256` cuando Amount es no negativo y `l × 32` cuando
es negativo; después aplica Amount Q15. Cada pitch word recibe su valor original
más siete veces el producto fraccionario, respetando el orden de saturación de
las sumas. Se conserva esa asimetría; no se reemplaza por una curva exponencial
propia ni se reutiliza la curva cúbica del MIDI LFO.

El estudio usa el simulador GNU BF524 con los tres parches de writeback y probes
ya documentados. Contrasta las rutinas originales bajo esos datos y modelo, no
hardware ni la voz completa. FX audio especial, FX kind 4 y otros auxiliares
siguen fuera de esta ampliación.

## Composición en el host

El estado original del coeficiente es compartido. El frontend del plugin lo
actualiza una vez por tick propio `44100/64` cuando Voice Level está seleccionado;
las voces capturan ese valor al iniciar su bloque de 128 muestras internas a
88,2 kHz. No se avanza un coeficiente independiente por voz. El original puede
recibir un orden de llamadas distinto: **no se ha recuperado su scheduler**.

La captura de target/cantidad/factor pendiente, activación, inicialización y pánico
son políticas propias. Retirar la ruta desactiva el puente; los cambios activos
se capturan al siguiente bloque. Cambiar a Pitch conserva el coeficiente de Level
sin seguir avanzándolo, hasta volver al destino correspondiente. Desactivar Bend
reinicia el frontend. Las notas nuevas conservan la fuente manual compartida.

Phase y Digital consumen pitch por muestra. Los motores bloqueados capturan el
factor con su siguiente bloque; FM/String/DNA conservan su pitch único por
callback. El gain se compone en el punto del buffer de envolvente, antes del
filtrado de salida; D-synth lo recibe dentro de su buffer gain anterior al núcleo.
Se mantienen los límites de pitch y demás adaptadores de cada motor. No se
recuperan por ello el productor original de ganancia, allocator ni ruteo completo.

Bend conserva su prioridad frente a MIDI/Random/Tremolo/Value y queda por debajo
de Element/Crank. Esta ruta no altera la curva MIDI por canal existente.

## Pruebas y siguientes pasos

[Resultados](releases/v0.47.0-validation.json). Las pruebas cubren captura retenida,
coeficiente compartido, rechazo de drum/legacy, siete tasas, diez motores,
igualdad exacta 64/257, cambios de entrada posteriores a su frontera, niveles
parciales para observar el aumento sin saturación, Amount de ambos signos y cero,
Mono/Legato/glide y pánico. El plugin contrasta persistencia y las rutas reales.

```
cmake -S research/bend-voice -B build-bend-voice -DCMAKE_BUILD_TYPE=Release
cmake --build build-bend-voice --config Release --parallel 2
ctest --test-dir build-bend-voice -C Release --output-on-failure
```

No hace falta firmware para estas pruebas. La reproducción original privada usa
`scripts/verify-bend-voice246.py` con el LDR y herramientas ya documentados; salidas
nuevas fuera del checkout. `--record` guarda solo entradas y observaciones numéricas.
Quedan ruteo/voz/productores completos, especiales de los demás LFO, Pulse/L1,
acústica, Live real, firma/notarización y todos los presets originales al final.
