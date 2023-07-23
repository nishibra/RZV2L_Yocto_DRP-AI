import socket
import cv2
import numpy as np

UDP_IP = '192.168.8.178' #"127.0.0.1"
UDP_PORT = 8080

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

while True:
    data, addr = sock.recvfrom(76800*2)
    img = cv2.imdecode(np.frombuffer(data, np.uint8), cv2.IMREAD_COLOR)
    h, w, c = img.shape
    
    img = cv2.resize(img, (w*2,h*2))
    cv2.imshow('video', img)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

sock.close()
cv2.destroyAllWindows()
