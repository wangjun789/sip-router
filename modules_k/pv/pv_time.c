/*
 * $Id: pv_shv.c 5291 2008-12-03 16:47:50Z miconda $
 *
 * Copyright (C) 2007 Elena-Ramona Modroiu
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
 */

#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../../dprint.h"
#include "../../pvar.h"

#include "pv_time.h"

static unsigned int _pv_msg_id = 0;
static time_t _pv_msg_tm = 0;
static int pv_update_time(struct sip_msg *msg, time_t *t)
{
	if(_pv_msg_id != msg->id || _pv_msg_tm==0)
	{
		_pv_msg_tm = time(NULL);
		_pv_msg_id = msg->id;
		
		if(t!=NULL)
			*t=_pv_msg_tm;

		return 1;
	}
	if(t!=NULL)
		*t=_pv_msg_tm;
	return 0;
}

int pv_parse_time_name(pv_spec_p sp, str *in)
{
	if(sp==NULL || in==NULL || in->len<=0)
		return -1;

	switch(in->len)
	{
		case 3: 
			if(strncmp(in->s, "sec", 3)==0)
				sp->pvp.pvn.u.isname.name.n = 0;
			else if(strncmp(in->s, "min", 3)==0)
				sp->pvp.pvn.u.isname.name.n = 1;
			else if(strncmp(in->s, "mon", 3)==0)
				sp->pvp.pvn.u.isname.name.n = 4;
			else goto error;
		break;
		case 4: 
			if(strncmp(in->s, "hour", 4)==0)
				sp->pvp.pvn.u.isname.name.n = 2;
			else if(strncmp(in->s, "mday", 4)==0)
				sp->pvp.pvn.u.isname.name.n = 3;
			else if(strncmp(in->s, "year", 4)==0)
				sp->pvp.pvn.u.isname.name.n = 5;
			else if(strncmp(in->s, "wday", 4)==0)
				sp->pvp.pvn.u.isname.name.n = 6;
			else if(strncmp(in->s, "yday", 4)==0)
				sp->pvp.pvn.u.isname.name.n = 7;
			else goto error;
		break;
		case 5: 
			if(strncmp(in->s, "isdst", 5)==0)
				sp->pvp.pvn.u.isname.name.n = 8;
			else goto error;
		break;
		default:
			goto error;
	}
	sp->pvp.pvn.type = PV_NAME_INTSTR;
	sp->pvp.pvn.u.isname.type = 0;

	return 0;

error:
	LM_ERR("unknown PV time name %.*s\n", in->len, in->s);
	return -1;
}

static struct tm _cfgutils_ts;
static unsigned int _cfgutils_msg_id = 0;

int pv_get_time(struct sip_msg *msg, pv_param_t *param,
		pv_value_t *res)
{
	time_t t;

	if(msg==NULL || param==NULL)
		return -1;

	if(_cfgutils_msg_id != msg->id)
	{
		pv_update_time(msg, &t);
		_cfgutils_msg_id = msg->id;
		if(localtime_r(&t, &_cfgutils_ts) == NULL)
		{
			LM_ERR("unable to break time to attributes\n");
			return -1;
		}
	}
	
	switch(param->pvn.u.isname.name.n)
	{
		case 1:
			return pv_get_uintval(msg, param, res, (unsigned int)_cfgutils_ts.tm_min);
		case 2:
			return pv_get_uintval(msg, param, res, (unsigned int)_cfgutils_ts.tm_hour);
		case 3:
			return pv_get_uintval(msg, param, res, (unsigned int)_cfgutils_ts.tm_mday);
		case 4:
			return pv_get_uintval(msg, param, res, 
					(unsigned int)(_cfgutils_ts.tm_mon+1));
		case 5:
			return pv_get_uintval(msg, param, res,
					(unsigned int)(_cfgutils_ts.tm_year+1900));
		case 6:
			return pv_get_uintval(msg, param, res, 
					(unsigned int)(_cfgutils_ts.tm_wday+1));
		case 7:
			return pv_get_uintval(msg, param, res, 
					(unsigned int)(_cfgutils_ts.tm_yday+1));
		case 8:
			return pv_get_sintval(msg, param, res, _cfgutils_ts.tm_isdst);
		default:
			return pv_get_uintval(msg, param, res, (unsigned int)_cfgutils_ts.tm_sec);
	}
}

int pv_get_timenows(struct sip_msg *msg, pv_param_t *param,
		pv_value_t *res)
{
	time_t t;
	t = time(NULL);
	return pv_get_uintval(msg, param, res, (unsigned int)t);
}


int pv_get_timenowf(struct sip_msg *msg, pv_param_t *param,
		pv_value_t *res)
{
	str s;
	time_t t;
	
	t = time(NULL);
	s.s = ctime(&t);
	s.len = strlen(s.s)-1;
	return pv_get_strintval(msg, param, res, &s, (int)t);
}

int pv_get_times(struct sip_msg *msg, pv_param_t *param,
		pv_value_t *res)
{
	time_t t;
	if(msg==NULL)
		return -1;

	pv_update_time(msg, &t);

	return pv_get_uintval(msg, param, res, (unsigned int)t);
}


int pv_get_timef(struct sip_msg *msg, pv_param_t *param,
		pv_value_t *res)
{
	str s;
	time_t t;
	
	if(msg==NULL)
		return -1;

	pv_update_time(msg, &t);
	
	s.s = ctime(&t);
	s.len = strlen(s.s)-1;
	return pv_get_strintval(msg, param, res, &s, (int)t);
}
