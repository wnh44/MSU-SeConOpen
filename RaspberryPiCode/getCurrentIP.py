import socket
import requests
import time


for i in range(0,10):
	try:
		# Makes quick socket, gets IP (Other methods return localhost)
		s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		s.connect(("8.8.8.8", 80))
		ip = s.getsockname()[0]
		s.close()

		response = requests.post(url="https://pbell97.pythonanywhere.com/piIP/", data=ip)
		with open("ipLog.txt", 'w') as f:
			f.write("IP: " + ip + "\nServer Response: " + str(response.status_code) + "\n\n")
		break
	except:
		print("Error uploading IP, trying again...")
		time.sleep(2)