extends Sprite


var speed = 10
var newPos = Vector2(0,0)
var curPos = Vector2(0,0)
var angle = 0

#Fixing the lower and higher range values of the sensor noise 
const sensorNoise_l = -2.0
const sensorNoise_h = 2.0

var prev_t = 0
var delta_t = 0

var commClient
var commServer

var kalman = Vector2(0,0)

func _ready():
	curPos = get_offset()
	print(curPos)
	commClient = PacketPeerUDP.new()
	commClient.set_dest_address("127.0.0.1",4243)
#	commClient.set_dest_address("169.254.49.190",4243)
	
	commServer = PacketPeerUDP.new()
	#commServer.set_dest_address("127.0.0.1",4244)
	commServer.set_dest_address("169.254.102.146",4244)
	if(commServer.listen(4244,"169.254.102.146") != OK):
		print("an error occurred listening on port " + str(4244))
	else:
		print("Listening on port " + str(4244) + " on " + "169.254.102.146")
	
func _peer_connected(id):
	var text = "\nUser " + str(id) + " connected"
	var userText = "Total Users:" + str(get_tree().get_network_connected_peers().size())
	print(text)
	print(userText)
  
func _peer_disconnected(id):
	var text = "\nUser " + str(id) + " connected"
	var userText = "Total Users:" + str(get_tree().get_network_connected_peers().size())
	print(text)
	print(userText)


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	curPos = get_offset()
	
	#Recieve correction
	receiveCorrection()
	
	#Setting the new position obtained
	newPos.x = curPos.x - 10 + kalman.x
	newPos.y = curPos.y + 20 + kalman.y
	
#	var slope = (newPos.y-curPos.y)/(newPos.x-curPos.x)
#	angle = tan(slope)
#	rotate(deg2rad(angle))
#	print(newPos)

	#Measuring system clock ticks
	var curr_t = OS.get_ticks_msec()
	delta_t = curr_t - prev_t
	prev_t = curr_t
#	print(curr_t)
	
	set_offset(newPos)

	#Sensor noise addition
	#var my_random_number = rng.randf_range(-10.0, 10.0)
	#Sending measurement 
	
	sendMeasurement(curr_t)

func plot():
	pass
	
func sendMeasurement(var curr_t):
	var separator = ":"
	var text = "$MEAS"+ separator + String(newPos.x + sensorNoise()) + separator + \
				String(newPos.y + sensorNoise()) + separator +  String(delta_t) + \
				separator + String(curr_t)
	print(text)
	commClient.put_packet(text.to_ascii())
	
	
	
func receiveCorrection():
	kalman = Vector2(0,0)
	if(commServer.get_available_packet_count() > 0):
		var recvMsg = commServer.get_packet().get_string_from_ascii ()
		var textArray = recvMsg.split(":",true)
		if(textArray[0] == String("$KALMAN1")):
			kalman = Vector2(float(textArray[1]),float(textArray[2]))
		
func sensorNoise():
		var noise = stepify(rand_range(sensorNoise_l,sensorNoise_h), 0.01)
#		print(String(noise) + ",")
		return(noise)
		
		
	

