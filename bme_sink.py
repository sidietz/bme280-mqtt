 
from paho.mqtt import client as mqtt_client
import random
import psycopg

#broker = '192.168.178.64'
broker = '127.0.0.1'
port = 1883
topic = "home/sensors/bme280"
client_id = f'python-mqtt-bme280'

PASSWORD = "N0m1596."

def get_db_connection():
    conn = psycopg.connect(host='localhost',
                            dbname='mqtt',
                            user='mqtt',
                            password=PASSWORD)
    return conn

def executeInsertSql(sql_query):

    sql_query = sql_query
    conn = get_db_connection()
    with conn.cursor() as cur:
        cur.execute(sql_query)
    conn.commit()

def handle_msg(temperature, humidity, pressure):
    sql_query = "INSERT INTO bme280(id, date, temperature, humidity, pressure) VALUES (DEFAULT, " + "now()" + ", " + temperature + ", " + humidity + ", " + pressure + ");"
    executeInsertSql(sql_query)


def on_connect(client, userdata, flags, rc, properties):
    # For paho-mqtt 2.0.0, you need to add the properties parameter.
    # def on_connect(client, userdata, flags, rc, properties):
    if rc == 0:
        print("Connected to MQTT Broker!")
    else:
        print("Failed to connect, return code %d\n", rc)
    # Set Connecting Client ID


def on_message(client, userdata, msg):
    payload = msg.payload.decode()
    print(f"Received `{payload}` from `{msg.topic}` topic")
    alist = payload.split(",")
    temperature = alist[0]
    humidity = alist[1]
    pressure = alist[2]
    handle_msg(temperature, humidity, pressure)

def on_subscribe(client, userdata, mid, reason_code_list, properties):
    print("Subscribed: " + str(mid) + " " + str(reason_code_list))
    # Since we subscribed only for a single channel, reason_code_list contains
    # a single entry
    #if reason_code_list[0].is_failure:
    ##    print(f"Broker rejected you subscription: {reason_code_list[0]}")
    #else:
    #    print(f"Broker granted the following QoS: {reason_code_list[0].value}")





def run():
    client = mqtt_client.Client(mqtt_client.CallbackAPIVersion.VERSION2, client_id)
    client.on_connect = on_connect
    client.on_message = on_message
    client.on_subscribe = on_subscribe
    client.username_pw_set("simon", PASSWORD)
    client.connect(broker, port, 60)
    client.subscribe(topic)
    #client.loop_start()
    
    #while True:
    #    try:
    client.loop_forever()
    #    except:
    #        client-loop_forever()

if __name__ == '__main__':
    run()
