/**
 * $Id$
 *
 * Copyright (C) 2011 Daniel-Constantin Mierla (asipto.com)
 *
 * This file is part of Kamailio, a free SIP server.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "../../sr_module.h"
#include "../../dprint.h"
#include "../../ut.h"
#include "../../pvar.h"
#include "../../timer_proc.h"
#include "../../route_struct.h"
#include "../../modules/tm/tm_load.h"

#include "async_sleep.h"

MODULE_VERSION

static int async_workers = 1;

static int  mod_init(void);
static int  child_init(int);
static void mod_destroy(void);

static int w_async_sleep(struct sip_msg* msg, char* sec, char* str2);
static int fixup_async_sleep(void** param, int param_no);

/* tm */
struct tm_binds tmb;


static cmd_export_t cmds[]={
	{"async_sleep", (cmd_function)w_async_sleep, 1, fixup_async_sleep,
		0, REQUEST_ROUTE|FAILURE_ROUTE|BRANCH_ROUTE},
	{0, 0, 0, 0, 0, 0}
};

static param_export_t params[]={
	{"workers",     INT_PARAM,   &async_workers},
	{0, 0, 0}
};

struct module_exports exports = {
	"async",
	DEFAULT_DLFLAGS, /* dlopen flags */
	cmds,
	params,
	0,
	0,              /* exported MI functions */
	0,              /* exported pseudo-variables */
	0,              /* extra processes */
	mod_init,       /* module initialization function */
	0,              /* response function */
	mod_destroy,    /* destroy function */
	child_init      /* per child init function */
};



/**
 * init module function
 */
static int mod_init(void)
{
	if (load_tm_api( &tmb ) == -1)
	{
		LM_ERR("cannot load the TM-functions\n");
		return -1;
	}

	if(async_init_timer_list()<0) {
		LM_ERR("cannot initialize internal structure\n");
		return -1;
	}

	register_dummy_timers(async_workers);

	return 0;
}

/**
 * @brief Initialize async module children
 */
static int child_init(int rank)
{
	if (rank!=PROC_MAIN)
		return 0;

	if(fork_dummy_timer(PROC_TIMER, "ASYNC MOD TIMER", 1 /*socks flag*/,
				async_timer_exec, NULL, 1 /*sec*/)<0) {
		LM_ERR("failed to register timer routine as process\n");
		return -1; /* error */
	}

	return 0;
}
/**
 * destroy module function
 */
static void mod_destroy(void)
{
	async_destroy_timer_list();
}

static int w_async_sleep(struct sip_msg* msg, char* sec, char* str2)
{
	int s;
	async_param_t *ai;
	
	if(msg==NULL)
		return -1;

	ai = (async_param_t*)sec;
	if(fixup_get_ivalue(msg, ai->pinterval, &s)!=0)
	{
		LM_ERR("no async sleep time value\n");
		return -1;
	}
	if(ai->type==0)
	{
		if(async_sleep(msg, s, ai->u.paction)<0)
			return -1;
		/* force exit in config */
		return 0;
	}

	return -1;
}

static int fixup_async_sleep(void** param, int param_no)
{
	async_param_t *ap;
	if(param_no!=1)
		return 0;
	ap = (async_param_t*)pkg_malloc(sizeof(async_param_t));
	if(ap==NULL)
	{
		LM_ERR("no more pkg\n");
		return -1;
	}
	memset(ap, 0, sizeof(async_param_t));
	ap->u.paction = get_action_from_param(param, param_no);
	if(fixup_igp_null(param, param_no)<0)
		return -1;
	ap->pinterval = (gparam_t*)(*param);
	*param = (void*)ap;
	return 0;
}