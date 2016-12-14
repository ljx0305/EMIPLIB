/*
    
  This file is a part of EMIPLIB, the EDM Media over IP Library.
  
  Copyright (C) 2006  Expertise Centre for Digital Media (EDM)
                      (http://www.edm.uhasselt.be)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  
  USA

*/

/**
 * \file mipmessagedumper.h
 */

#ifndef MIPMESSAGEDUMPER_H

#define MIPMESSAGEDUMPER_H

#include "mipconfig.h"
#include "mipcomponent.h"

/** Outputs information about the incoming messages.
 *  This component writes the message type and submessage type of incoming messages
 *  to stdout. This can be useful for debugging. No messages are generated by the
 *  component.
 */
class MIPMessageDumper : public MIPComponent
{
public:
	MIPMessageDumper();
	~MIPMessageDumper();
	bool push(const MIPComponentChain &chain, int64_t iteration, MIPMessage *pMsg);
	bool pull(const MIPComponentChain &chain, int64_t iteration, MIPMessage **pMsg);
};

#endif // MIPMESSAGEDUMPER_H

