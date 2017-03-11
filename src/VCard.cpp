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

// VCard
#include "VCard.h"
// Qt 5
#include <QString>
// Boost
#include <boost/shared_ptr.hpp>
// Swiften
#include <Swiften/Swiften.h>

VCard::VCard(QObject *parent) : QObject(parent)
{
	vCard = boost::shared_ptr<Swift::VCard>(new Swift::VCard());
	emit VCardChanged();
}

VCard::~VCard()
{

}

void VCard::fromSwiftVCard(Swift::VCard::ref vCard_)
{
	// only use the new VCard, if it's valid
	if (vCard_)
	{
		vCard = vCard_;
		emit VCardChanged();
	}
	else
	{
		std::cout << "VCard::fromSwiftVCard - Not Valid / Empty" << '\n';
	}
}

Swift::VCard::ref VCard::getSwiftVCard()
{
	return vCard;
}

void VCard::setVersion(const QString version_)
{
	vCard->setVersion(version_.toStdString());
}

const QString VCard::getVersion()
{
	return QString::fromStdString(vCard->getVersion());
}

void VCard::setFullName(const QString fullName_)
{
	vCard->setFullName(fullName_.toStdString());
}

const QString VCard::getFullName()
{
	return QString::fromStdString(vCard->getFullName());
}

void VCard::setFamilyName(const QString familyName_)
{
	vCard->setFamilyName(familyName_.toStdString());
}

const QString VCard::getFamilyName()
{
	return QString::fromStdString(vCard->getFamilyName());
}

void VCard::setGivenName(const QString givenName_)
{
	vCard->setGivenName(givenName_.toStdString());
}

const QString VCard::getGivenName()
{
	return QString::fromStdString(vCard->getGivenName());
}

void VCard::setMiddleName(const QString middleName_)
{
	vCard->setMiddleName(middleName_.toStdString());
}

const QString VCard::getMiddleName()
{
	return QString::fromStdString(vCard->getMiddleName());
}

void VCard::setPrefix(const QString prefix_)
{
	vCard->setPrefix(prefix_.toStdString());
}

const QString VCard::getPrefix()
{
	return QString::fromStdString(vCard->getPrefix());
}

void VCard::setSuffix(const QString suffix_)
{
	vCard->setSuffix(suffix_.toStdString());
}

const QString VCard::getSuffix()
{
	return QString::fromStdString(vCard->getSuffix());
}

void VCard::setNickname(const QString nickname_)
{
	vCard->setNickname(nickname_.toStdString());
}

const QString VCard::getNickname()
{
	return QString::fromStdString(vCard->getNickname());
}

const QString VCard::getPreferredEMailAddress()
{
	return QString::fromStdString(vCard->getPreferredEMailAddress().address);
}

const QString VCard::getPhoto()
{
	return QString::fromStdString(Swift::byteArrayToString(vCard->getPhoto()));
}

const QString VCard::getPhotoType()
{
	return QString::fromStdString(vCard->getPhotoType());
}
