import paho.mqtt.client as mqtt
import sqlite3
import time

#----------------------------------------------------------------------
broker_address= "broker URL"
port = 00000 #portNumber
user = "Your Username"
password = "Your Password"
topic = "Client"
#----------------------------------------------------------------------


#------------------ SQL -----------------------------------------------
filestatus = False
def connect():
    global conn
    conn = sqlite3.connect('pothole.db')
    global c
    c = conn.cursor()
        
def InsertPotholeRow(username,pothole_number,latitude,longitude):
    c.execute("INSERT INTO Potholes VALUES ('"+username+"','"+pothole_number+"','"+latitude+"','"+longitude+"')")
    print( "Pothole inserted: "+username)
    conn.commit()  
    
def InsertUsers(username,password):
    usrName_Search = c.execute("SELECT username FROM Users WHERE username = '%s'" % username).fetchone()
    if not usrName_Search:  #same as if usrName_Search ==None
        print("User Created")
        client.publish("Android","User_Created")
        c.execute("INSERT INTO Users VALUES ('"+username+"','"+password+"')")
    else:
        print("Username_Taken")
        client.publish("Android","Username_Taken")          
    conn.commit()  

def showallpotholes(username):
    c.execute("SELECT * FROM Potholes").fetchone()
    while True:
        row = c.fetchone()
        if row == None:
            break
        publish("Android",'pothole,'+username+','+row[2]+','+row[3])
        #publish("Android","pothole,Vijay,13.030150223079623,77.60565242387972")
    
def publish(topic,msg):
    client.publish(topic,msg) 
    #time.sleep(1)      

def CloseConnection():
    conn.close()
#-------------------------------------------------------------------------
    
# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with code "+str(rc))
	# Subscribing in on_connect() means that if we lose the connection and
	# reconnect then subscriptions will be renewed.
    client.subscribe(topic + "/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    csv=str(msg.payload)
    csv=csv[2:-1]
    csv_array = csv.split(",") 
    if csv_array[0]=='login' :
        response = c.execute("SELECT * FROM Users WHERE username = '"+csv_array[1]+"' AND passwords = '"+csv_array[2]+"'").fetchone()
        if response:  #same as if usrName_Search ==None
            print("Login Successful")
            client.publish("Android","Login_Successful")
        else:
            print("Wrong username or password")
            client.publish("Android","Login_Unsuccessful")

    elif csv_array[0] =='register' :
        InsertUsers(csv_array[1],csv_array[2])
    elif csv_array[0] == 'inslatlong' :
        InsertPotholeRow(csv_array[1],csv_array[2],csv_array[3],csv_array[4])
    elif csv_array[0] == 'getpotholes':
        showallpotholes(csv_array[1])
        
#-----------SQL scripts ---------------------------------------        
try:
    fh = open('pothole.db', 'r')
    filestatus = True
except FileNotFoundError:
    try:
        connect()
        c.execute('''CREATE TABLE Potholes
         (username text, pothole_number text, latitude text, longitude text)''')
        c.execute('''CREATE TABLE Users
         (username text, passwords text)''')
    except:
        pass
if filestatus:
    connect()
#---------------------------------------------------------------------

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client = mqtt.Client('Potholes')
client.on_connect = on_connect
client.on_message = on_message
client.connect(broker_address,port, 60)
client.username_pw_set(user,password)
client.loop_forever()