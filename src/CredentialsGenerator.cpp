/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2021 Kaidan developers and contributors
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

// Qt
#include <QRandomGenerator>
// Kaidan
#include "CredentialsGenerator.h"
#include "Globals.h"

static const QLatin1String VOWELS = QLatin1String("aeiou");
static const int VOWELS_LENGTH = VOWELS.size();

static const QLatin1String CONSONANTS = QLatin1String("bcdfghjklmnpqrstvwxyz");
static const int CONSONANTS_LENGTH = CONSONANTS.size();

CredentialsGenerator::CredentialsGenerator(QObject *parent)
	: QObject(parent)
{
}

QString CredentialsGenerator::generatePronounceableName(unsigned int length)
{
	QString randomString;
	randomString.reserve(length);
	bool startWithVowel = QRandomGenerator::global()->generate() % 2;
	length += startWithVowel;
	for (unsigned int i = startWithVowel; i < length; ++i) {
		if (i % 2)
			randomString.append(VOWELS.at(QRandomGenerator::global()->generate() % VOWELS_LENGTH));
		else
			randomString.append(CONSONANTS.at(QRandomGenerator::global()->generate() % CONSONANTS_LENGTH));
	}
	return randomString;
}

QString CredentialsGenerator::generateUsername()
{
	return generatePronounceableName(GENERATED_USERNAME_LENGTH);
}

QString CredentialsGenerator::generatePassword()
{
	return generatePassword(GENERATED_PASSWORD_LENGTH_LOWER_BOUND + QRandomGenerator::global()->generate() % (GENERATED_PASSWORD_LENGTH_UPPER_BOUND - GENERATED_PASSWORD_LENGTH_LOWER_BOUND + 1));
}

QString CredentialsGenerator::generatePassword(unsigned int length)
{
	QString password;
	password.reserve(length);

	for (unsigned int i = 0; i < length; i++)
		password.append(GENERATED_PASSWORD_ALPHABET.at(QRandomGenerator::global()->generate() % GENERATED_PASSWORD_ALPHABET_LENGTH));

	return password;
}
