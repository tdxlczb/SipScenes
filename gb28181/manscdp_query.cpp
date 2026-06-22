/**
*
================================================
<?xml version="1.0" encoding="GB2312"?>
<Query>
  <CmdType>PresetQuery</CmdType>
  <SN>8150640</SN>
  <DeviceID>34020000001320000002</DeviceID>
</Query>
================================================

================================================

*/

#include "manscdp_query.h"
#include <atomic>
#include <chrono>

namespace gb28181 {

std::string BuildQuery(const QueryBase& manscdp)
{
    std::string xml;
    xml.append("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
    xml.append("<Query>\r\n");
    xml.append("  <CmdType>" + manscdp.CmdType + "</CmdType>\r\n");
    xml.append("  <SN>" + std::to_string(manscdp.SN) + "</SN>\r\n");
    xml.append("  <DeviceID>" + manscdp.DeviceID + "</DeviceID>\r\n");
    xml.append("</Query>\r\n");
    return xml;
}

std::string BuildQueryCatalog(const QueryCatalog& manscdp)
{
    std::string xml;
    xml.append("<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n");
    xml.append("<Query>\r\n");
    xml.append("  <CmdType>" + manscdp.CmdType + "</CmdType>\r\n");
    xml.append("  <SN>" + std::to_string(manscdp.SN) + "</SN>\r\n");
    xml.append("  <DeviceID>" + manscdp.DeviceID + "</DeviceID>\r\n");
    if (!manscdp.StartTime.empty()) {
        xml.append("  <StartTime>" + manscdp.StartTime + "</StartTime>\r\n");
    }
    if (!manscdp.EndTime.empty()) {
        xml.append("  <EndTime>" + manscdp.EndTime + "</EndTime>\r\n");
    }
    xml.append("</Query>\r\n");
    return xml;
}



} // namespace gb28181