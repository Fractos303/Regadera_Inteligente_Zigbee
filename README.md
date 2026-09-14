| Supported Targets | ESP32-C5 | ESP32-C6 | ESP32-H2 |
| ----------------- | -------- | -------- | -------- |

# Regadera Inteligente Zigbee

Proyecto en desarrollo de una **regadera inteligente**
basada en un **ESP32-H2** y conectada a **Home Assistant mediante
Zigbee**.

## Estado actual

El proyecto se encuentra en fase de prototipo.

### Funcionalidades implementadas

-   [x] Dispositivo Zigbee basado en ESP32-H2.
-   [x] Integración con Home Assistant mediante ZHA.
-   [x] Control de una válvula mediante servomotor.
-   [x] Apertura y cierre de la válvula mediante el clúster Zigbee
    `On/Off`.
-   [x] Integración del clúster estándar `Analog Input`.
-   [x] Exposición del atributo `PresentValue` del clúster
    `Analog Input`.
-   [x] Lectura del atributo `PresentValue` desde la interfaz de
    administración de clústeres de ZHA.
-   [x] Configuración inicial de reporting para `PresentValue`.
-   [ ] Exposición del nivel de agua como entidad visible en el panel
    principal de Home Assistant.
-   [ ] Lectura real del sensor de nivel de agua.
-   [ ] Alimentación controlada del sensor para reducir su degradación.
-   [ ] Actualización automática y periódica del nivel de agua.
-   [ ] Automatizaciones de riego.
-   [ ] Gestión avanzada de estados, errores y seguridad.

## Hardware

### Controlador

-   Una placa de desarrollo con **ESP32-H2**.
-   Cable USB para alimentación, programación y monitorización.
-   Alimentación adecuada para el ESP32-H2.

### Válvula

-   Servomotor compatible con señal PWM.
-   Alimentación externa de `5 V` para el servomotor.
-   Señal de control PWM desde un GPIO del ESP32-H2.
-   Masa común entre la alimentación del servo y el ESP32-H2.

> El servomotor se controla mediante el componente `espressif/servo`,
> que utiliza el periférico LEDC para generar la señal PWM.

### Sensor de nivel de agua

Para las primeras pruebas se utiliza un sensor de nivel de agua
resistivo típico de Arduino.

En esta primera iteración:

-   El sensor se alimenta directamente a `3,3 V`.
-   Se realiza únicamente la lectura del sensor.
-   La lógica de lectura pertenece al módulo `water_level`.
-   El control de alimentación del sensor se incorporará en una
    iteración posterior.

## Arquitectura del proyecto

La aplicación se divide en módulos para mantener separadas la lógica de
hardware y la integración Zigbee.

``` text
Regadera_Zigbee/
├── main/
│   ├── config/
│   │   └── hardware_config.h
│   ├── valve.c
│   ├── valve.h
│   ├── servo.c
│   ├── servo.h
│   ├── water_level.c
│   ├── water_level.h
│   ├── zigbee_endpoint.c
│   ├── zigbee_endpoint.h
│   ├── zigbee_stack.c
│   ├── zigbee_stack.h
│   ├── app_main.c
│   └── idf_component.yml
└── README.md
```

### Responsabilidades de los módulos

  -----------------------------------------------------------------------
  Módulo                              Responsabilidad
  ----------------------------------- -----------------------------------
  `hardware_config.h`                 Definición de GPIO y configuración
                                      física del prototipo.

  `servo.c/.h`                        Control de bajo nivel del
                                      servomotor.

  `valve.c/.h`                        Lógica de apertura y cierre de la
                                      válvula.

  `water_level.c/.h`                  Lectura y gestión del sensor de
                                      nivel de agua.

  `zigbee_endpoint.c/.h`              Definición del endpoint,
                                      dispositivo y clústeres Zigbee.

  `zigbee_stack.c/.h`                 Inicialización de Zigbee,
                                      callbacks, comandos y reporting.

  `app_main.c`                        Punto de entrada de la aplicación.
  -----------------------------------------------------------------------

La intención es que `water_level` sea el propietario de la lógica del
nivel de agua. El módulo Zigbee únicamente debe exponer y actualizar el
valor correspondiente en el clúster Zigbee.

## Integración Zigbee

El dispositivo utiliza un endpoint principal, actualmente el endpoint
`10`.

### Clústeres utilizados

  Clúster        ID         Función
  -------------- ---------- ---------------------------------------------
  Basic          `0x0000`   Información básica del dispositivo.
  Identify       `0x0003`   Identificación del dispositivo.
  Groups         `0x0004`   Gestión de grupos Zigbee.
  On/Off         `0x0006`   Control de apertura y cierre de la válvula.
  Analog Input   `0x000C`   Exposición del valor del nivel de agua.

### Analog Input

El clúster `Analog Input` se utiliza inicialmente como solución estándar
para exponer el nivel de agua.

El atributo principal es:

``` text
PresentValue
ID: 0x0055
Tipo: float
```

Durante las pruebas se ha comprobado que:

1.  ZHA descubre el clúster `Analog Input`.
2.  El atributo `PresentValue` puede leerse desde la administración de
    clústeres.
3.  El dispositivo puede exponer valores como `75.0`.

Actualmente queda pendiente conseguir que Home Assistant mantenga y
muestre la entidad del sensor en el panel principal del dispositivo. El
problema parece estar relacionado con el proceso de descubrimiento y
registro de entidades de ZHA, no con la lectura básica del atributo
desde el ESP32-H2.

## Dependencias

El proyecto utiliza ESP-IDF y el gestor de componentes de Espressif.

Dependencias principales:

``` yaml
dependencies:
  espressif/esp-zboss-lib: "~1.6.0"
  espressif/esp-zigbee-lib: "~1.6.0"
  espressif/led_strip: "~3.0.0"
  espressif/servo: "^1.0.0"
```

La versión de ESP-IDF utilizada debe ser compatible con la configuración
del proyecto. El desarrollo actual se está realizando con ESP-IDF v6.0.

## Configuración del proyecto

Antes de compilar, seleccionar el objetivo correcto:

``` bash
idf.py --preview set-target esp32h2
```

Si se necesita volver a añadir el componente del servomotor:

``` bash
idf.py add-dependency "espressif/servo^1.0.0"
```

## Borrar la NVRAM Zigbee

Cuando se cambia la definición del dispositivo, el endpoint o los
clústeres, puede ser necesario borrar la información Zigbee almacenada
en la NVRAM.

``` bash
idf.py -p PORT erase-flash
```

Sustituir `PORT` por el puerto serie correspondiente.

> Al borrar la NVRAM será necesario volver a emparejar el dispositivo
> con Home Assistant.

## Compilar, grabar y monitorizar

Para compilar el proyecto:

``` bash
idf.py build
```

Para grabarlo en el ESP32-H2 y abrir el monitor serie:

``` bash
idf.py -p PORT flash monitor
```

Para salir del monitor serie:

``` text
Ctrl-]
```

## Ejemplo de funcionamiento

Al recibir un comando Zigbee `On/Off`, el dispositivo controla la
válvula mediante el servomotor.

Ejemplo de salida:

``` text
Received message: endpoint(10), cluster(0x6), attribute(0x0), data size(1)
Valve command: OPEN
Angle -> 90.0°
Valve OPEN

Received message: endpoint(10), cluster(0x6), attribute(0x0), data size(1)
Valve command: CLOSE
Angle -> 0.0°
Valve CLOSED
```

Durante las pruebas del sensor de nivel, el atributo `PresentValue` se
ha podido leer desde ZHA con valores como:

``` text
PresentValue = 75.0
```

## Próximas fases

### Fase 1 --- Integración Zigbee

-   [x] Añadir el clúster `Analog Input`.
-   [x] Exponer `PresentValue`.
-   [x] Comprobar la lectura desde ZHA.
-   [ ] Resolver la creación y permanencia de la entidad en Home
    Assistant.
-   [ ] Verificar el reporting automático del nivel de agua.

### Fase 2 --- Hardware del sensor

-   [ ] Conectar el sensor resistivo de nivel de agua.
-   [ ] Leer el valor mediante un ADC del ESP32-H2.
-   [ ] Calibrar los valores mínimo y máximo.
-   [ ] Convertir la lectura a un porcentaje de llenado.
-   [ ] Actualizar `PresentValue` con la lectura real.
-   [ ] Añadir alimentación controlada del sensor.
-   [ ] Evaluar la degradación del sensor y reducir el tiempo de
    alimentación.

### Fase 3 --- Automatización

-   [ ] Definir niveles mínimo, normal y máximo.
-   [ ] Crear avisos de depósito vacío.
-   [ ] Crear avisos de depósito lleno.
-   [ ] Añadir automatizaciones de riego.
-   [ ] Incorporar temporizadores y límites de seguridad.
-   [ ] Gestionar estados de error y pérdida de comunicación Zigbee.

## Notas de desarrollo

Este proyecto es un prototipo. Los valores de ángulo del
servomotor, los tiempos de alimentación del sensor, los límites del ADC
y las reglas de automatización deberán calibrarse con el hardware
definitivo.

La prioridad es mantener una arquitectura modular y validar cada etapa
antes de añadir complejidad:

1.  Hardware.
2.  Lectura local.
3.  Exposición Zigbee.
4.  Reporting.
5.  Entidades de Home Assistant.
6.  Automatizaciones.

## Recursos

-   [ESP Zigbee SDK
    Documentation](https://docs.espressif.com/projects/esp-zigbee-sdk)
-   [ESP Zigbee SDK
    Repository](https://github.com/espressif/esp-zigbee-sdk)
-   [ESP-IDF
    Documentation](https://docs.espressif.com/projects/esp-idf/)
-   [Espressif Servo
    Component](https://components.espressif.com/components/espressif/servo)
-   [ESP-IoT-Solution Servo
    Documentation](https://docs.espressif.com/projects/esp-iot-solution/en/latest/motor/servo.html)
-   [Home Assistant ZHA
    Documentation](https://www.home-assistant.io/integrations/zha/)

