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
<?xml version="1.0" encoding="GB2312" ?>
<Response>
    <CmdType>Catalog</CmdType>
    <SN>473175156</SN>
    <DeviceID>34020000001180000199</DeviceID>
    <SumNum>12</SumNum>
    <DeviceList Num="1">
        <Item>
            <DeviceID>34020000001320000001</DeviceID>
            <Name>IP CAMERA</Name>
            <Manufacturer>Manufacturer</Manufacturer>
            <Model>Camera</Model>
            <Owner>Owner</Owner>
            <CivilCode>CivilCode</CivilCode>
            <Address>172.16.19.236</Address>
            <Parental>0</Parental>
            <ParentID>34020000001180000199</ParentID>
            <SafetyWay>0</SafetyWay>
            <RegisterWay>1</RegisterWay>
            <Secrecy>0</Secrecy>
            <IPAddress>172.16.19.184</IPAddress>
            <Port>5060</Port>
            <Status>ON</Status>
        </Item>
    </DeviceList>
</Response>
================================================

================================================

*/

#include "manscdp.h"
#include <string>
#include <atomic>
#include <chrono>

#include "../3rd/tinyxml2/include/tinyxml2.h"

namespace gb28181 {

//str转int
bool str2int(const std::string& s, int& out)
{
    char* end = nullptr;
    errno = 0;
    long val = std::strtol(s.c_str(), &end, 10);

    // 1. 空串 / 无数字
    if (end == s.c_str()) return false;
    // 2. 后缀非法字符
    if (*end != '\0')       return false;
    // 3. 越界（long 转 int）
    if (errno == ERANGE || val < INT_MIN || val > INT_MAX) return false;

    out = static_cast<int>(val);
    return true;
}

// 线程安全、进程级单调递增
int GetSN() {
    static std::atomic<uint64_t> seq{ 0 };
    // 毫秒时间戳取低 32 位 + 自增 24 位 → 最多 56 位，字符串长度 <= 17
    uint64_t t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    uint64_t s = seq.fetch_add(1, std::memory_order_relaxed);
    return (t & 0xFFFFFFFF) * 1000000 + (s & 0xFFFFFF);
}

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



std::string GetChildTextString(tinyxml2::XMLElement* elmt, const std::string& name, const std::string& defaultValue = "")
{
    tinyxml2::XMLElement* childElmt = elmt->FirstChildElement(name.c_str());
    if (childElmt) {
        return std::string(childElmt->GetText());
    }
    return defaultValue;
}

int GetChildTextInt(tinyxml2::XMLElement* elmt, const std::string& name, int defaultValue = 0)
{
    tinyxml2::XMLElement* childElmt = elmt->FirstChildElement(name.c_str());
    if (childElmt) {
        int result = 0;
        if (str2int(childElmt->GetText(), result))
            return result;
    }
    return defaultValue;
}

itemType GetItem(tinyxml2::XMLElement* elmt)
{
    itemType item;
    if (!elmt)
        return item;
    item.DeviceID = GetChildTextString(elmt, "DeviceID");
    item.Name = GetChildTextString(elmt, "Name");
    item.Manufacturer = GetChildTextString(elmt, "Manufacturer");
    item.Model = GetChildTextString(elmt, "Model");
    item.Owner = GetChildTextString(elmt, "Owner");
    item.CivilCode = GetChildTextString(elmt, "CivilCode");
    item.Address = GetChildTextString(elmt, "Address");
    item.Parental = GetChildTextInt(elmt, "Parental");
    item.ParentID = GetChildTextString(elmt, "ParentID");
    item.SafetyWay = GetChildTextInt(elmt, "SafetyWay");
    item.RegisterWay = GetChildTextInt(elmt, "RegisterWay");
    item.Secrecy = GetChildTextInt(elmt, "Secrecy");
    item.IPAddress = GetChildTextString(elmt, "IPAddress");
    item.Port = GetChildTextInt(elmt, "Port");
    std::string statusStr = GetChildTextString(elmt, "Status");
    if (statusStr == "ON") {
        item.Status = kStatusON;
    }
    return item;
}

Catalog GetCatalog(const std::string& xml)
{
    Catalog rsp;
    tinyxml2::XMLDocument docXml;
    tinyxml2::XMLError errXml = docXml.Parse(xml.c_str());
    if (tinyxml2::XML_SUCCESS != errXml)
        return rsp;

    tinyxml2::XMLElement* elmtRoot = docXml.RootElement();
    if (!elmtRoot)
        return rsp;
    const char* rootName = elmtRoot->Name();
    if (!rootName)
        return rsp;
    if (std::string(rootName) != kResponse)
        return rsp;

    rsp.CmdType = GetChildTextString(elmtRoot, "CmdType");
    if (rsp.CmdType != kCatalog)
        return rsp;

    rsp.SN = GetChildTextInt(elmtRoot, "SN");
    rsp.DeviceID = GetChildTextString(elmtRoot, "DeviceID");
    rsp.SumNum = GetChildTextInt(elmtRoot, "SumNum");

    tinyxml2::XMLElement* DeviceList = elmtRoot->FirstChildElement("DeviceList");
    if (DeviceList) {
        for (auto elmt = DeviceList->FirstChildElement("Item"); elmt != nullptr; elmt = elmt->NextSiblingElement("Item"))
        {
            itemType item = GetItem(elmt);
            rsp.DeviceList.push_back(item);
        }
    }
    return rsp;
}

}