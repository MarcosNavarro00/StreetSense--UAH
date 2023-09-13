# StreetSense: Sistema de Monitoreo y Predicción en Tiempo Real de Datos de una Vía Pública

![StreetSense Logo](https://example.com/streetsense-logo.png)

Este repositorio es el hogar del proyecto StreetSense, un sistema completo de monitoreo y predicción en tiempo real de datos de una vía pública. StreetSense utiliza una combinación de tecnologías y herramientas para capturar, comunicar, analizar y presentar información vital sobre el tráfico y la seguridad en las vías públicas.

## Descripción del Proyecto

StreetSense se ha desarrollado para abordar la necesidad de un sistema eficiente y en tiempo real que monitorea y predice el flujo de tráfico, la seguridad peatonal y otros datos cruciales en una vía pública. El proyecto abarca varios componentes clave:

### Captación de Información
La captación de información se lleva a cabo utilizando una ESP8266 y una serie de sensores de presión conectados a un Arduino. Estos sensores recopilan datos esenciales sobre el tráfico vehicular y peatonal en la vía pública.

### Comunicación
La comunicación entre los dispositivos de monitoreo y el sistema central se realiza a través de la tecnología MQTT. Este protocolo garantiza la transmisión confiable de datos y permite determinar el estado actual de los semáforos en tiempo real.

### Algorimto del Funcionamiento de los Semáforos
Para determinar el color de cada semáforo en cada instante de tiempo de manera óptima, hemos desarrollado un algoritmo en Python. Este algoritmo utiliza datos de tráfico en tiempo real y patrones históricos para tomar decisiones precisas sobre la sincronización de semáforos.

### Almacenamiento de Datos
La información recabada se almacena en la nube utilizando Firebase Database RealTime. Esto garantiza que los datos estén disponibles de manera segura y en tiempo real para su análisis y visualización.

### Predicción con Red Neuronal
La predicción tanto de peatones en la vía pública como de vehículos se realiza a través de la red neuronal Brain.js. Esta tecnología permite analizar y predecir el comportamiento de tráfico y seguridad con precisión.

### Interfaz de Usuario
Se ha desarrollado una página web con un servidor utilizando Node.js para mostrar toda la información resultante de manera accesible y fácil de entender. Los usuarios pueden acceder a los datos de tráfico en tiempo real y las predicciones de seguridad.

## Contribuciones
¡Estamos emocionados de recibir contribuciones de la comunidad! Si tienes ideas para mejorar el proyecto, agregar nuevas características o solucionar problemas, no dudes en enviar solicitudes de extracción.

## Requisitos
Antes de implementar el sistema StreetSense, asegúrate de cumplir con los requisitos técnicos necesarios, que incluyen hardware compatible, configuración de MQTT, y acceso a Firebase Database RealTime.

