
#ifndef  SESSION_BUF_H
#define  SESSION_BUF_H
#include <stdio.h>
#include <pcap.h>
#include <semaphore.h>
#include <pthread.h>
#define MAX_SRING_BUFFER_COUNT 50

struct four_tuple {
        uint32_t ip_src;
        uint32_t ip_dst;    
        uint16_t port_src;
        uint16_t port_dst;
};
typedef struct four_tuple four_tuple_t;


/** 
 * Structure of Each node for a ring buffer 
 * ��Ԫ����Ϣ 
 *
 * **/
typedef struct five_attr {
	uint32_t ip_src;	//ԭIP
	uint32_t ip_dst;	//Ŀ��IP
	uint16_t port_src;	//Դ�˿�
	uint16_t port_dst;	//Ŀ�Ķ˿�
	uint32_t protocol;	//���ļ����紫���Э��
} session_ft;			//��Ԫ����Ϣ

/* 
 *ftpЭ�������Ϣ
 *
 * */
typedef struct session_buffer_node_tag {
	session_ft session_five_attr;	//��Ԫ����Ϣ
	char strname[128];	//���ش洢�ļ�0��
	char strpath[128];	//�ļ�·��
	char orgname[128];	//�����ϴ����ԭʼ�ļ���
	FILE *old_file;
	void *attr;
} SESSION_BUFFER_NODE;

/**Decleration of Header Structure**/

struct session_buffer_holder {
	SESSION_BUFFER_NODE *rb_base;
	int irb_qlength;
	int irb_head_index;
	int irb_tail_index;
	int irb_buffer_end_index;
	sem_t empty_buffer;
	sem_t full_buffer;
	pthread_mutex_t mutex;
	pthread_mutex_t pmutex;
} sb_holder[MAX_SRING_BUFFER_COUNT];

int sbuff_create(int irb_qlength);
int sbuff_putdata(int iringbuffer_ID, SESSION_BUFFER_NODE ftt);
int sbuff_getdata(int iringbuffer_ID, SESSION_BUFFER_NODE * ftt);
int sbuff_destroy(int iringbuffer_ID);

#endif