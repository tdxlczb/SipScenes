// SIPServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")
#include <Windows.h>

#include "SipServer.h"
#include "Log.h"


int main()
{
    int nRet;
    WSADATA wsaData;
    nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (0 != nRet) {
        LOGE("WSAStartup error %d", nRet);
        return -1;
    }

    ServerInfo serverInfo{
            "SipServer_Test",//服务器的名字
            "1234567890123456",//SIP服务随机数值
            "172.16.19.108",//SIP服务IP
            5060,//SIP服务端口
            "34020000001320000001",//SIP服务器ID
            "3402000000",//SIP服务器域
            "itc20232024",//SIP password
            1800,//SIP timeout
            3600,//SIP到期
            10000//SIP-RTP服务端口
    };

    SipServer stSipServer;
    if (false == stSipServer.Init(serverInfo)) {
        WSACleanup();
        return -1;
    }

    stSipServer.Loop();

    WSACleanup();
    return 0;
}


#include <eXosip2/eXosip.h>
#include <osip2/osip_mt.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Dnsapi.lib")

// gcc uas.c -o uas -losip2 -leXosip2 -lpthread -L/usr/local/lib -losipparser2

struct eXosip_t* context_eXosip;

int main2(int argc, char* argv[])
{
    eXosip_event_t* je = NULL;
    osip_message_t* ack = NULL;
    osip_message_t* invite = NULL;
    osip_message_t* answer = NULL;
    sdp_message_t* remote_sdp = NULL;

    int call_id, dialog_id;
    int i, j;
    int id;


    char command;
    char tmp[4096];
    char localip[128];

    int pos = 0;

    // 初始化sip
    context_eXosip = eXosip_malloc();
    i = eXosip_init(context_eXosip);
    if (i != 0)
    {
        printf("Can't initialize eXosip!\n");
        return -1;
    }
    else
    {
        printf("eXosip_init successfully!\n");
    }

    i = eXosip_listen_addr(context_eXosip, IPPROTO_UDP, NULL, 5060, AF_INET, 0);
    if (i != 0)
    {
        eXosip_quit(context_eXosip);
        fprintf(stderr, "eXosip_listen_addr error!\nCouldn't initialize transport layer!\n");
    }

    for (;;)
    {
        // 侦听是否有消息到来
        je = eXosip_event_wait(context_eXosip, 0, 50);

        // 处理401 407 3xx响应
        eXosip_lock(context_eXosip);
        eXosip_default_action(context_eXosip, je);
        eXosip_unlock(context_eXosip);

        if (je == NULL) // 没有接收到消息
            continue;
        // printf ("the cid is %s, did is %s/n", je->did, je->cid);
        switch (je->type)
        {
        case EXOSIP_MESSAGE_NEW: // 新的消息到来
            printf(" EXOSIP_MESSAGE_NEW!\n");
            if (MSG_IS_MESSAGE(je->request)) // 如果接受到的消息类型是MESSAGE
            {
                {
                    osip_body_t* body;
                    osip_message_get_body(je->request, 0, &body);
                    printf("I get the msg is: %s\n", body->body);
                    // printf ("the cid is %s, did is %s/n", je->did, je->cid);
                }
                // 按照规则，需要回复200 OK信息
                eXosip_message_build_answer(context_eXosip, je->tid, 200, &answer);
                eXosip_message_send_answer(context_eXosip, je->tid, 200, answer);
            }
            break;
        case EXOSIP_CALL_INVITE:
            // 得到接收到消息的具体信息
            printf("Received a INVITE msg from %s:%s, UserName is %s, password is %s\n", je->request->req_uri->host,
                je->request->req_uri->port, je->request->req_uri->username, je->request->req_uri->password);
            // 得到消息体,认为该消息就是SDP格式.
            remote_sdp = eXosip_get_remote_sdp(context_eXosip, je->did);
            call_id = je->cid;
            dialog_id = je->did;

            eXosip_lock(context_eXosip);
            eXosip_call_send_answer(context_eXosip, je->tid, 180, NULL);
            i = eXosip_call_build_answer(context_eXosip, je->tid, 200, &answer);
            if (i != 0)
            {
                printf("This request msg is invalid!Cann't response!\n");
                eXosip_call_send_answer(context_eXosip, je->tid, 400, NULL);
            }
            else
            {
                snprintf(tmp, 4096,
                    "v=0\r\n"
                    "o=anonymous 0 0 IN IP4 0.0.0.0\r\n"
                    "t=1 10\r\n"
                    "a=username:rainfish\r\n"
                    "a=password:123\r\n");

                osip_message_set_body(answer, tmp, strlen(tmp));
                osip_message_set_content_type(answer, "application/sdp");

                eXosip_call_send_answer(context_eXosip, je->tid, 200, answer);
                printf("send 200 over!\n");
            }
            eXosip_unlock(context_eXosip);

            printf("the INFO is :\n");
            while (!osip_list_eol(&remote_sdp->a_attributes, pos))
            {
                sdp_attribute_t* at;

                at = (sdp_attribute_t*)osip_list_get(&remote_sdp->a_attributes, pos);
                printf("%s : %s\n", at->a_att_field, at->a_att_value);

                pos++;
            }
            break;
        case EXOSIP_CALL_ACK:
            printf("ACK recieved!\n");
            // printf ("the cid is %s, did is %s/n", je->did, je->cid);
            break;
        case EXOSIP_CALL_CLOSED:
            printf("the remote hold the session!\n");
            // 这个版本库会自动回复200ok,不需要再次回复
            // i = eXosip_call_build_answer(context_eXosip, je->tid, 200, &answer);
            // if (i != 0)
            // {
            //     printf("This request msg is invalid!Cann't response!\n");
            //     eXosip_call_send_answer(context_eXosip, je->tid, 400, NULL);
            // }
            // else
            // {
            //     eXosip_call_send_answer(context_eXosip, je->tid, 200, answer);
            //     printf("bye send 200 over!\n");
            // }
            break;

        case EXOSIP_CALL_MESSAGE_NEW:
            /*
            // request related events within calls (except INVITE)
                  EXOSIP_CALL_MESSAGE_NEW,          < announce new incoming request.
            // response received for request outside calls
                     EXOSIP_MESSAGE_NEW,          < announce new incoming request.
            EXOSIP_CALL_MESSAGE_NEW是一个dialog的新的消息到来，invite事务内的不算
            EXOSIP_MESSAGE_NEW而是表示不是呼叫内的消息到来。例如MESSAGE，MESSAGE不需要INVITE建立会话即可直接发送
            */
            printf(" EXOSIP_CALL_MESSAGE_NEW\n");
            if (MSG_IS_INFO(je->request)) // 如果传输的是INFO方法
            {
                eXosip_lock(context_eXosip);
                i = eXosip_call_build_answer(context_eXosip, je->tid, 200, &answer);
                if (i == 0)
                {
                    eXosip_call_send_answer(context_eXosip, je->tid, 200, answer);
                }
                eXosip_unlock(context_eXosip);
                {
                    osip_body_t* body;
                    osip_message_get_body(je->request, 0, &body);
                    printf("the body is %s\n", body->body);
                }
            }
            break;
        default:
            printf("Could not parse the msg!\n");
        }
    }
}

// reg_server.c
#include <eXosip2/eXosip.h>
#include <stdio.h>
#include <time.h>

/* 每次启动换一个nonce即可 */
static void make_nonce(char* nonce, int len)
{
    snprintf(nonce, len, "%ld", (long)time(NULL));
}

int main3(void)
{
    eXosip_t* ctx;
    eXosip_event_t* evt;
    char nonce[64];

    ctx = eXosip_malloc();
    if (eXosip_init(ctx) != 0) {
        printf("eXosip_init fail\n");
        return -1;
    }

    /* 监听 UDP 5060 */
    if (eXosip_listen_addr(ctx, IPPROTO_UDP, NULL, 5060, AF_INET, 0) != 0) {
        printf("listen fail\n");
        eXosip_quit(ctx);
        return -1;
    }
    printf("SIP Register Server listening on udp *:5060\n");

    while (1) {
        evt = eXosip_event_wait(ctx, 0, 20);
        if (!evt) continue;

        eXosip_lock(ctx);
        eXosip_default_action(ctx, evt);   /* 自动回 100/180 等 */
        eXosip_unlock(ctx);

        switch (evt->type)
        {
        case EXOSIP_MESSAGE_NEW: {
            if (MSG_IS_REGISTER(evt->request)) {

                osip_message_t* ans = NULL;
                osip_authorization_t* auth = NULL;
                osip_message_get_authorization(evt->request, 0, &auth);

                if (auth == NULL) {      /* 首次注册，无认证头 */
                    make_nonce(nonce, sizeof(nonce));

                    /* 1. 拼完整的 WWW-Authenticate 头值 */
                    char auth_str[256];
                    snprintf(auth_str, sizeof(auth_str),
                        "Digest realm=\"example.com\", nonce=\"%s\", algorithm=MD5, qop=\"auth\"",
                        nonce);

                    /* 2. 构建 401 应答 */
                    eXosip_message_build_answer(ctx, evt->tid, 401, &ans);

                    /* 3. 设置头（整串一次给） */
                    osip_message_set_www_authenticate(ans, auth_str);

                    /* 4. 发出去 */
                    eXosip_message_send_answer(ctx, evt->tid, 401, ans);
                    printf("401 sent for %s\n", evt->request->from->url->username);
                }
                else {                 /* 带 Authorization，直接接受 */
                    eXosip_message_build_answer(ctx, evt->tid, 200, &ans);
                    eXosip_message_send_answer(ctx, evt->tid, 200, ans);
                    printf("200 OK for %s\n", evt->request->from->url->username);
                }
            }
            else if (MSG_IS_MESSAGE(evt->request)) {

                osip_body_t* body;
                osip_message_get_body(evt->request, 0, &body);
                printf("I get the msg is: %s\n", body->body);
                // 按照规则，需要回复200 OK信息
                osip_message_t* answer = NULL;
                eXosip_message_build_answer(ctx, evt->tid, 200, &answer);
                eXosip_message_send_answer(ctx, evt->tid, 200, answer);
            }
            break;
        }
        default:
            break;
        }

        eXosip_event_free(evt);
    }

    return 0;
}