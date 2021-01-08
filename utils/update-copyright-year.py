#!/usr/bin/env python3
#
# Kaidan - A user-friendly XMPP client for every device!
#
# Copyright (C) 2016-2021 Kaidan developers and contributors
# (see the LICENSE file for a full list of copyright authors)
#
# Kaidan is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# In addition, as a special exception, the author of Kaidan gives
# permission to link the code of its release with the OpenSSL
# project's "OpenSSL" library (or with modified versions of it that
# use the same license as the "OpenSSL" library), and distribute the
# linked executables. You must obey the GNU General Public License in
# all respects for all of the code used other than "OpenSSL". If you
# modify this file, you may extend this exception to your version of
# the file, but you are not obligated to do so.  If you do not wish to
# do so, delete this exception statement from your version.
#
# Kaidan is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Kaidan.  If not, see <http://www.gnu.org/licenses/>.
#

"""
This script updates the year in the copyright headers of files.

It should be run every new year but not before because it uses the current year as the new one.
"""

import logging
from datetime import datetime
from pathlib import Path
import re

# Use "logging.DEBUG" for debug output.
LOG_LEVEL = logging.INFO

# relative path from this script's directory to the root directory of SEARCHED_DIRECTORIES
RELATIVE_ROOT_DIRECTORY_PATH = ".."
# relative paths from RELATIVE_ROOT_DIRECTORY_PATH to the uppermost directories of the checked and maybe updated files
SEARCHED_DIRECTORIES = ["src", "tests", "utils"]

# template which us used to create the new copyright string by replacing "%s" with the current year
COPYRIGHT_STRING = "Copyright (C) 2016-%s Kaidan developers and contributors"
COPYRIGHT_REGULAR_EXPRESSION = "Copyright \(C\) (\d{4}-)?\d{4} Kaidan developers and contributors"

logging.basicConfig(level=LOG_LEVEL)

script_directory_path = Path(__file__).resolve().parent
logging.debug("Determined path of this script's directory: %s" % script_directory_path)
root_directory_path = Path(script_directory_path, RELATIVE_ROOT_DIRECTORY_PATH).resolve()
logging.debug("Determined path of searched directories' root: %s" % root_directory_path)

current_year = datetime.now().year
logging.debug("Set new copyright year to %s" % current_year)
new_copyright_string = COPYRIGHT_STRING % current_year
logging.debug("Generated copyright string: %s" % new_copyright_string)

for searched_directory in SEARCHED_DIRECTORIES:
	for node in sorted(root_directory_path.glob("%s/**/*" % searched_directory)):
		logging.debug("Checking %s" % node)

		if node.is_file():
			with open(node) as file:
				old_content = file.read()
				new_content = re.sub(COPYRIGHT_REGULAR_EXPRESSION, new_copyright_string, old_content)

			if new_content != old_content:
				with open(node, "w") as file:
					file.write(new_content)
					logging.info("Updated %s" % node)
