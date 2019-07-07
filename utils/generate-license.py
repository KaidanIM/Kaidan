#!/usr/bin/env python3
#
# Copyright (C) 2018-2019 Linus Jahn <lnj@kaidan.im>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

"""
This script generates a debian-formatted, machine-readable copyright file for
Kaidan. It uses git to get a list of contributors for the source code and the
translations.
"""

import git
import datetime

# These user ids will be replaced for the LICENSE file
# If you want to be added to this list, please open an issue or pull request!
REPLACE_USER_IDS = [
	("Ellenjott [LNJ] <git@lnj.li>", "Linus Jahn <lnj@kaidan.im>"),
	("LNJ <git@lnj.li>", "Linus Jahn <lnj@kaidan.im>"),
	("Linus Jahn <linus.jahn@searchmetrics.com>", "Linus Jahn <lnj@kaidan.im>"),
	("JBBgameich <jbbgameich@outlook.com>", "Jonah Brüchert <jbb@kaidan.im>"),
	("JBBgameich <jbbgameich@gmail.com>", "Jonah Brüchert <jbb@kaidan.im>"),
	("JBBgameich <jbb.prv@gmx.de>", "Jonah Brüchert <jbb@kaidan.im>"),
	("JBB <jbb.prv@gmx.de>", "Jonah Brüchert <jbb@kaidan.im>"),
	("Jonah Brüchert <jbb.prv@gmx.de>", "Jonah Brüchert <jbb@kaidan.im>"),
	("Jonah Brüchert <jbb.mail@gmx.de>", "Jonah Brüchert <jbb@kaidan.im>"),
	("Georg <s.g.b@gmx.de>", "geobra <s.g.b@gmx.de>"),
	("Muhammad Nur Hidayat Yasuyoshi (MNH48.com) <muhdnurhidayat96@yahoo.com>",
	 "Muhammad Nur Hidayat Yasuyoshi <mnh48mail@gmail.com>"),
	("X advocatux <advocatux@airpost.net>", "advocatux <advocatux@airpost.net>")
]

# These user ids will be excluded from any targets
EXCLUDE_USER_IDS = [
	"Weblate <noreply@weblate.org>",
	"anonymous <> <None>",
	"Kaidan Translations <translations@kaidan.im>",
	"Kaidan translations <translations@kaidan.im>",
	"Hosted Weblate <hosted@weblate.org>"
]

GPL3_OPENSSL_LICENSE = """This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

In addition, as a special exception, the author of this program gives permission
to link the code of its release with the OpenSSL project's "OpenSSL" library (or
with modified versions of it that use the same license as the "OpenSSL"
library), and distribute the linked executables. You must obey the GNU General
Public License in all respects for all of the code used other than "OpenSSL".
If you modify this file, you may extend this exception to your version of the
file, but you are not obligated to do so.  If you do not wish to do so, delete
this exception statement from your version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this package.  If not, see <http://www.gnu.org/licenses/>.

On Debian systems, the full text of the GNU General Public License version 3 can
be found in the file
`/usr/share/common-licenses/GPL-3'."""

GPL3_LICENSE = """This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this package; if not, write to the Free Software Foundation, Inc., 51 Franklin
St, Fifth Floor, Boston, MA  02110-1301 USA

On Debian systems, the full text of the GNU General Public
License version 3 can be found in the file
`/usr/share/common-licenses/GPL-3'."""

MIT_LICENSE = """Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE."""

MIT_APPLE_LICENSE = """Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE."""

class CopyrightAuthor:
	def __init__(self, score = 0, dates = None, years = "", uid = ""):
		self.dates = dates or list([]);
		self.score = score;
		self.years = years;
		self.uid = uid;

	def addTimestamp(self, x):
		self.dates.append(x);

def createField(name, contentList, fullIndent = True):
	retvar = "{}: {}\n".format(name, contentList[0]);

	if len(contentList) < 2:
		return retvar;

	indent = len(name) * " ";
	for i in range(1, len(contentList)):
		retvar += "{}  {}\n".format(indent, contentList[i]);

	return retvar;

def createLongField(name, heading, content):
	retvar = "{}: {}\n".format(name, heading);

	if not content:
		return retvar;

	for line in content.split('\n'):
		if not line.strip(): # if line is empty
			line = ".";
		retvar += " {}\n".format(line);
	return retvar;

class CopyrightTarget:
	def __init__(self, directories = None, files = None, licenseName = "",
		     licenseContent = "", replaceUids = None, excludeUids = None,
		     authorList = None, additionalAuthors = None, comment = ""):
		self.repo = git.Repo(".");
		self.files = files or list([]);
		self.directories = directories or list([]);
		self.licenseName = licenseName;
		self.licenseContent = licenseContent
		self.authorList = authorList or dict({});
		self.comment = comment
		self.replaceUids = replaceUids or list([]);
		self.excludeUids = excludeUids or list([]);
		self.replaceUids.extend(REPLACE_USER_IDS);
		self.excludeUids.extend(EXCLUDE_USER_IDS);

		self.authorList = authorList or self.getAuthorList();
		if additionalAuthors:
			self.authorList.update(additionalAuthors)

	def replaceUid(self, uid):
		for pair in self.replaceUids:
			if uid == pair[0]:
				uid = pair[1];
		return uid;

	def getAuthorList(self):
		paths = list(self.files);
		paths.extend(self.directories);

		commitList = self.repo.iter_commits(paths=paths);
		authorList = {};

		for commit in commitList:
			# create user id and check replacements and excludes
			uid = "{} <{}>".format(commit.author.name,
					       commit.author.email);
			uid = self.replaceUid(uid);

			if uid in self.excludeUids:
				continue;

			if not uid in authorList.keys():
				authorList[uid] = CopyrightAuthor(uid = uid);

			authorList[uid].addTimestamp(commit.authored_date);

		for uid in authorList:
			minT = min(int(t) for t in authorList[uid].dates);
			maxT = max(int(t) for t in authorList[uid].dates);
			authorList[uid].score = maxT - minT;

			minYear = datetime.datetime.fromtimestamp(minT).year;
			maxYear = datetime.datetime.fromtimestamp(maxT).year;
			if minYear == maxYear:
				authorList[uid].years = str(minYear);
			else:
				authorList[uid].years = "{}-{}".format(minYear, maxYear);

			authorList[uid].dates = [];

		return authorList;

	def toDebianCopyright(self):
		# Create copyright list
		copyrights = [];
		for item in sorted(self.authorList.items(), key=lambda x: x[1].score, reverse=True):
			copyrights.append("{}, {}".format(item[1].years, item[0]));

		files = list(self.files);
		for directory in self.directories:
			files.append(directory + "/*");

		retvar = createField("Files", files);
		retvar += createField("Copyright", copyrights);
		retvar += createLongField("License", self.licenseName, self.licenseContent);
		if self.comment:
			retvar += createLongField("Comment", "", self.comment);

		return retvar;

class LicenseTarget:
	def __init__(self, name = "", content = ""):
		self.name = name;
		self.content = content;

	def toDebianCopyright(self):
		return createLongField("License", self.name, self.content);

def main():
	copyrightTargets = [
		CopyrightTarget(
			directories = ["src", "utils", "misc"],
			licenseName = "GPL-3+ with OpenSSL exception",
			additionalAuthors = {
				"Eike Hein <hein@kde.org>": CopyrightAuthor(years = "2017-2018")
			}
		),
		CopyrightTarget(
			directories = ["i18n"],
			licenseName = "GPL-3+ with OpenSSL exception"
		),
		CopyrightTarget(
			files = ["src/StatusBar.cpp", "src/StatusBar.h", "src/singleapp/*",
				 "src/hsluv-c/*", "utils/generate-license.py"],
			licenseName = "MIT",
			authorList = {
				"J-P Nurmi <jpnurmi@gmail.com>": CopyrightAuthor(years = "2016"),
				"Linus Jahn <lnj@kaidan.im>": CopyrightAuthor(years = "2018-2019"),
				"Itay Grudev <itay+github.com@grudev.com>": CopyrightAuthor(years = "2015-2018"),
				"Alexei Boronine <alexei@boronine.com>": CopyrightAuthor(years = "2015"),
				"Roger Tallada <info@rogertallada.com>": CopyrightAuthor(years = "2015"),
				"Martin Mitas <mity@morous.org>": CopyrightAuthor(years = "2017"),
			}
		),
		CopyrightTarget(
			files = ["src/EmojiModel.cpp", "src/EmojiModel.h", "qml/elements/EmojiPicker.qml"],
			licenseName = "GPL-3+",
			authorList = {
				"Konstantinos Sideris <siderisk@auth.gr>": CopyrightAuthor(years = "2017"),
			},
		),
		CopyrightTarget(
			files = ["data/images/message_checkmark.svg"],
			licenseName = "GPL-3+",
			authorList = {
				"Michael Kurz <betheg@bitcloner.org>": CopyrightAuthor(years = "2014"),
			},
			comment = "message_checkmark.svg: Originally from conversations, optimized using SVGO by LNJ <git@lnj.li>"
		),
		CopyrightTarget(
			files = [
				"misc/kaidan.svg", "misc/kaidan-small-margin.svg", "misc/kaidan-128x128.png",
				"data/images/banner.png"
			],
			licenseName = "CC-BY-SA-4.0",
			authorList = {
				"Ilya Bizyaev <bizyaev@zoho.com>": CopyrightAuthor(years = "2016-2017"),
				"MBB <mbb-mail@gmx.de>": CopyrightAuthor(years = "2016"),
			}
		),
		CopyrightTarget(
			files = ["data/images/chat.png"],
			licenseName = "CC-BY-SA-3.0",
			authorList = {
				"Subtle Patterns, Toptal Designers": CopyrightAuthor(years = "2013")
			},
			comment = "data/images/chat.png: From <https://www.toptal.com/designers/subtlepatterns/inspiration-geometry/>"
		),
		CopyrightTarget(
			files = ["utils/convert-prl-libs-to-cmake.pl"],
			licenseName = "MIT-Apple",
			authorList = {
				"Konstantin Tokarev <annulen@yandex.ru>": CopyrightAuthor(years = "2016")
			}
		),
		LicenseTarget(
			name = "GPL-3+ with OpenSSL exception",
			content = GPL3_OPENSSL_LICENSE
		),
		LicenseTarget(
			name = "GPL-3+",
			content = GPL3_LICENSE
		),
		LicenseTarget(
			name = "MIT",
			content = MIT_LICENSE
		),
		LicenseTarget(
			name = "MIT-Apple",
			content = MIT_APPLE_LICENSE
		)
	];

	print("Upstream-Name: kaidan")
	print("Source: https://invent.kde.org/kde/kaidan/")
	print("Format: https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/")
	print()

	for target in copyrightTargets:
		print(target.toDebianCopyright());

if __name__ == "__main__":
	main();
