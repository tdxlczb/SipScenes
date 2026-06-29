#ifndef _GB28181_MANSCDP_QUERY_H_
#define _GB28181_MANSCDP_QUERY_H_

#include "manscdp_base.h"

namespace gb28181 {

struct QueryBase : public ManscdpBase
{
    QueryBase() {
        ManscdpType = kQuery;
    };
};

//==================== DeviceStatus ====================
// 设备状态查询请求
using QueryDeviceStatus = QueryBase;
//==================== DeviceStatus ====================


//==================== Catalog ====================
// 设备目录信息查询请求
struct QueryCatalog : public QueryBase
{
    // 增加设备的起始时间(可选)空表示不限
    dateTimeType StartTime;
    // 增加设备的终止时间(可选)空表示到当前时间
    dateTimeType EndTime;
};
//==================== Catalog ====================


//==================== DeviceInfo ====================
// 设备信息查询请求
using QueryDeviceInfo = QueryBase;
//==================== DeviceInfo ====================


//==================== RecordInfo ====================
// 文件目录检索请求
struct QueryRecordInfo : public QueryBase
{
    // 录像起始时间(必选)
    dateTimeType StartTime;
    // 录像终止时间(必选)
    dateTimeType EndTime;
    // 文件路径名 (可选)
    std::string FilePath;
    // 录像地址(可选 支持不完全查询)
    std::string Address;
    // 保密属性(可选)缺省为0; 0:不涉密; 1:涉密
    int Secrecy = 0;
    // 录像产生类型(可选) time 或 alarm 或 manual 或 all
    std::string Type;
    // 录像触发者ID(可选)
    std::string RecorderID;
    // 录像模糊查询属性(可选)缺省为0; 文档上格式是字符串，缺省是0
    // 0:不进行模糊查询,此时根据SIP消息中To头域URI中的ID值确定查询录像位置, 若ID值为本域系统ID则进行中心历史记录检索, 若为前端设备ID则进行前端设备历史记录检索; 
    // 1:进行模糊查询, 此时设备所在域应同时进行中心检索和前端检索并将结果统一返回。
    std::string IndistinctQuery;
};
//==================== RecordInfo ====================


std::string BuildQuery(const QueryBase& manscdp);
std::string BuildQueryCatalog(const QueryCatalog& manscdp);


} // namespace gb28181

#endif // _GB28181_MANSCDP_QUERY_H_
