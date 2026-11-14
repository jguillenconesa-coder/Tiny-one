# Dr. Wave: reinicio Mono y retirada con cola — 0.58.0

**Mono** de Dr. Wave usa ahora el servicio de reinicio diferido: conserva el
nivel y el DSP anteriores hasta la rampa, entrega los registros pendientes y
vuelve al ataque. La retirada de todas las voces Dr. Wave deja terminar los
filtros de salida y el SRC después del final de la envolvente. No hay controles
nuevos ni cambio de formato de sesión.

## Reinicio Mono

El [servicio polifónico de 0.57.0](drwave246-voice-restart.md) se reutiliza con
una política de dinámica distinta. En las [ramas activas de selectores 1/3](drwave246-note-modes.md),
la velocidad registrada y la ganancia por nota se actualizan al solicitar el
reinicio. La rampa ADSR y las palabras son contrastadas. Sus identidades
comerciales no se deducen de esos números; se usa esa aritmética en nuestro
modo Mono, sin afirmar que sea el selector comercial original.

Al siguiente límite interno, el adaptador solicita etapa 6 y actualiza volumen y
ganancia para la nota nueva, conservando el halfword alto de ganancia. Durante
la rampa mantiene el pitch previo. Al llegar a etapa 7, entrega el pitch y
reinicia Core usando el mismo consumidor propio de 0.57.0. La rama Poly sigue
conservando la ganancia anterior. Legato conserva su política de solapamiento.

La prioridad continúa siendo la última nota retenida por el host. Si se libera
una nota todavía pendiente y queda otra retenida, Mono sustituye el destino
pendiente por esa nota y solicita su reinicio; si no queda ninguna, cancela el
ataque y libera la voz. Sustain y bend siguen al canal de la nota prioritaria,
incluido el canal interno 17. Una nota nueva durante una liberación puede usar
la voz todavía activa mediante la rampa; no se corta su cola inmediatamente.
Glide conserva su trayectoria propia en tiempo del host, con pitch audible
cuantizado por los bloques internos y la entrega pendiente.

El cambio es audible frente a 0.57: el nivel no cae inmediatamente a cero al
recibir cada note-on Mono. No se introduce otro relevo flotante de 3 ms encima
de la rampa. Value/Random reciben la notificación al cambiar de propietario;
su cadencia y la del servicio continúan siendo propias.

## Retirada por voz

Antes, Engine retiraba Dr. Wave al observar etapa cero en la envolvente. Esa
etapa podía proceder de una muestra interna calculada por adelantado para el
SRC; los filtros de salida todavía podían contener señal.

Ahora el adaptador conserva la voz hasta observar **64 muestras internas
consecutivas con salida menor que 1e-7 en valor absoluto**, con una cota de
**2.048 muestras internas** desde el final de la ADSR. Después la finalización
queda enclavada hasta una petición nueva. Son umbral, ventana y cota propios;
no se atribuyen al servicio original. El SRC conserva su interpolación y los
filtros siguen decantando ceros procedentes de la envolvente inactiva.

Una petición nueva durante esa cola revoca la retirada antes de ejecutar su
rampa. Una petición pendiente impide liberar la voz prematuramente. Panic,
restauración y cambio de modo/motor siguen vaciando voces y peticiones.
El tiempo de cola anunciado al host incluye la cota de retirada y el margen de
SRC. No cambia la latencia declarada del plugin ni se mide latencia física.

## Evidencia y límites

[Validación](releases/v0.58.0-validation.json): la nueva prueba compone las
palabras Mono y la rampa con el modelo ADSR contrastado. Comprueba ganancia alta,
volumen, pitch previo durante la rampa y entrega de los registros. No son
extracciones originales nuevas. A seis tasas (8/44,1/48/88,2/96/192 kHz) comprueba
retirada posterior al final ADSR, salida residual por debajo del umbral de prueba,
reutilización durante la cola, fallback, sustain y Panic.

La prueba del procesador se amplía a **27 escenarios Poly/Mono/Legato**, con
callbacks 64/257 y A/B de retirada del dueño anterior: 81 renders en total.
Exige audio idéntico entre particiones y silencio exacto tras All Sound Off,
además de comprobar el margen de cola anunciado al host. El resto de pruebas,
sanitizadores y CI se registran en la evidencia de entrega.

Selector, contador, solicitud y kernel ADSR tienen evidencia acotada. Prioridad,
clock, consumidor pendiente, reset Core, cancelación, retirada y salida siguen
siendo híbridos. No se ha recuperado L1 ni medido equivalencia acústica. Los demás
motores mantienen su gestión de voz. No se generan instaladores.
