/* 
 * $Id$ 
 *
 * BDB Database Driver for SER
 *
 * Copyright (C) 2008 iptelorg GmbH
 *
 * This file is part of SER, a free SIP server.
 *
 * SER is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * SER is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _BDB_CON_H_
#define _BDB_CON_H_

/** \addtogroup bdb
 * @{ 
 */

/** \file 
 * Implementation of BDB per-connection related data structures and functions.
 */

#include <time.h>
#include <db.h>

#include "../../lib/srdb2/db_pool.h"
#include "../../lib/srdb2/db_con.h"
#include "../../lib/srdb2/db_uri.h"

#include "bdb_lib.h"

/** 
 * Per-connection flags for BDB connections.
 */
enum bdb_con_flags {
	BDB_CONNECTED      = (1 << 0), /**< The connection has been connected successfully */
};


/** A structure representing a connection to a BDB.
 * This structure represents connections to BDB. It contains
 * BDB specific per-connection data, 
 */

typedef struct _bdb_con {
	db_pool_entry_t gen; /**< Generic part of the structure */
	bdb_db_t	*dbp;	 /**< DB structure handle */
	unsigned int flags;	 /**< Flags */
} bdb_con_t, *bdb_con_p;

/** Create a new bdb_con structure.
 * This function creates a new bdb_con structure and attachs the structure to
 * the generic db_con structure in the parameter.
 * @param con A generic db_con structure to be extended with BDB payload
 * @retval 0 on success
 * @retval A negative number on error
 */
int bdb_con(db_con_t* con);


/** Establish a new connection to server.  
 * This function is called when a SER module calls db_connect to establish a
 * new connection to the database server.
 * @param con A structure representing database connection.
 * @retval 0 on success.
 * @retval A negative number on error.
 */
int bdb_con_connect(db_con_t* con);


/** Disconnected from BDB.
 * Disconnects a previously connected connection to BDB.
 * @param con A structure representing the connection to be disconnected.
 */
void bdb_con_disconnect(db_con_t* con);

/** @} */

#endif /* _BDB_CON_H_ */