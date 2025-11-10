#include "stream.h"
#include <sstream>
#include <iomanip>
#include <iostream>

namespace gb28181 {

std::string BuildPlayPauseCmd(int cseq)
{
    std::string content;
    content.append("PAUSE RTSP/1.0\r\n");
    content.append("CSeq: " + std::to_string(cseq) + "\r\n");
    content.append("PauseTime: now\r\n");
    return content;
}

std::string BuildPlayResumeCmd(int cseq)
{
    std::string content;
    content.append("PLAY RTSP/1.0\r\n");
    content.append("CSeq: " + std::to_string(cseq) + "\r\n");
    content.append("Range: npt=now-\r\n");
    return content;
}

std::string BuildPlaySeekCmd(int cseq, int64_t seekTime)
{
    std::string content;
    content.append("PLAY RTSP/1.0\r\n");
    content.append("CSeq: " + std::to_string(cseq) + "\r\n");
    content.append("Range: npt=" + std::to_string(seekTime) + "-\r\n");
    return content;
}

std::string BuildPlaySpeedCmd(int cseq, double scale)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6) << scale;

    std::string content;
    content.append("PLAY RTSP/1.0\r\n");
    content.append("CSeq: " + std::to_string(cseq) + "\r\n");
    content.append("Scale: " + oss.str() + "\r\n");
    return content;
}

} // namespace gb28181