/**
*
//请求点播的sdp，使用udp，播主码流
================================================
v=0
o=34020000001320000002 0 0 IN IP4 172.16.19.108
s=Play
c=IN IP4 172.16.19.108
t=0 0
m=video 30884 RTP/AVP 96 97 98 99
a=recvonly
a=rtpmap:96 PS/90000
a=rtpmap:97 MPEG4/90000
a=rtpmap:98 H264/90000
a=rtpmap:99 H265/90000
a=streamnumber:0
y=0200000001
f=
================================================
//请求回放的sdp，使用Tcp主动，播子码流1
================================================
v=0
o=34020000001320000002 0 0 IN IP4 172.16.19.108
s=PlayBack
c=IN IP4 172.16.19.108
t=1760457600 1760543999
m=video 30884 TCP/RTP/AVP 96 97 98 99
a=rtpmap:96 PS/90000
a=rtpmap:97 MPEG4/90000
a=rtpmap:98 H264/90000
a=rtpmap:99 H265/90000
a=recvonly
a=setup:active
a=connection:new
a=streamnumber:1
y=1200000001
f=
================================================
*/
#include "sdp.h"
#include <sstream>

namespace gb28181 {

std::string BuildInvateRequestSdp(const SdpParam& sdpParam)
{
    std::string content;
    content.append("v=0\r\n");
    content.append("o=" + sdpParam.id + " 0 0 IN IP4 " + sdpParam.ip + "\r\n");
    content.append("s=" + sdpParam.type + "\r\n");
    content.append("u=" + sdpParam.id + ":0\r\n");
    content.append("c=IN IP4 " + sdpParam.ip + "\r\n");
    content.append("t=" + std::to_string(sdpParam.start) + " " + std::to_string(sdpParam.end) + "\r\n");
    if (sdpParam.mode == kTransTcpPassive) {
        content.append("m=video " + std::to_string(sdpParam.port) + " TCP/RTP/AVP 96 97 98 99\r\n");
    }
    else if (sdpParam.mode == kTransTcpActive) {
        content.append("m=video " + std::to_string(sdpParam.port) + " TCP/RTP/AVP 96 97 98 99\r\n");
    }
    else {
        content.append("m=video " + std::to_string(sdpParam.port) + " RTP/AVP 96 97 98 99\r\n");
    }
    content.append("a=recvonly\r\n");
    content.append("a=rtpmap:96 PS/90000\r\n");
    content.append("a=rtpmap:97 MPEG4/90000\r\n");
    content.append("a=rtpmap:98 H264/90000\r\n");
    content.append("a=rtpmap:99 H265/90000\r\n");

    if (sdpParam.mode == kTransTcpPassive) {
        // tcp被动模式TCP-PASSIVE
        content.append("a=setup:passive\r\n");
        content.append("a=connection:new\r\n");
    }
    else if (sdpParam.mode == kTransTcpActive) {
        // tcp主动模式TCP-ACTIVE
        content.append("a=setup:active\r\n");
        content.append("a=connection:new\r\n");
    }
    content.append("a=streamnumber:" + std::to_string(sdpParam.streamnumber) + "\r\n");
    content.append("y=" + sdpParam.ssrc + "\r\n");
    return content;
}

}