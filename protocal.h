#ifndef PROTOCAL_H
#define PROTOCAL_H

const char ZHU_JI_WEN_DU_REQ = 0x01;    //主机温度查询命令
const char ZHU_JI_WEN_DU_RSP = 0x02;    //主机温度响应命令
const char ZHOU_SU_DU_REQ = 0X03;       //工作轴转速查询命令
const char ZHOU_SU_DU_RSP = 0x04;       //工作轴转速响应命令
const char YA_LI_CHANG_REQ = 0x05;      //压力仓压力查询命令
const char YA_LI_CHANG_RSP = 0x06;      //压力仓压力响应命令
const char GANG_YOU_LIANG_REQ = 0x07;   //工作缸油量查询命令
const char GANG_YOU_LIANG_RSP = 0x08;   //工作缸油量响应命令
const char JI_XIE_BI_REQ = 0x09;        //机械臂压力查询命令
const char JI_XIE_BI_RSP = 0x0A;        //机械臂压力响应命令
const char DONG_LI_GAN_SU_DU_REQ = 0x0B;//动力杆命令查询命令
const char DONG_LI_GAN_SU_DU_RSP = 0x0C;//动力杆命令响应命令

#endif // PROTOCAL_H
