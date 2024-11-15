# bme280-mqtt

## Beschreibung
bme280-mqtt ist ein Arduino Sketch und ein kleines Python Skript. In Kombination erfassen beide die Messwerte des BME280 von Bosch, Temperatur, Luftfeuchte und Luftdruck und speichern diese in einer postgres Datenbank.

## benötigte Libraries:
- Arduino IDE
- Postgres (Datenbank)
- mosquitto (MQTT Broker)
- python-paho (python MQTT Library)

## Besonderes
Zur Berechnung des Luftdrucks auf Meereshöhe wird statt der üblichen ausschließlich temperaturabhängigen Näherungsverfahren ein Präziseres verwendet, das auch die Luftfeuchtigkeit berücksichtigt. [Quelle](https://www.chemie.de/lexikon/Barometrische_H%C3%B6henformel.html)

## Benutzung
Die WLAN Zugangsdaten müssen geändert und das Passwort in der `secrets.h` eingetragen werden. Zusätzlich muss die Höhe (ALTITUDE) an die Höhe des Aufstellorts angepasst werden.
