CATATUMBO electronics  
2025 

Puya 

Secuenciador modular euclidiano polifónico de cuatro voces con interfaz gráfica generativa dinámica 

Puya es un secuenciador euclidiano polifónico avanzado para VCV Rack 2, que ofrece 4 
voces independientes y hasta 32 steps por voz. Diseñado para crear patrones rítmicos 
complejos y evoluciones musicales dinámicas. 

Especificaciones Técnicas 

● Desarrollo: C++
● Entorno: MSYS2 MinGW x64
● Capacidad: 32 steps máximos 
● Polifonía: 4 voces independientes 
● Presets (1): Culo e puya.vcvm 
● Display: 4 canales con visualización de patrones 
● Control de voltaje para todos los parámetros (±5V) 

Controles 

● Hits Knob: designa la cantidad de eventos/hits en la secuencia 
● Shift Knob: selecciona el beat inicial del ritmo 
● Rotation Knob: rota el punto inicial de la secuencia 
● Accent Rotation Knob: control de rotación para la secuencia de acentos 
● PAD Knob: control de PADs de sistema 
● Accent Knob: control de acentos 
● VOICE_PARAM Knob: selecciona el índice de las voces (1-4) 

Estilos de patrones: 

● Euclidean 
● Linear 
● Fibonacci 
● Random 
● Cantor  

Estilos de compuerta: 

● Gate: señal de compuerta estándar 
● Accent: compuerta con velocidad/intensidad aumentada 
● Fill: disparo completo al inicio de la secuencia 
● Guardado y Recuperación de Parámetros 

El sistema de guardado y recuperación JSON permite preservar y cargar configuraciones 
completas del módulo. Cada voz mantiene sus parámetros de forma independiente, 
incluyendo: 

● Número de hits 
● Rotación 
● Patrón de acentos 
● Configuración de PADs 
● Estado de sincronización 

Display y Visualización 

El display del Puya ofrece una representación visual dinámica de los patrones euclidianos en 
tiempo real. Cada voz tiene su propia visualización generativa que muestra el patrón actual 
en forma de 'collar rítmico' (rhythm necklace). La interfaz se actualiza en tiempo real con un 
buffer de un ciclo, proporcionando retroalimentación visual inmediata de los cambios en los 
patrones. 

Sincronización y Feedback Visual 

El botón SYNC proporciona sincronización precisa entre voces, garantizando que cada 
una de las secuencias comience en el primer hit y ejecute un fill completo al iniciar.  
Las luces LED proporcionan feedback visual con colores específicos para cada voz, con un decay rate de 
10.0f para una respuesta suave y natural. 

Conexiones 

Inputs 

● Clock Input: entrada de reloj principal 
● Sync Input: entrada de sincronización externa 
● Random CV input: afecta todos los parámetros, sumando el valor del parámetro actual + el voltaje aleatorio de entrada 
escalado. 
● CV Inputs: control de voltaje para todos los parámetros 

Outputs 

● Gate Output: salida principal de compuerta 
● Accent Output: salida de acentos 
● Clock Output: salida de reloj procesado 
