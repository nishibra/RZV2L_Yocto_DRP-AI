#!/usr/bin/env python
#! -*- coding: utf-8 -*-
#Created on Thu June 07 2023
# @author: Nishimura @ AiRRC
#
import ctypes
import numpy as np
import time
import cv2
import socket
#
w=320
h=240
cap = cv2.VideoCapture(-1)
cap.set(3,w*2)
cap.set(4,h)
cap.set(cv2.CAP_PROP_BUFFERSIZE, 2)
#
UDP_IP = "192.168.8.178"
UDP_PORT = 8080
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# read class file
f = open('obj.names', 'r', encoding='UTF-8')
classID = f.readlines()#f.read()
classNo=len(classID)
#classID=[]
for i in range(0,classNo):
  classID[i]=classID[i].strip()
print(classID)
f.close()
#ライブラリーを指定
lib = ctypes.cdll.LoadLibrary('/home/root/rz_work/arml/roboken/src/tinyyolov3_lib.so')
# 関数の引数と戻り値の型を指定
lib.get_string_array.argtypes = [ctypes.POINTER(ctypes.POINTER(ctypes.c_char_p)),
                                 ctypes.POINTER(ctypes.c_int)]
lib.get_string_array.restype = None
# drpai_iniの呼び出し(DRP-AIの初期化)
lib.drpai_ini()
print ('drp initialized')
#
def main():
#
    ifd=[]
    ret, frame = cap.read()
    height, width, channels = frame.shape
    size = height * width * channels
   # th_prob=50 #% int
   # th_nms=50  #% int
    # C言語の画像データバッファを確保
    c_buf = ctypes.create_string_buffer(size)
    # 推論関数の戻り値の型を設定
    result_ptr = ctypes.POINTER(ctypes.c_char_p)()
    length_ptr = ctypes.c_int(0)
#
    while (cap.isOpened()):
        #
        a=time.time()
        #
        ret, frame = cap.read()
        frame= cv2.rotate(frame, cv2.ROTATE_180)
        #frame = cv2.resize(frame, (640,240))
        #
        b=time.time()
        #
        # NumPy配列のデータをC言語のデータバッファにコピー
        ctypes.memmove(c_buf, frame.ctypes.data, size)
        #b=time.time()
        
        # C言語の推論関数に画像データを渡す
        lib.drpai_inf(c_buf, classNo) #, th_prob, th_nms)
        # C言語の関数から画像データを貰う
        lib.get_string_array(result_ptr, ctypes.byref(length_ptr))
        
        c=time.time()
        # C++から返された配列をNumPy配列に変換
        result_list = []
        #print(length_ptr.value)
        for i in range(length_ptr.value):
            result_list.append(int(ctypes.cast(result_ptr[i], ctypes.c_char_p).value))
        result_array = np.array(result_list)
        #
        #c=time.time()
        # 
        #結果データを画面に
        if len(result_array)>0:
           ifd=(result_array.reshape(int(length_ptr.value/6),6))
        #
           print('Result: ',ifd)
           
           for i in range(0,int(length_ptr.value/6)):
             cv2.putText(frame,classID[ifd[i][0]]+" "+str(ifd[i][1]),(int(ifd[i][2]),int(ifd[i][3])), cv2.FONT_HERSHEY_SIMPLEX, 1.0, (255, 255,255), thickness=1)
             cv2.circle(frame,(ifd[i][2],int(ifd[i][3])),int(ifd[i][4]/3), (0, 0, 255), thickness=2)
 
        #cv2.imshow('frameL',frame)
        frame = cv2.resize(frame, (320,120))
        _, data = cv2.imencode('.jpg', frame)
        sock.sendto(data.tobytes(), (UDP_IP, UDP_PORT))
        #
        #計算時間表示
        d=time.time()
        t_cap=int((b-a)*1000)
        t_inf=int((c-b)*1000)
        t_post=int((d-c)*1000)
        t_ttl=int((d-a)*1000)
        f=int(1000/t_ttl)
        print ('FPS=',f,'pre=',t_cap,'Inf=',t_inf,'Pos=',t_post,'Total=',t_ttl)
        #画面qで終了
        if cv2.waitKey(1) & 0xFF == ord('q'):
            print ("quit")
            break
    print ('drp inferenced')
    cap.release()
    cv2.destroyAllWindows()
#
if __name__ == "__main__":
    main()
