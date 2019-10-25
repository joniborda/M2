PENDIENTE
- Implementar la funcion determinarRiegoEnZonaN, que analiza el perEfectividad del ultimo censo y a su vez calcula las variaciones de Humedad Atmosferica y Luz
- Mantenimiento
. Leer variable de riego
- Integrar tareas de APP (Bluetooth).
- Establecer planes de contigencia (Por ejemplo si se desconecta la sd, el perDeRiego es una constante)
- Crear archivos con variables de riego
RESUELTO
- Borrar archivo de censos pasados en el setup.

DUDAS

PROBLEMAS

PROBAR

- Prender luces cuando el % de sea menor a tanto. (Podria estar la accion desde la APP para que envien la orden de prender o apagar manualmente)
- Borrar archivo de censos pasados en el setup.
- Guardar en archivo correspondiente a la zona el resultado del censo y su porcentaje de efectividad.
- Calcular sequedad del suelo = 100 - (Hs/1023)
- Obtener de la variable de volumen de riego y calcular en base a la Sequedad del suelo y enviarselo para regar
- Esclavo: despues de cierto tiempo de terminado el riego envia el % de humedad por cada zona.
- Tomar valor y ajustar dependiendo el resultado : SS 0 - 40 => Aumenta, 60 - 100 => Reduce.
- Esclavo: recibir % de intesidad y regar por un tiempo establecido (FIJO)
