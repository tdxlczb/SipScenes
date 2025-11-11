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
 * SIP的Invite请求地址中是使用的设备ID，官方文档的示例中使用的是一个媒体服务器的ID。
 * GB28181-2022的9.2.3中有提及，o行中的username应为本设备的设备编码，即为发送方的编码
 *
 * s字段，这里与[RFC4566]中不同的是GB28181中限定了只能是以下值
 * Play代表实时点播；Playback代表历史回放；Download代表文件下载；Talk代表语音对讲。
 *
 * GB28181中扩展的两个描述字段f和y
 * f字段描述了音视频格式相关参数。使用f字段时应保证视频和音频参数的结构完整性，即在任何时候f字段的结构都应是完整的结构
 *  f=v/编码格式/分辨率/帧率/码率类型/码率大小a/编码格式/码率大小/采样率
 *
 * y字段为10位的十进制整数字符串，格式为dddddddddd，表示SSRC值。
 * 第1位为历史或实时媒体流的标识位，0为实时，1为历史；
 * 第2位至第6位取20位SIP监控域ID之中的4到8位，例如"34020000002000000001"中取"20000"； 
 * 第7位至第10位作为域内媒体流标识，需要与当前域内产生的媒体流SSRC后4位不重复
 * 注意：某些厂家的实现为时效标识直接和SSRC的组合
 *  例如：y=0200000001
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
    std::string deviceId;
    std::string channelId;
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
