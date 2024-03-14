# Proyecto-Laberinto
Para este proyecto usaremos un algoritmo descrito por David Matuszek. Los laberintos solamente seran generados, grabados o leídos para ser desplegados, pero no seran resueltos. La interfaz debe ser gráfica. Toda la programación debe realizarse en C sobre Linux.


La interaccion con el usuario se hará por medio de interfaces graficas que deben ser desarrolladas con GTK y Glade. Ademas se debe usar Cairo para el despliegue del laberinto. La calidad estetica de la salida sera muy tomada en cuenta.


El programa tendra la posibilidad de grabar archivos con los datos particulares de cada laberinto generado, para que puedan ser cargados de nuevo. El formato de este archivo queda a discrecion de cada grupo de trabajo.


Habra botones para las siguientes funciones:


Generar: se abre una subventana que pedira la cantidad de filas y columnas del laberinto (pueden ser laberintos de hasta 2048 × 2048). Estos son campos que el usuario debe ingresar. El programa impedira el ingreso de cualquier entrada ilegal.


Leer: se carga y se despliega un laberinto previamente grabado.


Grabar: se graba el laberinto actual en un archivo. Este boton estara deshabilitado en general, pero se activa cuando se genera un nuevo laberinto.


Resolver: este boton solo dara un mensaje que diga “No implementado”.


Salir: se termina la ejecucion del programa.


Independientemente del tamano solicitado del laberinto, este sera desplegado completamente en la interfaz (puede que solo se vea una mancha negra...). Sin alterar el tamano del area de la interfaz dedicada al despliegue del laberinto, el usuario podra hacer acercamientos o alejamientos del laberinto (Zoom in/ Zoom out). Se puede llegar hasta a ver una sola celda del laberinto o hasta un despliegue total del laberinto. Tambien podra mover el area visible del laberinto hacia arriba, abajo, izquierda o derecha. Toda esta interaccion debe ser controlada con el mouse. Investigue las capacidades de GTK y de Cairo al respecto. Obviamente, esta funcionalidad solo tiene sentido si hay un laberinto desplegado.


Todo el codigo debe estar escrito en C (no C++). Todas las interfaces deben ser graficas. Se debe usar GTK, Glade y Cairo. No debe dar “Segmentation Fault” bajo ninguna circunstancia.

**Trabajos adicionales**
- [ ] Hacer que el piso y las paredes del laberinto tengan alguna textura apropiada.
- [ ] Implemente adicionalmente laberintos donde la casilla basica es un hexagono en vez de un cuadrado.


**TODO LIST**
- [x] Función Leer
- [x] Función Grabar
- [x] Botón Generar
  - [x] Subventana para tamaño (max 2048x2048)
- [x] Subventana de Botón Resolver
- [x] Cairo
  - [x] Dibujar Laberinto
  - [x] Zoom in/out (con mouse)
  - [x] Mover área visible (con mouse)
- [x] Lógica
  - [x] Struct celda con estado y barrera
  - [x] Matriz de celdas
  - [x] Array de punteros a celdas frontera
  - [x] Función generarLaberinto (algoritmo Matuszek)
    - [x] Función eliminarBarrera
    - [x] Función agregarFronteras
    - [x] eliminar Frontera
    - [x] Función direccionOpuesta
    - [x] Función tieneVecino
    - [x] Función posicionVecino
  - [x] Usar malloc
  - [x] Liberar memoria
- [x] Agrandar interfaz
