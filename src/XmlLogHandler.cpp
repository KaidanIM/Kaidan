/*
 *  Kaidan - A user-friendly XMPP client for every device!
 * 
 *  Copyright (C) 2017 LNJ <git@lnj.li>
 * 
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan. If not, see <http://www.gnu.org/licenses/>.
 */

#include "XmlLogHandler.h"
#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

void XmlLogHandler::handleLog(gloox::LogLevel level, gloox::LogArea area, const std::string &message)
{
	if (area == gloox::LogAreaXmlIncoming) {
		qDebug() << "[XML] [Incoming] <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
		qDebug().noquote() << makeXmlPretty(QString::fromStdString(message));
	} else if (area == gloox::LogAreaXmlOutgoing) {
		qDebug() << "[XML] [Outgoing] >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
		qDebug().noquote() << makeXmlPretty(QString::fromStdString(message));
	}
}

QString XmlLogHandler::makeXmlPretty(QString xmlIn)
{
	QString xmlOut;

	QXmlStreamReader reader(xmlIn);
	QXmlStreamWriter writer(&xmlOut);
	writer.setAutoFormatting(true);

	while (!reader.atEnd()) {
		reader.readNext();
		if (!reader.isWhitespace()) {
			writer.writeCurrentToken(reader);
		}
	}

	// remove xml header
	xmlOut.replace("<?xml version=\"1.0\"?>\n", "");
	// remove last char (\n)
	xmlOut = xmlOut.left(xmlOut.size() - 1);

	return xmlOut;
}
