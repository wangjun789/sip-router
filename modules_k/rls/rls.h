/*
 * $Id: rls.h 2230 2007-06-06 07:13:20Z anca_vamanu $
 *
 * rls module - resource list server
 *
 * Copyright (C) 2007 Voice Sistem S.R.L.
 *
 * This file is part of Kamailio, a free SIP server.
 *
 * Kamailio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * Kamailio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * History:
 * --------
 *  2007-09-11  initial version (anca)
 */

#ifndef _RLS_H_
#define _RLS_H_

#include "../../str.h"
#include "../xcap_client/xcap_functions.h"
#include "../pua/send_subscribe.h"
#include "../pua/send_publish.h"
#include "../pua/pidf.h"
#include "../presence/hash.h"
#include "../presence/event_list.h"
#include "../../modules/sl/sl.h"
#include "../../lib/srdb1/db_con.h"
#include "../../lib/srdb1/db.h"

#define NO_UPDATE_TYPE     -1 
#define UPDATED_TYPE        1 

#define NOT_KNOWN_STATE     0
#define ACTIVE_STATE        1<<1
#define PENDING_STATE       1<<2
#define TERMINATED_STATE    1<<3

typedef struct dialog_id
{
	str callid;
	str to_tag;
	str from_tag;

}dialog_id_t;

/*
	rls_presentity table structure:

	- LIST URI       (string)
	- presentity URI (string)
	- presence state (string)
	/-- the following ones needed when updating in db on timer --/
	- auth_state     (int)
	- reason		 (string)
	- updated        (int)
*/
typedef struct rls_resource
{
	str pres_uri;
	int auth_state;
	str reason;
	int updated;
	str* instance_id;
	str* cid;
	struct rls_resource* next;
	/* the last 2 parameters say if a query in database is needed */
}rls_res_t;


extern char* xcap_root;
extern unsigned int xcap_port;
extern str rls_server_address;
extern int waitn_time;
extern str rlsubs_table;
extern str rlpres_table;
extern str rls_xcap_table;
extern str db_url;
extern int hash_size;
extern shtable_t rls_table;
extern int pid;
extern int rls_max_expires;
extern int rls_integrated_xcap_server;
extern int rls_events;
extern int to_presence_code;
extern str rls_outbound_proxy;
extern int rls_max_notify_body_len;

/* database connection */
extern db1_con_t *rls_db;
extern db_func_t rls_dbf;

extern struct tm_binds tmb;
extern sl_api_t slb;

/** libxml api */
extern xmlDocGetNodeByName_t XMLDocGetNodeByName;
extern xmlNodeGetNodeByName_t XMLNodeGetNodeByName;
extern xmlNodeGetNodeContentByName_t XMLNodeGetNodeContentByName;
extern xmlNodeGetAttrContentByName_t XMLNodeGetAttrContentByName;

/* functions imported from presence to handle subscribe hash table */
extern new_shtable_t pres_new_shtable;
extern insert_shtable_t pres_insert_shtable;
extern search_shtable_t pres_search_shtable;
extern update_shtable_t pres_update_shtable;
extern delete_shtable_t pres_delete_shtable;
extern destroy_shtable_t pres_destroy_shtable;
extern mem_copy_subs_t  pres_copy_subs;
extern extract_sdialog_info_t pres_extract_sdialog_info;

/* functions imported from pua module*/
extern send_subscribe_t pua_send_subscribe;
extern get_record_id_t pua_get_record_id;

/* functions imported from presence module */
extern contains_event_t pres_contains_event;
extern search_event_t pres_search_event;
extern get_event_list_t pres_get_ev_list;

/* xcap client functions */
extern xcapGetNewDoc_t xcap_GetNewDoc;
extern xcap_nodeSel_init_t xcap_IntNodeSel;
extern xcap_nodeSel_add_step_t xcap_AddStep;
extern xcap_nodeSel_add_terminal_t xcap_AddTerminal;
extern xcap_nodeSel_free_t xcap_FreeNodeSel;

extern str str_rlsubs_did_col;
extern str str_resource_uri_col;
extern str str_updated_col;
extern str str_auth_state_col;
extern str str_reason_col;
extern str str_content_type_col;
extern str str_presence_state_col;
extern str str_expires_col;
extern str str_presentity_uri_col;
extern str str_event_col;
extern str str_event_id_col;
extern str str_to_user_col;
extern str str_to_domain_col;
extern str str_watcher_username_col;
extern str str_watcher_domain_col;
extern str str_callid_col;
extern str str_to_tag_col;
extern str str_from_tag_col;
extern str str_local_cseq_col;
extern str str_remote_cseq_col;
extern str str_record_route_col;
extern str str_socket_info_col;
extern str str_contact_col;
extern str str_local_contact_col;
extern str str_version_col;
extern str str_status_col;
extern str str_username_col;
extern str str_domain_col;
extern str str_doc_type_col;
extern str str_etag_col;
extern str str_doc_col;
extern str str_doc_uri_col;

#define RLS_DID_SEP       ";"
#define RLS_DID_SEP_LEN   strlen(RLS_DID_SEP)
#define RLS_DID_INIT_LEN  (2* sizeof(RLS_DID_SEP))
#define RLS_DID_MAX_LEN	255

/* did_str= *callid*RLS_DID_SEP*from_tag*RLS_DID_SEP*to_tag* */

static inline int CONSTR_RLSUBS_DID(subs_t* subs, str *did)
{
	int len;

	len= (RLS_DID_INIT_LEN+ subs->callid.len+ subs->to_tag.len+
			subs->from_tag.len+ 10)* sizeof(char);
	if(len > RLS_DID_MAX_LEN)
	{
		LM_ERR("new DID size is too big [%d > %d]\n",
				len, RLS_DID_MAX_LEN);
		return -1;
	}
	did->s= (char*)pkg_malloc(len);
	if(did->s== NULL) 
	{
		ERR_MEM(PKG_MEM_STR); 
	}
	
	did->len= sprintf(did->s, "%.*s%s%.*s%s%.*s", subs->callid.len, 
			subs->callid.s, RLS_DID_SEP,subs->from_tag.len, subs->from_tag.s,
			RLS_DID_SEP, subs->to_tag.len, subs->to_tag.s);

	if(did->len>= len)
	{
		LM_ERR("ERROR buffer size overflown\n");
		pkg_free(did->s);
		return -1;
	}
	did->s[did->len]= '\0';
	
	LM_DBG("did= %s\n", did->s);
	return 0;
error:
	return -1;
}

#endif
