/**
 * $Id$
 *
 * Copyright (C) 2008 Elena-Ramona Modroiu (asipto.com)
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
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <fcntl.h>

#include "../../sr_module.h"
#include "../../timer.h"
#include "../../dprint.h"
#include "../../lib/kmi/mi.h"

#include "../../pvar.h"
#include "ht_api.h"
#include "ht_db.h"
#include "ht_var.h"


MODULE_VERSION

int  ht_timer_interval = 20;

/** module functions */
static int ht_print(struct sip_msg*, char*, char*);
static int mod_init(void);
void destroy(void);

static int fixup_ht_rm(void** param, int param_no);
static int ht_rm_name_re(struct sip_msg* msg, char* key, char* foo);
static int ht_rm_value_re(struct sip_msg* msg, char* key, char* foo);

int ht_param(modparam_t type, void* val);

static struct mi_root* ht_mi_reload(struct mi_root* cmd_tree, void* param);

static pv_export_t mod_pvs[] = {
	{ {"sht", sizeof("sht")-1}, PVT_OTHER, pv_get_ht_cell, pv_set_ht_cell,
		pv_parse_ht_name, 0, 0, 0 },
	{ {"shtex", sizeof("shtex")-1}, PVT_OTHER, pv_get_ht_cell_expire,
		pv_set_ht_cell_expire,
		pv_parse_ht_name, 0, 0, 0 },
	{ {0, 0}, 0, 0, 0, 0, 0, 0, 0 }
};

static mi_export_t mi_cmds[] = {
	{ "sht_reload",     ht_mi_reload,  0,  0,  0},
	{ 0, 0, 0, 0, 0}
};


static cmd_export_t cmds[]={
	{"sht_print",  (cmd_function)ht_print,  0, 0, 0, 
		REQUEST_ROUTE | FAILURE_ROUTE |
		ONREPLY_ROUTE | BRANCH_ROUTE | ERROR_ROUTE | LOCAL_ROUTE},
	{"sht_rm_name_re",  (cmd_function)ht_rm_name_re,  1, fixup_ht_rm, 0, 
		REQUEST_ROUTE | FAILURE_ROUTE |
		ONREPLY_ROUTE | BRANCH_ROUTE | ERROR_ROUTE | LOCAL_ROUTE},
	{"sht_rm_value_re",  (cmd_function)ht_rm_value_re,  1, fixup_ht_rm, 0, 
		REQUEST_ROUTE | FAILURE_ROUTE |
		ONREPLY_ROUTE | BRANCH_ROUTE | ERROR_ROUTE | LOCAL_ROUTE},
	{0,0,0,0,0,0}
};

static param_export_t params[]={
	{"htable",             STR_PARAM|USE_FUNC_PARAM, (void*)ht_param},
	{"db_url",             STR_PARAM, &ht_db_url.s},
	{"key_name_column",    STR_PARAM, &ht_db_name_column.s},
	{"key_type_column",    STR_PARAM, &ht_db_ktype_column.s},
	{"value_type_column",  STR_PARAM, &ht_db_vtype_column.s},
	{"key_value_column",   STR_PARAM, &ht_db_value_column.s},
	{"array_size_suffix",  STR_PARAM, &ht_array_size_suffix.s},
	{"fetch_rows",         INT_PARAM, &ht_fetch_rows},
	{"timer_interval",     INT_PARAM, &ht_timer_interval},
	{0,0,0}
};


/** module exports */
struct module_exports exports= {
	"htable",
	DEFAULT_DLFLAGS, /* dlopen flags */
	cmds,
	params,
	0,          /* exported statistics */
	mi_cmds,    /* exported MI functions */
	mod_pvs,    /* exported pseudo-variables */
	0,          /* extra processes */
	mod_init,   /* module initialization function */
	0,
	(destroy_function) destroy,
	0           /* per-child init function */
};

/**
 * init module function
 */
static int mod_init(void)
{
	if(ht_shm_init()!=0)
		return -1;
	ht_db_init_params();

	if(ht_db_url.len>0)
	{
		if(ht_db_init_con()!=0)
			return -1;
		if(ht_db_open_con()!=0)
			return -1;
		if(ht_db_load_tables()!=0)
		{
			ht_db_close_con();
			return -1;
		}
		ht_db_close_con();
	}
	if(ht_has_autoexpire())
	{
		LM_DBG("starting auto-expire timer\n");
		if(ht_timer_interval<=0)
			ht_timer_interval = 20;
		if(register_timer(ht_timer, 0, ht_timer_interval)<0)
		{
			LM_ERR("failed to register timer function\n");
			return -1;
		}
	}

	return 0;
}

/**
 * destroy function
 */
void destroy(void)
{
	ht_destroy();
}

/**
 * print hash table content
 */
static int ht_print(struct sip_msg *msg, char *s1, char *s2)
{
	ht_dbg();
	return 1;
}

static int fixup_ht_rm(void** param, int param_no)
{
	pv_spec_t *sp;
	str s;

	sp = (pv_spec_t*)pkg_malloc(sizeof(pv_spec_t));
	if(param_no != 1)
	{
		LM_ERR("invalid parameter number %d\n", param_no);
		return -1;
	}
	if (sp == 0)
	{
		LM_ERR("no pkg memory left\n");
		return -1;
	}
	memset(sp, 0, sizeof(pv_spec_t));
	s.s = (char*)*param; s.len = strlen(s.s);
	if(pv_parse_ht_name(sp, &s)<0)
	{
		pkg_free(sp);
		LM_ERR("invalid parameter %d\n", param_no);
		return -1;
	}
	*param = (void*)sp;
	return 0;
}

static int ht_rm_name_re(struct sip_msg* msg, char* key, char* foo)
{
	ht_pv_t *hpv;
	str sre;
	pv_spec_t *sp;
	sp = (pv_spec_t*)key;

	hpv = (ht_pv_t*)sp->pvp.pvn.u.dname;

	if(hpv->ht==NULL)
	{
		hpv->ht = ht_get_table(&hpv->htname);
		if(hpv->ht==NULL)
			return 1;
	}
	if(pv_printf_s(msg, hpv->pve, &sre)!=0)
	{
		LM_ERR("cannot get $ht expression\n");
		return -1;
	}
	if(ht_rm_cell_re(&sre, hpv->ht, 0)<0)
		return -1;
	return 1;
}

static int ht_rm_value_re(struct sip_msg* msg, char* key, char* foo)
{
	ht_pv_t *hpv;
	str sre;
	pv_spec_t *sp;
	sp = (pv_spec_t*)key;

	hpv = (ht_pv_t*)sp->pvp.pvn.u.dname;

	if(hpv->ht==NULL)
	{
		hpv->ht = ht_get_table(&hpv->htname);
		if(hpv->ht==NULL)
			return 1;
	}
	if(pv_printf_s(msg, hpv->pve, &sre)!=0)
	{
		LM_ERR("cannot get $ht expression\n");
		return -1;
	}

	if(ht_rm_cell_re(&sre, hpv->ht, 1)<0)
		return -1;
	return 1;
}


int ht_param(modparam_t type, void *val)
{
	if(val==NULL)
		goto error;

	return ht_table_spec((char*)val);
error:
	return -1;

}

#define MI_ERR_RELOAD 			"ERROR Reloading data"
#define MI_ERR_RELOAD_LEN 		(sizeof(MI_ERR_RELOAD)-1)
static struct mi_root* ht_mi_reload(struct mi_root* cmd_tree, void* param)
{
	struct mi_node* node;
	str htname;
	ht_t *ht;
	ht_t nht;
	ht_cell_t *first;
	ht_cell_t *it;
	int i;

	if(ht_db_url.len<=0)
		return init_mi_tree(500, MI_ERR_RELOAD, MI_ERR_RELOAD_LEN);
	
	if(ht_db_init_con()!=0)
		return init_mi_tree(500, MI_ERR_RELOAD, MI_ERR_RELOAD_LEN);
	if(ht_db_open_con()!=0)
		return init_mi_tree(500, MI_ERR_RELOAD, MI_ERR_RELOAD_LEN);

	node = cmd_tree->node.kids;
	if(node == NULL)
		return init_mi_tree( 400, MI_MISSING_PARM_S, MI_MISSING_PARM_LEN);
	htname = node->value;
	if(htname.len<=0 || htname.s==NULL)
	{
		LM_ERR("bad hash table name\n");
		return init_mi_tree( 500, "bad hash table name", 19);
	}
	ht = ht_get_table(&htname);
	if(ht==NULL || ht->dbtable.len<=0)
	{
		LM_ERR("bad hash table name\n");
		return init_mi_tree( 500, "no such hash table", 18);
	}
	memcpy(&nht, ht, sizeof(ht_t));
	nht.entries = (ht_entry_t*)shm_malloc(nht.htsize*sizeof(ht_entry_t));
	if(nht.entries == NULL)
		return init_mi_tree(500, MI_ERR_RELOAD, MI_ERR_RELOAD_LEN);
	memset(nht.entries, 0, nht.htsize*sizeof(ht_entry_t));

	if(ht_db_load_table(&nht, &ht->dbtable, 0)<0)
		return init_mi_tree(500, MI_ERR_RELOAD, MI_ERR_RELOAD_LEN);

	/* replace old entries */
	for(i=0; i<nht.htsize; i++)
	{
		lock_get(&ht->entries[i].lock);
		first = ht->entries[i].first;
		ht->entries[i].first = nht.entries[i].first;
		ht->entries[i].esize = nht.entries[i].esize;
		lock_release(&ht->entries[i].lock);
		nht.entries[i].first = first;
	}
	/* free old entries */
	for(i=0; i<nht.htsize; i++)
	{
		first = nht.entries[i].first;
		while(first)
		{
			it = first;
			first = first->next;
			ht_cell_free(it);
		}
	}
	ht_db_close_con();
	return init_mi_tree( 200, MI_OK_S, MI_OK_LEN);
}
