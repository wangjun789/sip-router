/*
 * $Id$
 *
 * Copyright (C) 2011 Daniel-Constantin Mierla (asipto.com)
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


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../dprint.h"
#include "../../trim.h"
#include "sdpops_data.h"

/*

http://www.iana.org/assignments/rtp-parameters

Registry Name: RTP Payload types (PT) for standard audio and video encodings - Closed
Reference: [RFC3551]
Registration Procedures: Registry closed; see [RFC3551], Section 3

Note:
The RFC "RTP Profile for Audio and Video Conferences with Minimal
Control" [RFC3551] specifies an initial set "payload types".  This
list maintains and extends that list.

Registry:
PT        encoding name   audio/video (A/V)  clock rate (Hz)  channels (audio)  Reference
--------  --------------  -----------------  ---------------  ----------------  ---------
0         PCMU            A                  8000             1                 [RFC3551]
1         Reserved	  
2         Reserved
3         GSM             A                  8000             1                 [RFC3551]
4         G723            A                  8000             1                 [Kumar][RFC3551]
5         DVI4            A                  8000             1                 [RFC3551]
6         DVI4            A                  16000            1                 [RFC3551]
7         LPC             A                  8000             1                 [RFC3551]
8         PCMA            A                  8000             1                 [RFC3551]
9         G722            A                  8000             1                 [RFC3551]
10        L16             A                  44100            2                 [RFC3551]
11        L16             A                  44100            1                 [RFC3551]
12        QCELP           A                  8000             1                 [RFC3551]
13        CN              A                  8000             1                 [RFC3389]
14        MPA             A                  90000                              [RFC3551][RFC2250]
15        G728            A                  8000             1                 [RFC3551]
16        DVI4            A                  11025            1                 [DiPol]
17        DVI4            A                  22050            1                 [DiPol]
18        G729            A                  8000             1                 [RFC3551]
19        Reserved        A
20        Unassigned      A
21        Unassigned      A
22        Unassigned      A
23        Unassigned      A
24        Unassigned      V
25        CelB            V                  90000                              [RFC2029]
26        JPEG            V                  90000                              [RFC2435]
27        Unassigned      V
28        nv              V                  90000                              [RFC3551]
29        Unassigned      V
30        Unassigned      V
31        H261            V                  90000                              [RFC4587]
32        MPV             V                  90000                              [RFC2250]
33        MP2T            AV                 90000                              [RFC2250]
34        H263            V                  90000                              [Zhu]
35-71     Unassigned      ?
72-76     Reserved for RTCP conflict avoidance                                  [RFC3551]
77-95     Unassigned      ?
96-127    dynamic         ?                                                     [RFC3551] 


Registry Name: RTP Payload Format media types
Reference: [RFC4855]
Registration Procedures: Standards Action Process or expert approval

*/

typedef struct _codecsmap {
	str name;
	str ids;
} codecsmap_t;

codecsmap_t sdpops_codecsmap_table[] = {
	{ {"PCMU",  4},  {"0",         1} },
	{ {"GSM",   3},  {"3",         1} },
	{ {"G723",  4},  {"4",         1} },
	{ {"DVI4",  4},  {"5,6,16,17", 9} },
	{ {"LPC",   3},  {"7",         1} },
	{ {"PCMA",  4},  {"8",         1} },
	{ {"G722",  4},  {"9",         1} },
	{ {"L16",   3},  {"10,11",     5} },
	{ {"QCELP", 5},  {"12",        2} },
	{ {"CN",    2},  {"13",        5} },
	{ {"MPA",   3},  {"14",        2} },
	{ {"G728",  4},  {"15",        2} },
	{ {"G729",  4},  {"18",        2} },
	{ {0, 0}, {0, 0} }
};

/**
 * set the string with the IDs mapped to codec name
 * - return 0 if found, -1 if not found
 */
int sdpops_get_ids_by_name(str *name, str *ids)
{
	int i;
	for(i=0; sdpops_codecsmap_table[i].name.s!=0; i++)
	{
		if(name->len==sdpops_codecsmap_table[i].name.len
				&& strncasecmp(sdpops_codecsmap_table[i].name.s, name->s,
					name->len)==0)
		{
			*ids = sdpops_codecsmap_table[i].ids;
			return 0;
		}
	}
	ids->s = NULL;
	ids->len = 0;
	return -1;
}

/**
 * build the csv list of ids from csv list of names
 */
int sdpops_build_ids_list(str *names, str *ids)
{
#define SDPOPS_MAX_LIST_SIZE	64
	static char _local_idslist[SDPOPS_MAX_LIST_SIZE];
	str tmp;
	str codec;
	str cids;
	char *p;

	tmp = *names;
	ids->len = 0;
	ids->s   = 0;
	p = _local_idslist;
	while(str_find_token(&tmp, &codec, ',')==0
		&& codec.len>0)
	{
		tmp.len -= (int)(&codec.s[codec.len]-codec.s);
		tmp.s = codec.s + codec.len;

		if( sdpops_get_ids_by_name(&codec, &cids)==0) {
			LM_DBG("codecs list [%.*s] - at name [%.*s] with ids [%.*s]\n",
				names->len, names->s,
				codec.len, codec.s,
				cids.len,  cids.s);
			if(ids->len + cids.len>=SDPOPS_MAX_LIST_SIZE)
			{
				LM_ERR("the list with codecs ids is too big\n");
				ids->len = 0;
				ids->s = 0;
				return -1;
			}
			strncpy(p, cids.s, cids.len);
			p += cids.len;
			*p = ',';
			p++;
			ids->len += cids.len + 1;
		}
	}
	if(ids->len>0)
	{
		p--;
		ids->len--;
		*p = '\0';
		ids->s = _local_idslist;
		LM_DBG("codecs list [%.*s] - ids list [%.*s]\n",
				names->len, names->s,
				ids->len, ids->s);
		return 0;
	}
	return -1;
}


/**
 *
 */
int str_find_token(str *text, str *result, char delim)
{
	int i;
	if(text==NULL || result==NULL)
		return -1;
	if(text->s[0] == delim)
	{
		 text->s += 1;
		 text->len -= 1;
	}
	trim_leading(text);
	result->s = text->s;
	result->len = 0;
	for (i=0; i<text->len; i++)
	{
		if(result->s[i]==delim || result->s[i]=='\0'
				|| result->s[i]=='\r' || result->s[i]=='\n')
			return 0;
		result->len++;
	}
	return 0;
}
