#ifndef _GB28181_MANSCDP_NOTIFY_H_
#define _GB28181_MANSCDP_NOTIFY_H_

#include "manscdp_base.h"

namespace gb28181 {

using NotifyBase = ManscdpBase;

//==================== Keepalive ====================
//状态信息通知
struct NotifyKeepalive : public NotifyBase
{
    // 是否正常工作(必选)
    resultType Status = kResultERROR;
    // 还有一个故障设备列表
};
//==================== Keepalive ====================



} // namespace gb28181

#endif // _GB28181_MANSCDP_NOTIFY_H_
