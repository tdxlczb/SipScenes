#ifndef _GB28181_STREAM_H_
#define _GB28181_STREAM_H_

#include <string>

namespace gb28181 {

struct SSRCTran {
	std::string callId;
	std::string fromTag;
	std::string toTag;
	std::string branch;
};

struct SSRCInfo
{
	int port = 0;
	std::string ssrc;
	std::string app;
	std::string Stream;
	std::string timeOutTaskKey;
};

struct Stream
{
	std::string id;// 流id
	std::string deviceId;// 设备id
	std::string channelId;// 通道id
	SSRCTran ssrcTran; // SSRC 事务, 用于播放暂停、恢复
	SSRCInfo ssrc;
};

//外购nvr设备暂停1分钟之后就恢复不了，设备不支持长时间暂停
std::string BuildPlayPauseCmd(int cseq);
std::string BuildPlayResumeCmd(int cseq);
std::string BuildPlaySeekCmd(int cseq, int64_t seekTime);
std::string BuildPlaySpeedCmd(int cseq, double scale);

} // namespace gb28181

#endif // _GB28181_STREAM_H_
