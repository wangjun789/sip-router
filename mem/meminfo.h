/*
 * Copyright (C) 2001-2003 FhG Fokus
 *
 * This file is part of sip-router, a free SIP server.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * History:
 * --------
 *  2005-03-02  created (andrei)
 *  2005-07-25  renamed meminfo to mem_info due to name conflict on solaris
 */

/**
 * \file
 * \brief Memory manager (malloc) informations and statistics
 * 
 * Memory manager (malloc) informations and statistics, like the used memory,
 * fragmentations etc..
 * \ingroup mem
 */

#ifndef meminfo_h
#define meminfo_h


/** Memory information structure */
struct mem_info{
	unsigned long total_size; /** total size of memory pool */
	unsigned long free; /** free memory */
	unsigned long used; /** allocated size */
	unsigned long real_used; /** used size plus overhead from malloc */
	unsigned long max_used; /** maximum used size since server start? */
	unsigned long min_frag; /** minimum number of fragmentations? */
	unsigned long total_frags; /** number of total memory fragments */
};

#endif

