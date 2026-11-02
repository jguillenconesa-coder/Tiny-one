# Estudio del FM propio frente a la reconstrucción FM 246

Estudio sobre `b225b1f` (instrumento 0.26.0). No cambia el sonido del plugin.
El FM actual es un diseño propio de dos osciladores; las dependencias originales
ya contrastadas no están conectadas a él. Cambiar ratios o añadir operadores sin
recuperar su conexión y aritmética no demostraría mayor fidelidad al OP-1.

## Qué calcula hoy

En `Source/Engine.h`, `engine == 0 && !digital246`, cada voz produce:

```
ratio = 1 + round(7 * color)
beta = 7 * tone * (0.25 + 0.75 * level)
osc = sin(carrierPhase + beta * sin(modulatorPhase))
sum += osc * level * velocity * 0.16
```

Las fases avanzan con `frequency` y `frequency * ratio`, respectivamente. Es
modulación de fase sinusoidal: en régimen estacionario equivale a FM sinusoidal
con desviación `beta * modulatorFrequency`. El nombre FM es razonable; el problema
de fidelidad es la arquitectura y sus controles, no esa terminología.

| Aspecto | FM propio disponible | Evidencia del FM original 246 |
| --- | --- | --- |
| Osciladores | Dos senos en coma flotante | Cuatro subestados de operador; render todavía pendiente de reconstrucción |
| Ratios | Un ratio entero 1..8 | Tres índices de tres bits del control verde, tres bancos; significado/escalado completo pendiente |
| Topología | Una conexión fija modulador → portadora | Nueve destinos de render seleccionados por el control blanco; sus grafos completos aún no se afirman |
| Evolución interna | ADSR común escala amplitud e índice beta | Seis canales gain/angle contrastados, además de la envolvente de salida |
| Voces | Ocho; reutilización de la más antigua | Factoría de seis estados de 60 bytes contrastada |
| Arranque | Fases cero; control suavizado a 12 ms | Constructor y reset diferentes, estados/aritmética fija contrastados; integración de arranque pendiente |
| Sonido disponible | Tocabilidad y presets propios | Sin render FM original tocable ni comparación acústica |

El control Tone hace **dos trabajos**: índice beta y corte del filtro común
`900 + 14500*tone` Hz (limitado a `0.42*sampleRate`). Color cruza siete umbrales
`(k + 0.5)/7`, k=0..6. Su suavizado no convierte ese mapa en un ratio continuo:
cambia la velocidad de la fase moduladora al cruzarlos, sin reiniciar la fase.
No se ha demostrado un clic para cada cruce.

Envelope cambia el tiempo de ataque y release de la ADSR común, mientras que
`level` cambia simultáneamente volumen y espectro mediante beta. Effect controla
el eco propio, no una cuarta dimensión reconstruida del motor original.
Incluso con Drive y Effect a cero, la ruta propia contiene filtro y dos `tanh`:
una después del filtro y otra a la salida de Engine. El volumen interno también
escala esa última entrada; el procesador fija esa ganancia a 1 y aplica el volumen
maestro después del bus. El estudio aislado de Engine
no sustituye la cadena documentada en [output-routing.md](output-routing.md).

El manual suministrado, §5.2–5.4 (PDF 9), separa motor, ADSR, efecto/LFO y modos
de interpretación. Su receta Singing Birds, §16.4 (PDF 52), describe bajar el
azul hasta un seno limpio y usar mono/portamento. El FM propio alcanza un seno
en el oscilador con Tone cero, pero conserva su filtro y saturaciones; tampoco
implementa ahí los modos híbridos mono/legato/glide de Dr. Wave/Cluster. El manual
no proporciona la aritmética ni valida nuestros coeficientes.

## Medición del aliasing del modelo propio

[Herramienta reproducible](../research/fm-study/README.md): 45 casos estacionarios,
cinco notas, tres combinaciones de índice/ratio y tres tasas. La referencia es
la expansión de Bessel de **nuestra fórmula**, retirando bandas fuera de Nyquist;
no es una grabación, simulación ni espectro esperado del OP-1. [Resultados
numéricos](../research/fm-study/results.json). Se conservan los
signos de las frecuencias para sumar correctamente bandas que coinciden.

Con beta=7 y ratio=8, el residual RMS relativo al oscilador muestreado es:

| Nota / frecuencia | 44.1 kHz | 48 kHz | 96 kHz |
| --- | ---: | ---: | ---: |
| A3 / 220 Hz (MIDI 57) | −58.94 dB | −70.48 dB | < −200 dB |
| A4 / 440 Hz (MIDI 69) | −8.24 dB | −8.24 dB | −70.48 dB |
| A5 / 880 Hz (MIDI 81) | −1.73 dB | −1.73 dB | −8.24 dB |
| A6 / 1760 Hz (MIDI 93) | −0.41 dB | −0.41 dB | −1.73 dB |

Un residual cercano a 0 dB indica que las componentes plegadas son muy grandes
respecto a la señal cruda. **No es un porcentaje de energía ni de fidelidad.**
El control negativo beta=0 tiene residual menor de `1e-10` RMS en las quince
combinaciones nota/tasa. El filtro posterior puede atenuar componentes, pero no
recupera las frecuencias que ya se plegaron dentro de banda. No se presupone que
el aparato original esté libre de aliasing: eliminarlo a ciegas podría alejarnos.
La expansión completa, sin retirar bandas, reproduce la fórmula directa con
error máximo `2.54e-10` en estos casos. Dos ejecuciones independientes de la
herramienta y las capturas producen resultados JSON idénticos.

Doce capturas adicionales usan el Engine real, no una copia de la ecuación:
salida finita en 44.1/48/96 kHz. En el caso extremo MIDI 105, Tone/Color=1,
su RMS fue 0.07134 / 0.07088 / 0.05232, respectivamente, con idénticos controles
y volumen interno .7. Esto constata dependencia de tasa, incluyendo filtro y
no linealidades; no atribuye toda la diferencia al aliasing ni prueba equivalencia
entre tasas. No se ejecuta Ableton ni se valida el plugin completo en este ensayo.

## Decisión y siguiente bloque

1. Mantener identificado el FM propio como aproximación disponible. Sus seis
   presets son recetas propias; no usarlos como objetivo de calibración original.
2. Reconstruir los nueve destinos del callback FM 246, empezando por extraer sus
   grafos, orden de actualización, feedback, escalados y escrituras de scratch.
   Comparar muestras, cuatro estados de operador, seis gains/angles y memoria
   completa; incluir transiciones de selector y extremos de controles.
3. Conservar las 20 constantes legibles y declarar las 10 sustituciones. Separar
   la coincidencia bajo ese modelo de cualquier afirmación sobre el aparato.
4. Integrar después la voz de seis slots, envolvente de salida, reloj y controles
   originales en un único lote tocable. Repetir estudio de tasa y espectro sobre
   ese render. Decidir oversampling únicamente según evidencia y presupuesto RT.
5. Cerrar con CI nativo y prueba real en Ableton; la calibración acústica requiere
   una referencia del instrumento. La réplica completa de presets sigue al final.

La prioridad es recuperar la estructura y dinámica originales, antes de suavizar
o embellecer el FM propio. Este estudio mejora la base de decisiones, **no añade
un motor reconstruido**. Se mantiene la estimación de planificación **48 % ±10**;
no se interpreta como fidelidad sonora medida.
