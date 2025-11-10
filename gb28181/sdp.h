/**
 * 符合[RFC4566]并且可以在GB28181中使用的SDP内容
 * ================================================
 *  v=0
 *  o=国标ID 0 0 IN IP4 创建会话的计算机的地址
 *  s=Play
 *  c=IN IP4 流媒体接受者的地址
 *  t=0 0
 *  m=video 流媒体接受者端口号 RTP/AVP 96 97 98
 *  a=rtpmap:96 PS/90000
 *  a=rtpmap:97 MPEG4/90000
 *  a=rtpmap:98 H264/90000
 *  a=recvonly
 * ================================================
 *
 * o字段的国标ID可以填写媒体所有者（平台网关ID、媒体服务器ID）的ID或者媒体创建者（前端设备ID）的ID
 * 这里建议填写媒体所有者的ID，因为SIP的Invite请求地址中是使用的设备ID，并且官方文档的示例中使用的是一个媒体服务器的ID。
 *
 * s字段，这里与[RFC4566]中不同的是GB28181中限定了只能是以下值
 * Play代表实时点播；Playback代表历史回放；Download代表文件下载；Talk代表语音对讲。
 *
 * GB28181中扩展的两个描述字段f和y
 * f字段描述了音视频格式相关参数。使用f字段时应保证视频和音频参数的结构完整性，即在任何时候f字段的结构都应是完整的结构
 *  f=v/编码格式/分辨率/帧率/码率类型/码率大小a/编码格式/码率大小/采样率
 *
 * y字段描述了时效和SSRC，0为实时，1为历史
 * 第1位表示是否为历史 + 第2位至第6位取20位SIP监控域ID之中的4到8位 + 第7位至第10位是SSRC的后4位
 * 例如：Invite 12345678901234567890, SSRC: 0x063497bd(104110013)
 *  y=0456780013
 * 注意：某些厂家的实现为时效标识直接和SSRC的组合
 *  y=0104110013
 * 建议不要使用y字段。。。
 */

#ifndef _GB28181_SDP_H_
#define _GB28181_SDP_H_

#include <string>

namespace gb28181 {

using TransMode = int;//0: 不监听端口 1: 监听端口 2: 主动连接到服务端
const int kTransUdp = 0;         //UDP
const int kTransTcpPassive = 1;  //TCP-PASSIVE
const int kTransTcpActive = 2;   //TCP-ACTIVE

using TransType = std::string;
const std::string kTransPlay = "Play";
const std::string kTransPlayBack = "PlayBack";
const std::string kTransDownload = "Download";
const std::string kTransTalk = "Talk";

struct SdpParam
{
    std::string id;
    std::string ip;
    int port = 0;
    TransMode mode = kTransUdp;
    TransType type = kTransPlay;
    int start = 0; //时间戳
    int end = 0; //时间戳
    int streamnumber = 0;
    std::string ssrc;
};

/**
 * 创建Invate请求中的sdp消息
 * @param sdp : sdp参数
 * @return sdp字符串
 */
std::string BuildInvateRequestSdp(const SdpParam& sdpParam);


}
#endif // _GB28181_SDP_H_
