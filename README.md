# RZ_yocto_AI

ルネサスのARP-AIの使い方のサンプルを実行します。

### Refernce

[Yoctoを使って開発環境を構築](https://qiita.com/Lathe/items/63bed2701d91e098761c)

[Run Machine Learning on the RZBoard](https://www.hackster.io/monica/run-machine-learning-on-the-rzboard-326098)

### 目次
```
Renesas 画像処理ライブラリを使ってみよう。まずはYoloV3を使ってみます。
1.Ubuntu20.04のインストール
2.OpenCVのインストール
3.ONNXへ変換
4.Renesusモデルへ変換
5.プログラム作成
6.SDKのインストール/クロスコンパイラー
7.make
8.コピー
9.RZBoardで実行
10.デバッグの繰り返し
```

### Install PyTorch

```
$ pip3 install torch==1.13.0+cpu torchvision==0.14.0+cpu torchaudio==0.13.0+cpu -f https://download.pytorch.org/whl/cpu/torch_stable.html
```

https://www.linode.com/docs/guides/pytorch-installation-ubuntu-2004/
```
$ sudo apt update
$ sudo apt upgrade
$ sudo apt install python3-pip
$ pip3 install -f torch torchvision
$python3
>import torch
```

### darknet_yolo to pytorch model

rzv21l-drpai_sp.zip からrzv_ai-implementation-guideの中のdarknet_yoloを解答します。


```
$ cd ~/rz_ai_work/darknet_yolo/darknet/yolo
$ python3 convert_to_pytorch.py tinyyolov3


```

### pytorch model to ONNX
```
$ python3 convert_to_onnx.py tinyyolov3


convert_to_pytorch.py convert_to_onnx.py d-yolov3.onnx yolov3.pth
```

### ONNX to DRP-AI model

```
$ chmod +x DRP-AI_Translator-v1.82-Linux-x86_64-Install
$ ./DRP-AI_Translator-v1.82-Linux-x86_64-Install
```
homeにdrp-ai_translator_release かできる。

onnx
 d-tinyyolov3.onnx

UserConfig
addrmap_in_tiny_yolov3.yaml
prepost_tiny_yolov3.yaml



```
$ cd drp-ai_translator_release
($ ./run_DRP-AI_translator_V2L.sh tinyyolov3_bmp -onnx ./onnx/tinyyolov3_bmp.onnx)
$ ./run_DRP-AI_translator_V2L.sh d-tinyyolov3 -onnx ./onnx/d-tinyyolov3.onnx

$ python3 postprocess_yolo.py tinyyolov3
```

### netron

netronで入出力確認

```
$ sudo snap install netron
$ netron
```

### Comple sample program: app_yolo_img


```
$ source /opt/poky/3.1.17/environment-setup-aarch64-poky-linux
$ cd /home/nishi/drpai_sp/rzv2l_drpai-sample-application/app_yolo_img/src
cmake ..
$ make

$ tar -zcvf tinyyolov3_bmp.tar.gz tinyyolov3_bmp
  (tar -zcvf yi.tar.gz yi)

$ scp tinyyolov3_bmp.tar.gz root@192.168.8.99:~root/app
$ scp sample_app_tinyyolov3_img root@192.168.8.99:~root
  (scp yi.tar.gz root@192.168.8.99:~root)
# tar -xzvf tinyyolov3_bmp.tar.gz
  (tar -xzvf yi.tar.gz)
# chmod +x sample_app_tinyyolov3_img

$ scp root@192.168.8.99:~root/yi/sample_output.bmp /tmp

```
### app_tinyyolov2_cam

MIPI camra 

```
source /opt/poky/3.1.17/environment-setup-aarch64-poky-linux
cd ~/app_tinyyolov2_cam/src
make
tar -zcvf y2cam.tar.gz exe
scp y2cam.tar.gz root@192.168.8.99:~root/drp
tar -xzvf y2cam.tar.gz
chmod +x sample_app_tinyyolov2_cam
./sample_app_tinyyolov2_cam
```


### Linux command
```
$cp  #copy A to B 
$scp #copy A to B@IP
$rm  #remove
$rm -d #remove empity derectory
$rm -r #remove derectory
Sssh root@192.168.8.99
```

SSH パスワード認証対応法

https://zenn.dev/seiwell/articles/98ea9c97aa7992


### YOLOv2で物体検出を学習させてみよう

https://qiita.com/sudamasahiko/items/97441202897cb8976f82

https://qiita.com/garcoo/items/c21ff0e48e495420c0d1

https://github.com/leetenki/YOLOv2/blob/master/YOLOv2.md


### Sample--app_usbcam_http

home/app_usbcam_httpにコピーします。

```
$ source /opt/poky/3.1.17/environment-setup-aarch64-poky-linux
$ cd /home/nishi/app_usbcam_http/src
$ mkdir ./build
$ cd ./build
$ cmake ..
$ make
```

makeが完了したら

```
$ cp src/build/sample_app_usbcam_http /exe
$ tar -zcvf web.tar.gz exe
$ scp web.tar.gz root@192.168.8.99:~root

# tar -xzvf web.tar.gz
# chmod +x sample_app_usm_http
# ./sample_app_usm_http
```

etc/Websocket_Client directory をPCにコピーして表示します。

