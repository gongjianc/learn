/*
 *  Copyright (C) 2004-2008 Christos Tsantilas
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA.
 */

#include <zlog.h>
#include <jni.h>
#include "plog.h"
#include "extractmain.h"
#include "common.h"
#include "c-icap.h"
#include <stdio.h>
#include "net_io.h"
#include "debug.h"
#include "module.h"
#include "log.h"
#include "cfg_param.h"
#include "filetype.h"
#include "acl.h"
#include "txtTemplate.h"
#include "commands.h"

/*
extern char *PIDFILE;
extern char *RUN_USER;
extern char *RUN_GROUP;
extern int PORT;
*/
extern int ssbuffer_id;
int http_buffer_id = -1;
extern int http_buffer_id;
extern int DAEMON_MODE;
extern int MAX_SECS_TO_LINGER;
char MY_HOSTNAME[CI_MAXHOSTNAMELEN + 1];

void init_conf_tables();
int init_body_system();
int config(int, char **);
int init_server(char *address, int port, int *family);
int start_server();
int store_pid(char *pidfile);
int clear_pid(char *pidfile);
int is_icap_running(char *pidfile);
int set_running_permissions(char *user, char *group);
void init_internal_lookup_tables();
void request_stats_init();
int mem_init();
void init_http_auth();

void compute_my_hostname()
{
     char hostname[64];
     struct hostent *hent;
     int ret;
     ret = gethostname(hostname, 63);
     if (ret == 0) {
          hostname[63] = '\0';
          if ((hent = gethostbyname(hostname)) != NULL) {
               strncpy(MY_HOSTNAME, hent->h_name, CI_MAXHOSTNAMELEN);
               MY_HOSTNAME[CI_MAXHOSTNAMELEN] = '\0';
          }
          else
               strcpy(MY_HOSTNAME, hostname);
     }
     else
          strcpy(MY_HOSTNAME, "localhost");
}

#if ! defined(_WIN32)
void run_as_daemon()
{
     int fd;
     int pid, sid;
     pid = fork();
     if (pid < 0) {
          ci_debug_printf(1, "Unable to fork. exiting...");
          exit(-1);
     }
     if (pid > 0)
          exit(0);
     /* Change the file mode mask */
     umask(0);
     /* Create a new SID for the child process */
     sid = setsid();
     if (sid < 0) {
          ci_debug_printf(1, "Unable to create a new SID for the main process. exiting...");
          exit(-1);
     }
     /* Change the current working directory */
     if ((chdir("/")) < 0) {
          ci_debug_printf(1, "Unable to change the working directory. exiting...");
          exit(-1);
     }

     /* Direct standard file descriptors to "/dev/null"*/
     fd = open("/dev/null", O_RDWR);
     if (fd < 0) {
          ci_debug_printf(1, "Unable to open '/dev/null'. exiting...");
          exit(-1);
     }

     if (dup2(fd, STDIN_FILENO) < 0) {
          ci_debug_printf(1, "Unable to set stdin to '/dev/null'. exiting...");
          exit(-1);
     }

     if (dup2(fd, STDOUT_FILENO) < 0) {
          ci_debug_printf(1, "Unable to set stdout to '/dev/null'. exiting...");
          exit(-1);
     }

     if (dup2(fd, STDERR_FILENO) < 0) {
          ci_debug_printf(1, "Unable to set stderr to '/dev/null'. exiting...");
          exit(-1);
     }
     close(fd);
}
#endif
extern void* tika_consumer_thread(void* _id);
extern int file_extract_pthread (char *jsonstr, char *strpath, unsigned long max_filesize);
extern   	JNIEnv *env;
extern  	jclass cls;
extern int sbuff_putdata(const int iringbuffer_ID, SESSION_BUFFER_NODE ft);
/***************************
Function: http_file_thread()
Description: http协议处理线程池，此线程池主要是负责所有与http协议相关的内容识别，包括即时通信、上传文件、webmail邮件传输，
Calls: sbuff_getdata，webmail_mail
Called By: 
Table Accessed: 
Table Updated: 
Input: 线程临时ID号，组要通过它与CPU绑定
Output: 无
Return: 无
Others: 
***************************/

void* http_file_thread(void* _id) 
{
  u_int numCPU = sysconf( _SC_NPROCESSORS_ONLN );
  long thread_id = (long)_id;
  u_long core_id = thread_id % numCPU;

  if(numCPU > 1) {
  //  if(bind2core(core_id) == 0)
//	  log_printf(ZLOG_LEVEL_INFO, "http file process INFO: Set thread %lu on core %lu/%u !!", thread_id, core_id, numCPU);
  }
  SESSION_BUFFER_NODE *sshdr = NULL;
  sshdr = malloc(sizeof(SESSION_BUFFER_NODE));

  if(sshdr==NULL){
//	log_printf(ZLOG_LEVEL_FATAL, "http file process FATAL: malloc sshdr failed!!");
	return(NULL);
  }
  while(1){

    if(sbuff_getdata(http_buffer_id, sshdr)>0){//等待信号触发，如果触发成功，执行如下操作
	  log_printf(ZLOG_LEVEL_DEBUG, "\n\n\n\nhttp file process Sbuff_getdata OK ! ! ! ! \n\n\n\n");
#if 1
      HttpSession        sp_session;
      sp_session.http = sshdr->attr;
      sp_session.key.saddr = sshdr->session_five_attr.ip_src;
      sp_session.key.daddr = sshdr->session_five_attr.ip_dst;
      sp_session.key.source = sshdr->session_five_attr.port_src;
      sp_session.key.dest = sshdr->session_five_attr.port_dst;
      
      char *webmailjson=NULL;
      webmail_mail(&sp_session, &webmailjson, sbuff_putdata, ssbuffer_id);//http内容处理函数，负责所有http相关的处理
#endif
#if 0
      char *webmailjson=NULL;
			dlp_http_post_head stHttpPost;
			memset(&stHttpPost,0x0,sizeof(dlp_http_post_head));
			//stHttpPost.content_length = content_len;//5628013;
#if 0
			memcpy(stHttpPost.new_name,/*echo_data->body->filename*/"/var/tmp/CI_TMP_qrhLqu",strlen("/var/tmp/CI_TMP_qrhLqu")+1);
			memcpy(stHttpPost.content_type,"-------------- = multipart/form-data; boundary=----------------------------bd5fa42409b5",strlen("-------------- = multipart/form-data; boundary=----------------------------bd5fa42409b5")+1);
			memcpy(stHttpPost.boundary,"----------------------------236278130538",strlen("----------------------------236278130538")+1);
#endif
			stHttpPost.content_length = 79949;//67387;//67374;//5628013;
			memcpy(stHttpPost.new_name,"test3.eml",strlen("test3.eml")+1);
			//memcpy(stHttpPost.content_type,"-------------- = multipart/form-data; boundary=----------------------------bd5fa42409b5",strlen("-------------- = multipart/form-data; boundary=----------------------------bd5fa42409b5")+1);
			memcpy(stHttpPost.content_type,"-------------- = multipart/form-data; boundary=----------------------------270491477219123",strlen("-------------- = multipart/form-data; boundary=----------------------------270491477219123")+1);
			//memcpy(stHttpPost.boundary,"----------------------------bd5fa42409b5",strlen("----------------------------bd5fa42409b5")+1);
			memcpy(stHttpPost.boundary,"---------------------------270491477219123",strlen("---------------------------270491477219123")+1);
      HttpSession        sp_session;
      sp_session.http = &stHttpPost;
      sp_session.key.saddr = 12345;//sshdr->session_five_attr.ip_src;
      sp_session.key.daddr = 8080;//sshdr->session_five_attr.ip_dst;
      sp_session.key.source = 12345;//sshdr->session_five_attr.port_src;
      sp_session.key.dest = 8080;//sshdr->session_five_attr.port_dst;
      webmail_mail(&sp_session, &webmailjson, sbuff_putdata, ssbuffer_id);//http内容处理函数，负责所有http相关的处理
#endif
	}

  }
}
int main(int argc, char **argv)
{
#if 1
#if 1
			if (zlog_init("conf/session_cap.conf")) {//开启日志功能
				printf("zlog init failed");
				return (-1);
			}
			//设置日志等级
			ftl_log = zlog_get_category(LOG4C_CATEGORY_FTL);
			err_log = zlog_get_category(LOG4C_CATEGORY_ERR);
			wrn_log = zlog_get_category(LOG4C_CATEGORY_WRN);
			dbg_log = zlog_get_category(LOG4C_CATEGORY_DBG);
			inf_log = zlog_get_category(LOG4C_CATEGORY_INF);
			ntc_log = zlog_get_category(LOG4C_CATEGORY_NTC);
			if(0 != read_IM_config())
			{
				log_printf(ZLOG_LEVEL_ERROR, "failed to read IM config file: -----%s------%s------%d\n",__FILE__, __func__, __LINE__);
				return -1;
			}
			/* init kmem interface */
			ikmem_init(NULL);//内存管理器初始化函数
			build_JVM();//java虚拟机初始化函数 打开JVM //by niulw
#endif
			http_buffer_id = sbuff_create (100000);//http处理信号池申请
			if(http_buffer_id < 0)
				return -1;
			log_printf(ZLOG_LEVEL_DEBUG, "\n\n ------------------- http_buffer_id  = %d\n\n",http_buffer_id);
			int thd_id =0;
			int http_pthreads =500;
			int tika_pthreads=600;
			pthread_t http_session_thread[1000];
    for(thd_id=2; thd_id<http_pthreads; thd_id++)
	  pthread_create(&http_session_thread[thd_id], NULL, http_file_thread, &thd_id);//http协议内容解析线程池创建
		sleep(10);
#if 1
			dlp_http_post_head stHttpPost;
			memset(&stHttpPost,0x0,sizeof(dlp_http_post_head));
			//stHttpPost.content_length = content_len;//5628013;
#if 0
			memcpy(stHttpPost.new_name,/*echo_data->body->filename*/"/var/tmp/CI_TMP_qrhLqu",strlen("/var/tmp/CI_TMP_qrhLqu")+1);
			memcpy(stHttpPost.content_type,"-------------- = multipart/form-data; boundary=----------------------------bd5fa42409b5",strlen("-------------- = multipart/form-data; boundary=----------------------------bd5fa42409b5")+1);
			memcpy(stHttpPost.boundary,"----------------------------236278130538",strlen("----------------------------236278130538")+1);
#endif
			stHttpPost.content_length = 79949;//67387;//67374;//5628013;
			memcpy(stHttpPost.new_name,"test3.eml",strlen("test3.eml")+1);
			//memcpy(stHttpPost.content_type,"-------------- = multipart/form-data; boundary=----------------------------bd5fa42409b5",strlen("-------------- = multipart/form-data; boundary=----------------------------bd5fa42409b5")+1);
			memcpy(stHttpPost.content_type,"-------------- = multipart/form-data; boundary=----------------------------270491477219123",strlen("-------------- = multipart/form-data; boundary=----------------------------270491477219123")+1);
			//memcpy(stHttpPost.boundary,"----------------------------bd5fa42409b5",strlen("----------------------------bd5fa42409b5")+1);
			memcpy(stHttpPost.boundary,"---------------------------270491477219123",strlen("---------------------------270491477219123")+1);
  SESSION_BUFFER_NODE ft; 
  memset(&ft,0x0,sizeof(SESSION_BUFFER_NODE));
  ft.session_five_attr.ip_dst = 12345;//dlp_http->key.daddr;
  ft.session_five_attr.ip_src = 8080;// dlp_http->key.saddr;
  ft.session_five_attr.port_src = 8080;//dlp_http->key.source;
  ft.session_five_attr.port_dst = 12345;//dlp_http->key.dest;
  ft.attr = &stHttpPost;
  ft.session_five_attr.protocol = 0;//webmail->pro_id;
  sbuff_putdata(0, ft);

#endif
#if 0
			ssbuffer_id =sbuff_create(100000);
     log_printf(ZLOG_LEVEL_DEBUG,"\n\n########### sbuff_create  ssbuffer_id = %d  \n\n",ssbuffer_id);
			if(ssbuffer_id<0)
				return 0;
			int thd_id =0;
			int tika_pthreads=600;
			pthread_t tika_thread[1000];
	for(thd_id=1; thd_id<tika_pthreads; thd_id++)
	  pthread_create(&tika_thread[thd_id], NULL, tika_consumer_thread, &thd_id);//tika文件内容及属性提取处理线程池创建
		// while(1)
     ci_debug_printf(2, "\n\n########### pthread create End \n\n");
#endif
#endif
#if ! defined(_WIN32)
     __log_error = (void (*)(void *, const char *,...)) log_server;     /*set c-icap library log  function */
#else
     __vlog_error = vlog_server;        /*set c-icap library  log function */
#endif

     mem_init();
     init_internal_lookup_tables();
     ci_acl_init();
     init_http_auth();
     if (init_body_system() != CI_OK) {
         ci_debug_printf(1, "Can not initialize body system\n");
         exit(-1);
     }
     ci_txt_template_init();
     ci_txt_template_set_dir(DATADIR"templates");
     commands_init();

     if (!(CI_CONF.MAGIC_DB = ci_magic_db_load(CI_CONF.magics_file))) {
          ci_debug_printf(1, "Can not load magic file %s!!!\n",
                          CI_CONF.magics_file);
     }
     init_conf_tables();
     request_stats_init();
     init_modules();
     init_services();
     config(argc, argv);
     compute_my_hostname();
     ci_debug_printf(2, "My hostname is:%s\n", MY_HOSTNAME);

     if (!log_open()) {
          ci_debug_printf(1, "Can not init loggers. Exiting.....\n");
          exit(-1);
     }

#if ! defined(_WIN32)
     if (is_icap_running(CI_CONF.PIDFILE)) {
          ci_debug_printf(1, "c-icap server already running!\n");
          exit(-1);
     }
     if (DAEMON_MODE)
          run_as_daemon();
     if (!set_running_permissions(CI_CONF.RUN_USER, CI_CONF.RUN_GROUP))
          exit(-1);
     store_pid(CI_CONF.PIDFILE);
#endif

     if (!init_server(CI_CONF.ADDRESS, CI_CONF.PORT, &(CI_CONF.PROTOCOL_FAMILY)))
          return -1;
     post_init_modules();
     post_init_services();
     start_server();
     clear_pid(CI_CONF.PIDFILE);
//		 for(thd_id=1; thd_id<tika_pthreads; thd_id++)
	//	  pthread_join(tika_thread[thd_id], NULL);
     return 0;
}
