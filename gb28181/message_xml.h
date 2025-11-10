/**
* XML消息中，SN值只用于匹配请求和响应的XML消息，并且保证本端保证唯一和单调递增，推荐基于时间戳生成8-12位的数字字符串
* 
* 
* 
*/


#ifndef _GB28181_MESSAGE_XML_H_
#define _GB28181_MESSAGE_XML_H_

#include <string>

namespace gb28181 {

using XmlType = std::string;
const std::string kXmlControl = "Control";
const std::string kXmlQuery = "Query";
const std::string kXmlNotify = "Notify";
const std::string kXmlResponse = "Response";

using CmdType = std::string;
const std::string kCmdPresetQuery = "PresetQuery";
const std::string kCmdCatalog = "Catalog";

struct XmlParam
{
    CmdType cmdType;
    std::string sn;
    std::string deviceId;
};

std::string GetSN();

std::string BuildQueryXml(const XmlParam& xmlParam);

void Test();


}
#endif // _GB28181_MESSAGE_XML_H_
