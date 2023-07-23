/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : define.h
* Version      : 7.30
* Description  : RZ/V2L DRP-AI Sample Application for Darknet-PyTorch YOLO Image version
***********************************************************************************************************************/

#ifndef DEFINE_MACRO_H
#define DEFINE_MACRO_H

/*****************************************
* includes
******************************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <vector>
#include <map>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <float.h>
#include <math.h>

/*****************************************
* Tiny YOLOv3
******************************************/
/* Directory name of DRP-AI Object files (DRP-AI Translator output) */
const static std::string drpai_prefix   = "tinyyolov3_roboken";
/* Class labels to be classified */
//const static std::string label_list     = "obj.names";
/* Empty since labels will be loaded from label_list file */
static std::vector<std::string> label_file_map = {};
/* Number of class to be detected */
#define NUM_CLASS           (7)
/* Number for [yolo] layer num parameter */
#define NUM_BB              (3)
/* Number of output layers. This value MUST match with the length of num_grids[] below */
#define NUM_INF_OUT_LAYER   (2)
/* Number of grids in the image. The length of this array MUST match with the NUM_INF_OUT_LAYER */
const static uint8_t num_grids[] = { 13, 26 };
/* Number of DRP-AI output */
const static uint32_t num_inf_out =  (NUM_CLASS + 5)* NUM_BB * num_grids[0] * num_grids[0]
                                + (NUM_CLASS + 5)* NUM_BB * num_grids[1] * num_grids[1];
/* Anchor box information */
const static double anchors[] =
{
    10, 14,
    23, 27,
    37, 58,
    81, 82,
    135, 169,
    344, 319
};

/*****************************************
* Static Variables (No need to change)
* Following variables are the file name of each DRP-AI Object file
* drpai_file_path order must be same as the INDEX_* defined later.
******************************************/
const static std::string drpai_address_file = drpai_prefix+"/"+drpai_prefix+"_addrmap_intm.txt";
const static std::string drpai_file_path[5] =
{
    drpai_prefix+"/drp_desc.bin",
    drpai_prefix+"/"+drpai_prefix+"_drpcfg.mem",
    drpai_prefix+"/drp_param.bin",
    drpai_prefix+"/aimac_desc.bin",
    drpai_prefix+"/"+drpai_prefix+"_weight.dat",
};

/*****************************************
* Common Macro for YOLO
******************************************/
/* Size of input image to the model */
#define MODEL_IN_W              (416)
#define MODEL_IN_H              (416)
/* Thresholds */
#define TH_PROB                 (0.5f)
#define TH_NMS                  (0.5f)
/*****************************************
* Common Macro for Application
******************************************/
/*Maximum DRP-AI Timeout threshold*/
#define DRPAI_TIMEOUT           (5)

/*DRP-AI Input image information*/
#define DRPAI_IN_WIDTH          (640)
#define DRPAI_IN_HEIGHT         (240)
#define DRPAI_IN_CHANNEL_BGR    (3)

/*BMP Header size for Windows Bitmap v3*/
#define FILEHEADERSIZE          (14)
#define INFOHEADERSIZE_W_V3     (40)
/*For drawing the bounding box on image*/
#define CPU_DRAW_FONT_WIDTH     (6)
#define CPU_DRAW_FONT_HEIGHT    (8)
#define WHITE_DATA              (0xFFFFFFu)
#define BLACK_DATA              (0x000000u)

/*Buffer size for writing data to memory via DRP-AI Driver.*/
#define BUF_SIZE                (1024)

/*Index to access drpai_file_path[]*/
#define INDEX_D                 (0)
#define INDEX_C                 (1)
#define INDEX_P                 (2)
#define INDEX_A                 (3)
#define INDEX_W                 (4)

/*****************************************
* Typedef
******************************************/
/* For DRP-AI Address List */
typedef struct
{
    unsigned long desc_aimac_addr;
    unsigned long desc_aimac_size;
    unsigned long desc_drp_addr;
    unsigned long desc_drp_size;
    unsigned long drp_param_addr;
    unsigned long drp_param_size;
    unsigned long data_in_addr;
    unsigned long data_in_size;
    unsigned long data_addr;
    unsigned long data_size;
    unsigned long work_addr;
    unsigned long work_size;
    unsigned long data_out_addr;
    unsigned long data_out_size;
    unsigned long drp_config_addr;
    unsigned long drp_config_size;
    unsigned long weight_addr;
    unsigned long weight_size;
} st_addr_t;
#endif
