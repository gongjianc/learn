/*
 * PfcountWL.c
 *
 *  Created on: Aug 21, 2014
 *      Author: siyu
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pfring.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <getopt.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include "msgqueue.h"
#include "jansson.h"
#include "pfcountWL.h"
#include "plog.h"

#define SNAP_LEN 1518

extern FilterInfo *WL_rules_copy;
extern FilterInfo *WL_rules;

void *ReadShareMemory(void){
    FilterInfo *FilterMsg;
	int ret = 0;
	shmid1=shmget(SHMID1, 0, 0);
	if (shmid1 < 0) {
	   log_printf(ZLOG_LEVEL_ERROR, "WL->shmget(shmid1) open error :%s[%d]...",strerror(errno),__LINE__);
	   return NULL;
	}

	msg = shmat(shmid1, NULL, 0);
	if ((void *) msg == (void *) -1) {
	   log_printf(ZLOG_LEVEL_ERROR, "WL->shmat(shmid1)  error:%s[%d]\n...",strerror(errno),__LINE__);
	   return NULL;
	}

	//ret = shmctl(shmid1, SHM_LOCK, 0);
	ret = shmctl(shmid1, 0, 0);
	if (ret != 0) {
	   log_printf(ZLOG_LEVEL_ERROR, "WL->shmat(shmid1)  lock  failed:%s[%d]\n...",strerror(errno),__LINE__);
	   return NULL;
	}
    if((WL_rules->use_flag==0)&&(WL_rules->up_flag==0))
    {    
       log_printf(ZLOG_LEVEL_INFO, "WL->update Rules for WL_rules....\n");
       FilterMsg = WL_rules;
       WL_rules->use_flag = 0; 
       WL_rules->up_flag = 1; 
	   WL_rules_copy->up_flag=0;
	   WL_rules_copy->use_flag=0;
    }    
    else if((WL_rules_copy->use_flag==0)&&(WL_rules_copy->up_flag==0))
    {    
       log_printf(ZLOG_LEVEL_INFO, "WL->update WL_copy_Rules ....\n");
       FilterMsg = WL_rules_copy;        
       WL_rules_copy->use_flag = 0; 
       WL_rules_copy->up_flag = 1; 
	   WL_rules->up_flag=0;
	   WL_rules->use_flag=0;
    }    

    log_printf(ZLOG_LEVEL_INFO, "WL->update success!! ....\n");
	FilterMsg->JsonRoot=json_loads(msg,0,0);
	if(	FilterMsg->JsonRoot==NULL){
	   log_printf(ZLOG_LEVEL_ERROR,"WL->json_loads  error[%d]\n...",__LINE__);
	   return NULL;
	}
    log_printf(ZLOG_LEVEL_INFO, "WL->update success!! ....\n");

	FilterMsg->Wans = json_object_get(FilterMsg->JsonRoot, "wans");
	FilterMsg->Lans = json_object_get(FilterMsg->JsonRoot, "lans");
	FilterMsg->Lins = json_object_get(FilterMsg->JsonRoot, "lins");
	bzero(msg,strlen(msg));
	ret = shmdt(msg);
	if (ret != 0) {
	   log_printf(ZLOG_LEVEL_ERROR, "WL->shmdt error[%d]\n...",__LINE__);
	   return NULL;
	}
}
#if 1
int bfMatch(char *target, char * pattern) {
    int pos = -1;
    int i = 0, j = 0, k = 0;
    int len=strlen(target);
		int len_pat=strlen(pattern);
    // 在没找到匹配pattern的子串前，遍历整个target
    while (-1 == pos && i < len) {
        // 将目标串和模式串逐一比对，如果有不同的则退出
        while (j < len_pat && target[i] == pattern[j]) {
            i++;
            j++;
        }

        if (j >= len_pat) { // 如果模式串扫描完，说明目标串中含有这个子串
            pos = k;
        } else { // 反之，没有扫描完，则从目标串的下一个字符开始重新逐一比对
            j = 0;
            k++;
            i = k;
        }
    }
    return pos;
}
#endif
/*************************************************************
 * when testing whiteList arguemens is *FileName
 *************************************************************/

int checkLins(char *ips,json_t *jsontext,json_t *jsonstr){
	int i;
	const char *stringIP;
	char buf[32]={0};
	if(json_object_iter_at(jsonstr,ips)!=NULL){
		log_printf(ZLOG_LEVEL_INFO,"############### checkLins here return ");
		return 1;
	}
	int size=json_array_size(jsontext);
			log_printf(ZLOG_LEVEL_INFO,"\n\n#################### size =%d \n\n",size);
	unsigned int intips= htonl(inet_addr(ips));
	for ( i=0; i<size; i++){
		json_unpack(json_array_get(jsontext,i),"s",&stringIP);
		bzero(buf,sizeof(buf));
		strcpy(buf,stringIP);
		char *tempIP = strtok(buf, "/");					//获取 10.63.58.123
#if 1
		int ret =  bfMatch(tempIP,ips);
//			log_printf(ZLOG_LEVEL_INFO,"\n\n#################### Matching ret=%d \n\n",ret);
		if(ret!=-1)
		{
			log_printf(ZLOG_LEVEL_INFO,"\n\n#################### Matching OK \n\n");
			return 1;
		}
#endif
		log_printf(ZLOG_LEVEL_INFO,"\n\n########################## checkLins ######### ips =%s######\n\n",ips);
		log_printf(ZLOG_LEVEL_INFO,"\n\n#################     tempIP =%s               ###############\n\n",tempIP);
		log_printf(ZLOG_LEVEL_INFO,"\n\n########################## checkLins End ###############\n\n");
		char *tempMASK = strtok(NULL, "/");	//获取 8
		log_printf(ZLOG_LEVEL_INFO,"\n\n########################## checkLins tempMASK ###############\n\n",tempMASK);
		unsigned int ipMask = ((unsigned int ) -1)<< (32 - atoi(tempMASK ? tempMASK : "32"));
		unsigned int IntegerIP1 = htonl(inet_addr(tempIP)) & ipMask;	  //将字符串IP 转化为整数
		unsigned int IntegerIP2 = htonl(inet_addr(tempIP)) | ~ipMask;  // 将字符串IP 转化为整数
		if (IntegerIP1 >IntegerIP2) {                    //获取端点的最大值鹤最小值
			unsigned int tmp = IntegerIP2;
			IntegerIP2 = IntegerIP1;
			IntegerIP1 = tmp;
		}
		if(IntegerIP1<=intips && intips<=IntegerIP2){
			return 1;
		}
	}
	return 0;
}


int compareIP(char *sips, char *dips) {
	int i = 0;
	int ret1=0,ret11=0,ret2=0,ret22=0;
	json_t *Jsips = NULL;
	json_t *Jdips = NULL;
	json_t *pstr = NULL;
	FilterInfo *Info = NULL;
	const char *stringIP=NULL;
	char *tempIP=NULL,*tempMASK=NULL;
	char buf[32]={0};
	
	if((WL_rules->use_flag==0)&&(WL_rules->up_flag==1)){
	   log_printf(ZLOG_LEVEL_INFO,"WL->IP in WansIP... compareIP start 1 ...\n");
       Info = WL_rules;
	   WL_rules->up_flag=0;
	   WL_rules->use_flag=1;
	   WL_rules_copy->up_flag=0;
	   WL_rules_copy->use_flag=0;
	}else if((WL_rules_copy->use_flag==0)&&(WL_rules_copy->up_flag==1)){
	   log_printf(ZLOG_LEVEL_INFO,"WL->IP in WansIP... compareIP start 2 ...\n");
	   Info = WL_rules_copy;
	   WL_rules_copy->up_flag=0;
	   WL_rules_copy->use_flag=1;
	   WL_rules->up_flag=0;
	   WL_rules->use_flag=0;
	}else if(WL_rules->use_flag==1)
	   Info = WL_rules;
	else if(WL_rules_copy->use_flag==1)
	   Info = WL_rules_copy;
#if 1	
    if(Info->Wans!=NULL){
	   log_printf(ZLOG_LEVEL_INFO,"Enter into Wlans  ==========  Start================");
	    json_t *wansSinips=json_object_get(Info->Wans,"sinIP");
	    json_t *wansDuanips=json_object_get(Info->Wans,"wansDuan");
		ret11=checkLins(sips,wansSinips,wansSinips);
		ret1= checkLins(sips,wansDuanips,wansSinips);
		if((ret11==1)||(ret1==1))
		{
			return 1;
		}
		else
		{
			ret22=checkLins(dips,wansSinips,wansSinips);
			ret2=checkLins(dips,wansDuanips,wansSinips);
			if((ret2)||(ret22)){
				printf("Wans OK--------------------\n\n");
				return 1;	
			}
		}
	   log_printf(ZLOG_LEVEL_INFO,"Enter into Wlans  ==========  End================");

	 }

	if(Info->Lans!=NULL){
	   log_printf(ZLOG_LEVEL_INFO,"Enter into Lans  ==========  Start================");
	   json_t *lansSinips=json_object_get(Info->Lans,"sinIP");
	   json_t *jsonDuan=json_object_get(Info->Lans,"lansDuan");
	   ret1=checkLins(sips,jsonDuan,lansSinips);
	   ret11=checkLins(sips,lansSinips,lansSinips);
		if((ret1)||(ret11)){
				printf("IP in lans OK -----------------------\n\n");
				return 1;
		}else{
			ret2=checkLins(dips,jsonDuan,lansSinips);
			ret22=checkLins(dips,lansSinips,lansSinips);
			if((ret2)||(ret22)){
				printf("IP in lans OK2 ------------------------------\n\n");
				return 1;
			}
		}
	   log_printf(ZLOG_LEVEL_INFO,"Enter into Lans  ==========  End================");

	}
	if(Info->Lins!=NULL){
	   log_printf(ZLOG_LEVEL_INFO,"Enter into Lins  ==========  Start================");
	   int linsSize=json_array_size(Info->Lins);
	   for (i = 0; i <linsSize; i++) {
		pstr=json_array_get(Info->Lins,i);
					// 获取端到端的源IP
					Jsips=json_object_get(pstr,"sips");
					// 获取端到端的目的IP
					Jdips=json_object_get(pstr,"dips");
					// 获取端到端的源IP的段与单点IP
					json_t *sipSin=json_object_get(Jsips,"sinIP");
					json_t *sipDuan=json_object_get(Jsips,"linsDuan");
					// 获取端到端的目的IP的段与单点IP
					json_t *dipSin=json_object_get(Jdips,"sinIP");
					json_t *dipDuan=json_object_get(Jdips,"linsDuan");
					 ret1=checkLins(sips,sipDuan,sipSin);
					 ret11=checkLins(sips,sipSin,sipSin);
	   log_printf(ZLOG_LEVEL_INFO,"LINSIP checkLins sips ret1 = %d\n",ret1);
					 if((ret1)||(ret11)){
						 ret2=checkLins(dips,dipDuan,dipSin);
						 ret22=checkLins(dips,dipSin,dipSin);
	   log_printf(ZLOG_LEVEL_INFO,"LINSIP checkLins sips ret2 = %d\n",ret2);
						 if((ret2)||(ret22)){
							 printf("ip in lins(1) --------------------------------\n\n");
							 return 1;
						 }
					 }else{
						 ret1=checkLins(sips,dipDuan,dipSin);
						 ret11=checkLins(sips,dipSin,dipSin);
	   log_printf(ZLOG_LEVEL_INFO,"LINSIP checkLins dipDuan ret1 = %d\n",ret1);
						 if((ret1)||(ret11)){
							 ret2=checkLins(dips,sipDuan,sipSin);
							 ret22=checkLins(dips,sipSin,sipSin);
	   log_printf(ZLOG_LEVEL_INFO,"LINSIP checkLins dipDuan ret2 = %d\n",ret2);
							 if((ret2)||(ret22)){
								 printf("ip in lins(2) ---------------------------\n\n");
								 return 1;
							 }
						 }
					 }
	   log_printf(ZLOG_LEVEL_INFO,"Enter into Lins  ==========  End================");

	   }
	}

#endif
	return -1;
}

