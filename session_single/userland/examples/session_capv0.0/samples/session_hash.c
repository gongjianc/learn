/*
 * sessionlist.c - session list stuff
 *
 * 2009, 2010 Mike Schiffman <mschiffm@cisco.com> 
 *
 * Copyright (c) 2010 by Cisco Systems, Inc.
 * All rights reserved.
 * Based off of tcpxtract by Nicholas Harbour.
 */

/* This file is part of
   Tcpxtract, a sniffer that extracts files based on headers
   by Nick Harbour
*/

//#include "nfex.h"
#include "session_hash.h"
//#include "extract.h"
#include "confy.c"

ss_node_t *ht_insert(four_tuple_t * ft, ncc_t * ncc)
{
	uint32_t n, j;
	ss_node_t *p;

	n = ht_hash(ft);

	if (ncc->ht[n] == NULL) {
	/** create first entry in this slot */
		ncc->ht[n] = malloc(sizeof(ss_node_t));
		if (ncc->ht[n] == NULL) {
			return (NULL);
		}
		memcpy(&(ncc->ht[n]->ft), ft, sizeof(four_tuple_t));
		ncc->ht[n]->timestamp = time(NULL);
		ncc->ht[n]->data = NULL;
		ncc->ht[n]->next = NULL;
		ncc->ht[n]->prev = NULL;
		ncc->n_stats.ht_nc = 0;
		ncc->n_stats.ht_ic = 0;
		ncc->n_stats.ht_lc = 50;
		ncc->n_stats.ht_entries = 0;
		ncc->ht[n]->ssptr_list.total_packets = 0;
		ncc->ht[n]->ssptr_list.packet_bytes = 0;
		ncc->ht[n]->ssptr_list.total_bytes = 0;
		ncc->ht[n]->ssptr_list.packet_errors = 0;
		ncc->ht[n]->ssptr_list.ss_nc = 0;
		ncc->ht[n]->ssptr_list.ss_ic = 0;
		ncc->ht[n]->ssptr_list.ss_lc = 50;
		gettimeofday(&ncc->ht[n]->ssptr_list.ts_start, NULL);
		gettimeofday(&ncc->ht[n]->ssptr_list.ts_last, NULL);
		p = ncc->ht[n];
	/** update ht stats: non chained entry */
		ncc->n_stats.ht_nc++;
		ncc->n_stats.ht_entries++;
		return (p);
	} else {
/** collision */
		p = ht_find(ft, ncc);
		if (p != NULL) {
			for (p = ncc->ht[n], j = 0; p->next !=NULL ; p = p->next, j++) ;
	/** end of chain, no duplicate, add a new one */
			p->next = malloc(sizeof(ss_node_t));
			if (p->next == NULL) {
				return (NULL);
			}
			//memcpy(&(p->next->ft), ft, sizeof(four_tuple_t));
			p->next->timestamp = time(NULL);
			p->next->data = NULL;
			p->next->next = NULL;
			p->next->prev = p;

	/** update ht stats: chained entry */
			ncc->n_stats.ht_ic++;
	/** update ht stats: longest chain */
			ncc->n_stats.ht_lc = j > ncc->n_stats.ht_lc ? j : ncc->n_stats.ht_lc;
//	ncc->n_stats.ht_entries++;
		return (p->next);
		}
#if 1
		else {
			ncc->ht[n] = NULL;
	/** create first entry in this slot */
			ncc->ht[n] = malloc(sizeof(ss_node_t));
			if (ncc->ht[n] == NULL) {
//            fprintf(stderr, "ht_insert(): malloc(): %s\n", strerror(errno));
				return (NULL);
			}
			memcpy(&(ncc->ht[n]->ft), ft, sizeof(four_tuple_t));
			ncc->ht[n]->timestamp = time(NULL);
			ncc->ht[n]->data = NULL;
			ncc->ht[n]->next = NULL;
			ncc->ht[n]->prev = NULL;
			ncc->n_stats.ht_nc = 0;
			ncc->n_stats.ht_ic = 0;
			ncc->n_stats.ht_lc = 50;
			ncc->n_stats.ht_entries = 0;
			ncc->ht[n]->ssptr_list.total_packets = 0;
			ncc->ht[n]->ssptr_list.packet_bytes = 0;
			ncc->ht[n]->ssptr_list.total_bytes = 0;
			ncc->ht[n]->ssptr_list.packet_errors = 0;
			ncc->ht[n]->ssptr_list.ss_nc = 0;
			ncc->ht[n]->ssptr_list.ss_ic = 0;
			ncc->ht[n]->ssptr_list.ss_lc = 50;
			gettimeofday(&ncc->ht[n]->ssptr_list.ts_start, NULL);
			gettimeofday(&ncc->ht[n]->ssptr_list.ts_last, NULL);
			p = ncc->ht[n];
	/** update ht stats: non chained entry */
			ncc->n_stats.ht_nc++;
			ncc->n_stats.ht_entries;
//        printf("session table is reload!!!!!!\n");
                   return(p);
		}
#endif
	}
#if 0
	if (ncc->flags & NFEX_DEBUG) {
		fprintf(stderr, "new session: ");
		fprintip(stderr, ft->ip_src, ncc);
		fprintf(stderr, ":%d -> ", ntohs(ft->port_src));
		fprintip(stderr, ft->ip_dst, ncc);
		fprintf(stderr, ":%d\n", ntohs(ft->port_dst));
	}
#endif
    /** update ht stats: total entries */
}

uint32_t ht_hash(four_tuple_t * ft)
{
	int i;
	uint8_t *p;
	uint64_t hash;

	/* Fowler–Noll–Vo hash: http://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash */
	for (hash = 0, i = 0, p = (uint8_t *) ft; i < 12; p++, i++) {
		hash *= FNV_PRIME;
		hash ^= (*p);
	}

	return (hash % NFEX_HT_SIZE);
}

ss_node_t *ht_get(four_tuple_t * ft, ncc_t * ncc, uint32_t hash_num)
{
	uint32_t n;
	ss_node_t *p;

	n = ht_hash(ft);

        hash_num = n;

	if (ncc->ht[n] == NULL) {
//    printf("hash table number is %d\n",n);
		return (NULL);
	} else {
/** collision */
//		p = ht_find(ft, ncc);
//		if (p != NULL) {
//			return (p);
           return ncc->ht[n];
//		} else
//			ncc->ht[n] = NULL;
	}
}

ss_node_t *ht_find(four_tuple_t * ft, ncc_t * ncc)
{
	uint32_t n;
	ss_node_t *p;

	n = ht_hash(ft);
//    printf("hash table number is %d\n",n);
	for (p = ncc->ht[n]; p != NULL; p = p->next) {
		if (memcmp(ft, &p->ft, sizeof(four_tuple_t)) == 0) {
	    /** found him, update timestamp */
	       p->timestamp = time(NULL);
			return (p);
		}
	}
//    printf("hash table number is %d,but table is none!!\n",n);
	ncc->ht[n] = NULL;
	return (NULL);
}

void ht_session_setnum(four_tuple_t * ft, ncc_t * ncc, uint32_t num)
{
	uint32_t n;

	n = ht_hash(ft);

	if (ncc->ht[n] != NULL){ 
	    ncc->ht[n]->ssptr_list.total_packets++;
	    ncc->ht[n]->ssptr_list.total_bytes +=num;
	}
}

int delete_node(ncc_t * pHashTbl, ss_node_t * p, four_tuple_t * ft)
{
	ss_node_t *pNode;

	if ((NULL == pHashTbl) || (NULL == p))
		return 0;

	if (NULL == (pNode = ht_find(ft, pHashTbl)))
		return 0;

	if (p == pNode) //{
		pNode = p->next;
//		free(p->data);
//		free(p);
//		p = pNode;
//		return 1;
//	}
    goto final;  

	while (p != pNode->next)
		pNode = pNode->next;
	pNode = p->next;
//	free(p->data);
//	free(p);
//	p = pNode;
//	return 1;
#if 1
      final:
	free(p->data);
	free(p);
	p = pNode;
	return 1;
#endif
}

void ht_tabledown(four_tuple_t * ft, ncc_t * ncc, uint32_t hash_num)
{
    uint32_t n;
    ss_node_t *p;
//    n = ht_hash(ft);

    for (p = ncc->ht[hash_num]; p != NULL; p = p->next) {
        free(p->data);
	free(p);
    }
    ncc->ht[hash_num] = NULL;
}

void ht_dump(ncc_t * ncc)
{
	uint8_t addr[4];
	time_t now;
	uint32_t n;
	ss_node_t *p;

	if (ncc->n_stats.ht_entries == 0) {
		printf("session table empty\n");
		return;
	}

	now = time(NULL);

	for (n = 0; n < NFEX_HT_SIZE; n++) {
		for (p = ncc->ht[n]; p; p = p->next) {
			#if 0
			memcpy(addr, p->ft.ip_src, 4);
			fprintf(stdout, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
//            fprintip(stdout, p->ft.ip_src, ncc);
			fprintf(stdout, ":%d -> ", ntohs(p->ft.port_src));
			memcpy(addr, p->ft.ip_dst, 4);
			fprintf(stdout, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
//            fprintip(stdout, p->ft.ip_dst, ncc);
			fprintf(stdout, ":%d ", ntohs(p->ft.port_dst));
			fprintf(stdout, "%lds\n", now - p->timestamp);
            #endif

		}
	}
}

void ht_shutitdown(ncc_t * ncc)
{
	uint32_t n;
	ss_node_t *p, *q;

	for (n = 0; n < NFEX_HT_SIZE; n++) {
		for (p = ncc->ht[n]; p; p = q->next) {
			q = p;
			free(p);
		}
		ncc->ht[n] = NULL;
	}
	ncc->n_stats.ht_entries = 0;
}

void ht_expire_session(ncc_t * ncc)
{
	time_t now;
	uint32_t n;
	uint32_t j;
	ss_node_t *p, **q;

	if (ncc->n_stats.ht_entries == 0) {
		printf("session table empty\n");
		return;
	}

	now = time(NULL);

	for (j = 0, n = 0; n < NFEX_HT_SIZE; n++) {
		for (p = ncc->ht[n]; p; p = p->next) {
	    /** if the timestamp is older than SESSION_THRESHOLD, delete */
			if (now - p->timestamp >= SESSION_THRESHOLD) {
				if (p->prev == NULL) {
		    /** first entry in a chain */
					free(p);
					ncc->ht[n] = NULL;
		    /** update ht stats: non chained entry */
					ncc->n_stats.ht_nc--;
				} else {
					p->prev->next = p->next;
					if (p->next) {
						p->next->prev = p->prev;
					}
					free(p);
		    /** update ht stats: chained entry */
					ncc->n_stats.ht_ic--;
				}
				j++;
		/** update ht stats: total entries */
				ncc->n_stats.ht_entries--;
			}
		}
	}
//    if (j && ncc->flags & NFEX_DEBUG)
//    {
//        printf("[DEBUG MODE] expired %d sessions from hash table\n", j);
//    }
}

#if 0
uint32_t ht_count_extracts(ncc_t * ncc)
{
	uint16_t n, j;
	ss_node_t *p;

	for (n = 0, j = 0; n < NFEX_HT_SIZE; n++) {
		for (p = ncc->ht[n]; p; p = p->next) {
			if (p->extract_list) {
				if (p->extract_list->fd) {
					j++;
				}
			}
		}
	}
	return (j);
}
#endif

void ht_status(ncc_t * ncc)
{
	uint32_t n;

	if (ncc->n_stats.ht_entries == 0) {
		printf("session table empty\n");
		return;
	}

	printf("hash table status\n");
	printf("table size:\t\t\t%d\n", NFEX_HT_SIZE);
	printf("table population:\t\t%d\n", ncc->n_stats.ht_entries);
	printf("un-chained entries:\t\t%d\n", ncc->n_stats.ht_nc);
	printf("chained entries:\t\t%d\n", ncc->n_stats.ht_ic);
	printf("longest chain:\t\t\t%d\n", ncc->n_stats.ht_lc);
}

/** EOF */