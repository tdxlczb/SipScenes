#include "manscdp_base.h"
#include "pugixml/pugixml.hpp"

namespace gb28181 {

ManscdpBase GetManscdpBase(const std::string& xml)
{
    ManscdpBase manscdp;
    try {
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_string(xml.c_str(), pugi::parse_default);
        if (!result) {
            return manscdp;
        }
        pugi::xml_node root = doc.first_child();
        if (!root) {
            return manscdp;
        }
        manscdp.ManscdpType = root.name();
        if (manscdp.ManscdpType.empty()) {
            return manscdp;
        }
        manscdp.CmdType = root.child("CmdType").text().as_string();
        manscdp.SN = root.child("SN").text().as_int();
        manscdp.DeviceID = root.child("DeviceID").text().as_string();
        return manscdp;
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
    return manscdp;
}


} // namespace gb28181