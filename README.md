# RZ_yocto_AI

### 方法2：Yoctoを使って開発環境を構築

https://qiita.com/Lathe/items/63bed2701d91e098761c

### Run Machine Learning on the RZBoard

https://www.hackster.io/monica/run-machine-learning-on-the-rzboard-326098

### copy files to RZBoard

scp resnet50_cam_exe.tar.gz root@192.168.8.99:~root

```

Renesas 画像処理ライブラリを使ってみる。
YoloV3を使ってみよう。
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
```
cd darknet_yolo_work/darknet/yolo
$ python3 convert_to_pytorch.py tinyyolov3
```

### pytorch model to ONNX
```
$ python3 convert_to_onnx.py tinyyolov3


convert_to_pytorch.py convert_to_onnx.py d-yolov3.onnx yolov3.pth
```

### ONNX to DRP-AI model
```
cd drp-ai_translator_release 
./run_DRP-AI_translator_V2L.sh tinyyolov3_bmp -onnx ./onnx/tinyyolov3_bmp.onnx



```

### netron

```
$ sudo snap install netron
$ netron




```

### comple
```
$ source /opt/poky/3.1.14/environment-setup-aarch64-poky-linux
$ cd /home/nishi/drpai_sp/rzv2l_drpai-sample-application/app_yolo_img/src
cmake ..
$ make

$ tar -zcvf tinyyolov3_bmp.tar.gz tinyyolov3_bmp
tar -zcvf yi.tar.gz yi

$ scp tinyyolov3_bmp.tar.gz root@192.168.8.99:~root/app
$ scp sample_app_tinyyolov3_img root@192.168.8.99:~root
scp yi.tar.gz root@192.168.8.99:~root
# tar -xzvf tinyyolov3_bmp.tar.gz
tar -xzvf yi.tar.gz
# chmod +x sample_app_tinyyolov3_img

$ scp root@192.168.8.99:~root/yi/sample_output.bmp /tmp
cp 
rm
rm -d
rm -r

```

### SSH
```
ssh root@192.168.8.99

https://zenn.dev/seiwell/articles/98ea9c97aa7992

```

### YOLOv2で物体検出を学習させてみよう

https://qiita.com/sudamasahiko/items/97441202897cb8976f82

https://qiita.com/garcoo/items/c21ff0e48e495420c0d1

https://github.com/leetenki/YOLOv2/blob/master/YOLOv2.md

### app_usbcam_http
```
$ source /opt/poky/3.1.17/environment-setup-aarch64-poky-linux
$ cd /home/nishi/drpai_sp/rzv2l_drpai-sample-application/app_usbcam_http/src
$ mkdir ./build
$ cd ./build
$ cmake ..
$ make

cp src/build/sample_app_usbcam_http /exe
tar -zcvf web.tar.gz exe
scp web.tar.gz root@192.168.8.99:~root
tar -xzvf web.tar.gz
chmod +x sample_app_usm_http
./sample_app_usm_http

etc/Websocket_Client directory is copied to /<any_directory> in PC)

```