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
    <SN>553891</SN>
    <DeviceID>34020000001180000002</DeviceID>
    <SumNum>1</SumNum>
    <DeviceList Num="1">
        <Item>
            <DeviceID>34020000001320000002</DeviceID>
            <Name>47.5········ipc</Name>
            <Manufacturer>Manufacturer</Manufacturer>
            <Model>Camera</Model>
            <Owner>Owner</Owner>
            <CivilCode>CivilCode</CivilCode>
            <Address>172.16.47.5</Address>
            <Parental>0</Parental>
            <ParentID>34020000001180000002</ParentID>
            <SafetyWay>0</SafetyWay>
            <RegisterWay>1</RegisterWay>
            <Secrecy>0</Secrecy>
            <IPAddress>172.16.47.168</IPAddress>
            <Port>15060</Port>
            <Status>ON</Status>
        </Item>
    </DeviceList>
</Response>
================================================
<?xml version="1.0" encoding="GB2312" ?>
<Response>
    <CmdType>Catalog</CmdType>
    <SN>553891</SN>
    <DeviceID>34020000001180000002</DeviceID>
    <SumNum>1</SumNum>
    <DeviceList Num="1">
        <Item>
            <DeviceID>34020000001320000002</DeviceID>
            <Name>47.5········ipc</Name>
            <Manufacturer>Manufacturer</Manufacturer>
            <Model>Camera</Model>
            <Owner>Owner</Owner>
            <CivilCode>CivilCode</CivilCode>
            <Address>172.16.47.5</Address>
            <Parental>0</Parental>
            <ParentID>34020000001180000002</ParentID>
            <SafetyWay>0</SafetyWay>
            <RegisterWay>1</RegisterWay>
            <Secrecy>0</Secrecy>
            <IPAddress>172.16.47.168</IPAddress>
            <Port>15060</Port>
            <Status>ON</Status>
        </Item>
    </DeviceList>
</Response>
================================================
*/

#include "message_xml.h"
#include <string>
#include <atomic>
#include <chrono>

#include "../3rd/tinyxml2/include/tinyxml2.h"

namespace gb28181 {

// 线程安全、进程级单调递增
std::string GetSN() {
    static std::atomic<uint64_t> seq{ 0 };
    // 毫秒时间戳取低 32 位 + 自增 24 位 → 最多 56 位，字符串长度 <= 17
    uint64_t t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    uint64_t s = seq.fetch_add(1, std::memory_order_relaxed);
    return std::to_string((t & 0xFFFFFFFF) * 1000000 + (s & 0xFFFFFF));
}

std::string BuildQueryXml(const XmlParam& xmlParam)
{
    char xml[2048] = { 0 };
    sprintf_s(xml, sizeof(xml),
        "<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n"
        "<Query>\r\n"
        "  <CmdType>%s</CmdType>\r\n"
        "  <SN>%s</SN>\r\n"
        "  <DeviceID>%s</DeviceID>\r\n"
        "</Query>\r\n",
        xmlParam.cmdType.c_str(), xmlParam.sn.c_str(), xmlParam.deviceId.c_str());
    return std::string(xml);
}


std::string GetChildText(tinyxml2::XMLElement* elmt, const std::string& name)
{
    tinyxml2::XMLElement* childElmt = elmt->FirstChildElement(name.c_str());
    if (childElmt) {
        return std::string(childElmt->GetText());
    }
    return std::string();
}

void Test()
{

    std::string xml = R"(
<?xml version="1.0" encoding="GB2312" ?>
<Response>
    <CmdType>Catalog</CmdType>
    <SN>553891</SN>
    <DeviceID>34020000001180000002</DeviceID>
    <SumNum>1</SumNum>
    <DeviceList Num="1">
        <Item>
            <DeviceID>34020000001320000002</DeviceID>
            <Name>47.5········ipc</Name>
            <Manufacturer>Manufacturer</Manufacturer>
            <Model>Camera</Model>
            <Owner>Owner</Owner>
            <CivilCode>CivilCode</CivilCode>
            <Address>172.16.47.5</Address>
            <Parental>0</Parental>
            <ParentID>34020000001180000002</ParentID>
            <SafetyWay>0</SafetyWay>
            <RegisterWay>1</RegisterWay>
            <Secrecy>0</Secrecy>
            <IPAddress>172.16.47.168</IPAddress>
            <Port>15060</Port>
            <Status>ON</Status>
        </Item>
    </DeviceList>
</Response>
)";

    struct ResponseXml
    {
        CmdType cmdType;
        std::string sn;
        std::string deviceId;
    };

    ResponseXml rspXml;


    tinyxml2::XMLDocument docXml;
    tinyxml2::XMLError errXml = docXml.Parse(xml.c_str());
    if (tinyxml2::XML_SUCCESS != errXml)
        return;

    tinyxml2::XMLElement* elmtRoot = docXml.RootElement();
    if (!elmtRoot)
        return;
    const char* rootName = elmtRoot->Name();
    if (!rootName)
        return;
    if (std::string(rootName) == kXmlResponse) {

    }

    rspXml.cmdType = GetChildText(elmtRoot, "CmdType");
    rspXml.sn = GetChildText(elmtRoot, "SN");
    rspXml.deviceId = GetChildText(elmtRoot, "DeviceId");

    tinyxml2::XMLElement* deviceList = elmtRoot->FirstChildElement("DeviceList");
    tinyxml2::XMLElement* elmtAge = elmtRoot->NextSiblingElement();

    



}

}