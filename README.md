Demostración (Rack 2) [Ensamble de Culo e' Puya de Curiepe, 4 voces]: https://drive.google.com/file/d/1CecR6O2b5bJ4rIqYWFKHopivE34DJa-A/view?usp=drive_link

Nomenclatura de collares euclidianos 1-32 steps, posición 0: https://drive.google.com/file/d/1FwlpG91Kuk1gEuP0kHfU8A4NSJ3NVVz_/view?usp=drive_link

San Juan Bautista, Curiepe (2024): https://drive.google.com/file/d/12xCgV9dfaC-cLrsgkTJq929jWCk1ChXU/view?usp=drive_link

CATATUMBO electronics  
2025 

Puya Versión 2.1.1

Secuenciador modular euclidiano polifónico de cuatro voces con interfaz gráfica generativa dinámica 

Puya es un secuenciador euclidiano polifónico avanzado para VCV Rack 2, compilado en Rack 2 versión 2.6.3, que ofrece 4 
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

Puya Preview:

![Prima](https://github.com/user-attachments/assets/8860dc0f-0242-46bc-923c-d11e03e69d6e)
![Cruzao](https://github.com/user-attachments/assets/9720e04c-511b-43c9-8190-31290e662dbd)
![Pujao 1](https://github.com/user-attachments/assets/a8e71ee3-594a-431d-85fe-b1e0c7103835)
![Pujao 2](https://github.com/user-attachments/assets/804b9979-94e1-4e81-a6cb-d4853332a496)

