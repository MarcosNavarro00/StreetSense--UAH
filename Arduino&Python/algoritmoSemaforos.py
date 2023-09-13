import paho.mqtt.client as mqtt
from datetime import datetime
# Obtener la hora actual
hora_actual = datetime.now()

# Definir los colores inicales de los semaforos 
colorActual1 = "v1"
colorActual2 = "r2"

contador1 = 0
contador2 = 0

contadorMensajes = 0
coches1 = -1
coches2 = -1

mensajes = []

# Definir el nombre de usuario y contraseña del broker MQTT si es necesario
username = "emqx"
password = "public"

# Definir el broker y el puerto
broker_address = "broker.emqx.io"
broker_port = 1883

# Definir el topic y el mensaje
topicEscucha = "arduinoUAH2023/prueba"
topicEnviar = "arduinoUAH2023/prueba2"


# Callback que se ejecuta cuando se establece la conexión con el broker MQTT
def on_connect(client, userdata, flags, rc):
    print("Conectado al broker: " + broker_address)
    client.subscribe(topicEscucha)  # Suscribirse al topic cuando se establece la conexión

# Callback que se ejecuta cuando se recibe un mensaje en el topic suscrito
def on_message(client, userdata, msg):
    global colorActual1 
    global colorActual2 
    global contadorMensajes
    global contador1
    global contador2
    contadorMensajes +=1

    
    cochesActual = str(msg.payload)[2:][:-1]
    cochesActual2 = int (cochesActual)
    mensajes.append(cochesActual2)
    
    print ("Contador de Mensajes:" , contadorMensajes )
    print ("Mensajes Recibidos" , mensajes )

    if (len(mensajes)%2 == 0):
        
        #Si los dos estan activados que todo siga igual
        if( mensajes[0] == mensajes[1]):
            client.publish(topicEnviar, "igual")
            print("Envio1 --> igual")
        
        #Si el sensor 1 esta tiene MAS coches que el sensor 2
        if (mensajes[0] > mensajes[1]):
            if(contador2 >= 1):
                contador2 = contador2 - 1
            contador1 +=1
            if (contador1==1):
                switcher1 = {
                    "v1": "v1",
                    "r1": "r1",
                    "a1": "v1"
                }
                switcher2 = {
                    "v2": "a2",
                    "r2": "r2",
                    "a2": "r2"
                }
                color1 = switcher1.get(colorActual1, "Opción inválida")
                color2 = switcher2.get(colorActual2, "Opción inválida")
            else:
                switcher1 = {
                    "v1": "v1",
                    "r1": "v1",
                    "a1": "v1"
                }
                switcher2 = {
                    "v2": "a2",
                    "r2": "r2",
                    "a2": "r2"
                }
                color1 = switcher1.get(colorActual1, "Opción inválida")
                color2 = switcher2.get(colorActual2, "Opción inválida")

            colorActual1 = color1
            colorActual2 = color2

            contador1 = 0

            if (color1 == "Opción inválida" or color2 == "Opción inválida" ):
                print(colorActual1)
                print(colorActual2)

            coloresFinal = color1 + color2
            client.publish(topicEnviar, coloresFinal)
            print("Envio1 --> " + coloresFinal)
        
        #Si el sensor 1 esta tiene MENOS coches que el sensor 2 
        if (mensajes[0] < mensajes[1]):
            if(contador1 >= 1):
                contador1 = contador1 - 1
            
            contador2+=1
            if (contador2 == 1):
                switcher1 = {
                    "v1": "a1",
                    "r1": "r1",
                    "a1": "r1"
                }
                switcher2 = {
                    "v2": "v2",
                    "r2": "r2",
                    "a2": "v2"
                }
                color1 = switcher1.get(colorActual1, "Opción inválida")
                color2 = switcher2.get(colorActual2, "Opción inválida")
            else:
                switcher1 = {
                    "v1": "a1",
                    "r1": "r1",
                    "a1": "r1"
                }
                switcher2 = {
                    "v2": "v2",
                    "r2": "v2",
                    "a2": "v2"
                }
                color1 = switcher1.get(colorActual1, "Opción inválida")
                color2 = switcher2.get(colorActual2, "Opción inválida")

                
            colorActual1 = color1
            colorActual2 = color2
            
            if (color1 == "Opción inválida" or color2 == "Opción inválida" ):
                print(colorActual1)
                print(colorActual2)
            
            coloresFinal = color1 + color2
            client.publish(topicEnviar, coloresFinal)
            print("Envio2 --> " + coloresFinal)
        mensajes.clear()
        


    
    

    
# Crear una instancia del cliente MQTT
client = mqtt.Client()

# Establecer los callbacks
client.on_connect = on_connect
client.on_message = on_message

# Establecer el nombre de usuario y contraseña si se proporcionan
if username and password:
    client.username_pw_set(username, password)

# Conectarse al broker MQTT
client.connect(broker_address, broker_port)

# Mantener la conexión MQTT activa
client.loop_forever()


