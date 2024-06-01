# Juego 4 en línea utilizando modelo cliente/servidor

## Contenido

1. [Descripción](#descripción)
2. [Compilación y ejecución](#compilación-y-ejecución)
3. [Ejemplo](#ejemplo)
4. [Autores](#autores)

## Descripción

Los objetivos de este repositorio son utilizar diversos mecanismos relacionados con los procesos, incluyendo la creación y finalización de
procesos, y los mecanismos de comunicación. Definir e implementar un protocolo para la capa de aplicación y utilizar el protocolo orientado a la conexión TCP de la capa de transporte.

Para ello se realizó el juego 4 en línea utilizando el modelo cliente/servidor utlizando medio de sockets para manejar la comunicación.

## Compilación y ejecución

Prerequisitos:

   ```bash
   sudo apt-get install g++
   ```

   ```bash
   sudo apt-get install make
   ```
Ejecución:

1. Dentro del directorio `/server`, ejecute el siguiente comando para compilar el programa:

   ```bash
   make

2. Luego, ejecute el programa proporcionando un puerto, por ejemplo 7777:
   
   ```bash
   ./server <puerto>
   
3. Dentro del directorio `/client`, ejecute el siguiente comando para compilar el programa:

   ```bash
   make

4. Luego, ejecute el programa proporcionando la ip del servidor (ver en ifconfig) luego del puerto utilizado anteriormente:
   
   ```bash
   ./client <ip> <puerto>
   

## Ejemplo

Ejemplo de ejecución utilizando el puerto 7777 para conectar dos clientes al servidor.

![Ejemplo](https://i.imgur.com/aUGTAq3.png)

## Autores

- Diego Jiménez Muñoz
- John Valdebenido Sharp
