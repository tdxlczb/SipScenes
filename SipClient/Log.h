#pragma once
#include <time.h>
#include <string>

static std::string getTime() {
    static const char* sTimeFmt = "%Y-%m-%d %H:%M:%S";
    time_t tmVar = time(nullptr);
    char arrTime[64];
    tm stTm;
    localtime_s(&stTm, &tmVar);
    strftime(arrTime, sizeof(arrTime), sTimeFmt, &stTm);
    return arrTime;
}
//  __FILE__ ��ȡԴ�ļ������·��������
//  __LINE__ ��ȡ���д������ļ��е��к�
//  __func__ �� __FUNCTION__ ��ȡ������

#define LOGI(format, ...)  fprintf(stderr,"[INFO]%s [%s:%d %s()] " format "\n", getTime().data(),__FILE__,__LINE__,__func__ ,##__VA_ARGS__)
#define LOGE(format, ...)  fprintf(stderr,"[ERROR]%s [%s:%d %s()] " format "\n",getTime().data(),__FILE__,__LINE__,__func__ ,##__VA_ARGS__)
