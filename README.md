| Supported Targets | ESP32-C5 | ESP32-C6 | ESP32-H2 |
| ----------------- | -------- | -------- | -------- |

# Regadera Inteligente Zigbee

Regadera inteligente para macetas basada en un **ESP32-H2** que se integra en **Home Assistant** a través de **Zigbee (ZHA)**. Riega, mide y reporta; todo desde Home Assistant y sin Wi-Fi(necesitas una red Zigbee mediante un Dougle Zigbee).

> [!WARNING]
> **Proyecto en desarrollo.** Es un prototipo personal: el código, el cableado y la estructura cambian con frecuencia y algunas partes están a medio integrar. Puedes usarlo como referencia o punto de partida, pero no esperes que funcione "tal cual" ni que esté documentado al detalle.

## Compatibilidad

| | |
| --- | --- |
| **Microcontrolador** | ESP32-H2 (Zigbee 3.0 / IEEE 802.15.4) |
| **Domótica** | Home Assistant, mediante la integración **ZHA** |
| **Framework** | ESP-IDF 6.0 + `esp-zigbee-lib` / `esp-zboss-lib` 1.6 |

## Hardware

| Componente | Función |
| --- | --- |
| Bomba peristáltica 5 V (con MOSFET de nivel lógico) | Riego |
| Célula de carga 5 kg + HX711 | Nivel del depósito medido por peso |
| Sensor capacitivo de humedad del suelo (HW-390) | Humedad de la maceta |
| DHT11 | Temperatura y humedad ambiente |
| LDR | Luz ambiente |
| LED WS2812 integrado | Estado del dispositivo |

El mapa completo de pines y conexiones está en [`config/hardware_config.h`](config/hardware_config.h).

## Cómo funciona

El dispositivo se une a la red Zigbee como un dispositivo más. En Home Assistant aparece con:

- Un **interruptor** (clúster `On/Off`) para activar el riego.
- **Sensores** (clúster `Analog Input`) con las lecturas del depósito, suelo, temperatura, humedad y luz.

El código está dividido por módulos: cada sensor o actuador tiene su propio *driver*, de modo que cambiar un componente no obliga a tocar la capa Zigbee.

```text
Regadera_Zigbee/
├── app/        Lógica de la regadera (válvula/bomba, nivel de agua, LED de estado…)
├── drivers/    Acceso al hardware (LED, actuadores, sensores)
├── zigbee/     Endpoint, clústeres y stack Zigbee
├── config/     Pines y parámetros
└── main/       Punto de entrada
```

## Compilar y probar

Requiere [ESP-IDF](https://docs.espressif.com/projects/esp-idf/) instalado.

```bash
idf.py set-target esp32h2
idf.py build flash monitor
```

Después, en Home Assistant: **ZHA → Añadir dispositivo** y deja que se empareje.

> Si vuelves a emparejar el mismo dispositivo, elimínalo antes de ZHA y borra la memoria del ESP32 (`idf.py erase-flash`). Si no, ZHA reutiliza datos antiguos y puede no crear las entidades.

## Recursos

- [ESP Zigbee SDK](https://github.com/espressif/esp-zigbee-sdk)
- [Documentación de ESP-IDF](https://docs.espressif.com/projects/esp-idf/)
- [Integración ZHA de Home Assistant](https://www.home-assistant.io/integrations/zha/)
