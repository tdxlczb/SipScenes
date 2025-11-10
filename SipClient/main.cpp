// SIPClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Dnsapi.lib")

#include <Windows.h>
#include <eXosip2/eXosip.h>

#include <thread>
#include "SipClient.h"
#include "tools/log.h"

int main()
{
    int nRet;
    WSADATA wsaData;
    nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (0 != nRet) {
        LOGE("WSAStartup error %d", nRet);
        return -1;
    }

    ClientInfo clientInfo;
    clientInfo.sUser = "client1";
    clientInfo.sPwd = "client1";//为了能认证通过，服务端暂时设置了密码=账号的规则
    clientInfo.sRealm = "3402000000";
    clientInfo.sIp = "172.16.19.108"; // 客户端ip
    clientInfo.iPort = 54237;
    clientInfo.iSipExpiry = 3600;
    clientInfo.sRegUri = "172.16.19.108:5060";

    SipClient client;
    if (false == client.Init(clientInfo)) {
        WSACleanup();
        return -1;
    }

    std::thread th([&client]() {
        while (true)
        {
            std::string type;
            std::cin >> type;
            if (type == "1") {
                client.Register();
            }
            else if (type == "2") {
                //client.();
            }
            else if (type == "exit") {
                break;
            }
            Sleep(10);
        }
        });

    client.Loop();

    th.join();

    WSACleanup();
    return 0;
}


// gcc uac.c -o uac -losip2 -leXosip2 -lpthread -L/usr/local/lib -losipparser2
struct eXosip_t* context_eXosip;

int main2(int argc, char* argv[])
{
    eXosip_event_t* je;
    osip_message_t* reg = NULL;
    osip_message_t* invite = NULL;
    osip_message_t* ack = NULL;
    osip_message_t* info = NULL;
    osip_message_t* message = NULL;

    int call_id, dialog_id;
    int i, flag;
    int flag1 = 1;
    int id;

    const char* identity = "sip:140@127.0.0.1:15060";
    const char* registerer = "sip:127.0.0.1:15061";
    const char* source_call = "sip:140127.0.0.1:15060";
    const char* dest_call = "sip:133@127.0.0.1:15061";

    char command;
    char tmp[4096];
    char localip[128];

    printf("r     register Server \r\n");
    printf("c     cancle register \r\n");
    printf("i     invite \r\n");
    printf("h     hold \r\n");
    printf("q     quit \r\n");
    printf("s         excute INFO \r\n");
    printf("m     excute MESSAGE \r\n");

    context_eXosip = eXosip_malloc();
    // 初始化
    i = eXosip_init(context_eXosip);
    if (i != 0)
    {
        printf("Couldn't initialize eXosip!\n");
        return -1;
    }
    else
    {
        printf("eXosip_init successfully!\n");
    }

    i = eXosip_listen_addr(context_eXosip, IPPROTO_UDP, NULL, 15060, AF_INET, 0);
    if (i != 0)
    {
        eXosip_quit(context_eXosip);
        fprintf(stderr, "Couldn't initialize transport layer!\n");
        return -1;
    }
    flag = 1;
    while (flag)
    {
        printf("please input the comand:\n");

        scanf("%c", &command);
        getchar();

        switch (command)
        {
        case 'r':
            printf("This modal isn't commpleted!\n");
            break;
        case 'i': /* INVITE */
            i = eXosip_call_build_initial_invite(context_eXosip, &invite, dest_call, source_call, NULL, "This si a call for a conversation");
            if (i != 0)
            {
                printf("Intial INVITE failed!\n");
                break;
            }
            // 符合SDP格式,其中属性a是自定义格式,也就是说可以存放自己的信息
            // 但是经测试,格式:v o t必不可少,原因未知,估计是协议栈在传输时需要检查的
            snprintf(tmp, 4096,
                "v=0\r\n"
                "o=anonymous 0 0 IN IP4 0.0.0.0\r\n"
                "t=1 10\r\n"
                "a=username:rainfish\r\n"
                "a=password:123\r\n");
            osip_message_set_body(invite, tmp, strlen(tmp));
            osip_message_set_content_type(invite, "application/sdp");

            eXosip_lock(context_eXosip);
            i = eXosip_call_send_initial_invite(context_eXosip, invite);
            eXosip_unlock(context_eXosip);
            flag1 = 1;
            while (flag1)
            {
                je = eXosip_event_wait(context_eXosip, 0, 200);

                if (je == NULL)
                {
                    printf("No response or the time is over!\n");
                    break;
                }

                switch (je->type)
                {
                case EXOSIP_CALL_INVITE:
                    printf("a new invite reveived!\n");
                    break;
                case EXOSIP_CALL_PROCEEDING:
                    printf("proceeding!\n");
                    break;
                case EXOSIP_CALL_RINGING:
                    printf("ringing!\n");
                    printf("call_id is %d, dialog_id is %d \n", je->cid, je->did);
                    break;
                case EXOSIP_CALL_ANSWERED:
                    printf("ok! connected!\n");
                    call_id = je->cid;
                    dialog_id = je->did;
                    printf("call_id is %d, dialog_id is %d \n", je->cid, je->did);

                    eXosip_call_build_ack(context_eXosip, je->did, &ack);
                    eXosip_call_send_ack(context_eXosip, je->did, ack);
                    flag1 = 0;
                    break;
                case EXOSIP_CALL_CLOSED:
                    printf("the other sid closed!\n");
                    break;
                case EXOSIP_CALL_ACK:
                    printf("ACK received!\n");
                    break;
                default:
                    printf("other response!\n");
                    break;
                }
                eXosip_event_free(je);
            }
            break;
        case 'h':
            printf("Holded !\n");

            eXosip_lock(context_eXosip);
            eXosip_call_terminate(context_eXosip, call_id, dialog_id);
            eXosip_unlock(context_eXosip);
            break;
        case 'c':
            printf("This modal isn't commpleted!\n");
            break;
        case 's':
            // 传输INFO方法
            eXosip_call_build_info(context_eXosip, dialog_id, &info);
            snprintf(tmp, 4096,
                "hello,rainfish");
            osip_message_set_body(info, tmp, strlen(tmp));
            // 格式可以任意设定,text/plain代表文本信息
            osip_message_set_content_type(info, "text/plain");
            eXosip_call_send_request(context_eXosip, dialog_id, info);
            break;
        case 'm':
            // 传输MESSAGE方法,也就是即时消息，和INFO方法相比，MESSAGE不用建立连接，直接传输信息，而INFO必须在建立INVITE的基础上传输。
            printf("the mothed :MESSAGE\n");
            eXosip_message_build_request(context_eXosip, &message, "MESSAGE", dest_call, source_call, NULL);
            snprintf(tmp, 4096,
                "hellor rainfish");
            osip_message_set_body(message, tmp, strlen(tmp));
            // 假设格式是xml
            osip_message_set_content_type(message, "text/xml");
            eXosip_message_send_request(context_eXosip, message);
            break;
        case 'q':
            eXosip_quit(context_eXosip);
            printf("Exit the setup!\n");
            flag = 0;
            break;
        }
    }
    return 0;
}

void dump_osip_message(osip_message_t* msg) {
    char* s;
    size_t len;
    osip_message_to_str(msg, &s, &len);
    printf("dump message:\n%s", s);
}

// register_uac.c
#include <eXosip2/eXosip.h>
#include <stdio.h>
#include <stdlib.h>

/* ----------  用户配置  ---------- */
const char* local_ip = "172.16.19.108";        /* 本地地址 */
int         local_port = 54234;                /* 本地端口 */
const char* user = "1000";                 /* 用户名   */
const char* pwd = "1234";                  /* 密码     */
const char* realm = "example.com";         /* 服务器 realm */
const char* server_ip = "172.16.19.40";   /* 服务器 IP */
int         server_port = 5060;            /* 服务器端口 */
int         expires = 3600;                /* 注册到期时间 */

const char* server_uri = "sip:sip.pjsip.org";
/* -------------------------------- */

/* 统一发包函数：auth_str==NULL 表示首次无认证 */
static void send_register(eXosip_t* ctx, int id, const char* from,
    const char* proxy, const char* contact,
    const char* auth_str)
{
    osip_message_t* reg = NULL;

    /* 构建 REGISTER 请求 */
    int rid = (id < 0) ? eXosip_register_build_initial_register(ctx, from, proxy, contact, expires, &reg) : id;
    if (rid < 0) {
        printf("build initial register fail:%d\n", rid);
        return;
    }
    int ret = 0;
    if (auth_str) {
        rid = eXosip_register_build_register(ctx, id, expires, &reg);
        if (rid < 0) {
            printf("build register fail:%d\n", rid);
            return;
        }
        ret = osip_message_set_authorization(reg, auth_str);
        if (ret < 0) {
            printf("osip_message_set_authorization fail:%d\n", ret);
            return;
        }
    }

    dump_osip_message(reg);
    /* 发出去 */
    ret = eXosip_register_send_register(ctx, rid, reg);
    if (ret < 0) {
        printf("send register fail:%d\n", ret);
        return;
    }

    printf(">>> REGISTER sent (id=%d)\n", rid);
}

int main3()
{
    eXosip_t* ctx;
    eXosip_event_t* evt;

    ctx = eXosip_malloc();
    if (eXosip_init(ctx) != 0) {
        printf("eXosip_init error\n");
        return -1;
    }

    /* 监听本地 UDP 端口 */
    if (eXosip_listen_addr(ctx, IPPROTO_UDP, local_ip, local_port, AF_INET, 0) != 0) {
        printf("listen error\n");
        eXosip_quit(ctx);
        return -1;
    }
    printf("Local SIP endpoint: %s:%d\n", local_ip, local_port);

    char from[256], proxy[256], contact[256];
    snprintf(from, sizeof(from), "sip:%s@%s:%d", user, local_ip, local_port);
    snprintf(proxy, sizeof(proxy), "sip:%s:%d", server_ip, server_port);
    //snprintf(contact, sizeof(contact), "sip:%s@%s:%d", user, server_ip, server_port);

    //snprintf(from, sizeof(from), "sip:test1@pjsip.org");
    //snprintf(proxy, sizeof(proxy), "sip:sip.pjsip.org");
    //snprintf(contact, sizeof(contact), "sip:test1@1183.6.86.3:65476");

    //eXosip_masquerade_contact(ctx, local_ip, local_port);
    //snprintf(contact, sizeof(contact) - 1, "sip:%s@%s:%d", user, local_ip, local_port);
    //printf("contact: %s\n", contact);

    /* 首次 REGISTER：拿到 rid 并保存！ */
    //contact是可选的，如果传NULL，eXosip默认会使用本机的内网IP地址自动添加Contact头域
    //如果是要和外网通信，需要将contact设置为外网能访问的地址
    osip_message_t* reg = NULL;
    int rid = eXosip_register_build_initial_register(ctx, from, proxy, NULL, expires, &reg);
    if (rid < 0) { printf("build fail\n"); return -1; }
    eXosip_register_send_register(ctx, rid, reg);
    printf(">>> initial REGISTER  rid=%d\n", rid);


    /* 事件循环 */
    while (1) {
        evt = eXosip_event_wait(ctx, 0, 20);   /* 200 ms 超时 */
        if (!evt) continue;

        //eXosip_lock(ctx);
        //eXosip_default_action(ctx, evt);       /* 库自动回 401 挑战 */
        //eXosip_unlock(ctx);

        switch (evt->type) {
        case EXOSIP_REGISTRATION_SUCCESS:
            printf("==== REGISTER 200 OK ====\n");
            break;
        case EXOSIP_REGISTRATION_FAILURE:
            if (evt->response && evt->response->status_code == 401) {

                /* 二次 REGISTER：添加认证信息重新发送 */
                /* 取出 WWW-Authenticate 头 */
                osip_www_authenticate_t* www = NULL;
                osip_message_get_www_authenticate(evt->response, 0, &www);
                if (www && www->realm && www->nonce) {
                    printf("<<< 401 received, realm=%s nonce=%s\n", www->realm, www->nonce);

                    /* 1. 重新构建 REGISTER（同 rid） */
                    osip_message_t* reg2 = NULL;
                    eXosip_register_build_register(ctx, rid, expires, &reg2);

                    /* 2. 拼 Authorization 头 */
                    char auth[512];
                    snprintf(auth, sizeof(auth), "Digest username=\"%s\", realm=%s, nonce=%s, uri=\"%s\", response=\"\", algorithm=MD5",
                        user, www->realm, www->nonce, proxy);
                    osip_message_set_authorization(reg2, auth);
     
                    /* 3. 发送（保持 rid 不变） */
                    eXosip_register_send_register(ctx, rid, reg2);
                    printf(">>> REGISTER (with auth) sent  rid=%d\n", rid);
                }
            }
            break;
        default:
            break;
        }
        eXosip_event_free(evt);
    }
    return 0;
}