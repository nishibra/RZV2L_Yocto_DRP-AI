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
* File Name    : sample_app_yolo_img.cpp
* Version      : 7.30
* Description  : RZ/V2L DRP-AI Sample Application for Darknet-PyTorch YOLO Image version
***********************************************************************************************************************/
//
// 2023.07.15 ARRANGED FOR ROBO-KEN COMPETITION BY T.Nishimura@AiRRC
//
/*****************************************
* Includes
******************************************/
/*DRPAI Driver Header*/
#include <linux/drpai.h>
/*Definition of Macros & other variables*/
#include "define.h"
/*Image control*/
#include "image.h"
/*Image control*/
#include "box.h"
//
using namespace std;
/*****************************************
* Global Variables
******************************************/
st_addr_t drpai_address;
float drpai_output_buf[num_inf_out];
vector<detection> det;

uint64_t udmabuf_address = 0;
int8_t fd = 0;
char addr[1024];

int8_t n_class=NUM_CLASS;
float th_prbb=TH_PROB;
float th_nmss=TH_NMS;

int8_t drpai_fd;
fd_set rfds;
struct timeval tv;
int8_t ret_drpai;
drpai_data_t proc[DRPAI_INDEX_NUM];
drpai_status_t drpai_status;

/*****************************************
* Function Name : read_addrmap_txt
* Description   : Loads address and size of DRP-AI Object files into struct addr.
* Arguments     : addr_file = filename of addressmap file (from DRP-AI Object files)
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
int8_t read_addrmap_txt(string addr_file)
{
    string str;
    uint32_t l_addr;
    uint32_t l_size;
    string element, a, s;

    ifstream ifs(addr_file);
    if (ifs.fail())
    {
        fprintf(stderr, "[ERROR] Failed to open address map list : %s\n", addr_file.c_str());
        return -1;
    }

    while (getline(ifs, str))
    {
        istringstream iss(str);
        iss >> element >> a >> s;
        l_addr = strtol(a.c_str(), NULL, 16);
        l_size = strtol(s.c_str(), NULL, 16);

        if ("drp_config" == element)
        {
            drpai_address.drp_config_addr = l_addr;
            drpai_address.drp_config_size = l_size;
        }
        else if ("desc_aimac" == element)
        {
            drpai_address.desc_aimac_addr = l_addr;
            drpai_address.desc_aimac_size = l_size;
        }
        else if ("desc_drp" == element)
        {
            drpai_address.desc_drp_addr = l_addr;
            drpai_address.desc_drp_size = l_size;
        }
        else if ("drp_param" == element)
        {
            drpai_address.drp_param_addr = l_addr;
            drpai_address.drp_param_size = l_size;
        }
        else if ("weight" == element)
        {
            drpai_address.weight_addr = l_addr;
            drpai_address.weight_size = l_size;
        }
        else if ("data_in" == element)
        {
            drpai_address.data_in_addr = l_addr;
            drpai_address.data_in_size = l_size;
        }
        else if ("data" == element)
        {
            drpai_address.data_addr = l_addr;
            drpai_address.data_size = l_size;
        }
        else if ("data_out" == element)
        {
            drpai_address.data_out_addr = l_addr;
            drpai_address.data_out_size = l_size;
        }
        else if ("work" == element)
        {
            drpai_address.work_addr = l_addr;
            drpai_address.work_size = l_size;
        }
        else
        {
            /*Ignore other space*/
        }
    }

    return 0;
}

/*****************************************
* Function Name : load_data_to_mem
* Description   : Loads a file to memory via DRP-AI Driver
* Arguments     : data = filename to be written to memory
*                 drpai_fd = file descriptor of DRP-AI Driver
*                 from = memory start address where the data is written
*                 size = data size to be written
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/
int8_t load_data_to_mem(string data, int8_t drpai_fd, uint32_t from, uint32_t size)
{
    int8_t ret_load_data = 0;
    int8_t obj_fd;
    uint8_t drpai_buf[BUF_SIZE];
    drpai_data_t drpai_data;
    size_t ret = 0;
    int32_t i = 0;

    printf("Loading : %s\n", data.c_str());
    errno = 0;
    obj_fd = open(data.c_str(), O_RDONLY);
    if (0 > obj_fd)
    {
        fprintf(stderr, "[ERROR] Failed to open: %s errno=%d\n", data.c_str(), errno);
        ret_load_data = -1;
        goto end;
    }

    drpai_data.address = from;
    drpai_data.size = size;

    errno = 0;
    ret = ioctl(drpai_fd, DRPAI_ASSIGN, &drpai_data);
    if ( -1 == ret )
    {
        fprintf(stderr, "[ERROR] Failed to run DRPAI_ASSIGN: errno=%d\n", errno);
        ret_load_data = -1;
        goto end;
    }

    for (i = 0; i < (drpai_data.size / BUF_SIZE); i++)
    {
        errno = 0;
        ret = read(obj_fd, drpai_buf, BUF_SIZE);
        if ( 0 > ret )
        {
            fprintf(stderr, "[ERROR] Failed to read: %s errno=%d\n", data.c_str(), errno);
            ret_load_data = -1;
            goto end;
        }
        ret = write(drpai_fd, drpai_buf, BUF_SIZE);
        if ( -1 == ret )
        {
            fprintf(stderr, "[ERROR] Failed to write via DRP-AI Driver: errno=%d\n", errno);
            ret_load_data = -1;
            goto end;
        }
    }
    if ( 0 != (drpai_data.size % BUF_SIZE))
    {
        errno = 0;
        ret = read(obj_fd, drpai_buf, (drpai_data.size % BUF_SIZE));
        if ( 0 > ret )
        {
            fprintf(stderr, "[ERROR] Failed to read: %s errno=%d\n", data.c_str(), errno);
            ret_load_data = -1;
            goto end;
        }
        ret = write(drpai_fd, drpai_buf, (drpai_data.size % BUF_SIZE));
        if ( -1 == ret )
        {
            fprintf(stderr, "[ERROR] Failed to write via DRP-AI Driver: errno=%d\n", errno);
            ret_load_data = -1;
            goto end;
        }
    }
    goto end;

end:
    if (0 < obj_fd)
    {
        close(obj_fd);
    }
    return ret_load_data;
}

/*****************************************
* Function Name : load_drpai_data
* Description   : Loads DRP-AI Object files to memory via DRP-AI Driver.
* Arguments     : drpai_fd = file descriptor of DRP-AI Driver
* Return value  : 0 if succeeded
*               : not 0 otherwise
******************************************/
int8_t load_drpai_data(int8_t drpai_fd)
{
    uint32_t addr = 0;
    uint32_t size = 0;
    int32_t i = 0;
    size_t ret = 0;
    for ( i = 0; i < 5; i++ )
    {
        switch (i)
        {
            case (INDEX_W):
                addr = drpai_address.weight_addr;
                size = drpai_address.weight_size;
                break;
            case (INDEX_C):
                addr = drpai_address.drp_config_addr;
                size = drpai_address.drp_config_size;
                break;
            case (INDEX_P):
                addr = drpai_address.drp_param_addr;
                size = drpai_address.drp_param_size;
                break;
            case (INDEX_A):
                addr = drpai_address.desc_aimac_addr;
                size = drpai_address.desc_aimac_size;
                break;
            case (INDEX_D):
                addr = drpai_address.desc_drp_addr;
                size = drpai_address.desc_drp_size;
                break;
            default:
                break;
        }

        ret = load_data_to_mem(drpai_file_path[i], drpai_fd, addr, size);
        if (0 != ret)
        {
            fprintf(stderr,"[ERROR] Failed to load data from memory: %s\n",drpai_file_path[i].c_str());
            return -1;
        }
    }
    return 0;
}

/*****************************************
* Function Name : get_result
* Description   : Get DRP-AI Output from memory via DRP-AI Driver
* Arguments     : drpai_fd = file descriptor of DRP-AI Driver
*                 output_addr = memory start address of DRP-AI output
*                 output_size = output data size
* Return value  : 0 if succeeded
*                 not 0 otherwise
******************************************/

int8_t get_result(int8_t drpai_fd, uint32_t output_addr, uint32_t output_size)
{
    drpai_data_t drpai_data;
    float drpai_buf[BUF_SIZE];
    drpai_data.address = output_addr;
    drpai_data.size = output_size;
    int32_t i = 0;
    int8_t ret = 0;

    errno = 0;
    /* Assign the memory address and size to be read */
    ret = ioctl(drpai_fd, DRPAI_ASSIGN, &drpai_data);
    if (-1 == ret)
    {
        fprintf(stderr, "[ERROR] Failed to run DRPAI_ASSIGN: errno=%d\n", errno);
        return -1;
    }

    /* Read the memory via DRP-AI Driver and store the output to buffer */
    for (i = 0; i < (drpai_data.size / BUF_SIZE); i++)
    {
        errno = 0;
        ret = read(drpai_fd, drpai_buf, BUF_SIZE);
        if ( -1 == ret )
        {
            fprintf(stderr, "[ERROR] Failed to read via DRP-AI Driver: errno=%d\n", errno);
            return -1;
        }

        memcpy(&drpai_output_buf[BUF_SIZE/sizeof(float)*i], drpai_buf, BUF_SIZE);
    }

    if ( 0 != (drpai_data.size % BUF_SIZE))
    {
        errno = 0;
        ret = read(drpai_fd, drpai_buf, (drpai_data.size % BUF_SIZE));
        if ( -1 == ret)
        {
            fprintf(stderr, "[ERROR] Failed to read via DRP-AI Driver: errno=%d\n", errno);
            return -1;
        }

        memcpy(&drpai_output_buf[(drpai_data.size - (drpai_data.size%BUF_SIZE))/sizeof(float)], drpai_buf, (drpai_data.size % BUF_SIZE));
    }
    return 0;
}

/*****************************************
* Function Name : sigmoid
* Description   : Helper function for YOLO Post Processing
* Arguments     : x = input argument for the calculation
* Return value  : sigmoid result of input x
******************************************/
double sigmoid(double x)
{
    return 1.0/(1.0 + exp(-x));
}

/*****************************************
* Function Name : yolo_index
* Description   : Get the index of the bounding box attributes based on the input offset.
* Arguments     : n = output layer number.
*                 offs = offset to access the bounding box attributesd.
*                 channel = channel to access each bounding box attribute.
* Return value  : index to access the bounding box attribute.
******************************************/
int32_t yolo_index(uint8_t n, int32_t offs, int32_t channel)
{
    uint8_t num_grid = num_grids[n];
    return offs + channel * num_grid * num_grid;
}

/*****************************************
* Function Name : yolo_offset
* Description   : Get the offset nuber to access the bounding box attributes
*                 To get the actual value of bounding box attributes, use yolo_index() after this function.
* Arguments     : n = output layer number [0~2].
*                 b = Number to indicate which bounding box in the region [0~2]
*                 y = Number to indicate which region [0~13]
*                 x = Number to indicate which region [0~13]
* Return value  : offset to access the bounding box attributes.
******************************************/
int32_t yolo_offset(uint8_t n, int32_t b, int32_t y, int32_t x)
{
    uint8_t num = num_grids[n];
    uint32_t prev_layer_num = 0;
    int32_t i = 0;

    for (i = 0 ; i < n; i++)
    {
        prev_layer_num += NUM_BB *(n_class + 5)* num_grids[i] * num_grids[i];
    }
    return prev_layer_num + b *(n_class + 5)* num * num + y * num + x;
}
//
//print_result_yolo
//
int8_t print_result_yolo(float* floatarr, Image * img)
{
    /* Following variables are required for correct_yolo/region_boxes in Darknet implementation*/
    /* Note: This implementation refers to the "darknet detector test" */
    float new_w, new_h;
    float correct_w = 1.;
    float correct_h = 1.;
    if ((float) (MODEL_IN_W / correct_w) < (float) (MODEL_IN_H/correct_h) )
    {
        new_w = (float) MODEL_IN_W;
        new_h = correct_h * MODEL_IN_W / correct_w;
    }
    else
    {
        new_w = correct_w * MODEL_IN_H / correct_h;
        new_h = MODEL_IN_H;
    }

    int32_t n = 0;
    int32_t b = 0;
    int32_t y = 0;
    int32_t x = 0;
    int32_t offs = 0;
    int32_t i = 0;
    float tx = 0;
    float ty = 0;
    float tw = 0;
    float th = 0;
    float tc = 0;
    float center_x = 0;
    float center_y = 0;
    float box_w = 0;
    float box_h = 0;
    float objectness = 0;
    uint8_t num_grid = 0;
    uint8_t anchor_offset = 0;
    float classes[n_class];
    float max_pred = 0;
    int32_t pred_class = -1;
    float probability = 0;
    detection d;
    /* Clear the detected result list*/
    det.clear();

    for (n = 0; n<NUM_INF_OUT_LAYER; n++)
    {
        num_grid = num_grids[n];
        anchor_offset = 2 * NUM_BB * (NUM_INF_OUT_LAYER - (n + 1));

        for (b = 0;b<NUM_BB;b++)
        {
            for (y = 0;y<num_grid;y++)
            {
                for (x = 0;x<num_grid;x++)
                {
                    offs = yolo_offset(n, b, y, x);
                    tx = floatarr[offs];
                    ty = floatarr[yolo_index(n, offs, 1)];
                    tw = floatarr[yolo_index(n, offs, 2)];
                    th = floatarr[yolo_index(n, offs, 3)];
                    tc = floatarr[yolo_index(n, offs, 4)];

                    /* Compute the bounding box */
                    /*get_yolo_box/get_region_box in paper implementation*/
                    center_x = ((float) x + sigmoid(tx)) / (float) num_grid;
                    center_y = ((float) y + sigmoid(ty)) / (float) num_grid;
#if defined(YOLOV3) || defined(TINYYOLOV3)
                    box_w = (float) exp(tw) * anchors[anchor_offset+2*b+0] / (float) MODEL_IN_W;
                    box_h = (float) exp(th) * anchors[anchor_offset+2*b+1] / (float) MODEL_IN_W;
#elif defined(YOLOV2) || defined(TINYYOLOV2)
                    box_w = (float) exp(tw) * anchors[anchor_offset+2*b+0] / (float) num_grid;
                    box_h = (float) exp(th) * anchors[anchor_offset+2*b+1] / (float) num_grid;
#endif
                    /* Adjustment for VGA size */
                    /* correct_yolo/region_boxes */
                    center_x = (center_x - (MODEL_IN_W - new_w) / 2. / MODEL_IN_W) / ((float) new_w / MODEL_IN_W);
                    center_y = (center_y - (MODEL_IN_H - new_h) / 2. / MODEL_IN_H) / ((float) new_h / MODEL_IN_H);
                    box_w *= (float) (MODEL_IN_W / new_w);
                    box_h *= (float) (MODEL_IN_H / new_h);

                    center_x = round(center_x * DRPAI_IN_WIDTH);
                    center_y = round(center_y * DRPAI_IN_HEIGHT);
                    box_w = round(box_w * DRPAI_IN_WIDTH);
                    box_h = round(box_h * DRPAI_IN_HEIGHT);

                    objectness = sigmoid(tc);

                    Box bb = {center_x, center_y, box_w, box_h};
                    /* Get the class prediction */
                    for (i = 0;i < n_class;i++)
                    {
                        classes[i] = sigmoid(floatarr[yolo_index(n, offs, 5+i)]);
                    }

                    max_pred = 0;
                    pred_class = -1;
                    for (i = 0; i < n_class; i++)
                    {
                        if (classes[i] > max_pred)
                        {
                            pred_class = i;
                            max_pred = classes[i];
                        }
                        //printf("RZ");
                    }

                    /* Store the result into the list if the probability is more than the threshold */
                    probability = max_pred * objectness;
                    if (probability > th_prbb)
                    {
                        d = {bb, pred_class, probability};
                        det.push_back(d);
                    }
                }
            }
        }
    }
    /* Non-Maximum Supression filter */
    filter_boxes_nms(det, det.size(), th_nmss);

    /* Render boxes on image and print their details */
//    n = 1;
//    for (i = 0;i < det.size(); i++)
//    {
        /* Skip the overlapped bounding boxes */
//        if (det[i].prob == 0) continue;

        /* Print the box details on console */
//        print_box(det[i], n++);

        /* Draw the bounding box on the image */
//        stringstream stream;
//        stream << fixed << setprecision(2) << det[i].prob;
//        string result_str = label_file_map[det[i].c]+ " "+ stream.str();
//        img->draw_rect((int32_t) det[i].bbox.x, (int32_t)det[i].bbox.y,
//            (int32_t)det[i].bbox.w, (int32_t)det[i].bbox.h, result_str.c_str());
//    }
    return 0;
}
//----------------------------------
//
// DRP-AI initialize
//
//----------------------------------------------------------------------------------
extern "C" int32_t drpai_ini(int8_t classNo)
{
    
    printf("RZ/V2L DRP-AI Sample Application\n");
    printf("Model : Darknet YOLO      | %s\n", drpai_prefix.c_str());
    int32_t ret = 0;
    int32_t read_ret = 0;
    
    
    printf("initialize class=%d <- %d\n",n_class,classNo);
    n_class=classNo;      
    errno = 0;
    
    fd = open("/sys/class/u-dma-buf/udmabuf0/phys_addr", O_RDONLY);
    if (0 > fd)
    {
        fprintf(stderr, "[ERROR] Failed to open udmabuf0/phys_addr : errno=%d\n", errno);
        return -1;
    }
    read_ret = read(fd, addr, 1024);
    if (0 > read_ret)
    {
        fprintf(stderr, "[ERROR] Failed to read udmabuf0/phys_addr : errno=%d\n", errno);
        close(fd);
        return -1;
    }
    sscanf(addr, "%lx", &udmabuf_address);
    close(fd);
    /* Filter the bit higher than 32 bit */
    udmabuf_address &=0xFFFFFFFF;

    /**********************************************************************/
    /* Inference preparation                                              */
    /**********************************************************************/
    /* Read DRP-AI Object files address and size */
    ret = read_addrmap_txt(drpai_address_file);
    if (0 != ret)
    {
        fprintf(stderr, "[ERROR] Failed to read addressmap text file: %s\n", drpai_address_file.c_str());
        ret = -1;
        goto end_main1;
    }
    /* Open DRP-AI Driver */
    errno = 0;
    drpai_fd = open("/dev/drpai0", O_RDWR);
    if (0 > drpai_fd)
    {
        fprintf(stderr, "[ERROR] Failed to open DRP-AI Driver: errno=%d\n", errno);
        ret = -1;
        goto end_main1;
    }
    /* Load DRP-AI Data from Filesystem to Memory via DRP-AI Driver */
    ret = load_drpai_data(drpai_fd);
    if (0 > ret)
    {
        fprintf(stderr, "[ERROR] Failed to load DRP-AI Object files.\n");
        ret = -1;
        goto end_close_drpai1;
    }

    /* Set DRP-AI Driver Input (DRP-AI Object files address and size)*/
    proc[DRPAI_INDEX_INPUT].address       = udmabuf_address;
    proc[DRPAI_INDEX_INPUT].size          = drpai_address.data_in_size;
    proc[DRPAI_INDEX_DRP_CFG].address     = drpai_address.drp_config_addr;
    proc[DRPAI_INDEX_DRP_CFG].size        = drpai_address.drp_config_size;
    proc[DRPAI_INDEX_DRP_PARAM].address   = drpai_address.drp_param_addr;
    proc[DRPAI_INDEX_DRP_PARAM].size      = drpai_address.drp_param_size;
    proc[DRPAI_INDEX_AIMAC_DESC].address  = drpai_address.desc_aimac_addr;
    proc[DRPAI_INDEX_AIMAC_DESC].size     = drpai_address.desc_aimac_size;
    proc[DRPAI_INDEX_DRP_DESC].address    = drpai_address.desc_drp_addr;
    proc[DRPAI_INDEX_DRP_DESC].size       = drpai_address.desc_drp_size;
    proc[DRPAI_INDEX_WEIGHT].address      = drpai_address.weight_addr;
    proc[DRPAI_INDEX_WEIGHT].size         = drpai_address.weight_size;
    proc[DRPAI_INDEX_OUTPUT].address      = drpai_address.data_out_addr;
    proc[DRPAI_INDEX_OUTPUT].size         = drpai_address.data_out_size;

end_close_drpai1:
    errno = 0;
    ret = close(drpai_fd);
    if (0 != ret)
    {
        fprintf(stderr, "[ERROR] Failed to close DRP-AI Driver: errno=%d\n", errno);
        ret = -1;
    }
    goto end_main1;

end_main1:
    return -999;
}


extern "C" {

    void get_string_array(char** &result, int* length)
     {
        const char* array[1024];
        string str[1024];
        char numStr0[256];
        char numStr1[256];
        char numStr2[256];
        char numStr3[256];
        char numStr4[256];
        char numStr5[256];
        int i;
        int j;
        int s;
        int p;
        int n;       
        s=det.size();
        n=0;
        for (i = 0;i < s; i++){
           /* Skip the overlapped bounding boxes */
           
           //if (det[i].prob == 0) continue;

           
           
           
           str[n*6+0]=to_string(det[i].c);
           array[n*6+0]=str[n*6+0].c_str();
           
           str[n*6+1]=to_string(int(det[i].prob*100));
           array[n*6+1]=str[n*6+1].c_str();
           
           str[n*6+2]=to_string(int(det[i].bbox.x));
           array[n*6+2]=str[n*6+2].c_str();   
                 
           str[n*6+3]=to_string(int(det[i].bbox.y));
           
           array[n*6+3]=str[n*6+3].c_str();
          
           str[n*6+4]=to_string(int(det[i].bbox.w));
           array[n*6+4]=str[n*6+4].c_str(); 
          
           str[n*6+5]=to_string(int(det[i].bbox.h));
           array[n*6+5]=str[n*6+5].c_str(); 
        
          // printf("(X, Y, W, H)    :- (%d,%d,%d,%d)\n",(int32_t) det[i].bbox.x, (int32_t) det[i].bbox.y, (int32_t) det[i].bbox.w, (int32_t) det[i].bbox.h);
            
           n=n+1;
           
        }
      
       *length = s*6;
       result = new char*[*length];
       
       for (j = 0; j < *length; j++) {
          result[j] = new char[strlen(array[j]) + 1];
          strcpy(result[j], array[j]);
       }
        
    }
}

//------------------------------------------------
//
// DRP-AI inference
//
//------------------------------------------------

extern "C" {int32_t drpai_inf(unsigned char *c_buf,int8_t th_prb,int8_t th_nms){
    Image img;
    int32_t ret = 0;
    th_prbb=th_prb/100.;
    th_nmss=th_nms/100.;
    printf("prob nms=%d %d\n",th_prb,th_nms);
    
    img.init(DRPAI_IN_WIDTH, DRPAI_IN_HEIGHT, DRPAI_IN_CHANNEL_BGR);
    ret = img.get_bmp(c_buf);
    
    /**********************************************************************
    * START Inference
    **********************************************************************/
    printf("[START] DRP-AI\n");
    errno = 0;
    drpai_fd = open("/dev/drpai0", O_RDWR);
    ret = ioctl(drpai_fd, DRPAI_START, &proc[0]);
    if (0 != ret)
    {
        fprintf(stderr, "[ERROR] Failed to run DRPAI_START: errno=%d\n", errno);
        ret = -1;
        goto end_close_drpai;
    }

    /**********************************************************************
    * Wait until the DRP-AI finish (Thread will sleep)
    **********************************************************************/
    FD_ZERO(&rfds);
    FD_SET(drpai_fd, &rfds);
    tv.tv_sec = DRPAI_TIMEOUT;
    tv.tv_usec = 0;

    ret_drpai = select(drpai_fd+1, &rfds, NULL, NULL, &tv);
    if (0 == ret_drpai)
    {
        fprintf(stderr, "[ERROR] DRP-AI select() Timeout : errno=%d\n", errno);
        ret = -1;
        goto end_close_drpai;
    }
    else if (-1 == ret_drpai)
    {
        fprintf(stderr, "[ERROR] DRP-AI select() Error : errno=%d\n", errno);
        ret_drpai = ioctl(drpai_fd, DRPAI_GET_STATUS, &drpai_status);
        if (-1 == ret_drpai)
        {
            fprintf(stderr, "[ERROR] Failed to run DRPAI_GET_STATUS : errno=%d\n", errno);
        }
        ret = -1;
        goto end_close_drpai;
    }
    else
    {
        /*Do nothing*/
    }

    if (FD_ISSET(drpai_fd, &rfds))
    {
        errno = 0;
        ret_drpai = ioctl(drpai_fd, DRPAI_GET_STATUS, &drpai_status);
        if (-1 == ret_drpai)
        {
            fprintf(stderr, "[ERROR] Failed to run DRPAI_GET_STATUS : errno=%d\n", errno);
            ret = -1;
            goto end_close_drpai;
        }
        printf("[END] DRP-AI\n");
    }

    /**********************************************************************
    * CPU Post-processing
    **********************************************************************/
    /* Get the output data from memory */
    ret = get_result(drpai_fd, drpai_address.data_out_addr, drpai_address.data_out_size);
    if (0 != ret)
    {
        fprintf(stderr, "[ERROR] Failed to get result from memory.\n");
        ret = -1;
        goto end_close_drpai;
    }
    /* Compute the result, draw the result on img and display it on console */
    ret = print_result_yolo(drpai_output_buf, &img);
   
    if (0 != ret)
    {
        fprintf(stderr, "[ERROR] Failed to run CPU Post Processing.\n");
        ret = -1;
        goto end_close_drpai;
    }

    /* Terminating process */
end_close_drpai:
    errno = 0;
    ret = close(drpai_fd);
    if (0 != ret)
    {
        fprintf(stderr, "[ERROR] Failed to close DRP-AI Driver: errno=%d\n", errno);
        ret = -1;
    }
    goto end_main;

end_main:
    return ret;
}}
