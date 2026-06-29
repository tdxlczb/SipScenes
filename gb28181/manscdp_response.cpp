/**
*
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

#include "manscdp_response.h"
#include "pugixml/pugixml.hpp"

namespace gb28181 {


static itemType GetItem(pugi::xml_node itemNode)
{
    itemType item;
    if (itemNode.empty())
        return item;
    item.DeviceID = itemNode.child("DeviceID").text().as_string();
    item.Name = itemNode.child("Name").text().as_string();
    item.Manufacturer = itemNode.child("Manufacturer").text().as_string();
    item.Model = itemNode.child("Model").text().as_string();
    item.Owner = itemNode.child("Owner").text().as_string();
    item.CivilCode = itemNode.child("CivilCode").text().as_string();
    item.Block = itemNode.child("Block").text().as_string();
    item.Address = itemNode.child("Address").text().as_string();
    item.Parental = itemNode.child("Parental").text().as_int();
    item.ParentID = itemNode.child("ParentID").text().as_string();
    item.SafetyWay = itemNode.child("SafetyWay").text().as_int();
    item.RegisterWay = itemNode.child("RegisterWay").text().as_int(1);
    item.CertNum = itemNode.child("CertNum").text().as_string();
    item.Certifiable = itemNode.child("Certifiable").text().as_int();
    item.ErrCode = itemNode.child("ErrCode").text().as_int();
    item.EndTime = itemNode.child("EndTime").text().as_string();
    item.Secrecy = itemNode.child("Secrecy").text().as_int();
    item.IPAddress = itemNode.child("IPAddress").text().as_string();
    item.Port = itemNode.child("Port").text().as_int();
    item.Password = itemNode.child("Password").text().as_string();
    item.Status = itemNode.child("Status").text().as_string();
    item.Longitude = itemNode.child("Longitude").text().as_double();
    item.Latitude = itemNode.child("Latitude").text().as_double();
    // 检测是否存在 <Info> 子节点
    pugi::xml_node infoNode = itemNode.child("Info");
    if (infoNode) {
        item.PTZType = infoNode.child("PTZType").text().as_int();
        item.PositionType = infoNode.child("PositionType").text().as_int();
        item.RoomType = infoNode.child("RoomType").text().as_int(1);
        item.UseType = infoNode.child("UseType").text().as_int();
        item.SupplyLightType = infoNode.child("SupplyLightType").text().as_int(1);
        item.DirectionType = infoNode.child("DirectionType").text().as_int();
        item.Resolution = infoNode.child("Resolution").text().as_string();
        item.BusinessGroupID = infoNode.child("BusinessGroupID").text().as_string();
        item.DownloadSpeed = infoNode.child("DownloadSpeed").text().as_string();
        item.SVCSpaceSupportMode = infoNode.child("SVCSpaceSupportMode").text().as_int();
        item.SVCTimeSupportMode = infoNode.child("SVCTimeSupportMode").text().as_int();
    }
    return item;
}

ResponseCatalog GetResponseCatalog(const std::string& xml)
{
    ResponseCatalog rsp;
    try {
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_string(xml.c_str(), pugi::parse_default);
        if (!result) {
            return rsp;
        }
        pugi::xml_node root = doc.child(std::data(kResponse));
        if (!root) {
            return rsp;
        }
        rsp.CmdType = root.child("CmdType").text().as_string();
        if (rsp.CmdType != kCatalog)
            return rsp;

        rsp.SN = root.child("SN").text().as_int();
        rsp.DeviceID = root.child("DeviceID").text().as_string();
        rsp.SumNum = root.child("SumNum").text().as_int();

        pugi::xml_node deviceList = root.child("DeviceList");
        if (!deviceList) {
            return rsp;
        }
        int listNum = deviceList.attribute("Num").as_int();
        for (pugi::xml_node itemNode : deviceList.children("Item")) {
            itemType item = GetItem(itemNode);
            rsp.DeviceList.push_back(item);
        }
        return rsp;
    } catch (const std::bad_alloc& e) {
        // 【安全策略4】捕获内存不足异常
        //std::cerr << "致命错误：内存不足，无法分配 XML 解析所需内存 (已捕获，不崩溃)。" << std::endl;
        //return -2;
    } catch (const std::exception& e) {
        // 捕获所有其他标准异常（比如文件流异常等）
        //std::cerr << "发生未知标准异常 (已捕获): " << e.what() << std::endl;
        //return -3;
    } catch (...) {
        // 兜底：捕获任何非标准的异常（极少发生）
        //std::cerr << "发生未知非标准异常 (已捕获，程序不崩溃)。" << std::endl;
        //return -4;
    }
    return rsp;
}


} // namespace gb28181