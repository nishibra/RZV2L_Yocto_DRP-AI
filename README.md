# RZV2L_Yocto_DRP-AI

ルネサスのDRP-AI~を使いサンプルを実行し学びます。

## Refernce

[Yoctoを使って開発環境を構築](https://qiita.com/Lathe/items/63bed2701d91e098761c)

[Run Machine Learning on the RZBoard](https://www.hackster.io/monica/run-machine-learning-on-the-rzboard-326098)

## 進め方

Renesas 画像処理ライブラリDRP-AIを使ってみよう。

まずUbuntu PCを準備します。
```
1.Ubuntu20.04のインストール(省略)
2.OpenCVのインストール(省略)
3.Pytorchのインストール

4.YoloからPytorchへ変換
5.PytorchからONNXへ変換
6.DRP-AIモデルへ変換

7.SDKのインストール/クロスコンパイラーのYocto bit bake(省略)
8.サンプルプログラム確認と修正
9.make
10.RZへコピー
11.RZBoardで実行
12.デバッグの繰り返し(8以降を繰り返す)
```

### Install PyTorch

以下を実行し、PyTorchをインストールします。
```
$ pip3 install torch==1.13.0+cpu torchvision==0.14.0+cpu torchaudio==0.13.0+cpu -f https://download.pytorch.org/whl/cpu/torch_stable.html
```
以下をご参照下さい。

https://www.linode.com/docs/guides/pytorch-installation-ubuntu-2004/

### darknet_yolo to pytorch model

Rexsasのサイトからダウンロードしたrzv21l-drpai_sp.zip からrzv_ai-implementation-guideの中のdarknet_yoloを解答します。
yolo.iniで出力名を変更できます。

```
[tinyyolov3]
cfg     =yolov3-tiny.cfg
weights =yolov3-tiny.weights
pth     =yolov3-tiny.pth
input   =["input1"]
output  =["output1", "output2"]
onnx    =tinyyolov3_roboken.onnx
```
onnxをtinyyolov3_roboken.onnxとします。

```
$ cd ~/rz_ai_work/darknet/yolo
$ python3 convert_to_pytorch.py tinyyolov3
```
yolov3.pthに変換されます。


### pytorch model to ONNX

convert_to_onnx.py内のinput sizeの変更もできますがtranslatorが対応していないので416*416で実施します。

```
$ python3 convert_to_onnx.py tinyyolov3
```

tinyyolov3_roboken.onnxに変換されます。


### ONNX to DRP-AI model

```
$ chmod +x DRP-AI_Translator-v1.82-Linux-x86_64-Install
$ ./DRP-AI_Translator-v1.82-Linux-x86_64-Install
```
インストールが終わるとhomeにdrp-ai_translator_release フォルダーができます。

 onnxフォルダーに作成したtinyyolov3_roboken.onnx file をコピーします。

次にUserConfigに以下のファイルを作成します。

```
 addrmap_in_tinyyolov3_roboken.yaml
 prepost_tiny_yolov3_roboken.yaml
```

### DRP-AI_Translator

RP-AI用データに変換します>
```
$ cd drp-ai_translator_release
$ ./run_DRP-AI_translator_V2L.sh tinyyolov3_robo-ken -onnx ./onnx/tinyyolov3_roboken.onnx
```

### netron

netronでmodelの入出力を確認します。class数が変わった場合は確認が必要です。

```
$ sudo snap install netron
$ netron
```

### Comple sample program: app_yolo_img

画像ファイルを読み込み推論結果を画像として出力します。

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

MIPI camraを使って推論します。

```
$ source /opt/poky/3.1.17/environment-setup-aarch64-poky-linux
$ cd ~/app_tinyyolov2_cam/src
$ make
$ tar -zcvf y2cam.tar.gz exe
$ scp y2cam.tar.gz root@192.168.8.99:~root/drp

# tar -xzvf y2cam.tar.gz
# chmod +x sample_app_tinyyolov2_cam
# ./sample_app_tinyyolov2_cam
```

### Linux command

よく使うLinuxコマンドをリストアップしました。
```
$ cp  #copy A to B 
$ scp #copy A to B@IP
$ rm  #remove
$ rm -d #remove empity derectory
$ rm -r #remove derectory
S ssh root@192.168.8.99
```

SSH パスワード認証対応法

https://zenn.dev/seiwell/articles/98ea9c97aa7992


### YOLOv2で物体検出を学習させてみよう

https://qiita.com/sudamasahiko/items/97441202897cb8976f82

https://qiita.com/garcoo/items/c21ff0e48e495420c0d1

https://github.com/leetenki/YOLOv2/blob/master/YOLOv2.md


### Sample--app_usbcam_http

USBカメラの動画から推論結果をScketに送ります。

まずhome/app_usbcam_httpにコピーします。

```
$ source /opt/poky/3.1.17/environment-setup-aarch64-poky-linux
$ cd /home/nishi/app_usbcam_http/src
$ mkdir ./build
$ cd ./build
$ cmake ..
$ make
```

makeが完了したらRZにコピーして実行します。

```
$ cp src/build/sample_app_usbcam_http /exe
$ tar -zcvf web.tar.gz exe
$ scp web.tar.gz root@192.168.8.99:~root

# tar -xzvf web.tar.gz
# chmod +x sample_app_usm_http
# ./sample_app_usm_http
```

etc/Websocket_Client directory をPCにコピーして表示します。

DRP-AIのerror
```
cd drp-ai_translator_release
cd output
tar -zcvf mod.tar.gz d-tinyyolov3
scp mod.tar.gz root@192.168.8.99:~root

別画面
$ ssh root@192.168.8.99
# tar -xzvf mod.tar.gz exe/
```

### yolo.iniの編集
```
[tinyyolov2]
cfg     =yolov2-tiny.cfg
weights =yolov2-tiny.weights
pth     =yolov2-tiny.pth
input   =["input1"]
output  =["output1"]
onnx    =d-tinyyolov2.onnx
```

```
python3 convert_to_pytorch.py tinyyolov2
```
yolov2-tiny.pthができる。
```
python3 convert_to_onnx.py tinyyolov2
```
d-tinyyolov2.onnxができる。

```
cd drp-ai_translator_release
./run_DRP-AI_translator_V2L.sh d-tinyyolov2 -onnx ./onnx/d-tinyyolov2.onnx
```


{"shinai","edge","kote","dou","men","Stand","down"};

