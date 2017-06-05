/*
 *  Kaidan - Cross platform XMPP client
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

#ifndef VCARD_H
#define VCARD_H

// Qt
#include <QObject>
#include <QString>
// Boost
#include <boost/shared_ptr.hpp>
// Swiften
#include <Swiften/Elements/VCard.h>

class VCard : public QObject
{
	Q_OBJECT
	Q_PROPERTY(const QString version READ getVersion WRITE setVersion NOTIFY VCardChanged)
	Q_PROPERTY(const QString fullName READ getFullName WRITE setFullName NOTIFY VCardChanged)
	Q_PROPERTY(const QString familyName READ getFamilyName WRITE setFamilyName NOTIFY VCardChanged)
	Q_PROPERTY(const QString givenName READ getGivenName WRITE setGivenName NOTIFY VCardChanged)
	Q_PROPERTY(const QString middleName READ getGivenName WRITE setGivenName NOTIFY VCardChanged)
	Q_PROPERTY(const QString prefix READ getPrefix WRITE setPrefix NOTIFY VCardChanged)
	Q_PROPERTY(const QString suffix READ getSuffix WRITE setSuffix NOTIFY VCardChanged)
	Q_PROPERTY(const QString nickname READ getNickname WRITE setNickname NOTIFY VCardChanged)
	Q_PROPERTY(const QString preferredEMailAddress READ getPreferredEMailAddress NOTIFY VCardChanged)
	Q_PROPERTY(const QString photo READ getPhoto NOTIFY VCardChanged)
	Q_PROPERTY(const QString photoType READ getPhotoType NOTIFY VCardChanged)

public:
	VCard(QObject *parent = 0);
	~VCard();
	void fromSwiftVCard(Swift::VCard::ref vCard_);
	Swift::VCard::ref getSwiftVCard();

	void setVersion(const QString);
	const QString getVersion();
	void setFullName(const QString);
	const QString getFullName();
	void setFamilyName(const QString);
	const QString getFamilyName();
	void setGivenName(const QString);
	const QString getGivenName();
	void setMiddleName(const QString);
	const QString getMiddleName();
	void setPrefix(const QString);
	const QString getPrefix();
	void setSuffix(const QString);
	const QString getSuffix();
	void setNickname(const QString);
	const QString getNickname();
	const QString getPreferredEMailAddress();
	const QString getPhoto();
	const QString getPhotoType();

signals:
	void VCardChanged();

private:
	Swift::VCard::ref vCard;
};

#endif // VCARD_H
