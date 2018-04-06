/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2018 Kaidan developers and contributors
 *  (see the LICENSE file for a full list of copyright authors)
 *
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  In addition, as a special exception, the author of Kaidan gives
 *  permission to link the code of its release with the OpenSSL
 *  project's "OpenSSL" library (or with modified versions of it that
 *  use the same license as the "OpenSSL" library), and distribute the
 *  linked executables. You must obey the GNU General Public License in
 *  all respects for all of the code used other than "OpenSSL". If you
 *  modify this file, you may extend this exception to your version of
 *  the file, but you are not obligated to do so.  If you do not wish to
 *  do so, delete this exception statement from your version.
 *
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bitsofbinarymanager.h"
#include "bitsofbinaryhandler.h"
#include "bitsofbinarycache.h"
#include "bitsofbinarydata.h"
#include "gloox-extensions.h"
#include <gloox/client.h>
#include <gloox/iq.h>

using namespace gloox;

BitsOfBinaryManager::BitsOfBinaryManager(Client *client)
	: client(client)
{
	client->registerIqHandler(this, EXT_BITSOFBINARY);
}

void BitsOfBinaryManager::registerBobHandler(BitsOfBinaryHandler* handler)
{
	this->handler = handler;
}

void BitsOfBinaryManager::registerBobCache(BitsOfBinaryCache* cache)
{
	this->cache = cache;
}

void BitsOfBinaryManager::requestBobData(std::string cid, JID &to)
{
	// don't do anything, if no handler registered
	if (!handler)
		return;

	// try to load from cache
	if (cache && cache->hasCid(cid)) {
		handler->handleBobData(cache->getByCid(cid));
		return;
	}

	// create a request
	int id = getNewJobId();
	jobIdMap[id] = cid;

	IQ iq(IQ::Get, to);
	BitsOfBinaryData *request = new BitsOfBinaryData(cid);
	iq.addExtension(request);

	client->send(iq, this, id);
}

bool BitsOfBinaryManager::handleIq(const IQ& iq)
{
	if (iq.subtype() != IQ::Get)
		return false;

	// Send result of request, if cid is in cache
	BitsOfBinaryData* data = (BitsOfBinaryData*) iq.findExtension(EXT_BITSOFBINARY);
	if (cache && data && data->valid() && cache->hasCid(data->contentId())) {
		IQ answer(IQ::Result, iq.from());
		answer.addExtension(cache->getByCid(data->contentId()));

		client->send(answer);
	}
	return true;
}

void BitsOfBinaryManager::handleIqID(const IQ& iq, int context)
{
	BitsOfBinaryData* data = (BitsOfBinaryData*) iq.findExtension(EXT_BITSOFBINARY);
	if (data && data->valid() && data->data().length()) {
		// add to cache
		cache->addBobData(data);

		handler->handleBobData(data);
	} else if (jobIdMap.count(context)) {
		// if job is existing, but wasn't valid, send failure
		handler->handleBobRequestFailure(jobIdMap[context]);
	}

	// remove job
	jobIdMap.erase(context);
}
