#ifndef _GB28181_MANSCDP_CONTROL_TYPE_H_
#define _GB28181_MANSCDP_CONTROL_TYPE_H_

#include <string>
/*
* 设备控制类型定义
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
//字节1 : 指令的首字节为 A5H。
//字节2 : 组合码1, 高4位是版本信息, 低4位是校验位。本标准的版本号是1.0, 版本信息为0H。校验位 = (字节1的高4位 + 字节1的低4位 + 字节2的高4位) % 16。
//字节3 : 地址的低8位。
//字节4 : 指令码。
//字节5 : 数据1。
//字节6 : 数据2。
//字节7 : 组合码2, 高4位是数据3, 低4位是地址的高4位; 在后续叙述中, 没有特别指明的高4位, 表示该4位与所指定的功能无关。
//字节8 : 校验码, 为前面的第1~7字节的算术和的低8位, 即算术和对256取模后的结果。字节8 = (字节1 + 字节2 + 字节3 + 字节4 + 字节5 + 字节6 + 字节7) % 256。
// 
//地址范围000H~FFFH(即0~4095), 其中000H 地址作为广播地址。
//注 : 前端设备控制中, 不使用字节3和字节7的低4位地址码, 使用前端设备控制消息体中的<DeviceID>统一编码标识控制的前端设备。
//

namespace gb28181 {


/**
 * @brief 生成指令字符串，前3个字节固定为A50F01
 * @param cmdCode       字节4指令码
 * @param parameter1    字节5数据1
 * @param parameter2    字节6数据2
 * @param combineCode2  字节7组合码2
 * @return
 */
std::string CreateCmdString(uint8_t cmdCode, uint8_t parameter1, uint8_t parameter2, uint8_t combineCode2);


///////////////////////////////////////////////////////////////////////////////////////////////////
// 字节   |bit7     |bit6     |bit5     |bit4     |bit3     |bit2     |bit1     |bit0     |
// 字节4: |0        |0        |out(-)   |in(+)    |up       |down     |left     |right    |

struct PTZCmdType
{
    int leftRight = 0;  //云台左右： -1: 左, 0: 停止, 1: 右
    int upDown = 0;     //云台上下： -1: 上, 0: 停止, 1: 下
    int inOut = 0;      //镜头缩放： -1: 缩小, 0: 停止, 1: 放大
    int horSpeed = 0;   //左右移动速度：0~255
    int verSpeed = 0;   //上下移动速度：0~255
    int zoomSpeed = 0;  //镜头缩放速度：0~15

    std::string ToString() const;
    void FromString(const std::string& str);

};




} // namespace gb28181

#endif // _GB28181_MANSCDP_CONTROL_TYPE_H_
